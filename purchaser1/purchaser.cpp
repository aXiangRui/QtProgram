#include "purchaser.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFormLayout>
#include <QMessageBox>
#include <QHeaderView>
#include <QDateTime>
#include <QScrollArea>
#include <QCheckBox>
#include <QTabWidget>
#include <algorithm>
#include <functional>
#include <QGroupBox>
#include <QComboBox>

Purchaser::Purchaser(QWidget *parent)
    : QMainWindow(parent),
      currentUser(nullptr),    // 先初始化这个
      userManager(),           // 然后这个
      categoryRoot(nullptr),   // 最后这个
      isLoggedIn(false)
{
    initData();
    initUI();
    initConnections();
    showLoginPage();
}

Purchaser::~Purchaser()
{
    delete categoryRoot;
}

void Purchaser::initData()
{
    // 初始化类别树
    categoryRoot = new CategoryNode("root", "所有分类");

    // 添加示例分类
    CategoryNode *fiction = new CategoryNode("fiction", "小说", categoryRoot);
    CategoryNode *tech = new CategoryNode("tech", "科技", categoryRoot);
    CategoryNode *edu = new CategoryNode("edu", "教育", categoryRoot);

    categoryRoot->addChild(fiction);
    categoryRoot->addChild(tech);
    categoryRoot->addChild(edu);

    // 小说子分类
    fiction->addChild(new CategoryNode("fiction_novel", "长篇小说", fiction));
    fiction->addChild(new CategoryNode("fiction_short", "短篇小说", fiction));

    // 科技子分类
    tech->addChild(new CategoryNode("tech_computer", "计算机", tech));
    tech->addChild(new CategoryNode("tech_physics", "物理学", tech));

    // 添加示例图书
    allBooks.append(Book("001", "深入理解计算机系统", "tech", "tech_computer", 99.0, 4.8, 1000, 95));
    allBooks.append(Book("002", "C++ Primer", "tech", "tech_computer", 128.0, 4.9, 800, 90));
    allBooks.append(Book("003", "三体", "fiction", "fiction_novel", 48.0, 4.7, 1500, 88));
    allBooks.append(Book("004", "活着", "fiction", "fiction_novel", 35.0, 4.8, 1200, 85));
    allBooks.append(Book("005", "时间简史", "tech", "tech_physics", 45.0, 4.6, 600, 80));
    allBooks.append(Book("006", "红楼梦", "fiction", "fiction_novel", 68.0, 4.9, 2000, 92));
    allBooks.append(Book("007", "Python编程", "tech", "tech_computer", 79.0, 4.7, 1500, 87));
    allBooks.append(Book("008", "百年孤独", "fiction", "fiction_novel", 39.0, 4.8, 1800, 89));
    allBooks.append(Book("009", "算法导论", "tech", "tech_computer", 118.0, 4.8, 1200, 86));
    allBooks.append(Book("010", "小王子", "fiction", "fiction_short", 25.0, 4.9, 2500, 94));

    // 添加到bookMap
    for (const auto &book : allBooks) {
        bookMap[book.getId()] = book;
    }

    // 为分类添加图书
    // 计算机分类
    CategoryNode *computerCat = nullptr;
    for (auto child : tech->getChildren()) {
        if (child->getId() == "tech_computer") {
            computerCat = child;
            break;
        }
    }

    if (computerCat) {
        computerCat->addBook("001");
        computerCat->addBook("002");
        computerCat->addBook("007");
        computerCat->addBook("009");
    }

    // 物理分类
    CategoryNode *physicsCat = nullptr;
    for (auto child : tech->getChildren()) {
        if (child->getId() == "tech_physics") {
            physicsCat = child;
            break;
        }
    }

    if (physicsCat) {
        physicsCat->addBook("005");
    }

    // 长篇小说分类
    CategoryNode *novelCat = nullptr;
    for (auto child : fiction->getChildren()) {
        if (child->getId() == "fiction_novel") {
            novelCat = child;
            break;
        }
    }

    if (novelCat) {
        novelCat->addBook("003");
        novelCat->addBook("004");
        novelCat->addBook("006");
        novelCat->addBook("008");
    }

    // 短篇小说分类
    CategoryNode *shortCat = nullptr;
    for (auto child : fiction->getChildren()) {
        if (child->getId() == "fiction_short") {
            shortCat = child;
            break;
        }
    }

    if (shortCat) {
        shortCat->addBook("010");
    }
}

