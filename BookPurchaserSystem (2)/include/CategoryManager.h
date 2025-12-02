#ifndef CATEGORYMANAGER_H
#define CATEGORYMANAGER_H

#include <QObject>
#include <QString>
#include <QList>
#include <QMap>
#include "Purchaser.h"

// 分类管理器类，负责分类数据的加载、缓存和更新
class CategoryManager : public QObject
{
    Q_OBJECT
public:
    CategoryManager(QObject *parent = nullptr);
    ~CategoryManager();

    // 获取分类树
    Category getCategoryTree();
    
    // 获取所有一级分类
    QList<Category> getRootCategories();
    
    // 获取指定分类的子分类
    QList<Category> getSubCategories(const QString& categoryId);
    
    // 根据ID获取分类
    Category getCategoryById(const QString& categoryId);
    
    // 添加分类
    bool addCategory(const Category& category);
    
    // 更新分类
    bool updateCategory(const Category& category);
    
    // 删除分类
    bool deleteCategory(const QString& categoryId);
    
    // 加载分类数据
    void loadCategories();
    
    // 缓存分类数据
    void cacheCategories();

private:
    // 从本地缓存加载分类数据
    void loadCategoriesFromCache();
    
    // 从服务器加载分类数据
    void loadCategoriesFromServer();
    
    // 模拟分类数据（用于测试）
    void generateMockCategories();
    
    // 递归构建分类树
    Category buildCategoryTree(const QList<Category>& allCategories, const QString& parentId = "");

private:
    Category m_categoryTree;
    QList<Category> m_allCategories;
    QMap<QString, Category> m_categoryMap; // 用于快速查找
    bool m_isDataLoaded;
};

#endif // CATEGORYMANAGER_H