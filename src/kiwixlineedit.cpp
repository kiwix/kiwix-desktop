#include "kiwixlineedit.h"

KiwixLineEdit::KiwixLineEdit(QWidget *parent) : QLineEdit(parent)
{
    installEventFilter(this);
}

KiwixLineEdit::~KiwixLineEdit()
{
}

void KiwixLineEdit::resizeEvent(QResizeEvent *event)
{
    QLineEdit::resizeEvent(event);
    adjustSize();
}
bool KiwixLineEdit::eventFilter(QObject* object, QEvent* event)
{
    Q_UNUSED(object);

    if (event->type() == QEvent::MouseButtonPress) {
        emit(clicked());
    } else if (event->type() == QEvent::FocusIn) {
        emit(focusedIn());
    } else if (event->type() == QEvent::FocusOut) {
        emit(focusedOut());
    }
    return false;
}