void Purchaser::initUI()
{
    setWindowTitle("图书购买系统 - 买家模块");
    setMinimumSize(1200, 800);

    // 创建堆叠窗口
    stackedWidget = new QStackedWidget(this);
    this->setStyleSheet("background-color:#f5f7fa;");
    setCentralWidget(stackedWidget);

    // 1. 登录页面
    loginPage = new QWidget();
    QVBoxLayout *loginLayout = new QVBoxLayout(loginPage);
    loginPage->setWindowFlags(Qt::FramelessWindowHint);

    QLabel *loginTitle = new QLabel("图书购买系统");
    loginTitle->setAlignment(Qt::AlignCenter);
    QFont titleFont = loginTitle->font();
    titleFont.setFamily("Microsoft YaHei");
    titleFont.setBold(1000);
    titleFont.setPointSize(24);
    loginTitle->setFont(titleFont);

    QFont textFont;
    textFont.setFamily("Microsoft YaHei Light");
    textFont.setPointSize(12);

    QWidget *loginForm = new QWidget();
    QFormLayout *formLayout = new QFormLayout(loginForm);

    loginUsername = new QLineEdit();
    loginUsername->setPlaceholderText("请输入用户名");
    loginUsername->setMaximumWidth(500);
    loginUsername->setMinimumHeight(70);
    loginUsername->setMinimumWidth(450);
    loginUsername->setMinimumHeight(60);
    loginUsername->setStyleSheet("background-color: #ffffff;");
    loginUsername->setFont(textFont);

    loginPassword = new QLineEdit();
    loginPassword->setPlaceholderText("请输入密码");
    loginPassword->setEchoMode(QLineEdit::Password);
    loginPassword->setMaximumWidth(500);
    loginPassword->setMinimumHeight(50);
    loginPassword->setMinimumWidth(450);
    loginPassword->setMinimumHeight(60);
    loginPassword->setStyleSheet("background-color: #ffffff;");
    loginPassword->setFont(textFont);

    QLabel *Username01 = new QLabel("用户名:");
    QLabel *Password01 = new QLabel("密码:");
    Username01->setFont(textFont);
    Password01->setFont(textFont);

    formLayout->addRow(Username01, loginUsername);
    formLayout->addRow(Password01, loginPassword);

    // 添加状态标签
    QLabel *loginStatusLabel = new QLabel();
    loginStatusLabel->setStyleSheet("color: red;");
    loginStatusLabel->setAlignment(Qt::AlignCenter);
    loginStatusLabel->setObjectName("loginStatusLabel");

    QWidget *buttonWidget = new QWidget();
    QHBoxLayout *buttonLayout = new QHBoxLayout(buttonWidget);

    loginButton = new QPushButton("登录");
    registerButton = new QPushButton("注册");

    loginButton->setMinimumSize(200, 50);
    registerButton->setMinimumSize(200, 50);
    loginButton->setStyleSheet(R"(
       QPushButton {
           height: 80px;
           width: 300px;
           border: none; /* 浅灰边框，增强轮廓 */
           border-radius: 40px;
           font-size: 30px;
           font-family: "Microsoft YaHei";
           font-weight: 600;
           color: white; /* 蓝色文字，与获取按钮呼应 */
           background-color: #2f80ed; /* 白色底色，匹配样例图 */
       }
       QPushButton:hover {
           background-color: #2b75e6; /* 轻微灰色，提升交互感 */
       }
       QPushButton:disabled {
           color: #b3d4fc;
           background-color: white;
       })");
    registerButton->setStyleSheet(R"(
      QPushButton {
          height: 80px;
          width: 300px;
          border: none; /* 浅灰边框，增强轮廓 */
          border-radius: 40px;
          font-size: 30px;
          font-family: "Microsoft YaHei";
          font-weight: 600;
          color: white; /* 蓝色文字，与获取按钮呼应 */
          background-color: #2f80ed; /* 白色底色，匹配样例图 */
      }
      QPushButton:hover {
          background-color: #2b75e6; /* 轻微灰色，提升交互感 */
      }
      QPushButton:disabled {
          color: #b3d4fc;
          background-color: white;
      })");

    buttonLayout->addStretch();
    buttonLayout->addWidget(loginButton);
    buttonLayout->addWidget(registerButton);
    buttonLayout->addStretch();

    loginLayout->addStretch();
    loginLayout->addWidget(loginTitle);
    loginLayout->addStretch();
    loginLayout->addWidget(loginForm, 0, Qt::AlignCenter);
    loginLayout->addWidget(loginStatusLabel);
    loginLayout->addWidget(buttonWidget);
    loginLayout->addStretch();

    stackedWidget->addWidget(loginPage);

    // 2. 注册页面
    registerPage = new QWidget();
    QVBoxLayout *registerLayout = new QVBoxLayout(registerPage);

    QLabel *registerTitle = new QLabel("用户注册");
    registerTitle->setAlignment(Qt::AlignCenter);
    registerTitle->setFont(titleFont);

    QWidget *registerForm = new QWidget();
    QFormLayout *regFormLayout = new QFormLayout(registerForm);

    regUsername = new QLineEdit();
    regUsername->setPlaceholderText("请输入用户名（3-20位字符）");
    regUsername->setMaximumWidth(500);
    regUsername->setMinimumWidth(450);
    regUsername->setMinimumHeight(60);
    regUsername->setStyleSheet("background-color: #ffffff");
    regUsername->setFont(textFont);

    regPassword = new QLineEdit();
    regPassword->setPlaceholderText("请输入密码（至少6位）");
    regPassword->setEchoMode(QLineEdit::Password);
    regPassword->setMaximumWidth(500);
    regPassword->setMinimumWidth(450);
    regPassword->setMinimumHeight(60);
    regPassword->setStyleSheet("background-color: #ffffff");
    regPassword->setFont(textFont);

    regConfirmPassword = new QLineEdit();
    regConfirmPassword->setPlaceholderText("请确认密码");
    regConfirmPassword->setEchoMode(QLineEdit::Password);
    regConfirmPassword->setMaximumWidth(500);
    regConfirmPassword->setMinimumWidth(450);
    regConfirmPassword->setMinimumHeight(60);
    regConfirmPassword->setStyleSheet("background-color: #ffffff");
    regConfirmPassword->setFont(textFont);

    QLabel *username02 = new QLabel("用户名:");
    QLabel *password02 = new QLabel("密码:");
    QLabel *confirmpassword = new QLabel("确认密码:");
    username02->setFont(textFont);
    password02->setFont(textFont);
    confirmpassword->setFont(textFont);

    regFormLayout->addRow(username02, regUsername);
    regFormLayout->addRow(password02, regPassword);
    regFormLayout->addRow(confirmpassword,regConfirmPassword);

    QWidget *regButtonWidget = new QWidget();
    QHBoxLayout *regButtonLayout = new QHBoxLayout(regButtonWidget);

    QPushButton *confirmRegisterBtn = new QPushButton("确认注册");
    QPushButton *backToLoginBtn = new QPushButton("返回登录");

    confirmRegisterBtn->setMinimumSize(100, 40);
    confirmRegisterBtn->setStyleSheet(R"(
      QPushButton {
          height: 80px;
          width: 300px;
          border: none; /* 浅灰边框，增强轮廓 */
          border-radius: 40px;
          font-size: 30px;
          font-family: "Microsoft YaHei";
          font-weight: 600;
          color: white; /* 蓝色文字，与获取按钮呼应 */
          background-color: #2f80ed; /* 白色底色，匹配样例图 */
      }
      QPushButton:hover {
          background-color: #2b75e6; /* 轻微灰色，提升交互感 */
      }
      QPushButton:disabled {
          color: #b3d4fc;
          background-color: white;
      })");
    backToLoginBtn->setMinimumSize(100, 40);
    backToLoginBtn->setStyleSheet(R"(
      QPushButton {
          height: 80px;
          width: 300px;
          border: none; /* 浅灰边框，增强轮廓 */
          border-radius: 40px;
          font-size: 30px;
          font-family: "Microsoft YaHei";
          font-weight: 600;
          color: white; /* 蓝色文字，与获取按钮呼应 */
          background-color: #2f80ed; /* 白色底色，匹配样例图 */
      }
      QPushButton:hover {
          background-color: #2b75e6; /* 轻微灰色，提升交互感 */
      }
      QPushButton:disabled {
          color: #b3d4fc;
          background-color: white;
      })");

    regButtonLayout->addStretch();
    regButtonLayout->addWidget(confirmRegisterBtn);
    regButtonLayout->addWidget(backToLoginBtn);
    regButtonLayout->addStretch();

    registerLayout->addStretch();
    registerLayout->addWidget(registerTitle);
    registerLayout->addStretch();
    registerLayout->addWidget(registerForm, 0, Qt::AlignCenter);
    registerLayout->addWidget(regButtonWidget);
    registerLayout->addStretch();

    stackedWidget->addWidget(registerPage);

    // 3. 主页面
    mainPage = new QWidget();
    QVBoxLayout *mainLayout = new QVBoxLayout(mainPage);

    // 顶部工具栏
    QWidget *toolbar = new QWidget();
    QHBoxLayout *toolbarLayout = new QHBoxLayout(toolbar);

    searchInput = new QLineEdit();
    searchInput->setPlaceholderText("搜索图书...");
    searchInput->setMinimumWidth(300);

    searchButton = new QPushButton("搜索");
    cartButton = new QPushButton("购物车");
    ordersButton = new QPushButton("我的订单");
    profileButton = new QPushButton("个人中心");
    serviceButton = new QPushButton("客服");
    logoutButton = new QPushButton("退出登录");

    toolbarLayout->addWidget(searchInput);
    toolbarLayout->addWidget(searchButton);
    toolbarLayout->addStretch();
    toolbarLayout->addWidget(cartButton);
    toolbarLayout->addWidget(ordersButton);
    toolbarLayout->addWidget(profileButton);
    toolbarLayout->addWidget(serviceButton);
    toolbarLayout->addWidget(logoutButton);

    // 主体内容区
    QWidget *contentWidget = new QWidget();
    QHBoxLayout *contentLayout = new QHBoxLayout(contentWidget);

    // 左侧分类树
    QWidget *leftPanel = new QWidget();
    leftPanel->setMaximumWidth(250);
    QVBoxLayout *leftLayout = new QVBoxLayout(leftPanel);

    QLabel *categoryLabel = new QLabel("分类导航");
    categoryTree = new QTreeWidget();
    categoryTree->setHeaderHidden(true);

    // 加载分类树
    loadCategories();

    leftLayout->addWidget(categoryLabel);
    leftLayout->addWidget(categoryTree);

    // 右侧图书列表
    QWidget *rightPanel = new QWidget();
    QVBoxLayout *rightLayout = new QVBoxLayout(rightPanel);

    QLabel *recommendLabel = new QLabel("热门推荐");
    recommendList = new QListWidget();
    recommendList->setViewMode(QListWidget::IconMode);
    recommendList->setIconSize(QSize(100, 150));
    recommendList->setResizeMode(QListWidget::Adjust);
    recommendList->setSpacing(10);

    // 加载推荐
    updateRecommendations();

    rightLayout->addWidget(recommendLabel);
    rightLayout->addWidget(recommendList);

    contentLayout->addWidget(leftPanel);
    contentLayout->addWidget(rightPanel, 1);

    mainLayout->addWidget(toolbar);
    mainLayout->addWidget(contentWidget, 1);

    stackedWidget->addWidget(mainPage);

    // 4. 图书详情页面
    bookDetailPage = new QWidget();
    QVBoxLayout *detailLayout = new QVBoxLayout(bookDetailPage);

    QWidget *detailHeader = new QWidget();
    QHBoxLayout *headerLayout = new QHBoxLayout(detailHeader);

    backToMainBtn = new QPushButton("返回");
    headerLayout->addWidget(backToMainBtn);
    headerLayout->addStretch();

    QWidget *bookInfoWidget = new QWidget();
    QFormLayout *infoLayout = new QFormLayout(bookInfoWidget);

    bookTitleLabel = new QLabel();
    QFont titleFont2 = bookTitleLabel->font();
    titleFont2.setPointSize(18);
    bookTitleLabel->setFont(titleFont2);

    bookAuthorLabel = new QLabel();
    bookPriceLabel = new QLabel();
    bookScoreLabel = new QLabel();
    bookDescription = new QTextEdit();
    bookDescription->setReadOnly(true);

    infoLayout->addRow("书名:", bookTitleLabel);
    infoLayout->addRow("作者:", bookAuthorLabel);
    infoLayout->addRow("价格:", bookPriceLabel);
    infoLayout->addRow("评分:", bookScoreLabel);
    infoLayout->addRow("描述:", bookDescription);

    QWidget *actionWidget = new QWidget();
    QHBoxLayout *actionLayout = new QHBoxLayout(actionWidget);

    quantitySpinBox = new QSpinBox();
    quantitySpinBox->setRange(1, 99);
    quantitySpinBox->setValue(1);

    addToCartBtn = new QPushButton("加入购物车");
    buyNowBtn = new QPushButton("立即购买");
    addToFavoriteBtn = new QPushButton("加入收藏");

    actionLayout->addWidget(new QLabel("数量:"));
    actionLayout->addWidget(quantitySpinBox);
    actionLayout->addStretch();
    actionLayout->addWidget(addToCartBtn);
    actionLayout->addWidget(buyNowBtn);
    actionLayout->addWidget(addToFavoriteBtn);

    detailLayout->addWidget(detailHeader);
    detailLayout->addWidget(bookInfoWidget, 1);
    detailLayout->addWidget(actionWidget);

    stackedWidget->addWidget(bookDetailPage);

    // 5. 购物车页面
    cartPage = new QWidget();
    QVBoxLayout *cartLayout = new QVBoxLayout(cartPage);

    QWidget *cartHeader = new QWidget();
    QHBoxLayout *cartHeaderLayout = new QHBoxLayout(cartHeader);

    backFromCartBtn = new QPushButton("返回");
    cartHeaderLayout->addWidget(backFromCartBtn);
    cartHeaderLayout->addStretch();

    cartTable = new QTableWidget();
    cartTable->setColumnCount(5);
    cartTable->setHorizontalHeaderLabels(QStringList() << "选择" << "书名" << "单价" << "数量" << "小计");
    cartTable->horizontalHeader()->setStretchLastSection(true);

    QWidget *cartFooter = new QWidget();
    QHBoxLayout *cartFooterLayout = new QHBoxLayout(cartFooter);

    removeFromCartBtn = new QPushButton("移除选中");
    cartTotalLabel = new QLabel("总计: 0.00元");
    checkoutBtn = new QPushButton("结算");

    cartFooterLayout->addWidget(removeFromCartBtn);
    cartFooterLayout->addStretch();
    cartFooterLayout->addWidget(cartTotalLabel);
    cartFooterLayout->addWidget(checkoutBtn);

    cartLayout->addWidget(cartHeader);
    cartLayout->addWidget(cartTable, 1);
    cartLayout->addWidget(cartFooter);

    stackedWidget->addWidget(cartPage);

    // 6. 订单页面
    ordersPage = new QWidget();
    QVBoxLayout *ordersLayout = new QVBoxLayout(ordersPage);

    QWidget *ordersHeader = new QWidget();
    QHBoxLayout *ordersHeaderLayout = new QHBoxLayout(ordersHeader);

    backFromOrdersBtn = new QPushButton("返回");
    ordersHeaderLayout->addWidget(backFromOrdersBtn);
    ordersHeaderLayout->addStretch();

    ordersTable = new QTableWidget();
    ordersTable->setColumnCount(6);
    ordersTable->setHorizontalHeaderLabels(QStringList() << "订单号" << "日期" << "商品" << "总金额" << "状态" << "操作");
    ordersTable->horizontalHeader()->setStretchLastSection(true);

    ordersLayout->addWidget(ordersHeader);
    ordersLayout->addWidget(ordersTable, 1);

    stackedWidget->addWidget(ordersPage);

    // 7. 个人资料页面
    profilePage = new QWidget();
    QVBoxLayout *profileLayout = new QVBoxLayout(profilePage);

    QWidget *profileHeader = new QWidget();
    QHBoxLayout *profileHeaderLayout = new QHBoxLayout(profileHeader);

    backFromProfileBtn = new QPushButton("返回");
    profileHeaderLayout->addWidget(backFromProfileBtn);
    profileHeaderLayout->addStretch();

    QTabWidget *profileTabs = new QTabWidget();

    // 基本信息标签页
    QWidget *basicInfoTab = new QWidget();
    QFormLayout *basicLayout = new QFormLayout(basicInfoTab);

    profileUsername = new QLineEdit();
    profileUsername->setReadOnly(true);  // 用户名不能修改
    profilePhone = new QLineEdit();
    profileEmail = new QLineEdit();
    profileAddress = new QLineEdit();

    profileLevelLabel = new QLabel();
    profileBalanceLabel = new QLabel();

    basicLayout->addRow("用户名:", profileUsername);
    basicLayout->addRow("电话:", profilePhone);
    basicLayout->addRow("邮箱:", profileEmail);
    basicLayout->addRow("地址:", profileAddress);
    basicLayout->addRow("会员等级:", profileLevelLabel);
    basicLayout->addRow("账户余额:", profileBalanceLabel);

    updateProfileBtn = new QPushButton("更新信息");
    basicLayout->addRow("", updateProfileBtn);

    // 收藏夹标签页
    QWidget *favoriteTab = new QWidget();
    QVBoxLayout *favoriteLayout = new QVBoxLayout(favoriteTab);

    favoriteList = new QListWidget();
    favoriteLayout->addWidget(favoriteList);

    profileTabs->addTab(basicInfoTab, "基本信息");
    profileTabs->addTab(favoriteTab, "我的收藏");

    profileLayout->addWidget(profileHeader);
    profileLayout->addWidget(profileTabs, 1);

    stackedWidget->addWidget(profilePage);

    // 8. 客服页面
    servicePage = new QWidget();
    QVBoxLayout *serviceLayout = new QVBoxLayout(servicePage);

    QWidget *serviceHeader = new QWidget();
    QHBoxLayout *serviceHeaderLayout = new QHBoxLayout(serviceHeader);

    backFromServiceBtn = new QPushButton("返回");
    serviceHeaderLayout->addWidget(backFromServiceBtn);
    serviceHeaderLayout->addStretch();

    chatDisplay = new QTextEdit();
    chatDisplay->setReadOnly(true);

    feedbackInput = new QTextEdit();
    feedbackInput->setPlaceholderText("请输入您的反馈...");
    feedbackInput->setMaximumHeight(100);

    QPushButton *sendFeedbackBtn = new QPushButton("发送反馈");

    serviceLayout->addWidget(serviceHeader);
    serviceLayout->addWidget(chatDisplay, 1);
    serviceLayout->addWidget(new QLabel("问题反馈:"));
    serviceLayout->addWidget(feedbackInput);
    serviceLayout->addWidget(sendFeedbackBtn, 0, Qt::AlignRight);

    stackedWidget->addWidget(servicePage);
}

