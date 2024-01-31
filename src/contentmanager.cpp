#include "contentmanager.h"

#include "kiwixapp.h"
#include <kiwix/manager.h>
#include <kiwix/tools.h>

#include <QDebug>
#include <QUrlQuery>
#include <QUrl>
#include <QDir>
#include <QStorageInfo>
#include <QMessageBox>
#include "contentmanagermodel.h"
#include <zim/error.h>
#include <zim/item.h>
#include <QHeaderView>
#include "contentmanagerdelegate.h"
#include "node.h"
#include "rownode.h"
#include "descriptionnode.h"
#include "kiwixconfirmbox.h"
#include <QtConcurrent/QtConcurrentRun>
#include "contentmanagerheader.h"
#include <QDesktopServices>

ContentManager::ContentManager(Library* library, kiwix::Downloader* downloader, QObject *parent)
    : QObject(parent),
      mp_library(library),
      mp_remoteLibrary(kiwix::Library::create()),
      mp_downloader(downloader),
      m_remoteLibraryManager()
{
    // mp_view will be passed to the tab who will take ownership,
    // so, we don't need to delete it.
    mp_view = new ContentManagerView();
    managerModel = new ContentManagerModel(this);
    const auto booksList = getBooksList();
    managerModel->setBooksData(booksList);
    auto treeView = mp_view->getView();
    treeView->setModel(managerModel);
    treeView->show();

    auto header = new ContentManagerHeader(Qt::Orientation::Horizontal, treeView);
    treeView->setHeader(header);
    header->setSectionResizeMode(0, QHeaderView::Fixed);
    header->setSectionResizeMode(1, QHeaderView::Stretch);
    header->setSectionResizeMode(2, QHeaderView::Fixed);
    header->setSectionResizeMode(3, QHeaderView::Fixed);
    header->setSectionResizeMode(4, QHeaderView::Fixed);
    header->setDefaultAlignment(Qt::AlignLeft);
    header->setStretchLastSection(false);
    header->setSectionsClickable(true);
    header->setHighlightSections(true);
    treeView->setWordWrap(true);
    treeView->resizeColumnToContents(4);
    treeView->setColumnWidth(0, 80);
    treeView->setColumnWidth(5, 120);
    // TODO: set width for all columns based on viewport

    setCurrentLanguage(KiwixApp::instance()->getSettingsManager()->getLanguageList());
    setCurrentCategoryFilter(KiwixApp::instance()->getSettingsManager()->getCategoryList());
    setCurrentContentTypeFilter(KiwixApp::instance()->getSettingsManager()->getContentType());
    connect(mp_library, &Library::booksChanged, this, [=]() {emit(this->booksChanged());});
    connect(this, &ContentManager::filterParamsChanged, this, &ContentManager::updateLibrary);
    connect(this, &ContentManager::booksChanged, this, [=]() {
        const auto nBookList = getBooksList();
        managerModel->setBooksData(nBookList);
        managerModel->refreshIcons();
    });
    connect(&m_remoteLibraryManager, &OpdsRequestManager::requestReceived, this, &ContentManager::updateRemoteLibrary);
    connect(mp_view->getView(), SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(onCustomContextMenu(const QPoint &)));
    connect(this, &ContentManager::pendingRequest, mp_view, &ContentManagerView::showLoader);
    connect(treeView, &QTreeView::doubleClicked, this, &ContentManager::openBookWithIndex);
    connect(&m_remoteLibraryManager, &OpdsRequestManager::languagesReceived, this, &ContentManager::updateLanguages);
    connect(&m_remoteLibraryManager, &OpdsRequestManager::categoriesReceived, this, &ContentManager::updateCategories);
    setCategories();
    setLanguages();
}

