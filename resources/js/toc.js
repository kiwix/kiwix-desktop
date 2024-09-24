function setupTOC()
{
    var tocSideDiv = document.createElement('div');
    tocSideDiv.id = "kiwix-toc-side";

    document.body.prepend(tocSideDiv);
}

document.body.style.marginLeft = "310px";
document.body.style.maxWidth = "calc(100vw - 310px)";
setupTOC();
