#include "../include/MainWindow.h"
#include <QMessageBox>
#include <QScrollArea>
#include <QGridLayout>
#include <QLabel>
#include <QHeaderView>
#include <QDebug>
#include <QGroupBox>
#include <QFormLayout>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // 初始化核心管理器
    m_networkController = new NetworkController(this);
    m_bookManager = new BookManager(this);
    m_categoryManager = new CategoryManager(this);
    m_cartManager = new CartManager(this);
    m_orderManager = new OrderManager(this);
    m_purchaser = new Purchaser(this);

    // 设置窗口属性
    setWindowTitle("图书购买系统");
    setMinimumSize(1024, 768);

    // 设置UI
    setupUI();
    setupMenus();
    setupConnections();

    // 初始状态
    m_isConnected = false;
    updateStatusBar();

    // 尝试连接服务器
    m_networkController->connectToServer();
}

MainWindow::~MainWindow()
{
    // 清理资源
    delete m_purchaser;
    delete m_bookManager;
    delete m_categoryManager;
    delete m_cartManager;
    delete m_orderManager;
    delete m_networkController;
}

void MainWindow::setupUI()
{
    // 主标签页
    m_tabWidget = new QTabWidget(this);

    // 首页
    m_homePage = new QWidget();
    QHBoxLayout *homeLayout = new QHBoxLayout(m_homePage);

    // 左侧分类列表
    m_categoryList = new QListWidget();
    m_categoryList->setMaximumWidth(200);
    m_categoryList->setStyleSheet("background-color: #f5f5f5;");
    homeLayout->addWidget(m_categoryList);

    // 右侧内容区域
    m_homeStack = new QStackedWidget();
    
    // 图书列表页面
    QWidget *bookListContainer = new QWidget();
    QVBoxLayout *bookListLayout = new QVBoxLayout(bookListContainer);

    // 搜索栏
    QHBoxLayout *searchLayout = new QHBoxLayout();
    m_searchEdit = new QLineEdit();
    m_searchEdit->setPlaceholderText("搜索图书...");
    m_searchButton = new QPushButton("搜索");
    QPushButton *refreshButton = new QPushButton("刷新");
    
    searchLayout->addWidget(m_searchEdit);
    searchLayout->addWidget(m_searchButton);
    searchLayout->addWidget(refreshButton);
    
    bookListLayout->addLayout(searchLayout);

    // 图书列表
    m_bookListWidget = new QWidget();
    QScrollArea *scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setWidget(m_bookListWidget);
    
    bookListLayout->addWidget(scrollArea);
    m_homeStack->addWidget(bookListContainer);
    
    homeLayout->addWidget(m_homeStack);

    // 购物车页面
    m_cartPage = new QWidget();
    QVBoxLayout *cartLayout = new QVBoxLayout(m_cartPage);
    
    m_cartTable = new QTableWidget();
    m_cartTable->setColumnCount(6);
    m_cartTable->setHorizontalHeaderLabels({"图书", "作者", "价格", "数量", "小计", "操作"});
    m_cartTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    
    cartLayout->addWidget(m_cartTable);
    
    // 结算按钮
    QHBoxLayout *checkoutLayout = new QHBoxLayout();
    checkoutLayout->addStretch();
    QPushButton *checkoutButton = new QPushButton("结算");
    checkoutButton->setStyleSheet("background-color: #ff2d55; color: white; padding: 10px 20px;");
    checkoutLayout->addWidget(checkoutButton);
    
    cartLayout->addLayout(checkoutLayout);

    // 订单页面
    m_orderPage = new QWidget();
    QVBoxLayout *orderLayout = new QVBoxLayout(m_orderPage);
    
    m_orderTable = new QTableWidget();
    m_orderTable->setColumnCount(6);
    m_orderTable->setHorizontalHeaderLabels({"订单号", "时间", "金额", "状态", "图书数量", "操作"});
    m_orderTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    
    orderLayout->addWidget(m_orderTable);

    // 收藏页面
    m_favoritePage = new QWidget();
    QVBoxLayout *favoriteLayout = new QVBoxLayout(m_favoritePage);
    
    m_favoriteTable = new QTableWidget();
    m_favoriteTable->setColumnCount(5);
    m_favoriteTable->setHorizontalHeaderLabels({"图书", "作者", "价格", "评分", "操作"});
    m_favoriteTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    
    favoriteLayout->addWidget(m_favoriteTable);

    // 个人资料页面
    m_profilePage = new QWidget();
    QVBoxLayout *profileLayout = new QVBoxLayout(m_profilePage);
    
    QGroupBox *userInfoGroup = new QGroupBox("用户信息");
    QFormLayout *userInfoLayout = new QFormLayout(userInfoGroup);
    
    userInfoLayout->addRow("用户名:", new QLabel("未登录"));
    userInfoLayout->addRow("邮箱:", new QLabel("-"));
    userInfoLayout->addRow("手机号:", new QLabel("-"));
    userInfoLayout->addRow("会员等级:", new QLabel("-"));
    
    profileLayout->addWidget(userInfoGroup);

    // 添加标签页
    m_tabWidget->addTab(m_homePage, "首页");
    m_tabWidget->addTab(m_cartPage, "购物车");
    m_tabWidget->addTab(m_orderPage, "订单");
    m_tabWidget->addTab(m_favoritePage, "收藏");
    m_tabWidget->addTab(m_profilePage, "个人中心");

    // 设置中心部件
    setCentralWidget(m_tabWidget);

    // 状态栏
    m_statusBar = statusBar();
    m_statusLabel = new QLabel("未连接");
    m_userLabel = new QLabel("未登录");
    m_statusBar->addWidget(m_statusLabel);
    m_statusBar->addPermanentWidget(m_userLabel);
}