ContentManager::BookInfoList ContentManager::getBooksList()
{
    const auto bookIds = getBookIds();
    BookInfoList bookList;
    QStringList keys = {"title", "tags", "date", "id", "size", "description", "faviconUrl"};
    QIcon bookIcon;
    for (auto bookId : bookIds) {
        auto mp = getBookInfos(bookId, keys);
        bookList.append(mp);
    }
    return bookList;
}

void ContentManager::onCustomContextMenu(const QPoint &point)
{
    QModelIndex index = mp_view->getView()->indexAt(point);
    if (!index.isValid())
        return;
    QMenu contextMenu("optionsMenu", mp_view->getView());
    auto bookNode = static_cast<RowNode*>(index.internalPointer());
    const auto id = bookNode->getBookId();

    QAction menuDeleteBook(gt("delete-book"), this);
    QAction menuOpenBook(gt("open-book"), this);
    QAction menuDownloadBook(gt("download-book"), this);
    QAction menuPauseBook(gt("pause-download"), this);
    QAction menuResumeBook(gt("resume-download"), this);
    QAction menuCancelBook(gt("cancel-download"), this);
    QAction menuOpenFolder(gt("open-folder"), this);

    if (const auto download = bookNode->getDownloadState()) {
        if (download->getDownloadInfo().paused) {
            contextMenu.addAction(&menuResumeBook);
        } else {
            contextMenu.addAction(&menuPauseBook);
        }
        contextMenu.addAction(&menuCancelBook);
    } else {
        try {
            const auto book = KiwixApp::instance()->getLibrary()->getBookById(id);
            auto bookPath = QString::fromStdString(book.getPath());
            contextMenu.addAction(&menuOpenBook);
            contextMenu.addAction(&menuDeleteBook);
            contextMenu.addAction(&menuOpenFolder);
            connect(&menuOpenFolder, &QAction::triggered, [=]() {
                QFileInfo fileInfo(bookPath);
                QDir bookDir = fileInfo.absoluteDir();
                bool dirOpen = bookDir.exists() && bookDir.isReadable() && QDesktopServices::openUrl(bookDir.absolutePath());
                if (!dirOpen) {
                    QString failedText = gt("couldnt-open-location-text");
                    failedText = failedText.replace("{{FOLDER}}", "<b>" + bookDir.absolutePath() + "</b>");
                    showInfoBox(gt("couldnt-open-location"), failedText, mp_view);
                }
            });
        } catch (...) {
            contextMenu.addAction(&menuDownloadBook);
        }
    }

    connect(&menuDeleteBook, &QAction::triggered, [=]() {
        eraseBook(id);
    });
    connect(&menuOpenBook, &QAction::triggered, [=]() {
        openBook(id);
    });
    connect(&menuDownloadBook, &QAction::triggered, [=]() {
        downloadBook(id, index);
    });
    connect(&menuPauseBook, &QAction::triggered, [=]() {
        pauseBook(id, index);
    });
    connect(&menuCancelBook, &QAction::triggered, [=]() {
        cancelBook(id, index);
    });
    connect(&menuResumeBook, &QAction::triggered, [=]() {
        resumeBook(id, index);
    });

    contextMenu.exec(mp_view->getView()->viewport()->mapToGlobal(point));
}

void ContentManager::setLocal(bool local) {
    if (local == m_local) {
        return;
    }
    m_local = local;
    emit(filterParamsChanged());
    setCategories();
    setLanguages();
}

QStringList ContentManager::getTranslations(const QStringList &keys)
{
    QStringList translations;

    for(auto& key: keys) {
        translations.append(KiwixApp::instance()->getText(key));
    }
    return translations;
}

void ContentManager::setCategories()
{
    QStringList categories;
    if (m_local) {
        auto categoryData = mp_library->getKiwixLibrary()->getBooksCategories();
        for (auto category : categoryData) {
            auto categoryName = QString::fromStdString(category);
            categories.push_back(categoryName);
        }
        m_categories = categories;
        emit(categoriesLoaded(m_categories));
        return;
    }
    m_remoteLibraryManager.getCategoriesFromOpds();
}

