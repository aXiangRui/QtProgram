#ifndef BOOK_H
#define BOOK_H

#include <QString>
#include <QList>
#include <QDate>

// 图书类
class Book
{
public:
    Book();
    Book(const QString &id, const QString &title, const QString &category1,
         const QString &category2, double price, double score = 0.0,
         int sales = 0, int heat = 0);

    // Getter方法
    QString getId() const { return bookId; }
    QString getTitle() const { return title; }
    QString getCategory1() const { return categoryId1; }
    QString getCategory2() const { return categoryId2; }
    double getPrice() const { return price; }
    double getScore() const { return score; }
    int getSales() const { return sales; }
    int getHeat() const { return heat; }
    int getFavoriteCount() const { return favoriteCount; }
    QString getAuthor() const { return author; }
    QString getPublisher() const { return publisher; }
    QString getDescription() const { return description; }
    QDate getPublishDate() const { return publishDate; }
    QString getCoverImage() const { return coverImage; }
    int getMerchantId() const { return merchantId; }

    // Setter方法
    void setScore(double s) { score = s; }
    void setHeat(int h) { heat = h; }
    void setSales(int s) { sales = s; }
    void setAuthor(const QString &a) { author = a; }
    void setPublisher(const QString &p) { publisher = p; }
    void setDescription(const QString &d) { description = d; }
    void setPublishDate(const QDate &d) { publishDate = d; }
    void setCoverImage(const QString &img) { coverImage = img; }
    void setMerchantId(int id) { merchantId = id; }

public:
    QString bookId;          // 图书编号
    QString title;          // 书名
    QString categoryId1;    // 一级分类
    QString categoryId2;    // 二级分类
    double price;           // 价格
    double score;           // 评分
    int sales;              // 销量
    int heat;               // 热度值
    int favoriteCount;      // 收藏量
    QString author;         // 作者
    QString publisher;      // 出版社
    QString description;    // 描述
    QDate publishDate;      // 出版日期
    QString coverImage;     // 封面图片(Base64编码)
    int merchantId;         // 商家ID
};

// 分类树节点
class CategoryNode
{
public:
    CategoryNode(const QString &id, const QString &name, CategoryNode *parent = nullptr);
    ~CategoryNode();

    QString getId() const { return categoryId; }
    QString getName() const { return name; }
    CategoryNode* getParent() const { return parent; }
    QList<CategoryNode*> getChildren() const { return children; }
    QList<QString> getBookIds() const { return bookIds; }

    void addChild(CategoryNode *child);
    void addBook(const QString &bookId);
    void removeBook(const QString &bookId);

private:
    QString categoryId;
    QString name;
    CategoryNode *parent;
    QList<CategoryNode*> children;
    QList<QString> bookIds;
};

#endif // BOOK_H
