#include <QProxyStyle>

class MenuProxyStyle: public QProxyStyle
{
	Q_OBJECT

	public:
    MenuProxyStyle(QStyle * style = 0) : QProxyStyle(style) {}
	MenuProxyStyle(const QString & key) : QProxyStyle(key) {}

    virtual int pixelMetric(QStyle::PixelMetric metric, const QStyleOption * option = 0, const QWidget * widget = 0 ) const {
	    switch ( metric ) {
            case QStyle::PM_SmallIconSize:
                return 25;
            default:
                return QProxyStyle::pixelMetric( metric, option, widget );
        }
	}
};