#ifndef CONTENTMANAGERVIEW_H
#define CONTENTMANAGERVIEW_H

#include <QOpenGLWidget>
#include "ui_contentmanagerview.h"
#include "kiwixloader.h"

namespace Ui {
class contentmanagerview;
}

// Inherit from QOpenGLWidget rather than QWidget so that the top-level native
// window is created with an OpenGL-compatible surface from application startup.
// In Qt 6, dynamically adding the first QWebEngineView (which embeds a QOpenGLWidget)
// into an active raster-backed window forces Qt to destroy and recreate the native
// window handle, causing the window to momentarily disappear and reappear (#1454).
class ContentManagerView : public QOpenGLWidget
{
    Q_OBJECT

public:
    explicit ContentManagerView(QWidget *parent = nullptr);
    ~ContentManagerView();
    QTreeView* getView() { return mp_ui->m_view; }

public slots:
    void showLoader(bool show);
    void onClicked(QModelIndex index);
    void onExpanded(QModelIndex index);
    void updateSizeHint();

signals:
    void sizeHintChanged(const QModelIndex& index);

private:
    Ui::contentmanagerview *mp_ui;
    KiwixLoader *loader;
};

#endif // CONTENTMANAGERVIEW_H