void Purchaser::initConnections()
{
    // 登录注册
    connect(loginButton, &QPushButton::clicked, this, &Purchaser::onLoginClicked);
    connect(registerButton, &QPushButton::clicked, this, &Purchaser::onRegisterClicked);

    // 找到注册页面的按钮并连接
    QList<QPushButton*> registerButtons = registerPage->findChildren<QPushButton*>();
    for (auto btn : registerButtons) {
        if (btn->text() == "确认注册") {
            connect(btn, &QPushButton::clicked, this, &Purchaser::onRegisterConfirmed);
        } else if (btn->text() == "返回登录") {
            connect(btn, &QPushButton::clicked, this, &Purchaser::showLoginPage);
        }
    }

    connect(logoutButton, &QPushButton::clicked, this, &Purchaser::onLogoutClicked);

    // 浏览相关
    connect(searchButton, &QPushButton::clicked, this, &Purchaser::onSearchClicked);
    connect(categoryTree, &QTreeWidget::itemClicked, this, &Purchaser::onCategoryItemClicked);
    connect(recommendList, &QListWidget::itemClicked, this, &Purchaser::onBookItemClicked);

    // 购物车相关
    connect(addToCartBtn, &QPushButton::clicked, this, &Purchaser::onAddToCartClicked);
    connect(removeFromCartBtn, &QPushButton::clicked, this, &Purchaser::onRemoveFromCartClicked);
    connect(checkoutBtn, &QPushButton::clicked, this, &Purchaser::onCheckoutClicked);
    connect(buyNowBtn, &QPushButton::clicked, this, &Purchaser::onDirectBuyClicked);
     connect(cartButton, &QPushButton::clicked, this, &Purchaser::onViewCartClicked);

    // 收藏相关
    connect(addToFavoriteBtn, &QPushButton::clicked, this, &Purchaser::onAddToFavoriteClicked);

    // 订单相关
    connect(ordersButton, &QPushButton::clicked, this, &Purchaser::onViewOrderClicked);

    // 个人信息相关
    connect(updateProfileBtn, &QPushButton::clicked, this, &Purchaser::onUpdateProfileClicked);
    connect(profileButton, &QPushButton::clicked, this, &Purchaser::onViewProfileClicked);

    // 客服相关
    connect(serviceButton, &QPushButton::clicked, this, &Purchaser::onCustomerServiceClicked);

    // 找到客服页面的发送按钮并连接
    QList<QPushButton*> serviceButtons = servicePage->findChildren<QPushButton*>();
    for (auto btn : serviceButtons) {
        if (btn->text() == "发送反馈") {
            connect(btn, &QPushButton::clicked, this, &Purchaser::onSendFeedbackClicked);
        }
    }

    // 返回按钮
    connect(backToMainBtn, &QPushButton::clicked, this, &Purchaser::showMainPage);
    connect(backFromCartBtn, &QPushButton::clicked, this, &Purchaser::showMainPage);
    connect(backFromOrdersBtn, &QPushButton::clicked, this, &Purchaser::showMainPage);
    connect(backFromProfileBtn, &QPushButton::clicked, this, &Purchaser::showMainPage);
    connect(backFromServiceBtn, &QPushButton::clicked, this, &Purchaser::showMainPage);
}

