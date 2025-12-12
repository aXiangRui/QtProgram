#include "book.h"

Book::Book() : price(0.0), score(0.0), sales(0), heat(0) {}

Book::Book(const QString &id, const QString &title, const QString &category1,
           const QString &category2, double price, double score, int sales, int heat)
    : bookId(id), title(title), categoryId1(category1), categoryId2(category2),
      price(price), score(score), sales(sales), heat(heat) {}

CategoryNode::CategoryNode(const QString &id, const QString &name, CategoryNode *parent)
    : categoryId(id), name(name), parent(parent) {}

CategoryNode::~CategoryNode()
{
    qDeleteAll(children);
}

void CategoryNode::addChild(CategoryNode *child)
{
    child->parent = this;
    children.append(child);
}

void CategoryNode::addBook(const QString &bookId)
{
    if (!bookIds.contains(bookId))
        bookIds.append(bookId);
}

void CategoryNode::removeBook(const QString &bookId)
{
    bookIds.removeAll(bookId);
}