void MainWindow::setupMenus()
{
    // 菜单栏
    QMenuBar *menuBar = this->menuBar();
    
    // 文件菜单
    QMenu *fileMenu = menuBar->addMenu("文件");
    
    QAction *loginAction = new QAction("登录", this);
    QAction *logoutAction = new QAction("退出登录", this);
    QAction *exitAction = new QAction("退出", this);
    
    connect(loginAction, &QAction::triggered, this, &MainWindow::onLogin);
    connect(logoutAction, &QAction::triggered, this, &MainWindow::onLogout);
    connect(exitAction, &QAction::triggered, this, &QMainWindow::close);
    
    fileMenu->addAction(loginAction);
    fileMenu->addAction(logoutAction);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAction);
    
    // 视图菜单
    QMenu *viewMenu = menuBar->addMenu("视图");
    
    QAction *refreshAction = new QAction("刷新图书", this);
    connect(refreshAction, &QAction::triggered, this, &MainWindow::onRefreshBooks);
    
    viewMenu->addAction(refreshAction);
}

void MainWindow::setupConnections()
{
    // 搜索
    connect(m_searchButton, &QPushButton::clicked, this, &MainWindow::onSearch);
    connect(m_searchEdit, &QLineEdit::returnPressed, this, &MainWindow::onSearch);
    
    // 分类选择
    connect(m_categoryList, &QListWidget::itemClicked, this, &MainWindow::onCategoryClicked);
    
    // 标签页切换
    connect(m_tabWidget, &QTabWidget::currentChanged, this, &MainWindow::onTabChanged);
    
    // 网络状态
    connect(m_networkController, &NetworkController::connected, this, [this]() {
        m_isConnected = true;
        updateStatusBar();
    });
    
    connect(m_networkController, &NetworkController::disconnected, this, [this]() {
        m_isConnected = false;
        updateStatusBar();
    });
}

void MainWindow::loadBooks()
{
    QList<Book> books;
    
    // 根据当前分类加载图书
    if (!m_currentCategory.isEmpty()) {
        books = m_bookManager->getBooksByCategory(m_currentCategory);
    } else {
        books = m_bookManager->getAllBooks();
    }
    
    // 清空图书列表
    QLayout *oldLayout = m_bookListWidget->layout();
    if (oldLayout) {
        delete oldLayout;
    }
    
    // 创建网格布局
    QGridLayout *bookLayout = new QGridLayout(m_bookListWidget);
    bookLayout->setSpacing(20);
    
    // 添加图书项
    int row = 0;
    int col = 0;
    const int maxCols = 4;
    
    for (const Book &book : books) {
        BookItemWidget *bookItem = new BookItemWidget(book, m_purchaser, this);
        connect(bookItem, &BookItemWidget::bookClicked, this, &MainWindow::onBookClicked);
        connect(bookItem, &BookItemWidget::addToCart, this, &MainWindow::onAddToCart);
        
        bookLayout->addWidget(bookItem, row, col);
        
        col++;
        if (col >= maxCols) {
            col = 0;
            row++;
        }
    }
}

void MainWindow::loadCategories()
{
    m_categoryList->clear();
    
    // 添加"全部"分类
    QListWidgetItem *allItem = new QListWidgetItem("全部图书");
    allItem->setData(Qt::UserRole, "");
    m_categoryList->addItem(allItem);
    
    // 添加分类
    QList<Category> categories = m_categoryManager->getRootCategories();
    for (const Category &category : categories) {
        QListWidgetItem *item = new QListWidgetItem(category.name);
        item->setData(Qt::UserRole, category.categoryId);
        m_categoryList->addItem(item);
    }
    
    // 默认选择"全部"
    m_categoryList->setCurrentItem(allItem);
}

