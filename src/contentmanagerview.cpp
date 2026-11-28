#include "contentmanagerview.h"
#include <QFile>
#include <QOpenGLFunctions>
#include <QOpenGLContext>
#include "kiwixapp.h"
#include "contentmanagerdelegate.h"
#include <QLineEdit>
#include "ui_contentmanagerview.h"

ContentManagerView::ContentManagerView(QWidget *parent)
    : QOpenGLWidget(parent), mp_ui(new Ui::contentmanagerview)
{
    mp_ui->setupUi(this);
    mp_ui->m_view->setSortingEnabled(true);

    // Ensure child widgets use stylesheet-defined background colors rather than
    // inheriting the OS palette (which causes grey flashes on some platforms, e.g. Ubuntu).
    mp_ui->stackedWidget->setAttribute(Qt::WA_StyledBackground, true);
    mp_ui->contents->setAttribute(Qt::WA_StyledBackground, true);
    mp_ui->loading->setAttribute(Qt::WA_StyledBackground, true);

    auto applyStyle = [this]() {
        mp_ui->m_view->setStyleSheet(themeAwareCss(":/css/_contentManager.css"));
        mp_ui->m_view->viewport()->update();
        this->update();
    };
    applyStyle();
    connect(KiwixApp::instance()->getSettingsManager(), &SettingsManager::themeChanged,
            this, [applyStyle](SettingsManager::Theme) { applyStyle(); });
    mp_ui->m_view->setContextMenuPolicy(Qt::CustomContextMenu);
    auto managerDelegate = new ContentManagerDelegate();
    mp_ui->m_view->setItemDelegate(managerDelegate);
    mp_ui->m_view->setCursor(Qt::PointingHandCursor);

    loader = new KiwixLoader(mp_ui->loading);
    mp_ui->stackedWidget->setCurrentIndex(0);

    connect(mp_ui->m_view, &QTreeView::clicked, this, &ContentManagerView::onClicked);
    connect(mp_ui->m_view, &QTreeView::expanded, this, &ContentManagerView::onExpanded);
    connect(this, &ContentManagerView::sizeHintChanged, managerDelegate, &QStyledItemDelegate::sizeHintChanged);

    // Needed to reveal the situation with downloads not being updated timely
    // (due to aria2c becoming unresponsive when saving to slow storage)
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, [this]() { mp_ui->m_view->viewport()->update(); });
    timer->start(1000);
}

ContentManagerView::~ContentManagerView()
{

}

void ContentManagerView::initializeGL()
{
}

void ContentManagerView::paintGL()
{
    const bool dark = KiwixApp::instance() && KiwixApp::instance()->isDarkTheme();
    QOpenGLFunctions *f = QOpenGLContext::currentContext() ? QOpenGLContext::currentContext()->functions() : nullptr;
    if (f) {
        if (dark) {
            f->glClearColor(0.125f, 0.129f, 0.141f, 1.0f);
        } else {
            f->glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        }
        f->glClear(GL_COLOR_BUFFER_BIT);
    }
}

void ContentManagerView::showLoader(bool show)
{
    mp_ui->stackedWidget->setCurrentIndex(show);
    if (show) {
        loader->startAnimation();
    } else {
        loader->stopAnimation();
    }
}

void ContentManagerView::onClicked(QModelIndex index)
{
    if (index.column() == (mp_ui->m_view->model()->columnCount() - 1))
        return;

    auto zeroColIndex = index.siblingAtColumn(0);
    if (mp_ui->m_view->isExpanded(zeroColIndex)) {
        mp_ui->m_view->collapse(zeroColIndex);
    } else {
        mp_ui->m_view->expand(zeroColIndex);
    }
}

void ContentManagerView::onExpanded(QModelIndex index)
{
    if (!mp_ui->m_view->isFirstColumnSpanned(0, index))
        mp_ui->m_view->setFirstColumnSpanned(0, index, true);
}

/**
 * @brief Notify delegate to update size hint of the visible description rows.
 */
void ContentManagerView::updateSizeHint()
{
    auto view = this->getView();
    if (!view->isVisible())
        return;

    auto visibleIndex = view->indexAt(view->rect().topLeft());
    while (visibleIndex.isValid())
    {
        if (isDescriptionIndex(visibleIndex))
            emit sizeHintChanged(visibleIndex);
        visibleIndex = view->indexBelow(visibleIndex);
    }
}
