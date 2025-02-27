function isHeaderElement(elem)
{
    return elem.nodeName.match(/^H\d+$/) && elem.textContent;
}

function getDOMElementsPreorderDFS(elem, pred)
{
    var result = [];
    if (pred(elem))
        result.push(elem);

    for ( const child of elem.children)
        result = result.concat(getDOMElementsPreorderDFS(child, pred));
    return result;
}

function anchorHeaderElements(headers)
{
    return Array.from(headers, function(elem, i)
    {
        const text = elem.textContent.trim().replace(/"/g, '\\"');
        const level = parseInt(elem.nodeName.substr(1));
        const anchor = `kiwix-toc-${i}`;

        const anchorElem = document.createElement("a");
        anchorElem.id = anchor;

        /* Mark header content with something we can reference. */
        elem.insertAdjacentElement("afterbegin", anchorElem);
        return { text, level, anchor };
    });
}

function getHeadersJSONStr()
{
    const headerInfo = { url: window.location.href.replace(location.hash,""), headers: [] };

    if (document.body !== undefined)
    {
        const headers = getDOMElementsPreorderDFS(document.body, isHeaderElement);
        headerInfo.headers = anchorHeaderElements(headers);
    }
    return JSON.stringify(headerInfo);
}

// Track the current anchor for history state
let currentAnchor = null;
let isNavigating = false;

// Function to scroll to an anchor with animation
function scrollToAnchor(anchor, updateHistory = false) {
    if (!anchor || typeof anchor !== 'string') {
        console.error("Invalid anchor:", anchor);
        return false;
    }

    if (isNavigating || anchor === currentAnchor) {
        console.log("Already navigating to or at anchor: " + anchor);
        return true;
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
                    } catch (historyError) {
                        console.error("Error updating history:", historyError);
                    }
                }

                // Reset navigation flag after a short delay
                setTimeout(() => {
                    isNavigating = false;
                }, 100);
            }, 10);

            return true;
        }
        console.error("Anchor not found: " + anchor);
        isNavigating = false;
        return false;
    } catch (error) {
        console.error("Error scrolling to anchor:", error);
        isNavigating = false;
        return false;
    }
}

function initializeWebChannel() {
    try {
        if (typeof qt === 'undefined' || typeof qt.webChannelTransport === 'undefined') {
            console.error("Qt WebChannel not available");
            return;
        }

        new QWebChannel(qt.webChannelTransport, function(channel) {
            if (!channel || !channel.objects || !channel.objects.kiwixChannelObj) {
                console.error("Kiwix channel object not available");
                return;
            }

            var kiwixObj = channel.objects.kiwixChannelObj;

            try {
                kiwixObj.sendHeadersJSONStr(getHeadersJSONStr());
            } catch (e) {
                console.error("Error sending headers:", e);
            }

            kiwixObj.navigationRequested.connect(function(url, anchor) {
                if (isNavigating || anchor === currentAnchor) {
                    return;
                }

                if (window.location.href.replace(location.hash, "") == url) {
                    scrollToAnchor(anchor, false);
                }
            });

            window.addEventListener('popstate', function(event) {
                if (isNavigating) {
                    return;
                }

                // Handle navigation from history
                if (location.hash) {
                    const anchor = location.hash.substring(1);
                    if (anchor !== currentAnchor) {
                        scrollToAnchor(anchor, false);
                    }
                } else if (event.state && event.state.anchor) {
                    if (event.state.anchor !== currentAnchor) {
                        scrollToAnchor(event.state.anchor, false);
                    }
                }
            });
        });
    } catch (error) {
        console.error("Error initializing web channel:", error);
    }
}

if (document.readyState === 'loading') {
    document.addEventListener('DOMContentLoaded', initializeWebChannel);
} else {
    initializeWebChannel();
}