void MainWindow::loadCart()
{
    if (!m_currentUser.userId) {
        m_cartTable->setRowCount(0);
        return;
    }
    
    ShoppingCart cart  = m_purchaser->ViewShoppingCart(m_currentUser.userId);
       m_cartTable->setRowCount(cart.items.size());
    
    double total = 0;
    
    for (int i = 0; i < cart.items.size(); ++i) {
        const CartItem &item = cart.items[i];
        double subtotal = item.price * item.quantity;
        total += subtotal;
        // 获取图书信息
        Book book = m_bookManager->getBookById(item.bookId);
        
        m_cartTable->setItem(i, 0, new QTableWidgetItem(book.title));
        m_cartTable->setItem(i, 1, new QTableWidgetItem(book.author));
        m_cartTable->setItem(i, 2, new QTableWidgetItem(QString("¥%1").arg(item.price)));
        m_cartTable->setItem(i, 3, new QTableWidgetItem(QString::number(item.quantity)));
        m_cartTable->setItem(i, 4, new QTableWidgetItem(QString("¥%1").arg(subtotal)));
        
        QPushButton *removeButton = new QPushButton("删除");
        connect(removeButton, &QPushButton::clicked, [this, item]() {
            m_purchaser->RemoveFromCart(item.bookId);
            loadCart();
        });
        
        m_cartTable->setCellWidget(i, 5, removeButton);
    }
    
    // 更新状态栏显示总价
    m_statusBar->showMessage(QString("购物车总价: ¥%1").arg(total));
}

void MainWindow::loadOrders()
{
    if (!m_currentUser.userId) {
        m_orderTable->setRowCount(0);
        return;
    }
    
    QList<Order> orders = m_purchaser->ViewMyOrder(m_currentUser.userId);
    m_orderTable->setRowCount(orders.size());
    
    for (int i = 0; i < orders.size(); ++i) {
        const Order &order = orders[i];
        
        m_orderTable->setItem(i, 0, new QTableWidgetItem(order.orderId));
        m_orderTable->setItem(i, 1, new QTableWidgetItem(order.createTime.toString("yyyy-MM-dd HH:mm:ss")));
        m_orderTable->setItem(i, 2, new QTableWidgetItem(QString("¥%1").arg(order.totalAmount)));
        m_orderTable->setItem(i, 3, new QTableWidgetItem(order.status));
        m_orderTable->setItem(i, 4, new QTableWidgetItem(QString::number(order.items.size())));
        
        QPushButton *detailButton = new QPushButton("查看详情");
        connect(detailButton, &QPushButton::clicked, [order]() {
            // 显示订单详情
            QString detail = QString("订单号: %1\n时间: %2\n金额: ¥%3\n状态: %4\n\n图书列表:\n").arg(
                order.orderId,
                order.createTime.toString("yyyy-MM-dd HH:mm:ss"),
                QString::number(order.totalAmount),
                order.status
            );
            
            for (const OrderItem &item : order.items) {
                detail += QString("- %1 (¥%2 x %3)\n").arg(item.title, QString::number(item.price), QString::number(item.quantity));
            }
            
            QMessageBox::information(nullptr, "订单详情", detail);
        });
        
        m_orderTable->setCellWidget(i, 5, detailButton);
    }
}

void MainWindow::loadFavorites()
{
    if (!m_currentUser.userId) {
        m_favoriteTable->setRowCount(0);
        return;

    
    QList<Book> favorites = m_currentUser.favorites;
    m_favoriteTable->setRowCount(favorites.size());
    
    for (int i = 0; i < favorites.size(); ++i) {
        const Book &book = favorites[i];
        
        m_favoriteTable->setItem(i, 0, new QTableWidgetItem(book.title));
        m_favoriteTable->setItem(i, 1, new QTableWidgetItem(book.author));
        m_favoriteTable->setItem(i, 2, new QTableWidgetItem(QString("¥%1").arg(book.price)));
        m_favoriteTable->setItem(i, 3, new QTableWidgetItem(QString::number(book.score)));
        
        QPushButton *addButton = new QPushButton("加入购物车");
        connect(addButton, &QPushButton::clicked, [this, book]() {
            m_purchaser->AddToCart(book.bookId, 1);
            QMessageBox::information(this, "成功", "已添加到购物车");
        });
        
        m_favoriteTable->setCellWidget(i, 4, addButton);
    }
}
}
void MainWindow::updateStatusBar()
{
    m_statusLabel->setText(m_isConnected ? "已连接" : "未连接");
    m_statusLabel->setStyleSheet(m_isConnected ? "color: green;" : "color: red;");
    
    if (m_currentUser.userId) {
        m_userLabel->setText(QString("用户: %1 (等级: %2)").arg(m_currentUser.username).arg(m_currentUser.membershipLevel));
    } else {
        m_userLabel->setText("未登录");
    }
}

