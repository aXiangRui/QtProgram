#ifndef BOOKMANAGER_H
#define BOOKMANAGER_H

#include <QObject>
#include <QList>
#include <QMap>
#include <QString>
#include "Purchaser.h"

// 图书管理器类，负责图书数据的加载、缓存和更新
class BookManager : public QObject
{
    Q_OBJECT
public:
    BookManager(QObject *parent = nullptr);
    ~BookManager();

    // 获取图书列表
    QList<Book> getAllBooks();
    
    // 根据ID获取图书
    Book getBookById(const QString& bookId);
    
    // 根据分类获取图书
    QList<Book> getBooksByCategory(const QString& categoryId);
    
    // 根据关键词搜索图书
    QList<Book> searchBooks(const QString& keyword);
    
    // 获取热门图书
    QList<Book> getPopularBooks();
    
    // 更新图书信息
    bool updateBook(const Book& book);
    
    // 加载图书数据
    void loadBooks();
    
    // 缓存图书数据
    void cacheBooks();

private:
    // 从本地缓存加载图书数据
    void loadBooksFromCache();
    
    // 从服务器加载图书数据
    void loadBooksFromServer();
    
    // 模拟图书数据（用于测试）
    void generateMockBooks();

private:
    QList<Book> m_books;
    QMap<QString, Book> m_bookMap; // 用于快速查找
    bool m_isDataLoaded;
};

#endif // BOOKMANAGER_H