void Purchaser::loadCategories()
{
    categoryTree->clear();

    std::function<void(CategoryNode*, QTreeWidgetItem*)> addNode;
    addNode = [&](CategoryNode* node, QTreeWidgetItem* parentItem) {
        QTreeWidgetItem* item = new QTreeWidgetItem();
        item->setText(0, node->getName());
        item->setData(0, Qt::UserRole, node->getId());

        if (parentItem) {
            parentItem->addChild(item);
        } else {
            categoryTree->addTopLevelItem(item);
        }

        for (auto child : node->getChildren()) {
            addNode(child, item);
        }
    };

    addNode(categoryRoot, nullptr);
    categoryTree->expandAll();
}

void Purchaser::updateRecommendations()
{
    recommendList->clear();

    QList<Book> recommended = PopularRecommend(10);
    for (const auto &book : recommended) {
        QListWidgetItem *item = new QListWidgetItem();
        item->setText(QString("%1\n¥%2\n评分:%3")
                     .arg(book.getTitle())
                     .arg(book.getPrice())
                     .arg(book.getScore()));
        item->setData(Qt::UserRole, book.getId());
        recommendList->addItem(item);
    }
}

void Purchaser::updateCartDisplay()
{
    cartTable->clearContents();
    cartTable->setRowCount(0);

    if (!currentUser) return;

    double total = 0.0;
    for (const auto &item : currentUser->getCartItems()) {
        int row = cartTable->rowCount();
        cartTable->insertRow(row);

        // 选择框
        QCheckBox *checkBox = new QCheckBox();
        checkBox->setProperty("bookId", item.bookId);
        cartTable->setCellWidget(row, 0, checkBox);

        // 书名
        QTableWidgetItem *titleItem = new QTableWidgetItem(item.bookTitle);
        titleItem->setFlags(titleItem->flags() & ~Qt::ItemIsEditable);
        cartTable->setItem(row, 1, titleItem);

        // 单价
        QTableWidgetItem *priceItem = new QTableWidgetItem(QString::number(item.price, 'f', 2));
        priceItem->setFlags(priceItem->flags() & ~Qt::ItemIsEditable);
        cartTable->setItem(row, 2, priceItem);

        // 数量
        QTableWidgetItem *quantityItem = new QTableWidgetItem(QString::number(item.quantity));
        quantityItem->setFlags(quantityItem->flags() & ~Qt::ItemIsEditable);
        cartTable->setItem(row, 3, quantityItem);

        // 小计
        double subtotal = item.getTotal();
        QTableWidgetItem *subtotalItem = new QTableWidgetItem(QString::number(subtotal, 'f', 2));
        subtotalItem->setFlags(subtotalItem->flags() & ~Qt::ItemIsEditable);
        cartTable->setItem(row, 4, subtotalItem);

        total += subtotal;
    }

    cartTotalLabel->setText(QString("总计: %1元").arg(total, 0, 'f', 2));
}