void ContentManager::setLanguages()
{
    LanguageList languages;
    if (m_local) {
        auto languageData = mp_library->getKiwixLibrary()->getBooksLanguages();
        for (auto language : languageData) {
            auto langCode = QString::fromStdString(language);
            auto selfName = QString::fromStdString(kiwix::getLanguageSelfName(language));
            languages.push_back({langCode, selfName});
        }
        m_languages = languages;
        emit(languagesLoaded(m_languages));
        return;
    }
    m_remoteLibraryManager.getLanguagesFromOpds();
}

#define ADD_V(KEY, METH) {if(key==KEY) values.insert(key, QString::fromStdString((b->METH())));}
ContentManager::BookInfo ContentManager::getBookInfos(QString id, const QStringList &keys)
{
    BookInfo values;
    const kiwix::Book* b = [=]()->const kiwix::Book* {
        try {
            return &mp_library->getBookById(id);
        } catch (...) {
            try {
                QMutexLocker locker(&remoteLibraryLocker);
                return &mp_remoteLibrary->getBookById(id.toStdString());
            } catch(...) { return nullptr; }
        }
    }();

    if (nullptr == b){
        for(auto& key:keys) {
            (void) key;
            values.insert(key, "");
        }
        return values;
    }

    for(auto& key: keys){
        ADD_V("id", getId);
        ADD_V("path", getPath);
        ADD_V("title", getTitle);
        ADD_V("description", getDescription);
        ADD_V("date", getDate);
        ADD_V("url", getUrl);
        ADD_V("name", getName);
        ADD_V("downloadId", getDownloadId);
        if (key == "faviconMimeType") {
            std::string mimeType;
            try {
                auto item = b->getIllustration(48);
                mimeType = item->mimeType;
            } catch (...) {
                const kiwix::Book::Illustration tempIllustration;
                mimeType = tempIllustration.mimeType;
            }
            values.insert(key, QString::fromStdString(mimeType));
        }
        if (key == "faviconUrl") {
            std::string url;
            try {
                auto item = b->getIllustration(48);
                url = item->url;
            } catch (...) {
                const kiwix::Book::Illustration tempIllustration;
                url = tempIllustration.url;
            }
            values.insert(key, QString::fromStdString(url));
        }
        if (key == "size") {
            values.insert(key, QString::number(b->getSize()));
        }
        if (key == "tags") {
            QStringList tagList = QString::fromStdString(b->getTags()).split(';');
            QMap<QString, bool> displayTagMap;
            for(auto tag: tagList) {
              if (tag[0] == "_") {
                auto splitTag = tag.split(":");
                displayTagMap[splitTag[0]] = splitTag[1] == "yes" ? true:false;
              }
            }
            QStringList displayTagList;
            if (displayTagMap["_videos"]) displayTagList << tr("Videos");
            if (displayTagMap["_pictures"]) displayTagList << tr("Pictures");
            if (!displayTagMap["_details"]) displayTagList << tr("Introduction only");
            QString s = displayTagList.join(", ");
            values.insert(key, s);
        }
    }
    return values;
}
#undef ADD_V

void ContentManager::openBookWithIndex(const QModelIndex &index)
{
    try {
        QString bookId;
        auto bookNode = static_cast<Node*>(index.internalPointer());
        bookId = bookNode->getBookId();
        // check if the book is available in local library, will throw std::out_of_range if it isn't.
        KiwixApp::instance()->getLibrary()->getBookById(bookId);
        if (getBookInfos(bookId, {"downloadId"})["downloadId"] != "")
            return;
        openBook(bookId);
    } catch (std::out_of_range &e) {}
}

