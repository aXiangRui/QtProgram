#include "../include/CategoryManager.h"
#include <QDebug>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

CategoryManager::CategoryManager(QObject *parent)
    : QObject(parent), m_isDataLoaded(false)
{
    // 加载分类数据
    loadCategories();
}

CategoryManager::~CategoryManager()
{
}

Category CategoryManager::getCategoryTree()
{
    if (!m_isDataLoaded) {
        loadCategories();
    }
    
    return m_categoryTree;
}

QList<Category> CategoryManager::getRootCategories()
{
    if (!m_isDataLoaded) {
        loadCategories();
    }
    
    return m_categoryTree.subCategories;
}

QList<Category> CategoryManager::getSubCategories(const QString& categoryId)
{
    if (!m_isDataLoaded) {
        loadCategories();
    }
    
    if (m_categoryMap.contains(categoryId)) {
        return m_categoryMap[categoryId].subCategories;
    }
    
    return QList<Category>();
}

Category CategoryManager::getCategoryById(const QString& categoryId)
{
    if (!m_isDataLoaded) {
        loadCategories();
    }
    
    if (m_categoryMap.contains(categoryId)) {
        return m_categoryMap[categoryId];
    }
    
    // 返回空分类对象
    return Category();
}

bool CategoryManager::addCategory(const Category& category)
{
    if (!m_isDataLoaded) {
        loadCategories();
    }
    
    // 检查分类ID是否已存在
    if (m_categoryMap.contains(category.categoryId)) {
        return false;
    }
    
    // 添加到列表和映射
    m_allCategories.append(category);
    m_categoryMap[category.categoryId] = category;
    
    // 重新构建分类树
    m_categoryTree = buildCategoryTree(m_allCategories);
    
    // 更新缓存
    cacheCategories();
    
    return true;
}

bool CategoryManager::updateCategory(const Category& category)
{
    if (!m_isDataLoaded) {
        loadCategories();
    }
    
    if (!m_categoryMap.contains(category.categoryId)) {
        return false;
    }
    
    // 更新列表和映射
    for (int i = 0; i < m_allCategories.size(); ++i) {
        if (m_allCategories[i].categoryId == category.categoryId) {
            m_allCategories[i] = category;
            break;
        }
    }
    m_categoryMap[category.categoryId] = category;
    
    // 重新构建分类树
    m_categoryTree = buildCategoryTree(m_allCategories);
    
    // 更新缓存
    cacheCategories();
    
    return true;
}

bool CategoryManager::deleteCategory(const QString& categoryId)
{
    if (!m_isDataLoaded) {
        loadCategories();
    }
    
    if (!m_categoryMap.contains(categoryId)) {
        return false;
    }
    
    // 检查是否有子分类
    if (!m_categoryMap[categoryId].subCategories.isEmpty()) {
        return false; // 不允许删除有子分类的分类
    }
    
    // 从列表和映射中删除
    m_allCategories.removeOne(m_categoryMap[categoryId]);
    m_categoryMap.remove(categoryId);
    
    // 重新构建分类树
    m_categoryTree = buildCategoryTree(m_allCategories);
    
    // 更新缓存
    cacheCategories();
    
    return true;
}

void CategoryManager::loadCategories()
{
    // 尝试从缓存加载
    loadCategoriesFromCache();
    
    // 如果缓存不存在或加载失败，生成模拟数据
    if (!m_isDataLoaded) {
        generateMockCategories();
    }
}

void CategoryManager::loadCategoriesFromCache()
{
    QFile file(":/cache/categories.json");
    if (!file.exists() || !file.open(QIODevice::ReadOnly)) {
        return;
    }
    
    QByteArray data = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull()) {
        return;
    }
    
    QJsonArray categoriesArray = doc.array();
    for (const QJsonValue& value : categoriesArray) {
        QJsonObject obj = value.toObject();
        Category category;
        category.categoryId = obj["categoryId"].toString();
        category.name = obj["name"].toString();
        category.parentId = obj["parentId"].toString();
        
        m_allCategories.append(category);
        m_categoryMap[category.categoryId] = category;
    }
    
    // 构建分类树
    m_categoryTree = buildCategoryTree(m_allCategories);
    
    m_isDataLoaded = true;
}

void CategoryManager::loadCategoriesFromServer()
{
    // 这里应该从服务器加载分类数据
    // 由于是模拟环境，我们使用模拟数据
    generateMockCategories();
}