void Purchaser::updateOrderDisplay()
{
    ordersTable->clearContents();
    ordersTable->setRowCount(0);

    if (!currentUser) return;

    QList<Order> userOrders = ViewMyOrder(currentUser->getId());
    for (const auto &order : userOrders) {
        int row = ordersTable->rowCount();
        ordersTable->insertRow(row);

        // 订单号
        ordersTable->setItem(row, 0, new QTableWidgetItem(order.getOrderId()));

        // 日期
        ordersTable->setItem(row, 1, new QTableWidgetItem(order.getOrderDate().toString("yyyy-MM-dd")));

        // 商品
        QString itemsStr;
        int itemCount = order.getItems().size();
        for (int i = 0; i < qMin(itemCount, 2); i++) {  // 最多显示2个商品
            const auto &item = order.getItems()[i];
            itemsStr += QString("%1×%2").arg(item.bookTitle).arg(item.quantity);
            if (i < itemCount - 1 && i < 1) itemsStr += ", ";
        }
        if (itemCount > 2) itemsStr += QString(" 等%1件商品").arg(itemCount);
        ordersTable->setItem(row, 2, new QTableWidgetItem(itemsStr));

        // 总金额
        ordersTable->setItem(row, 3, new QTableWidgetItem(QString::number(order.getTotalAmount(), 'f', 2)));

        // 状态
        ordersTable->setItem(row, 4, new QTableWidgetItem(order.getStatus()));

        // 操作
        QPushButton *viewBtn = new QPushButton("查看详情");
        viewBtn->setProperty("orderId", order.getOrderId());
        connect(viewBtn, &QPushButton::clicked, [this, order]() {
            Order detailedOrder = ViewOrder(order.getOrderId());
            QString details = QString("订单详情:\n\n订单号: %1\n日期: %2\n总金额: ¥%3\n状态: %4\n\n商品列表:")
                .arg(detailedOrder.getOrderId())
                .arg(detailedOrder.getOrderDate().toString("yyyy-MM-dd"))
                .arg(detailedOrder.getTotalAmount())
                .arg(detailedOrder.getStatus());

            for (const auto &item : detailedOrder.getItems()) {
                details += QString("\n%1 × %2 = ¥%3")
                    .arg(item.bookTitle)
                    .arg(item.quantity)
                    .arg(item.getTotal());
            }

            QMessageBox::information(this, "订单详情", details);
        });
        ordersTable->setCellWidget(row, 5, viewBtn);
    }
}

void Purchaser::updateProfileDisplay()
{
    if (!currentUser) return;

    profileUsername->setText(currentUser->getUsername());
    profilePhone->setText(currentUser->getPhone());
    profileEmail->setText(currentUser->getEmail());
    profileAddress->setText(currentUser->getAddress());
    profileLevelLabel->setText(QString::number(currentUser->getMembershipLevel()));
    profileBalanceLabel->setText(QString::number(currentUser->getBalance(), 'f', 2));

    favoriteList->clear();
    for (const auto &bookId : currentUser->getFavoriteBooks()) {
        if (bookMap.contains(bookId)) {
            Book book = bookMap[bookId];
            QListWidgetItem *item = new QListWidgetItem(
                QString("%1\n¥%2 - 评分:%3")
                    .arg(book.getTitle())
                    .arg(book.getPrice())
                    .arg(book.getScore()));
            item->setData(Qt::UserRole, bookId);
            favoriteList->addItem(item);
        }
    }
}

// 核心功能实现
QList<Book> Purchaser::PopularRecommend(int quantity)
{
    QList<Book> recommended;

    // 如果有登录用户，考虑用户偏好
    if (currentUser && !currentUser->getPreferences().isEmpty()) {
        // 获取用户偏好
        QList<UserPreference> prefs = currentUser->getPreferences();

        // 按偏好权重和热度综合排序
        QList<Book> sortedBooks = allBooks;

        std::sort(sortedBooks.begin(), sortedBooks.end(),
                  [&prefs](const Book &a, const Book &b) {
                      // 计算偏好得分
                      double scoreA = a.getHeat();
                      double scoreB = b.getHeat();

                      for (const auto &pref : prefs) {
                          if (a.getCategory1() == pref.category ||
                              a.getCategory2() == pref.category) {
                              scoreA += pref.weight * 10;
                          }
                          if (b.getCategory1() == pref.category ||
                              b.getCategory2() == pref.category) {
                              scoreB += pref.weight * 10;
                          }
                      }

                      return scoreA > scoreB;
                  });

        // 取前quantity本
        for (int i = 0; i < qMin(quantity, sortedBooks.size()); i++) {
            recommended.append(sortedBooks[i]);
        }
    } else {
        // 无用户偏好，按热度排序
        QList<Book> sortedBooks = allBooks;

        std::sort(sortedBooks.begin(), sortedBooks.end(),
                  [](const Book &a, const Book &b) {
                      return a.getHeat() > b.getHeat();
                  });

        for (int i = 0; i < qMin(quantity, sortedBooks.size()); i++) {
            recommended.append(sortedBooks[i]);
        }
    }

    return recommended;
}

