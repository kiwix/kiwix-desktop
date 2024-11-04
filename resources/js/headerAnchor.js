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

function getHeaders()
{
    const headerInfo = { url: window.location.href.replace(location.hash,""), headers: [] };

    if (document.body !== undefined)
    {
        const headers = getDOMElementsPreorderDFS(document.body, isHeaderElement);
        headerInfo.headers = anchorHeaderElements(headers);
    }
    return headerInfo;
}

new QWebChannel(qt.webChannelTransport, function(channel) {
    var kiwixObj = channel.objects.kiwixChannelObj;
    kiwixObj.sendHeaders(getHeaders());
});
