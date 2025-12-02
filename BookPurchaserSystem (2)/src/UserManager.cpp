#include "../include/UserManager.h"
#include <QDebug>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QCryptographicHash>

UserManager::UserManager(QObject *parent)
    : QObject(parent), m_isDataLoaded(false)
{
    // 加载用户数据
    loadUsers();
}

UserManager::~UserManager()
{
}

User UserManager::getUserById(int userId)
{
    if (!m_isDataLoaded) {
        loadUsers();
    }
    
    if (m_userMap.contains(userId)) {
        return m_userMap[userId];
    }
    
    // 返回空用户对象
    return User();
}

User UserManager::getUserByUsername(const QString& username)
{
    if (!m_isDataLoaded) {
        loadUsers();
    }
    
    if (m_usernameToIdMap.contains(username)) {
        int userId = m_usernameToIdMap[username];
        return getUserById(userId);
    }
    
    // 返回空用户对象
    return User();
}

bool UserManager::registerUser(const QString& username, const QString& password)
{
    if (!m_isDataLoaded) {
        loadUsers();
    }
    
    // 检查用户名是否已存在
    if (m_usernameToIdMap.contains(username)) {
        return false;
    }
    
    // 创建新用户
    User newUser;
    newUser.userId = m_users.size() + 1;
    newUser.username = username;
    // 加密密码
    QByteArray passwordHash = QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256);
    newUser.password = QString(passwordHash.toHex());
    newUser.membershipLevel = 1; // 初始会员等级
    
    // 添加到列表和映射
    m_users.append(newUser);
    m_userMap[newUser.userId] = newUser;
    m_usernameToIdMap[username] = newUser.userId;
    
    // 更新缓存
    cacheUsers();
    
    return true;
}

bool UserManager::loginUser(const QString& username, const QString& password, User& user)
{
    if (!m_isDataLoaded) {
        loadUsers();
    }
    
    if (!m_usernameToIdMap.contains(username)) {
        return false;
    }
    
    int userId = m_usernameToIdMap[username];
    User storedUser = m_userMap[userId];
    
    // 验证密码
    QByteArray passwordHash = QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256);
    QString hashedPassword = QString(passwordHash.toHex());
    
    if (storedUser.password == hashedPassword) {
        user = storedUser;
        return true;
    }
    
    return false;
}

bool UserManager::updateUser(const User& user)
{
    if (!m_isDataLoaded) {
        loadUsers();
    }
    
    if (m_userMap.contains(user.userId)) {
        // 更新用户信息
        m_userMap[user.userId] = user;
        
        // 更新列表中的用户
        for (int i = 0; i < m_users.size(); ++i) {
            if (m_users[i].userId == user.userId) {
                m_users[i] = user;
                break;
            }
        }
        
        // 更新缓存
        cacheUsers();
        
        return true;
    }
    
    return false;
}

bool UserManager::updateUserPreferences(int userId, const QMap<QString, double>& preferences)
{
    if (!m_isDataLoaded) {
        loadUsers();
    }
    
    if (m_userMap.contains(userId)) {
        User user = m_userMap[userId];
        user.preferences = preferences;
        return updateUser(user);
    }
    
    return false;
}

int UserManager::getMembershipLevel(int userId)
{
    if (!m_isDataLoaded) {
        loadUsers();
    }
    
    if (m_userMap.contains(userId)) {
        return m_userMap[userId].membershipLevel;
    }
    
    return 0;
}

bool UserManager::upgradeMembershipLevel(int userId, int newLevel)
{
    if (!m_isDataLoaded) {
        loadUsers();
    }
    
    if (m_userMap.contains(userId)) {
        User user = m_userMap[userId];
        if (newLevel > user.membershipLevel) {
            user.membershipLevel = newLevel;
            return updateUser(user);
        }
    }
    
    return false;
}

void UserManager::loadUsers()
{
    // 尝试从缓存加载
    loadUsersFromCache();
    
    // 如果缓存不存在或加载失败，生成模拟数据
    if (!m_isDataLoaded) {
        generateMockUsers();
    }
}

void UserManager::loadUsersFromCache()
{
    QFile file(":/cache/users.json");
    if (!file.exists() || !file.open(QIODevice::ReadOnly)) {
        return;
    }
    
    QByteArray data = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull()) {
        return;
    }
    
    QJsonArray usersArray = doc.array();
    for (const QJsonValue& value : usersArray) {
        QJsonObject obj = value.toObject();
        User user;
        user.userId = obj["userId"].toInt();
        user.username = obj["username"].toString();
        user.password = obj["password"].toString();
        user.email = obj["email"].toString();
        user.phone = obj["phone"].toString();
        user.membershipLevel = obj["membershipLevel"].toInt();
        
        // 加载偏好
        QJsonObject preferencesObj = obj["preferences"].toObject();
        for (auto it = preferencesObj.begin(); it != preferencesObj.end(); ++it) {
            user.preferences[it.key()] = it.value().toDouble();
        }
        
        m_users.append(user);
        m_userMap[user.userId] = user;
        m_usernameToIdMap[user.username] = user.userId;
    }
    
    m_isDataLoaded = true;
}

void UserManager::loadUsersFromServer()
{
    // 这里应该从服务器加载用户数据
    // 由于是模拟环境，我们使用模拟数据
    generateMockUsers();
}

void UserManager::cacheUsers()
{
    // 将用户数据缓存到本地
    QJsonArray usersArray;
    for (const User& user : m_users) {
        QJsonObject obj;
        obj["userId"] = user.userId;
        obj["username"] = user.username;
        obj["password"] = user.password;
        obj["email"] = user.email;
        obj["phone"] = user.phone;
        obj["membershipLevel"] = user.membershipLevel;
        
        // 保存偏好
        QJsonObject preferencesObj;
        for (auto it = user.preferences.begin(); it != user.preferences.end(); ++it) {
            preferencesObj[it.key()] = it.value();
        }
        obj["preferences"] = preferencesObj;
        
        usersArray.append(obj);
    }
    
    QJsonDocument doc(usersArray);
    QFile file(":/cache/users.json");
    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson());
        file.close();
    }
}

void UserManager::generateMockUsers()
{
    // 生成模拟用户数据
    QStringList usernames = {"user1", "user2", "user3", "user4", "user5"};
    QStringList categories = {"文学", "科技", "教育", "艺术", "历史", "经济", "哲学", "心理学"};
    
    for (int i = 1; i <= 5; ++i) {
        User user;
        user.userId = i;
        user.username = usernames[i-1];
        
        // 生成密码哈希
        QByteArray passwordHash = QCryptographicHash::hash(QString("password%1").arg(i).toUtf8(), QCryptographicHash::Sha256);
        user.password = QString(passwordHash.toHex());
        
        user.email = QString("user%1@example.com").arg(i);
        user.phone = QString("1380000%1").arg(i, 4, 10, QChar('0'));
        user.membershipLevel = i % 5 + 1;
        
        // 生成偏好
        for (int j = 0; j < categories.size(); ++j) {
            double weight = (i + j) % 10 * 0.1;
            if (weight > 0) {
                user.preferences[categories[j]] = weight;
            }
        }
        
        m_users.append(user);
        m_userMap[user.userId] = user;
        m_usernameToIdMap[user.username] = user.userId;
    }
    
    m_isDataLoaded = true;
    
    // 缓存模拟数据
    cacheUsers();
}