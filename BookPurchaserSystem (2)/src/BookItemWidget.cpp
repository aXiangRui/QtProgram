#include "../include/BookItemWidget.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QPixmap>
#include <QDebug>

BookItemWidget::BookItemWidget(const Book &book, Purchaser *purchaser, QWidget *parent)
    : QWidget(parent), m_book(book), m_purchaser(purchaser)
{
    setFixedSize(200, 300);
    setStyleSheet("border: 1px solid #ccc; border-radius: 5px; padding: 10px;");
    
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    // 封面图片
    m_coverLabel = new QLabel();
    m_coverLabel->setFixedSize(180, 200);
    m_coverLabel->setAlignment(Qt::AlignCenter);
    m_coverLabel->setStyleSheet("background-color: #f0f0f0;");
    
    // 加载图片
    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    connect(manager, &QNetworkAccessManager::finished, this, &BookItemWidget::onImageLoaded);
    manager->get(QNetworkRequest(QUrl(book.coverImage)));
    
    mainLayout->addWidget(m_coverLabel);
    
    // 标题
    QLabel *titleLabel = new QLabel(book.title);
    titleLabel->setWordWrap(true);
    titleLabel->setMaximumHeight(40);
    titleLabel->setStyleSheet("font-weight: bold;");
    mainLayout->addWidget(titleLabel);
    
    // 价格
    QLabel *priceLabel = new QLabel(QString("¥%1").arg(book.price));
    priceLabel->setStyleSheet("color: red; font-weight: bold;");
    mainLayout->addWidget(priceLabel);
    
    // 按钮
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    
    QPushButton *addToCartButton = new QPushButton("加入购物车");
    QPushButton *viewDetailButton = new QPushButton("查看详情");
    
    buttonLayout->addWidget(addToCartButton);
    buttonLayout->addWidget(viewDetailButton);
    
    mainLayout->addLayout(buttonLayout);
    
    connect(addToCartButton, &QPushButton::clicked, this, &BookItemWidget::onAddToCart);
    connect(viewDetailButton, &QPushButton::clicked, this, &BookItemWidget::onViewDetail);

    // 设置标签可点击
        m_coverLabel->setCursor(Qt::PointingHandCursor);

        // 安装事件过滤器
        m_coverLabel->installEventFilter(this);

}
void BookItemWidget::onImageLoaded(QNetworkReply *reply)
{
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray data = reply->readAll();
        QPixmap pixmap;
        pixmap.loadFromData(data);
        m_coverLabel->setPixmap(pixmap.scaled(m_coverLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
    reply->deleteLater();
}

void BookItemWidget::onAddToCart()
{
    emit addToCart(m_book.bookId);
}

void BookItemWidget::onViewDetail()
{
    emit bookClicked(m_book);
}

bool BookItemWidget::eventFilter(QObject *obj, QEvent *event)
{
     if (obj == m_coverLabel && event->type() == QEvent::MouseButtonPress)  {
        emit bookClicked(m_book);
        return true;
    }
    return QWidget::eventFilter(obj, event);
}
