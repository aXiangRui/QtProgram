#ifndef BOOKDETAILWIDGET_H
#define BOOKDETAILWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QTabWidget>
#include <QTextEdit>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include "Purchaser.h"

class BookDetailWidget : public QWidget
{
    Q_OBJECT
public:
    BookDetailWidget(const Book &book, Purchaser *purchaser, QWidget *parent = nullptr);

signals:
    void backToHome();
    void addToCart(const QString &bookId);
    void buyNow(const QString &bookId, int quantity);

private slots:
    void onCoverImageLoaded(QNetworkReply *reply);
    void onAddToCart();
    void onBuyNow();
    void onAddToFavorites();

private:
    Book m_book;
    Purchaser *m_purchaser;
    QLabel *m_coverLabel;
    QSpinBox *m_quantitySpinBox;
};

#endif // BOOKDETAILWIDGET_H
