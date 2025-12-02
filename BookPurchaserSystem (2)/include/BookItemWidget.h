#ifndef BOOKITEMWIDGET_H
#define BOOKITEMWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QMouseEvent>
#include "Purchaser.h"

class BookItemWidget : public QWidget
{
    Q_OBJECT
public:
    BookItemWidget(const Book &book, Purchaser *purchaser, QWidget *parent = nullptr);

signals:
    void addToCart(const QString &bookId);
    void bookClicked(const Book &book);

private slots:
    void onImageLoaded(QNetworkReply *reply);
    void onAddToCart();
    void onViewDetail();
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    Book m_book;
    Purchaser *m_purchaser;
    QLabel *m_coverLabel;
};

#endif // BOOKITEMWIDGET_H
