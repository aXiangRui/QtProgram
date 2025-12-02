#include "../include/BookDetailWidget.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QPixmap>
#include <QMessageBox>
#include <QDebug>

BookDetailWidget::BookDetailWidget(const Book &book, Purchaser *purchaser, QWidget *parent)
    : QWidget(parent), m_book(book), m_purchaser(purchaser)
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    // 顶部导航栏
    QHBoxLayout *navLayout = new QHBoxLayout();
    QPushButton *backButton = new QPushButton("返回");
    navLayout->addWidget(backButton);
    navLayout->addStretch();
    mainLayout->addLayout(navLayout);
    
    // 顶部信息
    QHBoxLayout *topLayout = new QHBoxLayout();
    
    // 封面图片
    m_coverLabel = new QLabel();
    m_coverLabel->setFixedSize(200, 250);
    m_coverLabel->setStyleSheet("background-color: #f0f0f0;");
    
    // 加载图片
    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    connect(manager, &QNetworkAccessManager::finished, this, &BookDetailWidget::onCoverImageLoaded);
    manager->get(QNetworkRequest(QUrl(book.coverImage)));
    
    topLayout->addWidget(m_coverLabel);
    
    // 图书信息
    QVBoxLayout *infoLayout = new QVBoxLayout();
    
    QLabel *titleLabel = new QLabel(book.title);
    titleLabel->setStyleSheet("font-size: 20px; font-weight: bold;");
    infoLayout->addWidget(titleLabel);
    
    QLabel *authorLabel = new QLabel(QString("作者: %1").arg(book.author));
    infoLayout->addWidget(authorLabel);
    
    QLabel *categoryLabel = new QLabel(QString("分类: %1 / %2").arg(book.category).arg(book.subCategory));
    infoLayout->addWidget(categoryLabel);
    
    QLabel *priceLabel = new QLabel(QString("价格: ¥%1").arg(book.price));
    priceLabel->setStyleSheet("color: red; font-size: 18px; font-weight: bold;");
    infoLayout->addWidget(priceLabel);
    
    QLabel *originalPriceLabel = new QLabel(QString("原价: ¥%1").arg(book.originalPrice));
    originalPriceLabel->setStyleSheet("color: #999; text-decoration: line-through;");
    infoLayout->addWidget(originalPriceLabel);
    
    QLabel *salesLabel = new QLabel(QString("销量: %1").arg(book.sales));
    infoLayout->addWidget(salesLabel);
    
    QLabel *scoreLabel = new QLabel(QString("评分: %1").arg(book.score));
    infoLayout->addWidget(scoreLabel);
    
    // 购买区域
    QHBoxLayout *buyLayout = new QHBoxLayout();
    
    QLabel *quantityLabel = new QLabel("数量:");
    buyLayout->addWidget(quantityLabel);
    
    m_quantitySpinBox = new QSpinBox();
    m_quantitySpinBox->setMinimum(1);
    m_quantitySpinBox->setMaximum(book.stock);
    buyLayout->addWidget(m_quantitySpinBox);
    
    buyLayout->addStretch();
    
    QPushButton *addToCartButton = new QPushButton("加入购物车");
    addToCartButton->setStyleSheet("background-color: #ff9500; color: white;");
    buyLayout->addWidget(addToCartButton);
    
    QPushButton *buyNowButton = new QPushButton("立即购买");
    buyNowButton->setStyleSheet("background-color: #ff2d55; color: white;");
    buyLayout->addWidget(buyNowButton);
    
    infoLayout->addLayout(buyLayout);
    
    // 收藏按钮
    QPushButton *addToFavoritesButton = new QPushButton("加入收藏");
    infoLayout->addWidget(addToFavoritesButton);
    
    topLayout->addLayout(infoLayout);
    
    mainLayout->addLayout(topLayout);
    
    // 详情标签页
    QTabWidget *tabWidget = new QTabWidget();
    
    // 商品详情
    QTextEdit *descriptionEdit = new QTextEdit(book.description);
    descriptionEdit->setReadOnly(true);
    tabWidget->addTab(descriptionEdit, "商品详情");
    
    // 评价
    QTextEdit *reviewsEdit = new QTextEdit("暂无评价");
    reviewsEdit->setReadOnly(true);
    tabWidget->addTab(reviewsEdit, "用户评价");
    
    mainLayout->addWidget(tabWidget);
    
    connect(backButton, &QPushButton::clicked, this, &BookDetailWidget::backToHome);
    connect(addToCartButton, &QPushButton::clicked, this, &BookDetailWidget::onAddToCart);
    connect(buyNowButton, &QPushButton::clicked, this, &BookDetailWidget::onBuyNow);
    connect(addToFavoritesButton, &QPushButton::clicked, this, &BookDetailWidget::onAddToFavorites);
}

void BookDetailWidget::onCoverImageLoaded(QNetworkReply *reply)
{
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray data = reply->readAll();
        QPixmap pixmap;
        pixmap.loadFromData(data);
        m_coverLabel->setPixmap(pixmap.scaled(m_coverLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
    reply->deleteLater();
}

void BookDetailWidget::onAddToCart()
{
    int quantity = m_quantitySpinBox->value();
    emit addToCart(m_book.bookId);
}

void BookDetailWidget::onBuyNow()
{
    int quantity = m_quantitySpinBox->value();
    emit buyNow(m_book.bookId, quantity);
}

void BookDetailWidget::onAddToFavorites()
{
    if (m_purchaser->AddToLove(m_book.bookId)) {
        QMessageBox::information(this, "成功", "已添加到收藏");
    } else {
        QMessageBox::warning(this, "失败", "添加到收藏失败或已在收藏中");
    }
}