void MainWindow::onLogin()
{
    LoginDialog loginDialog(m_purchaser, this);
    if (loginDialog.exec() == QDialog::Accepted) {
        m_currentUser = m_purchaser->getCurrentUser();
        updateStatusBar();
        loadCart();
        loadOrders();
        loadFavorites();
        QMessageBox::information(this, "登录成功", "欢迎回来，" + m_currentUser.username);
    }
}

void MainWindow::onLogout()
{
    // 清除用户登录状态
    m_currentUser = User();
    m_currentUser = User();
    updateStatusBar();
    loadCart();
    loadOrders();
    loadFavorites();
    QMessageBox::information(this, "退出成功", "已退出登录");
}

void MainWindow::onSearch()
{
    QString keyword = m_searchEdit->text();
    if (keyword.isEmpty()) {
        loadBooks();
        return;
    }
    
    QList<Book> books = m_bookManager->searchBooks(keyword);
    
    // 清空图书列表
    QLayout *oldLayout = m_bookListWidget->layout();
    if (oldLayout) {
        delete oldLayout;
    }
    
    // 创建网格布局
    QGridLayout *bookLayout = new QGridLayout(m_bookListWidget);
    bookLayout->setSpacing(20);
    
    // 添加搜索结果
    int row = 0;
    int col = 0;
    const int maxCols = 4;
    
    for (const Book &book : books) {
        BookItemWidget *bookItem = new BookItemWidget(book, m_purchaser, this);
        connect(bookItem, &BookItemWidget::bookClicked, this, &MainWindow::onBookClicked);
        connect(bookItem, &BookItemWidget::addToCart, this, &MainWindow::onAddToCart);
        
        bookLayout->addWidget(bookItem, row, col);
        
        col++;
        if (col >= maxCols) {
            col = 0;
            row++;
        }
    }
}

void MainWindow::onCategoryClicked(QListWidgetItem *item)
{
    m_currentCategory = item->data(Qt::UserRole).toString();
    loadBooks();
}

void MainWindow::onBookClicked(const Book &book)
{
    BookDetailWidget *detailWidget = new BookDetailWidget(book, m_purchaser, this);
    connect(detailWidget, &BookDetailWidget::backToHome, this, &MainWindow::onBackToHome);
    connect(detailWidget, &BookDetailWidget::addToCart, this, &MainWindow::onAddToCart);
    connect(detailWidget, &BookDetailWidget::buyNow, this, &MainWindow::onBuyNow);
    
    m_homeStack->addWidget(detailWidget);
    m_homeStack->setCurrentWidget(detailWidget);
}

void MainWindow::onAddToCart(const QString &bookId)
{
    if (!m_currentUser.userId) {
        QMessageBox::warning(this, "请先登录", "请先登录后再添加商品到购物车");
        onLogin();
        return;
    }
    
    if (m_purchaser->AddToCart(bookId, 1)) {
        QMessageBox::information(this, "成功", "已添加到购物车");
    } else {
        QMessageBox::warning(this, "失败", "添加到购物车失败");
    }
}

void MainWindow::onViewDetail(const QString &bookId)
{
    Book book = m_bookManager->getBookById(bookId);
    if (!book.bookId.isEmpty()) {
        onBookClicked(book);
    }
}

void MainWindow::onBackToHome()
{
    m_homeStack->setCurrentIndex(0);
}

void MainWindow::onBuyNow(const QString &bookId, int quantity)
{
    if (!m_currentUser.userId) {
        QMessageBox::warning(this, "请先登录", "请先登录后再购买商品");
        onLogin();
        return;
    }
    
    Order order = m_purchaser->CheckoutByBook(bookId, quantity, "", m_currentUser.membershipLevel);
    if (!order.orderId.isEmpty()) {
        QMessageBox::information(this, "成功", "订单创建成功，订单号: " + order.orderId);
        loadOrders();
        m_tabWidget->setCurrentIndex(2); // 切换到订单页面
    } else {
        QMessageBox::warning(this, "失败", "订单创建失败");
    }
}

void MainWindow::onTabChanged(int index)
{
    switch (index) {
    case 0: // 首页
        loadCategories();
        loadBooks();
        break;
    case 1: // 购物车
        loadCart();
        break;
    case 2: // 订单
        loadOrders();
        break;
    case 3: // 收藏
        loadFavorites();
        break;
    }
}

void MainWindow::onRefreshBooks()
{
    m_bookManager->loadBooks();
    loadBooks();
    QMessageBox::information(this, "刷新成功", "图书列表已更新");
}

void MainWindow::onNetworkStatusChanged(bool connected)
{
    m_isConnected = connected;
    updateStatusBar();
}
