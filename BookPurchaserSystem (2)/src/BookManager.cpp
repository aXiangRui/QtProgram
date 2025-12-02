#include "../include/BookManager.h"
#include <QDebug>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

BookManager::BookManager(QObject *parent)
    : QObject(parent), m_isDataLoaded(false)
{
    // 加载图书数据
    loadBooks();
}

BookManager::~BookManager()
{
}

QList<Book> BookManager::getAllBooks()
{
    if (!m_isDataLoaded) {
        loadBooks();
    }
    return m_books;
}

Book BookManager::getBookById(const QString& bookId)
{
    if (!m_isDataLoaded) {
        loadBooks();
    }
    
    if (m_bookMap.contains(bookId)) {
        return m_bookMap[bookId];
    }
    
    // 返回空图书对象
    return Book();
}

QList<Book> BookManager::getBooksByCategory(const QString& categoryId)
{
    if (!m_isDataLoaded) {
        loadBooks();
    }
    
    QList<Book> result;
    for (const Book& book : m_books) {
        if (book.category == categoryId || book.subCategory == categoryId) {
            result.append(book);
        }
    }
    
    return result;
}

QList<Book> BookManager::searchBooks(const QString& keyword)
{
    if (!m_isDataLoaded) {
        loadBooks();
    }
    
    QList<Book> result;
    QString lowerKeyword = keyword.toLower();
    
    for (const Book& book : m_books) {
        if (book.title.toLower().contains(lowerKeyword) ||
            book.author.toLower().contains(lowerKeyword) ||
            book.description.toLower().contains(lowerKeyword) ||
            book.tags.contains(lowerKeyword)) {
            result.append(book);
        }
    }
    
    return result;
}

QList<Book> BookManager::getPopularBooks()
{
    if (!m_isDataLoaded) {
        loadBooks();
    }
    
    // 按销量排序
    QList<Book> sortedBooks = m_books;
    std::sort(sortedBooks.begin(), sortedBooks.end(), [](const Book& a, const Book& b) {
        return a.sales > b.sales;
    });
    
    // 返回前20本
    return sortedBooks.mid(0, 20);
}

bool BookManager::updateBook(const Book& book)
{
    if (!m_isDataLoaded) {
        loadBooks();
    }
    
    if (m_bookMap.contains(book.bookId)) {
        // 更新图书信息
        m_bookMap[book.bookId] = book;
        
        // 更新列表中的图书
        for (int i = 0; i < m_books.size(); ++i) {
            if (m_books[i].bookId == book.bookId) {
                m_books[i] = book;
                break;
            }
        }
        
        // 更新缓存
        cacheBooks();
        
        return true;
    }
    
    return false;
}

void BookManager::loadBooks()
{
    // 尝试从缓存加载
    loadBooksFromCache();
    
    // 如果缓存不存在或加载失败，生成模拟数据
    if (!m_isDataLoaded) {
        generateMockBooks();
    }
}

void BookManager::loadBooksFromCache()
{
    QFile file(":/cache/books.json");
    if (!file.exists() || !file.open(QIODevice::ReadOnly)) {
        return;
    }
    
    QByteArray data = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull()) {
        return;
    }
    
    QJsonArray booksArray = doc.array();
    for (const QJsonValue& value : booksArray) {
        QJsonObject obj = value.toObject();
        Book book;
        book.bookId = obj["bookId"].toString();
        book.title = obj["title"].toString();
        book.author = obj["author"].toString();
        book.category = obj["category"].toString();
        book.subCategory = obj["subCategory"].toString();
        book.price = obj["price"].toDouble();
        book.originalPrice = obj["originalPrice"].toDouble();
        book.stock = obj["stock"].toInt();
        book.sales = obj["sales"].toInt();
        book.score = obj["score"].toDouble();
        book.description = obj["description"].toString();
        book.coverImage = obj["coverImage"].toString();
        
        // 加载详情图片
        QJsonArray detailImagesArray = obj["detailImages"].toArray();
        for (const QJsonValue& imgValue : detailImagesArray) {
            book.detailImages.append(imgValue.toString());
        }
        
        // 加载标签
        QJsonArray tagsArray = obj["tags"].toArray();
        for (const QJsonValue& tagValue : tagsArray) {
            book.tags.append(tagValue.toString());
        }
        
        m_books.append(book);
        m_bookMap[book.bookId] = book;
    }
    
    m_isDataLoaded = true;
}

