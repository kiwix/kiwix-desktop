/**
 * Construct recurseData.str as a nested list of headers by recursively going 
 * through all children of elem that has header tags.
 * 
 * References:
 * https://stackoverflow.com/questions/187619/is-there-a-javascript-solution-to-generating-a-table-of-contents-for-a-page
 * @param elem DOM element
 * @param recurseData Object with fields: { int: level, int: count, str: toc }
 */
function recurseChild(elem, recurseData)
{
    if (elem !== "undefined") 
    {
        if(elem.nodeName.match(/^H\d+$/) && elem.textContent)
        {
            var headerText = elem.textContent;
            var prevLevel = recurseData.level;
            var level = elem.nodeName.substr(1);
            var anchor = "kiwix-toc-" + recurseData.count;
            recurseData.count += 1;

            /* Wrap header content with something we can reference. */
            elem.innerHTML = '<a href="#' + anchor + '" id="' + anchor + '" class="kiwix-toc-a">' + headerText + '</a>';

            /* Start or end a list or item based on current and previous level */
            if (level > prevLevel) 
                recurseData.toc += '<ul class="kiwix-toc-list">';
            else if (level < prevLevel)
                recurseData.toc += '</li></ul>';
            else
                recurseData.toc += '</li>';

            recurseData.level = parseInt(level);
            recurseData.toc += '<li class="kiwix-toc-item"><a href="#' + anchor + '" class="kiwix-toc-item-a">' +
            '<span class="kiwix-tool-tip">'  + headerText + '</span>' + headerText + '</a>';
        }

        var c = elem.children;
        for (var i = 0; i < c.length; i++)
            recurseChild(c[i], recurseData);
    }
}

function tocHTMLStr()
{
    /* level used to track current header level.
       toc used to store constructed list.
       count used to uniquely identify each list item in toc.
    */
    var recurseData = { level: 0, toc: "", count: 0,};
    recurseChild(document.body, recurseData);

    /* End list when non-empty */
    if (recurseData.level)
        recurseData.toc += (new Array(recurseData.level + 1)).join('</ul>');
    return recurseData.toc;
}

function makeTOCVisible(visible)
{
    var tocElem = document.getElementById("kiwix-toc-side");
    tocElem.style.display = visible ? "block" : "none";
    document.body.style.marginLeft = visible ? "310px" : null;
    document.body.style.maxWidth = visible ?  "calc(100vw - 310px)" : null;
}

function showToolTip (elem) {
    var tooltip = elem.getElementsByClassName("kiwix-tool-tip")[0];

    /* Check if there is overflow. */ 
    if (tooltip && elem.offsetWidth < elem.scrollWidth)
    {
        tooltip.style.display = "block";

        var rect = elem.getBoundingClientRect();
        tooltip.style.top = (rect.top).toString() + "px";
        tooltip.style.left = "306px";
    }
}

function hideToolTip (elem) {
    var tooltip = elem.getElementsByClassName("kiwix-tool-tip")[0];

    if (tooltip)
        tooltip.style.display = "";
}

function setupTOCItems()
{
    var c = document.getElementsByClassName("kiwix-toc-item-a");
    for (var i = 0; i < c.length; i++)
    {
        if (c[i] !== "undefined" && c[i].style) 
        {
            var p = c[i].parentNode;
            var count = -1;
            while (p)
            {
                if (p.nodeName.match(/^UL$/))
                    count += 1;
                p = p.parentNode;
            }

            /* We need manual padding to achieve visual effects on hover. */
            c[i].style.paddingLeft = ((count == -1 ? 0 : count) * 30).toString() + "px";

            c[i].addEventListener("mouseover", (event) => { showToolTip(event.target); });
            c[i].addEventListener("mouseout", (event) => { hideToolTip(event.target); });
        }
    }
}

function setupTOC()
{
    var toc = document.createElement('div');
    toc.id = "kiwix-toc";
    toc.innerHTML += tocHTMLStr();

    var tocTitle = document.createElement('p');
    tocTitle.id = "kiwix-toc-title";

    var tocHideButton = document.createElement('button');
    tocHideButton.id = "kiwix-toc-hide-button"

    var tocTitleDiv = document.createElement('div');
    tocTitleDiv.id = "kiwix-toc-title-div";
    tocTitleDiv.append(tocTitle);
    tocTitleDiv.append(tocHideButton);

    var tocSideDiv = document.createElement('div');
    tocSideDiv.id = "kiwix-toc-side";
    tocSideDiv.append(tocTitleDiv);
    tocSideDiv.append(toc);

    document.body.prepend(tocSideDiv);
}

new QWebChannel(qt.webChannelTransport, function(channel) {

    var kiwixObj = channel.objects.kiwixChannelObj
    setupTOC();
    setupTOCItems();

    document.getElementById("kiwix-toc-title").textContent = kiwixObj.tocTitle;

    var tocHideButton = document.getElementById("kiwix-toc-hide-button");
    tocHideButton.textContent = kiwixObj.hideButtontext;
    tocHideButton.onclick = () => { kiwixObj.tocVisible = false; };

    kiwixObj.tocVisibleChanged.connect(function(visible) {
        makeTOCVisible(visible);
    });
    makeTOCVisible(kiwixObj.tocVisible);
});

