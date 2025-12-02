#ifndef USERMANAGER_H
#define USERMANAGER_H

#include <QObject>
#include <QString>
#include <QList>
#include <QMap>
#include "Purchaser.h"

// 用户管理器类，负责用户数据的加载、缓存和更新
class UserManager : public QObject
{
    Q_OBJECT
public:
    UserManager(QObject *parent = nullptr);
    ~UserManager();

    // 根据ID获取用户
    User getUserById(int userId);
    
    // 根据用户名获取用户
    User getUserByUsername(const QString& username);
    
    // 注册新用户
    bool registerUser(const QString& username, const QString& password);
    
    // 用户登录验证
    bool loginUser(const QString& username, const QString& password, User& user);
    
    // 更新用户信息
    bool updateUser(const User& user);
    
    // 更新用户偏好
    bool updateUserPreferences(int userId, const QMap<QString, double>& preferences);
    
    // 获取用户会员等级
    int getMembershipLevel(int userId);
    
    // 升级用户会员等级
    bool upgradeMembershipLevel(int userId, int newLevel);
    
    // 加载用户数据
    void loadUsers();
    
    // 缓存用户数据
    void cacheUsers();

private:
    // 从本地缓存加载用户数据
    void loadUsersFromCache();
    
    // 从服务器加载用户数据
    void loadUsersFromServer();
    
    // 模拟用户数据（用于测试）
    void generateMockUsers();

private:
    QList<User> m_users;
    QMap<int, User> m_userMap; // 用于快速查找
    QMap<QString, int> m_usernameToIdMap; // 用户名到ID的映射
    bool m_isDataLoaded;
};

#endif // USERMANAGER_H