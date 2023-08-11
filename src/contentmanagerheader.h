#ifndef CONTENTMANAGERHEADER_H
#define CONTENTMANAGERHEADER_H

#include <QHeaderView>

class ContentManagerHeader : public QHeaderView
{
    Q_OBJECT
public:
    explicit ContentManagerHeader(Qt::Orientation orientation, QWidget *parent = nullptr);
    ~ContentManagerHeader();

protected:
    void paintSection(QPainter* painter, const QRect& rect, int logicalIndex) const override;

};

#endif // CONTENTMANAGERHEADER_H
