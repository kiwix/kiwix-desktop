// Track the current anchor for history state
let currentAnchor = null;
let isNavigating = false;

function isHeaderElement(elem) {
    return elem.nodeName.match(/^H\d+$/) && elem.textContent;
}

function getDOMElementsPreorderDFS(elem, pred) {
    var result = [];
    if (pred(elem))
        result.push(elem);

    for (const child of elem.children)
        result = result.concat(getDOMElementsPreorderDFS(child, pred));
    return result;
}

function anchorHeaderElements(headers) {
    return Array.from(headers, function(elem, i) {
        const text = elem.textContent.trim().replace(/"/g, '\\"');
        const level = parseInt(elem.nodeName.substr(1));
        const anchor = `kiwix-toc-${i}`;

        const anchorElem = document.createElement("a");
        anchorElem.id = anchor;

        // Mark header content with something we can reference
        elem.insertAdjacentElement("afterbegin", anchorElem);
        return { text, level, anchor };
    });
}

function getHeadersJSONStr() {
    const headerInfo = { url: window.location.href.replace(location.hash,""), headers: [] };

    if (document.body !== undefined) {
        const headers = getDOMElementsPreorderDFS(document.body, isHeaderElement);
        headerInfo.headers = anchorHeaderElements(headers);
    }
    return JSON.stringify(headerInfo);
}

function scrollToAnchor(anchor, updateHistory = false) {
    if (!anchor || typeof anchor !== 'string') {
        return false;
    }

    // Skip if already navigating to the same anchor, unless triggered by history
    if (isNavigating && anchor === currentAnchor && !updateHistory) {
        // Continue if from history navigation
        const isFromHistory = document.referrer === '' || 
                             (window.history.state && window.history.state.anchor === anchor);
        if (!isFromHistory) {
            return true;
        }
    }

    try {
        isNavigating = true;
        const element = document.getElementById(anchor);
        if (element) {
            setTimeout(() => {
                element.scrollIntoView({behavior: 'smooth'});
                currentAnchor = anchor;

                // Update the URL in history if requested
                if (updateHistory && window.history && window.history.pushState) {
                    try {
                        const baseUrl = window.location.href.replace(location.hash, "");
                        window.history.pushState({ anchor: anchor }, "", baseUrl + "#" + anchor);
                    } catch (e) { }
                }

                // Reset navigation flag after a short delay
                setTimeout(() => {
                    isNavigating = false;
                }, 150);
            }, 10);

            return true;
        }
        isNavigating = false;
        return false;
    } catch (e) {
        isNavigating = false;
        return false;
    }
}

function initializeWebChannel() {
    try {
        if (typeof qt === 'undefined' || typeof qt.webChannelTransport === 'undefined') {
            return;
        }

        new QWebChannel(qt.webChannelTransport, function(channel) {
            if (!channel || !channel.objects || !channel.objects.kiwixChannelObj) {
                return;
            }

            var kiwixObj = channel.objects.kiwixChannelObj;

            try {
                kiwixObj.sendHeadersJSONStr(getHeadersJSONStr());
            } catch (e) { }

            // Handle navigation requests from Qt
            kiwixObj.navigationRequested.connect(function(url, anchor) {
                // Skip if already navigating to the same anchor
                if (isNavigating && anchor === currentAnchor) {
                    return;
                }

                if (window.location.href.replace(location.hash, "") == url) {
                    scrollToAnchor(anchor, false);
                }
            });

            // Handle browser history navigation (back/forward buttons)
            window.addEventListener('popstate', function(event) {
                if (isNavigating) {
                    return;
                }

                // Handle navigation from history
                let anchorFound = false;
                if (location.hash) {
                    const anchor = location.hash.substring(1);
                    anchorFound = scrollToAnchor(anchor, false);
                } else if (event.state && event.state.anchor) {
                    anchorFound = scrollToAnchor(event.state.anchor, false);
                }
                
                // Notify Qt about the navigation to update TOC
                if (anchorFound && kiwixObj) {
                    setTimeout(function() {
                        try {
                            const currentHash = location.hash ? location.hash.substring(1) : 
                                               (event.state && event.state.anchor ? event.state.anchor : null);
                            
                            if (currentHash) {
                                kiwixObj.sendConsoleMessage(JSON.stringify({
                                    type: "history-navigation",
                                    message: "Browser history navigation event",
                                    anchor: currentHash,
                                    url: window.location.href,
                                    timestamp: Date.now()
                                }));
                            }
                        } catch (e) { }
                    }, 150);
                }
            });
        });
    } catch (e) { }
}

// Initialize web channel when document is ready
if (document.readyState === 'loading') {
    document.addEventListener('DOMContentLoaded', initializeWebChannel);
} else {
    initializeWebChannel();
}