void CategoryManager::cacheCategories()
{
    // 将分类数据缓存到本地
    QJsonArray categoriesArray;
    for (const Category& category : m_allCategories) {
        QJsonObject obj;
        obj["categoryId"] = category.categoryId;
        obj["name"] = category.name;
        obj["parentId"] = category.parentId;
        categoriesArray.append(obj);
    }
    
    QJsonDocument doc(categoriesArray);
    QFile file(":/cache/categories.json");
    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson());
        file.close();
    }
}

void CategoryManager::generateMockCategories()
{
    // 生成模拟分类数据
    QStringList rootCategoryNames = {"文学", "科技", "教育", "艺术", "历史", "经济", "哲学", "心理学"};
    QMap<QString, QStringList> subCategoryNames;
    
    QStringList literatureSubs;
    literatureSubs << "小说" << "散文" << "诗歌" << "剧本" << "传记" << "文学理论";
    subCategoryNames["文学"] = literatureSubs;
    
    QStringList techSubs;
    techSubs << "计算机" << "物理" << "化学" << "生物" << "天文" << "地理" << "医学";
    subCategoryNames["科技"] = techSubs;
    
    QStringList eduSubs;
    eduSubs << "教材" << "教辅" << "外语" << "考试" << "教育理论" << "儿童教育";
    subCategoryNames["教育"] = eduSubs;
    
    QStringList artSubs;
    artSubs << "绘画" << "音乐" << "设计" << "摄影" << "建筑" << "电影" << "舞蹈";
    subCategoryNames["艺术"] = artSubs;
    
    QStringList historySubs;
    historySubs << "古代史" << "近代史" << "现代史" << "世界史" << "人物传记" << "历史研究";
    subCategoryNames["历史"] = historySubs;
    
    QStringList econSubs;
    econSubs << "经济学" << "管理学" << "金融" << "投资" << "会计" << "市场营销";
    subCategoryNames["经济"] = econSubs;
    
    QStringList philoSubs;
    philoSubs << "哲学理论" << "伦理学" << "逻辑学" << "美学" << "宗教学" << "哲学史";
    subCategoryNames["哲学"] = philoSubs;
    QStringList psychoSubs;
    psychoSubs << "认知心理学" << "发展心理学" << "社会心理学" << "临床心理学" << "教育心理学" << "人格心理学";
    subCategoryNames["心理学"] = psychoSubs;
    
    // 生成一级分类
    for (int i = 0; i < rootCategoryNames.size(); ++i) {
        Category rootCategory;
        rootCategory.categoryId = QString("C%1").arg(i + 1, 2, 10, QChar('0'));
        rootCategory.name = rootCategoryNames[i];
        rootCategory.parentId = "";
        
        m_allCategories.append(rootCategory);
        m_categoryMap[rootCategory.categoryId] = rootCategory;
        
        // 生成二级分类
        const QStringList& subs = subCategoryNames[rootCategoryNames[i]];
        for (int j = 0; j < subs.size(); ++j) {
            Category subCategory;
            subCategory.categoryId = QString("C%1%2").arg(i + 1, 2, 10, QChar('0')).arg(j + 1, 2, 10, QChar('0'));
            subCategory.name = subs[j];
            subCategory.parentId = rootCategory.categoryId;
            
            m_allCategories.append(subCategory);
            m_categoryMap[subCategory.categoryId] = subCategory;
        }
    }
    
    // 构建分类树
    m_categoryTree = buildCategoryTree(m_allCategories);
    
    m_isDataLoaded = true;
    
    // 缓存模拟数据
    cacheCategories();
}

Category CategoryManager::buildCategoryTree(const QList<Category>& allCategories, const QString& parentId)
{
    Category result;
    
    // 查找所有父分类为parentId的分类
    QList<Category> children;
    for (const Category& category : allCategories) {
        if (category.parentId == parentId) {
            // 递归构建子分类树
            Category child = buildCategoryTree(allCategories, category.categoryId);
            children.append(child);
        }
    }
    
    if (parentId.isEmpty()) {
        // 根节点
        result.categoryId = "ROOT";
        result.name = "根分类";
        result.parentId = "";
        result.subCategories = children;
    } else {
        // 非根节点，从映射中获取分类信息
        if (m_categoryMap.contains(parentId)) {
            result = m_categoryMap[parentId];
            result.subCategories = children;
        }
    }
    
    return result;
}