QList<Book> Purchaser::GetBooksByCategory(const QString &categoryId1, const QString &categoryId2)
{
    QList<Book> result;

    std::function<void(CategoryNode*)> searchCategory;
    searchCategory = [&](CategoryNode* node) {
        if (node->getId() == categoryId1 ||
            (categoryId2.isEmpty() && node->getId() == categoryId1) ||
            (!categoryId2.isEmpty() && node->getId() == categoryId2)) {
            // 找到分类，获取所有图书
            for (const auto &bookId : node->getBookIds()) {
                if (bookMap.contains(bookId)) {
                    result.append(bookMap[bookId]);
                }
            }

            // 如果是父分类，还需要获取子分类的图书
            for (auto child : node->getChildren()) {
                for (const auto &bookId : child->getBookIds()) {
                    if (bookMap.contains(bookId)) {
                        result.append(bookMap[bookId]);
                    }
                }
            }
        }

        for (auto child : node->getChildren()) {
            searchCategory(child);
        }
    };

    searchCategory(categoryRoot);
    return result;
}

QList<Book> Purchaser::SearchBooks(const QString &keyword)
{
    QList<Book> result;

    for (const auto &book : allBooks) {
        if (book.getTitle().contains(keyword, Qt::CaseInsensitive) ||
            book.getAuthor().contains(keyword, Qt::CaseInsensitive) ||
            book.getDescription().contains(keyword, Qt::CaseInsensitive)) {
            result.append(book);
        }
    }

    return result;
}

Book Purchaser::ViewBookDetail(const QString &bookId)
{
    if (bookMap.contains(bookId)) {
        Book book = bookMap[bookId];
        // 完善图书信息
        if (book.getId() == "001") {
            book.setAuthor("Randal E.Bryant, David O'Hallaron");
            book.setPublisher("机械工业出版社");
            book.setDescription("本书从程序员的视角详细阐述计算机系统的本质概念，并展示这些概念如何实实在在地影响应用程序的正确性、性能和实用性。");
        } else if (book.getId() == "002") {
            book.setAuthor("Stanley B. Lippman, Josée Lajoie, Barbara E. Moo");
            book.setPublisher("电子工业出版社");
            book.setDescription("久负盛名的C++经典教程，内容涵盖了C++语言的全貌和现代C++编程风格。");
        } else if (book.getId() == "003") {
            book.setAuthor("刘慈欣");
            book.setPublisher("重庆出版社");
            book.setDescription("中国科幻文学的里程碑之作，讲述了地球人类文明和三体文明的信息交流、生死搏杀及两个文明在宇宙中的兴衰历程。");
        } else if (book.getId() == "004") {
            book.setAuthor("余华");
            book.setPublisher("作家出版社");
            book.setDescription("讲述一个人一生的故事，这是一个历尽世间沧桑和磨难老人的人生感言，是一幕演绎人生苦难经历的戏剧。");
        }
        return book;
    }
    return Book();
}

bool Purchaser::AddToCart(const QString &bookId, int quantity)
{
    if (!bookMap.contains(bookId) || !currentUser) return false;

    Book book = bookMap[bookId];
    return currentUser->addToCart(bookId, quantity, book.getTitle(), book.getPrice());
}

Order Purchaser::CheckoutByBook(const QString &cartId, const QString &couponCode, int membershipLevel)
{
    if (!currentUser) return Order();

    Order order(QDateTime::currentDateTime().toString("yyyyMMddhhmmss"),
                currentUser->getId(), QDate::currentDate());

    // 添加当前图书
    OrderItem item;
    item.bookId = currentBook.getId();
    item.bookTitle = currentBook.getTitle();
    item.quantity = quantitySpinBox->value();
    item.price = currentBook.getPrice();
    item.status = "待发货";

    order.addItem(item);

    // 计算总金额（应用折扣）
    double total = item.getTotal();
    double discount = 1.0;
    if (membershipLevel >= 3) discount = 0.9;
    if (!couponCode.isEmpty()) discount *= 0.95;

    order.setTotalAmount(total * discount);
    order.setStatus("待付款");

    // 添加到订单列表
    allOrders.append(order);

    return order;
}

Order Purchaser::CheckoutByCart(const QString &cartId, const QString &couponCode, int membershipLevel)
{
    if (!currentUser) return Order();

    Order order(QDateTime::currentDateTime().toString("yyyyMMddhhmmss"),
                currentUser->getId(), QDate::currentDate());

    double total = 0.0;
    for (const auto &cartItem : currentUser->getCartItems()) {
        OrderItem item;
        item.bookId = cartItem.bookId;
        item.bookTitle = cartItem.bookTitle;
        item.quantity = cartItem.quantity;
        item.price = cartItem.price;
        item.status = "待发货";

        order.addItem(item);
        total += item.getTotal();
    }

    // 应用折扣
    double discount = 1.0;
    if (membershipLevel >= 3) discount = 0.9;
    if (!couponCode.isEmpty()) discount *= 0.95;

    order.setTotalAmount(total * discount);
    order.setStatus("待付款");

    // 添加到订单列表
    allOrders.append(order);

    // 清空购物车
    currentUser->clearCart();

    return order;
}

Order Purchaser::ViewOrder(const QString &orderId)
{
    for (const auto &order : allOrders) {
        if (order.getOrderId() == orderId) {
            return order;
        }
    }
    return Order();
}

User* Purchaser::Login(const QString &username, const QString &password)
{
    return userManager.login(username, password);
}

bool Purchaser::Register(const QString &username, const QString &password)
{
    return userManager.registerUser(username, password);
}

bool Purchaser::ChangeInformation(const QString &field, const QString &value)
{
    if (!currentUser) return false;

    if (field == "phone") {
        currentUser->setPhone(value);
        return true;
    } else if (field == "email") {
        currentUser->setEmail(value);
        return true;
    } else if (field == "address") {
        currentUser->setAddress(value);
        return true;
    }
    return false;
}

int Purchaser::LevelUp()
{
    if (!currentUser) return 0;

    int currentLevel = currentUser->getMembershipLevel();
    if (currentLevel < 5) {
        currentUser->setMembershipLevel(currentLevel + 1);
        return currentLevel + 1;
    }
    return currentLevel;
}

QList<Order> Purchaser::ViewMyOrder(int userId)
{
    QList<Order> userOrders;
    for (const auto &order : allOrders) {
        if (order.getUserId() == userId) {
            userOrders.append(order);
        }
    }
    return userOrders;
}

QList<CartItem> Purchaser::ViewShoppingCart(int userId)
{
    if (!currentUser || currentUser->getId() != userId) {
        return QList<CartItem>();
    }
    return currentUser->getCartItems();
}