void BookManager::loadBooksFromServer()
{
    // 这里应该从服务器加载图书数据
    // 由于是模拟环境，我们使用模拟数据
    generateMockBooks();
}

void BookManager::cacheBooks()
{
    // 将图书数据缓存到本地
    QJsonArray booksArray;
    for (const Book& book : m_books) {
        QJsonObject obj;
        obj["bookId"] = book.bookId;
        obj["title"] = book.title;
        obj["author"] = book.author;
        obj["category"] = book.category;
        obj["subCategory"] = book.subCategory;
        obj["price"] = book.price;
        obj["originalPrice"] = book.originalPrice;
        obj["stock"] = book.stock;
        obj["sales"] = book.sales;
        obj["score"] = book.score;
        obj["description"] = book.description;
        obj["coverImage"] = book.coverImage;
        
        // 保存详情图片
        QJsonArray detailImagesArray;
        for (const QString& img : book.detailImages) {
            detailImagesArray.append(img);
        }
        obj["detailImages"] = detailImagesArray;
        
        // 保存标签
        QJsonArray tagsArray;
        for (const QString& tag : book.tags) {
            tagsArray.append(tag);
        }
        obj["tags"] = tagsArray;
        
        booksArray.append(obj);
    }
    
    QJsonDocument doc(booksArray);
    QFile file(":/cache/books.json");
    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson());
        file.close();
    }
}

void BookManager::generateMockBooks()
{
    // 生成模拟图书数据
    QStringList categories = {"文学", "科技", "教育", "艺术", "历史", "经济", "哲学", "心理学"};
    QStringList subCategories = {"小说", "散文", "诗歌", "计算机", "物理", "化学", "数学", "外语", "设计", "音乐", "绘画", "古代史", "近代史", "世界史", "经济学", "管理学", "哲学理论", "伦理学", "认知心理学", "发展心理学"};
    
    for (int i = 1; i <= 100; ++i) {
        Book book;
        book.bookId = QString("B%1").arg(i, 6, 10, QChar('0'));
        book.title = QString("图书标题%1").arg(i);
        book.author = QString("作者%1").arg(i % 20 + 1);
        book.category = categories[i % categories.size()];
        book.subCategory = subCategories[i % subCategories.size()];
        book.price = 20.0 + (i % 50) * 2.5;
        book.originalPrice = book.price * 1.2;
        book.stock = 100 + (i % 50);
        book.sales = 1000 + (i % 1000);
        book.score = 4.0 + (i % 10) * 0.1;
        book.description = QString("这是一本关于%1的图书，内容丰富，值得一读。\n\n本书详细介绍了%1的基础知识和进阶技巧，适合初学者和有一定基础的读者阅读。通过阅读本书，读者将能够掌握%1的核心概念和应用方法。").arg(book.category);
        book.coverImage = QString("https://picsum.photos/seed/book%1/300/400").arg(i);
        
        // 添加详情图片
        for (int j = 1; j <= 3; ++j) {
            book.detailImages.append(QString("https://picsum.photos/seed/book%1detail%2/800/600").arg(i).arg(j));
        }
        
        // 添加标签
        book.tags.append(book.category);
        book.tags.append(book.subCategory);
        if (i % 3 == 0) book.tags.append("畅销");
        if (i % 4 == 0) book.tags.append("推荐");
        if (i % 5 == 0) book.tags.append("新书");
        
        m_books.append(book);
        m_bookMap[book.bookId] = book;
    }
    
    m_isDataLoaded = true;
    
    // 缓存模拟数据
    cacheBooks();
}