void ContentManager::openBook(const QString &id)
{
    QUrl url("zim://"+id+".zim/");
    try {
        KiwixApp::instance()->openUrl(url, true);
    } catch (const std::exception& e) {
        auto tabBar = KiwixApp::instance()->getTabWidget();
        tabBar->closeTab(1);
        auto text = gt("zim-open-fail-text");
        text = text.replace("{{ZIM}}", QString::fromStdString(mp_library->getBookById(id).getPath()));
        auto title = gt("zim-open-fail-title");
        KiwixApp::instance()->showMessage(text, title, QMessageBox::Warning);
        mp_library->removeBookFromLibraryById(id);
        tabBar->setCurrentIndex(0);
        emit(booksChanged());
        return;
    }
}

#define ADD_V(KEY, METH) {if(key==KEY) {values.insert(key, QString::fromStdString((d->METH()))); continue;}}
QMap<QString, QVariant> ContentManager::updateDownloadInfos(QString id, const QStringList &keys)
{
    QMap<QString, QVariant> values;
    if (!mp_downloader) {
        for(auto& key: keys) {
            (void) key;
            values.insert(key, "");
        }
        return values;
    }
    auto& b = mp_library->getBookById(id);
    std::shared_ptr<kiwix::Download> d;
    try {
        d = mp_downloader->getDownload(b.getDownloadId());
    } catch(...) {
        kiwix::Book bCopy(b);
        bCopy.setDownloadId("");
        mp_library->getKiwixLibrary()->addOrUpdateBook(bCopy);
        mp_library->save();
        emit(mp_library->booksChanged());
        return values;
    }

    d->updateStatus(true);
    if (d->getStatus() == kiwix::Download::K_COMPLETE) {
        QString tmp(QString::fromStdString(d->getPath()));
        kiwix::Book bCopy(b);
        bCopy.setPath(QDir::toNativeSeparators(tmp).toStdString());
        bCopy.setDownloadId("");
        bCopy.setPathValid(true);
        // removing book url so that download link in kiwix-serve is not displayed.
        bCopy.setUrl("");
        mp_library->getKiwixLibrary()->addOrUpdateBook(bCopy);
        mp_library->save();
        mp_library->bookmarksChanged();
        if (!m_local) {
            emit(oneBookChanged(id));
        } else {
            emit(mp_library->booksChanged());
        }
    }
    for(auto& key: keys){
        ADD_V("id", getDid);
        if(key == "status") {
            switch(d->getStatus()){
            case kiwix::Download::K_ACTIVE:
                values.insert(key, "active");
                break;
            case kiwix::Download::K_WAITING:
                values.insert(key, "waiting");
                break;
            case kiwix::Download::K_PAUSED:
                values.insert(key, "paused");
                break;
            case kiwix::Download::K_ERROR:
                values.insert(key, "error");
                break;
            case kiwix::Download::K_COMPLETE:
                values.insert(key, "completed");
                break;
            case kiwix::Download::K_REMOVED:
                values.insert(key, "removed");
                break;
            default:
                values.insert(key, "unknown");
            }
            continue;
        }
        ADD_V("followedBy", getFollowedBy);
        ADD_V("path", getPath);
        if(key == "totalLength") {
            values.insert(key, QString::number(d->getTotalLength()));
        }
        if(key == "completedLength") {
            values.insert(key, QString::number(d->getCompletedLength()));
        }
        if(key == "downloadSpeed") {
            values.insert(key, QString::number(d->getDownloadSpeed()));
        }
        if(key == "verifiedLength") {
            values.insert(key, QString::number(d->getVerifiedLength()));
        }
    }
    return values;
}
#undef ADD_V

QString ContentManager::downloadBook(const QString &id, QModelIndex index)
{
    QString downloadStatus =  downloadBook(id);
    QString dialogHeader, dialogText;
    if (downloadStatus.size() == 0) {
        dialogHeader = gt("download-unavailable");
        dialogText = gt("download-unavailable-text");
    } else if (downloadStatus == "storage_error") {
        dialogHeader = gt("download-storage-error");
        dialogText = gt("download-storage-error-text");
    } else {
        emit managerModel->startDownload(index);
        return downloadStatus;
    }
    showInfoBox(dialogHeader, dialogText, mp_view);
    return downloadStatus;
}


