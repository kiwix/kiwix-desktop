const units = ['bytes', 'KB', 'MB', 'GB', 'TB', 'PB', 'EB', 'ZB', 'YB'];
function niceBytes(x){
  var unitIndex = 0;
  var n = parseInt(x, 10) || 0;
  while(n >= 1024 && ++unitIndex)
      n = n/1024;
  return(n.toFixed(n >= 10 || unitIndex < 1 ? 0 : 2) + ' ' + units[unitIndex]);
}

function createDict(keys, values) {
    var d = {}
    for(var i=0; i<keys.length; i++) {
      d[keys[i]] = values[i];
    }
    return d;
}
const BOOK_KEYS = ["id", "name", "path", "url", "size", "description", "title", "tags", "date", "faviconUrl", "faviconMimeType", "downloadId"];
function addBook(values) {
  var b = createDict(BOOK_KEYS, values);
  if (b.downloadId && !downloadUpdaters.hasOwnProperty(b.id)) {
    downloadUpdaters[b.id] = setInterval(function() { getDownloadInfo(b.id); }, 1000);
  }
  app.books.push(b);
}
function onBooksChanged () {
  app.books = [];
  for(var i=0; i<contentManager.bookIds.length; i++) {
    var id = contentManager.bookIds[i];
    contentManager.getBookInfos(id, BOOK_KEYS, addBook);
  }
  app.displayedBooksNb = 20;
}

downloadUpdaters = {}
const DOWNLOAD_KEYS = ["id", "status", "followedBy", "path", "totalLength", "completedLength", "downloadSpeed", "verifiedLength"];
function getDownloadInfo(id) {
  contentManager.updateDownloadInfos(id, DOWNLOAD_KEYS, function(values) {
    if (values.length == 0) {
      clearInterval(downloadUpdaters[id]);
      return;
    }
    d = createDict(DOWNLOAD_KEYS, values);
    if (d.status == "completed") {
      clearInterval(downloadUpdaters[id]);
      Vue.delete(app.downloads, id);
      return;
    }
    Vue.set(app.downloads, id, createDict(DOWNLOAD_KEYS, values));
  });
}

function displayLoadIcon(display) {
    if (display) {
        document.getElementById("load-icon").classList.remove("do-not-display")  
        document.getElementById("bookList").classList.add("do-not-display");  
    } else {
        document.getElementById("load-icon").classList.add("do-not-display");
        document.getElementById("bookList").classList.remove("do-not-display")      
    }
}

function init() {
  new QWebChannel(qt.webChannelTransport, function(channel) {
    contentManager = channel.objects.contentManager;
    app = new Vue({
      el: "#app",
      data: {
        contentManager: contentManager,
        displayedBooksNb: 20,
        books: [],
        downloads: {}
      },
      methods: {
        openBook : function(book) {
          contentManager.openBook(book.id, function() {});
        },
        downloadBook : function(book) {
          contentManager.downloadBook(book.id, function(did)  {
            if (did.length == 0)
                return;
            book.downloadId = did;
            downloadUpdaters[book.id] = setInterval(function() { getDownloadInfo(book.id); }, 1000);
          });
        },
        eraseBook : function(book) {
            contentManager.eraseBook(book.id);
        },
        pauseBook : function(book) {
          contentManager.pauseBook(book.id);
        },
        resumeBook : function(book) {
          contentManager.resumeBook(book.id);
        },
        cancelBook : function(book) {
            contentManager.cancelBook(book.id);
            clearInterval(downloadUpdaters[book.id]);
            Vue.delete(app.downloads, book.id);
        },
        displayedBooks : function(books, nb) {
            var a = books.slice(0, nb);
            return a;
        },
        getBookFromMousePosition : function() {
            var elements = document.elementsFromPoint(mouseX, mouseY);
            var bookId = null;
            for(var i = 0; i < elements.length; i++) {
                if (elements[i].localName == "summary" && elements[i].classList.contains("book-summary")) {
                    bookId = elements[i].id;
                    break;
                }
            }
            var book = null;
            for(var i = 0; i < app.books.length; i++) {
                if (app.books[i]["id"] == bookId) {
                    book = app.books[i];
                    break;
                }
            }
            return book;
        },
        niceBytes : niceBytes
      }
    });
    contentManager.booksChanged.connect(onBooksChanged);
    contentManager.pendingRequest.connect(displayLoadIcon);
    onBooksChanged();
    displayLoadIcon(false);
  });
}

futurCall = null;
function setSearch(value) {
  clearTimeout(futurCall);
  futurCall = setTimeout(function(){contentManager.setSearch(value)}, 100);
}

function scrolled(e) {
  if (e.offsetHeight + e.scrollTop >= e.scrollHeight) {
    app.displayedBooksNb = Math.min(app.displayedBooksNb+20, app.books.length);
  }
}

window.addEventListener("click", e => {
    if (menuVisible)
        displayMenu(null);
});

var mouseX, mouseY = 0;
window.addEventListener("contextmenu", e => {
    e.preventDefault();
    mouseX = e.pageX;
    mouseY = e.pageY;
    setContextMenuPosition();
    var book = app.getBookFromMousePosition();
    displayMenu(book);
});

var menuVisible = false;
function displayMenu(book) {
    var menu = document.getElementById("menu");
    menu.style.display = (book) ? "block" : "none";
    menuVisible = (book) ? true : false;
    if (!book)
        return;
    var localElement = document.getElementsByClassName("local-option")[0];
    localElement.style.display = (book.path) ? "block" : "none";
};

function setContextMenuPosition() {
    var menu = document.getElementById("menu");
    menu.style.left = `${mouseX}px`;
    menu.style.top = `${mouseY}px`;
};