bool Purchaser::RemoveFromCart(const QString &bookId)
{
    if (!currentUser) return false;
    return currentUser->removeFromCart(bookId);
}

bool Purchaser::AddToFavorite(const QString &bookId)
{
    if (!currentUser) return false;
    return currentUser->addToFavorite(bookId);
}

void Purchaser::SaleChat()
{
    // 显示客服聊天界面
    showServicePage();
}

void Purchaser::ProductFeedback(const QString &feedback)
{
    QMessageBox::information(this, "反馈提交", "感谢您的反馈！我们会认真考虑您的建议。");
}

// 槽函数实现
void Purchaser::onLoginClicked()
{
    QString username = loginUsername->text().trimmed();
    QString password = loginPassword->text().trimmed();

    if (username.isEmpty() || password.isEmpty()) {
        QLabel *statusLabel = loginPage->findChild<QLabel*>("loginStatusLabel");
        if (statusLabel) {
            statusLabel->setText("请输入用户名和密码");
        }
        return;
    }

    currentUser = Login(username, password);
    if (currentUser != nullptr) {
        isLoggedIn = true;
        QLabel *statusLabel = loginPage->findChild<QLabel*>("loginStatusLabel");
        if (statusLabel) {
            statusLabel->clear();
        }
        loginUsername->clear();
        loginPassword->clear();
        showMainPage();
        QMessageBox::information(this, "登录成功", QString("欢迎回来，%1！").arg(username));
    } else {
        QLabel *statusLabel = loginPage->findChild<QLabel*>("loginStatusLabel");
        if (statusLabel) {
            statusLabel->setText("用户名或密码错误");
        }
    }
}

void Purchaser::onRegisterClicked()
{
    stackedWidget->setCurrentWidget(registerPage);
}

void Purchaser::onRegisterConfirmed()
{
    QString username = regUsername->text().trimmed();
    QString password = regPassword->text().trimmed();
    QString confirmPassword = regConfirmPassword->text().trimmed();

    // 验证输入
    if (username.isEmpty() || password.isEmpty() || confirmPassword.isEmpty()) {
        QMessageBox::warning(this, "注册失败", "请填写所有字段");
        return;
    }

    if (username.length() < 3 || username.length() > 20) {
        QMessageBox::warning(this, "注册失败", "用户名长度应为3-20位");
        return;
    }

    if (password.length() < 6) {
        QMessageBox::warning(this, "注册失败", "密码长度不能少于6位");
        return;
    }

    if (password != confirmPassword) {
        QMessageBox::warning(this, "注册失败", "两次输入的密码不一致");
        return;
    }

    // 检查用户名是否已存在
    if (userManager.userExists(username)) {
        QMessageBox::warning(this, "注册失败", "用户名已存在");
        return;
    }

    // 注册用户
    if (Register(username, password)) {
        QMessageBox::information(this, "注册成功", "注册成功！请使用新账号登录");

        // 自动填充登录表单并返回登录页面
        loginUsername->setText(username);
        loginPassword->clear();
        showLoginPage();
    } else {
        QMessageBox::warning(this, "注册失败", "注册失败，请重试");
    }
}

void Purchaser::onLogoutClicked()
{
    int result = QMessageBox::question(this, "确认退出", "确定要退出登录吗？",
                                      QMessageBox::Yes | QMessageBox::No);
    if (result == QMessageBox::Yes) {
        currentUser = nullptr;
        isLoggedIn = false;
        showLoginPage();
    }
}

void Purchaser::onSearchClicked()
{
    QString keyword = searchInput->text().trimmed();
    if (keyword.isEmpty()) {
        updateRecommendations();
        return;
    }

    QList<Book> results = SearchBooks(keyword);
    recommendList->clear();

    if (results.isEmpty()) {
        QListWidgetItem *item = new QListWidgetItem("未找到相关图书");
        item->setTextAlignment(Qt::AlignCenter);
        recommendList->addItem(item);
        return;
    }

    for (const auto &book : results) {
        QListWidgetItem *item = new QListWidgetItem();
        item->setText(QString("%1\n¥%2\n评分:%3")
                     .arg(book.getTitle())
                     .arg(book.getPrice())
                     .arg(book.getScore()));
        item->setData(Qt::UserRole, book.getId());
        recommendList->addItem(item);
    }
}

void Purchaser::onCategoryItemClicked(QTreeWidgetItem *item, int column)
{
    QString categoryId = item->data(0, Qt::UserRole).toString();

    // 获取该分类及子分类的所有图书
    QList<Book> books = GetBooksByCategory(categoryId, "");

    recommendList->clear();
    if (books.isEmpty()) {
        QListWidgetItem *noItem = new QListWidgetItem("该分类暂无图书");
        noItem->setTextAlignment(Qt::AlignCenter);
        recommendList->addItem(noItem);
        return;
    }

    for (const auto &book : books) {
        QListWidgetItem *listItem = new QListWidgetItem();
        listItem->setText(QString("%1\n¥%2\n评分:%3")
                         .arg(book.getTitle())
                         .arg(book.getPrice())
                         .arg(book.getScore()));
        listItem->setData(Qt::UserRole, book.getId());
        recommendList->addItem(listItem);
    }
}

void Purchaser::onBookItemClicked(QListWidgetItem *item)
{
    if (item->text() == "未找到相关图书" || item->text() == "该分类暂无图书") {
        return;
    }

    QString bookId = item->data(Qt::UserRole).toString();
    currentBook = ViewBookDetail(bookId);

    // 更新图书详情显示
    bookTitleLabel->setText(currentBook.getTitle());
    bookAuthorLabel->setText(currentBook.getAuthor());
    bookPriceLabel->setText(QString("¥%1").arg(currentBook.getPrice()));
    bookScoreLabel->setText(QString::number(currentBook.getScore()));
    bookDescription->setText(currentBook.getDescription());

    showBookDetailPage();
}

void Purchaser::onAddToCartClicked()
{
    if (!isLoggedIn || !currentUser) {
        QMessageBox::warning(this, "操作失败", "请先登录");
        showLoginPage();
        return;
    }

    int quantity = quantitySpinBox->value();
    if (AddToCart(currentBook.getId(), quantity)) {
        QMessageBox::information(this, "成功", "已添加到购物车");
        updateCartDisplay();
    } else {
        QMessageBox::warning(this, "失败", "添加到购物车失败");
    }
}