QString ContentManager::downloadBook(const QString &id)
{
    if (!mp_downloader)
        return "";
    const auto& book = [&]()->const kiwix::Book& {
        try {
            QMutexLocker locker(&remoteLibraryLocker);
            return mp_remoteLibrary->getBookById(id.toStdString());
        } catch (...) {
            return mp_library->getBookById(id);
        }
    }();
    auto downloadPath = KiwixApp::instance()->getSettingsManager()->getDownloadDir();
    QStorageInfo storage(downloadPath);
    auto bytesAvailable = storage.bytesAvailable();
    if (bytesAvailable == -1 || book.getSize() > (unsigned long long) bytesAvailable) {
        return "storage_error";
    }
    auto booksList = mp_library->getBookIds();
    for (auto b : booksList)
        if (b.toStdString() == book.getId())
            return "";
    std::shared_ptr<kiwix::Download> download;
    try {
        std::pair<std::string, std::string> downloadDir("dir", downloadPath.toStdString());
        const std::vector<std::pair<std::string, std::string>> options = { downloadDir };
        download = mp_downloader->startDownload(book.getUrl(), options);
    } catch (std::exception& e) {
        return "";
    }
    kiwix::Book bookCopy(book);
    bookCopy.setDownloadId(download->getDid());
    mp_library->addBookToLibrary(bookCopy);
    mp_library->save();
    emit(oneBookChanged(id));
    return QString::fromStdString(download->getDid());
}

void ContentManager::eraseBookFilesFromComputer(const QString dirPath, const QString fileName, const bool moveToTrash)
{
    if (fileName == "*") {
        return;
    }
    QDir dir(dirPath, fileName);
    for(const QString& file: dir.entryList()) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
        if (moveToTrash)
            QFile::moveToTrash(dir.filePath(file));
        else
#endif
        dir.remove(file); // moveToTrash will always be false here, no check required.
    }
}

QString formatText(QString text)
{
    QString finalText = "<br><br><i>";
    finalText += text;
    finalText += "</i>";
    return finalText;
}

void ContentManager::eraseBook(const QString& id)
{
    auto text = gt("delete-book-text");
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    const auto moveToTrash = KiwixApp::instance()->getSettingsManager()->getMoveToTrash();
#else
    const auto moveToTrash = false; // we do not support move to trash functionality for qt versions below 5.15
#endif
    if (moveToTrash) {
        text += formatText(gt("move-files-to-trash-text"));
    } else {
        text += formatText(gt("perma-delete-files-text"));
    }
    text = text.replace("{{ZIM}}", QString::fromStdString(mp_library->getBookById(id).getTitle()));
    KiwixConfirmBox *dialog = new KiwixConfirmBox(gt("delete-book"), text, false, mp_view);
    dialog->show();
    connect(dialog, &KiwixConfirmBox::yesClicked, [=]() {
        auto tabBar = KiwixApp::instance()->getTabWidget();
        tabBar->closeTabsByZimId(id);
        kiwix::Book book = mp_library->getBookById(id);
        QString dirPath = QString::fromStdString(kiwix::removeLastPathElement(book.getPath()));
        QString fileName = QString::fromStdString(kiwix::getLastPathElement(book.getPath())) + "*";
        eraseBookFilesFromComputer(dirPath, fileName, moveToTrash);
        mp_library->removeBookFromLibraryById(id);
        mp_library->save();
        emit mp_library->bookmarksChanged();
        if (m_local) {
            emit(bookRemoved(id));
        } else {
            emit(oneBookChanged(id));
        }
        KiwixApp::instance()->getSettingsManager()->deleteSettings(id);
        dialog->deleteLater();
        emit booksChanged();
    });
    connect(dialog, &KiwixConfirmBox::noClicked, [=]() {
        dialog->deleteLater();
    });
}

