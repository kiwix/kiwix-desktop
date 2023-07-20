#ifndef KIWIXLINEEDIT_H
#define KIWIXLINEEDIT_H

#include<QLineEdit>
#include<QEvent>

class KiwixLineEdit : public QLineEdit {
    Q_OBJECT;
public:
    KiwixLineEdit(QWidget *parent = nullptr);
    ~KiwixLineEdit();
protected:
    void resizeEvent(QResizeEvent *event) override;
    bool eventFilter(QObject* object, QEvent* event) override;
signals:
    void clicked();
    void focusedIn();
    void focusedOut();
};
#endif // KIWIXLINEEDIT_H
