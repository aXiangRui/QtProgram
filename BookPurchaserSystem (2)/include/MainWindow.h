#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTabWidget>
#include <QWidget>
#include <QListWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QStackedWidget>
#include <QMenuBar>
#include <QStatusBar>
#include <QToolBar>
#include <QAction>
#include "Purchaser.h"
#include "BookManager.h"
#include "CategoryManager.h"
#include "CartManager.h"
#include "OrderManager.h"
#include "NetworkController.h"
#include "LoginDialog.h"
#include "BookItemWidget.h"
#include "BookDetailWidget.h"

// 主窗口类
class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onLogin();
    void onLogout();
    void onSearch();
    void onCategoryClicked(QListWidgetItem *item);
    void onBookClicked(const Book &book);
    void onAddToCart(const QString &bookId);
    void onViewDetail(const QString &bookId);
    void onBackToHome();
    void onBuyNow(const QString &bookId, int quantity);
    void onTabChanged(int index);
    void onRefreshBooks();
    void onNetworkStatusChanged(bool connected);

private:
    void setupUI();
    void setupMenus();
    void setupConnections();
    void loadBooks();
    void loadCategories();
    void loadCart();
    void loadOrders();
    void loadFavorites();
    void updateStatusBar();

    // 核心管理器
    Purchaser *m_purchaser;
    BookManager *m_bookManager;
    CategoryManager *m_categoryManager;
    CartManager *m_cartManager;
    OrderManager *m_orderManager;
    NetworkController *m_networkController;

    // UI组件
    QTabWidget *m_tabWidget;
    QWidget *m_homePage;
    QWidget *m_cartPage;
    QWidget *m_orderPage;
    QWidget *m_favoritePage;
    QWidget *m_profilePage;
    QStackedWidget *m_homeStack;
    QListWidget *m_categoryList;
    QLineEdit *m_searchEdit;
    QPushButton *m_searchButton;
    QWidget *m_bookListWidget;
    QTableWidget *m_cartTable;
    QTableWidget *m_orderTable;
    QTableWidget *m_favoriteTable;
    QStatusBar *m_statusBar;
    QLabel *m_statusLabel;
    QLabel *m_userLabel;

    // 当前状态
    User m_currentUser;
    QString m_currentCategory;
    bool m_isConnected;
};

#endif // MAINWINDOW_H