void ContentManager::pauseBook(const QString& id, QModelIndex index)
{
    pauseBook(id);
    emit managerModel->pauseDownload(index);
}

void ContentManager::pauseBook(const QString& id)
{
    if (!mp_downloader) {
        return;
    }
    auto& b = mp_library->getBookById(id);
    auto download = mp_downloader->getDownload(b.getDownloadId());
    if (download->getStatus() == kiwix::Download::K_ACTIVE)
        download->pauseDownload();
}

void ContentManager::resumeBook(const QString& id, QModelIndex index)
{
    resumeBook(id);
    emit managerModel->resumeDownload(index);
}

void ContentManager::resumeBook(const QString& id)
{
    if (!mp_downloader) {
        return;
    }
    auto& b = mp_library->getBookById(id);
    auto download = mp_downloader->getDownload(b.getDownloadId());
    if (download->getStatus() == kiwix::Download::K_PAUSED)
        download->resumeDownload();
}

void ContentManager::cancelBook(const QString& id, QModelIndex index)
{
    auto text = gt("cancel-download-text");
    text = text.replace("{{ZIM}}", QString::fromStdString(mp_library->getBookById(id).getTitle()));
    KiwixConfirmBox *dialog = new KiwixConfirmBox(gt("cancel-download"), text, false, mp_view);
    dialog->show();
    connect(dialog, &KiwixConfirmBox::yesClicked, [=]() {
        cancelBook(id);
        emit managerModel->cancelDownload(index);
        dialog->deleteLater();
    });
    connect(dialog, &KiwixConfirmBox::noClicked, [=]() {
        dialog->deleteLater();
    });
}

void ContentManager::cancelBook(const QString& id)
{
    if (!mp_downloader) {
        return;
    }
    auto& b = mp_library->getBookById(id);
    auto download = mp_downloader->getDownload(b.getDownloadId());
    if (download->getStatus() != kiwix::Download::K_COMPLETE) {
        download->cancelDownload();
    }
    QString dirPath = QString::fromStdString(kiwix::removeLastPathElement(download->getPath()));
    QString filename = QString::fromStdString(kiwix::getLastPathElement(download->getPath())) + "*";
    // incompleted downloaded file should be perma deleted
    eraseBookFilesFromComputer(dirPath, filename, false);
    mp_library->removeBookFromLibraryById(id);
    mp_library->save();
    emit(oneBookChanged(id));
}

QStringList ContentManager::getDownloadIds()
{
    QStringList list;
    if (!mp_downloader)
        return list;
    for(auto& id: mp_downloader->getDownloadIds()) {
        qInfo() << QString::fromStdString(id);
        list.append(QString::fromStdString(id));
    }
    return list;
}

void ContentManager::setCurrentLanguage(FilterList langPairList)
{
    QStringList languageList;
    for (auto &langPair : langPairList) {
        languageList.append(langPair.second);
    }
    languageList.sort();
    for (auto &language : languageList) {
        if (language.length() == 2) {
          try {
            language = QString::fromStdString(
                         kiwix::converta2toa3(language.toStdString()));
          } catch (std::out_of_range&) {}
        }
    }
    auto newLanguage = languageList.join(",");
    if (m_currentLanguage == newLanguage)
        return;
    m_currentLanguage = newLanguage;
    KiwixApp::instance()->getSettingsManager()->setLanguage(langPairList);
    emit(currentLangChanged());
    emit(filterParamsChanged());
}

void ContentManager::setCurrentCategoryFilter(FilterList categoryPairList)
{
    QStringList categoryList;
    for (auto &catPair : categoryPairList) {
        categoryList.append(catPair.second);
    }
    categoryList.sort();
    if (m_categoryFilter == categoryList.join(","))
        return;
    m_categoryFilter = categoryList.join(",");
    KiwixApp::instance()->getSettingsManager()->setCategory(categoryPairList);
    emit(filterParamsChanged());
}