void Purchaser::onRemoveFromCartClicked()
{
    if (!currentUser) return;

    // 获取选中的图书
    QList<QString> bookIdsToRemove;
    for (int i = 0; i < cartTable->rowCount(); i++) {
        QCheckBox *checkBox = qobject_cast<QCheckBox*>(cartTable->cellWidget(i, 0));
        if (checkBox && checkBox->isChecked()) {
            QString bookId = checkBox->property("bookId").toString();
            bookIdsToRemove.append(bookId);
        }
    }

    if (bookIdsToRemove.isEmpty()) {
        QMessageBox::information(this, "提示", "请先选择要移除的商品");
        return;
    }

    int result = QMessageBox::question(this, "确认移除",
                                      QString("确定要移除选中的%1件商品吗？").arg(bookIdsToRemove.size()),
                                      QMessageBox::Yes | QMessageBox::No);

    if (result == QMessageBox::Yes) {
        bool removed = false;
        for (const auto &bookId : bookIdsToRemove) {
            if (RemoveFromCart(bookId)) {
                removed = true;
            }
        }

        if (removed) {
            updateCartDisplay();
            QMessageBox::information(this, "成功", "已从购物车移除");
        }
    }
}

void Purchaser::onCheckoutClicked()
{
    if (!isLoggedIn || !currentUser) {
        QMessageBox::warning(this, "操作失败", "请先登录");
        showLoginPage();
        return;
    }

    if (currentUser->getCartItems().isEmpty()) {
        QMessageBox::warning(this, "操作失败", "购物车为空");
        return;
    }

    // 显示确认对话框
    int result = QMessageBox::question(this, "确认结算",
                                      QString("总计: %1元\n确认结算吗？")
                                      .arg(currentUser->getCartTotal(), 0, 'f', 2),
                                      QMessageBox::Yes | QMessageBox::No);

    if (result == QMessageBox::Yes) {
        // 检查余额
        double total = currentUser->getCartTotal();
        if (currentUser->deductBalance(total)) {
            Order order = CheckoutByCart("cart", "", currentUser->getMembershipLevel());
            QMessageBox::information(this, "支付成功",
                QString("订单创建成功！\n订单号: %1\n支付金额: %2元")
                .arg(order.getOrderId())
                .arg(order.getTotalAmount()));
            updateCartDisplay();
        } else {
            QMessageBox::warning(this, "支付失败", "余额不足，请充值");
        }
    }
}

void Purchaser::onDirectBuyClicked()
{
    if (!isLoggedIn || !currentUser) {
        QMessageBox::warning(this, "操作失败", "请先登录");
        showLoginPage();
        return;
    }

    int quantity = quantitySpinBox->value();
    double total = currentBook.getPrice() * quantity;

    // 显示确认对话框
    int result = QMessageBox::question(this, "确认购买",
                                      QString("%1 × %2 = %3元\n确认购买吗？")
                                      .arg(currentBook.getTitle())
                                      .arg(quantity)
                                      .arg(total),
                                      QMessageBox::Yes | QMessageBox::No);

    if (result == QMessageBox::Yes) {
        if (currentUser->deductBalance(total)) {
            Order order = CheckoutByBook("direct", "", currentUser->getMembershipLevel());
            QMessageBox::information(this, "购买成功",
                QString("订单创建成功！\n订单号: %1\n支付金额: %2元")
                .arg(order.getOrderId())
                .arg(order.getTotalAmount()));
            showMainPage();
        } else {
            QMessageBox::warning(this, "支付失败", "余额不足，请充值");
        }
    }
}
void Purchaser::onViewCartClicked()
{
    if (!isLoggedIn || !currentUser) {
        QMessageBox::warning(this, "操作失败", "请先登录");
        showLoginPage();
        return;
    }

    updateCartDisplay();
    showCartPage();
}

void Purchaser::onAddToFavoriteClicked()
{
    if (!isLoggedIn || !currentUser) {
        QMessageBox::warning(this, "操作失败", "请先登录");
        showLoginPage();
        return;
    }

    if (AddToFavorite(currentBook.getId())) {
        QMessageBox::information(this, "成功", "已添加到收藏");
        updateProfileDisplay();
    } else {
        if (currentUser->isFavorite(currentBook.getId())) {
            QMessageBox::information(this, "提示", "已经在收藏中了");
        } else {
            QMessageBox::warning(this, "失败", "添加到收藏失败");
        }
    }
}

void Purchaser::onViewOrderClicked()
{
    if (!isLoggedIn || !currentUser) {
        QMessageBox::warning(this, "操作失败", "请先登录");
        showLoginPage();
        return;
    }

    updateOrderDisplay();
    showOrdersPage();
}

void Purchaser::onUpdateProfileClicked()
{
    if (!currentUser) return;

    ChangeInformation("phone", profilePhone->text());
    ChangeInformation("email", profileEmail->text());
    ChangeInformation("address", profileAddress->text());

    QMessageBox::information(this, "成功", "个人信息已更新");
}

void Purchaser::onViewProfileClicked()
{
    if (!isLoggedIn || !currentUser) {
        QMessageBox::warning(this, "操作失败", "请先登录");
        showLoginPage();
        return;
    }

    updateProfileDisplay();
    showProfilePage();
}

void Purchaser::onCustomerServiceClicked()
{
    SaleChat();
}

void Purchaser::onSendFeedbackClicked()
{
    QString feedback = feedbackInput->toPlainText().trimmed();
    if (feedback.isEmpty()) {
        QMessageBox::warning(this, "反馈失败", "请输入反馈内容");
        return;
    }

    ProductFeedback(feedback);
    feedbackInput->clear();

    // 在聊天记录中添加反馈
    chatDisplay->append("\n用户反馈: " + feedback);
    chatDisplay->append("客服: 感谢您的反馈，我们会尽快处理！");
}

// 页面切换函数
void Purchaser::showLoginPage()
{
    stackedWidget->setCurrentWidget(loginPage);
}

void Purchaser::showMainPage()
{
    updateRecommendations();
    stackedWidget->setCurrentWidget(mainPage);
}

void Purchaser::showBookDetailPage()
{
    stackedWidget->setCurrentWidget(bookDetailPage);
}

void Purchaser::showCartPage()
{
    // 更新购物车显示
    updateCartDisplay();

    // 设置当前页面
    stackedWidget->setCurrentWidget(cartPage);

    // 如果购物车为空，显示提示信息
    if (!currentUser || currentUser->getCartItems().isEmpty()) {
        QMessageBox::information(this, "购物车", "您的购物车是空的");
    }
}

void Purchaser::showOrdersPage()
{
    stackedWidget->setCurrentWidget(ordersPage);
}

void Purchaser::showProfilePage()
{
    stackedWidget->setCurrentWidget(profilePage);
}

void Purchaser::showServicePage()
{
    // 添加一些默认的客服消息
    chatDisplay->setText("客服系统\n"
                        "====================\n"
                        "客服: 您好！欢迎来到图书购买系统客服中心。\n"
                        "客服: 我们可以为您提供以下帮助：\n"
                        "客服: 1. 会员权益咨询\n"
                        "客服: 2. 图书推荐\n"
                        "客服: 3. 售后问题处理\n"
                        "客服: 4. 退换货服务\n"
                        "客服: 5. 图书配套资源使用教程\n\n"
                        "客服: 请在下方输入框输入您的问题或反馈。");
    stackedWidget->setCurrentWidget(servicePage);
}
