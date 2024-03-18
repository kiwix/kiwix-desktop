#ifndef PORTUTILS_H
#define PORTUTILS_H

#include <QEvent>

namespace portutils {


#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0) // Earlier than Qt6
    inline int getX(const QMouseEvent& e) {
        return e.x();
    }

    inline int getY(const QMouseEvent& e) {
        return e.y();
    }

    inline QPoint getGlobalPos(const QMouseEvent& e) {
        return e.globalPos();
    }

#else // Qt6 and later
    inline int getX(const QMouseEvent& e) {
    return e.position().x();
    }

    inline int getY(const QMouseEvent& e) {
    return e.position().y();
    }

    inline QPoint getGlobalPos(const QMouseEvent& e) {
        return e.globalPosition().toPoint();
    }

#endif

}

#endif // PORTUTILS_H