void ContentManager::setCurrentContentTypeFilter(FilterList contentTypeFiltersPairList)
{
    QStringList contentTypeFilters;
    for (auto &ctfPair : contentTypeFiltersPairList) {
        contentTypeFilters.append(ctfPair.second);
    }
    m_contentTypeFilters = contentTypeFilters;
    KiwixApp::instance()->getSettingsManager()->setContentType(contentTypeFiltersPairList);
    emit(filterParamsChanged());
}

void ContentManager::updateLibrary() {
    if (m_local) {
        emit(pendingRequest(false));
        emit(booksChanged());
        return;
    }
    try {
        emit(pendingRequest(true));
        m_remoteLibraryManager.doUpdate(m_currentLanguage, m_categoryFilter);
    } catch (std::runtime_error&) {}
}

void ContentManager::updateRemoteLibrary(const QString& content) {
    QtConcurrent::run([=]() {
        QMutexLocker locker(&remoteLibraryLocker);
        mp_remoteLibrary = kiwix::Library::create();
        kiwix::Manager manager(mp_remoteLibrary);
        const auto catalogUrl = m_remoteLibraryManager.getCatalogHost();
        manager.readOpds(content.toStdString(), catalogUrl.toStdString());
        emit(this->booksChanged());
        emit(this->pendingRequest(false));
    });
}

void ContentManager::updateLanguages(const QString& content) {
    auto languages = kiwix::readLanguagesFromFeed(content.toStdString());
    LanguageList tempLanguages;
    for (auto language : languages) {
        auto code = QString::fromStdString(language.first);
        auto title = QString::fromStdString(language.second);
        tempLanguages.push_back({code, title});
    }
    m_languages = tempLanguages;
    emit(languagesLoaded(m_languages));
}

void ContentManager::updateCategories(const QString& content) {;
    auto categories = kiwix::readCategoriesFromFeed(content.toStdString());
    QStringList tempCategories;
    for (auto catg : categories) {
        tempCategories.push_back(QString::fromStdString(catg));
    }
    m_categories = tempCategories;
    emit(categoriesLoaded(m_categories));
}

void ContentManager::setSearch(const QString &search)
{
    m_searchQuery = search;
    emit(booksChanged());
}

QStringList ContentManager::getBookIds()
{
    kiwix::Filter filter;
    std::vector<std::string> acceptTags, rejectTags;

    for (auto &contentTypeFilter : m_contentTypeFilters) {
        acceptTags.push_back(contentTypeFilter.toStdString());
    }

    filter.acceptTags(acceptTags);
    filter.rejectTags(rejectTags);
    filter.query(m_searchQuery.toStdString());
    if (m_currentLanguage != "")
        filter.lang(m_currentLanguage.toStdString());
    if (m_categoryFilter != "")
        filter.category(m_categoryFilter.toStdString());

    if (m_local) {
        filter.local(true);
        filter.valid(true);
        return mp_library->listBookIds(filter, m_sortBy, m_sortOrderAsc);
    } else {
        filter.remote(true);
        QMutexLocker locker(&remoteLibraryLocker);
        auto bookIds = mp_remoteLibrary->filter(filter);
        mp_remoteLibrary->sort(bookIds, m_sortBy, m_sortOrderAsc);
        QStringList list;
        for(auto& bookId:bookIds) {
            list.append(QString::fromStdString(bookId));
        }
        return list;
    }
}

void ContentManager::setSortBy(const QString& sortBy, const bool sortOrderAsc)
{
    if (sortBy == "unsorted") {
        m_sortBy = kiwix::UNSORTED;
    } else if (sortBy == "title") {
        m_sortBy = kiwix::TITLE;
    } else if (sortBy == "size") {
        m_sortBy = kiwix::SIZE;
    } else if (sortBy == "date") {
        m_sortBy = kiwix::DATE;
    }
    m_sortOrderAsc = sortOrderAsc;
    emit(booksChanged());
}
