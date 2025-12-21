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
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QThread>
#include <QTimer>
#include <QElapsedTimer>
#include <QCoreApplication>
#include <QInputDialog>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QFileDialog>
#include <QPixmap>
#include <QBuffer>
#include <QDialog>
#include <QGroupBox>
#include <QStandardItemModel>

// #region agent log
// 调试日志辅助函数
static void writeDebugLog(const QString& location, const QString& message, const QJsonObject& data, const QString& hypothesisId = "")
{
    QFile logFile("f:\\Qt\\project\\bookmall\\bookadmin\\.cursor\\debug.log");
    if (logFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        QTextStream out(&logFile);
        QJsonObject logEntry;
        logEntry["timestamp"] = QDateTime::currentMSecsSinceEpoch();
        logEntry["location"] = location;
        logEntry["message"] = message;
        logEntry["data"] = data;
        logEntry["sessionId"] = "debug-session";
        logEntry["runId"] = "run1";
        if (!hypothesisId.isEmpty()) {
            logEntry["hypothesisId"] = hypothesisId;
        }
        out << QJsonDocument(logEntry).toJson(QJsonDocument::Compact) << "\n";
        logFile.close();
    }
}
// #endregion

Purchaser::Purchaser(QWidget *parent)
    : QMainWindow(parent),
      currentUser(nullptr),    // 先初始化这个
      userManager(),           // 然后这个
      categoryRoot(nullptr),   // 最后这个
      isLoggedIn(false),
      serverIp("127.0.0.1"),       // 初始化服务器IP（本地服务器）
      serverPort(8888),           // 初始化服务器端口
      licenseImagePath(""),       // 初始化营业执照图片路径
      licenseImageBase64(""),     // 初始化营业执照图片Base64
      autoRefreshTimer(nullptr),   // 初始化自动刷新定时器
      chatRefreshTimer(nullptr),    // 初始化聊天刷新定时器
      currentSellerId(-1),  // 初始化当前卖家ID
      sellerChatRefreshTimer(nullptr)  // 初始化卖家聊天刷新定时器
{  
    apiService = new ApiService(this);  // 使用TCP API服务
    
    // 创建自动刷新定时器
    autoRefreshTimer = new QTimer(this);
    autoRefreshTimer->setInterval(AUTO_REFRESH_INTERVAL);  // 30秒
    connect(autoRefreshTimer, &QTimer::timeout, this, &Purchaser::onAutoRefresh);
    
    // 初始化聊天刷新定时器
    chatRefreshTimer = new QTimer(this);
    chatRefreshTimer->setInterval(2000);  // 每2秒刷新一次
    connect(chatRefreshTimer, &QTimer::timeout, this, &Purchaser::loadChatHistory);
    
    // 初始化卖家聊天刷新定时器
    sellerChatRefreshTimer = new QTimer(this);
    sellerChatRefreshTimer->setInterval(2000);  // 每2秒刷新一次
    connect(sellerChatRefreshTimer, &QTimer::timeout, this, &Purchaser::loadSellerChatHistory);
    
    initData();
    initUI();
    initConnections();
    applyStyle();  // 应用统一样式
    showLoginPage();

   // 服务器地址已在初始化列表中设置
   qDebug() << "服务器配置 - IP:" << serverIp << "端口:" << serverPort;

   // 连接API服务信号
   connect(apiService, &ApiService::connected, this, [this]() {
       qDebug() << "已连接到服务器";
   });
   connect(apiService, &ApiService::disconnected, this, [this]() {
       qDebug() << "与服务器断开连接";
   });
   connect(apiService, &ApiService::errorOccurred, this, &Purchaser::onNetworkError);

   // 程序启动时自动连接服务器
   QTimer::singleShot(500, this, [this]() {
       qDebug() << "自动连接服务器...";
       if (!apiService->isConnected()) {
           if (apiService->connectToServer(serverIp, serverPort)) {
               qDebug() << "自动连接服务器成功";
           } else {
               qDebug() << "自动连接服务器失败，将在用户操作时重试";
           }
       }
   });
}

Purchaser::~Purchaser()
{
    delete categoryRoot;
}

void Purchaser::initData()
{
    // 初始化类别树
    categoryRoot = new CategoryNode("root", "所有分类");

    // 1. 文学小说
    CategoryNode *literature = new CategoryNode("literature", "文学小说", categoryRoot);
    literature->addChild(new CategoryNode("contemporary_fiction", "当代小说", literature));
    literature->addChild(new CategoryNode("mystery_thriller", "悬疑/推理", literature));
    literature->addChild(new CategoryNode("sci_fantasy", "科幻/奇幻", literature));
    literature->addChild(new CategoryNode("chinese_classics", "中国古典文学", literature));
    literature->addChild(new CategoryNode("foreign_literature", "外国文学", literature));
    literature->addChild(new CategoryNode("martial_arts", "武侠/仙侠", literature));
    literature->addChild(new CategoryNode("essay", "散文/随笔", literature));
    literature->addChild(new CategoryNode("poetry_drama", "诗歌/戏剧", literature));
    literature->addChild(new CategoryNode("literature_other", "其他", literature));
    categoryRoot->addChild(literature);

    // 2. 人文社科
    CategoryNode *humanities = new CategoryNode("humanities", "人文社科", categoryRoot);
    humanities->addChild(new CategoryNode("history", "历史（中国史/世界史）", humanities));
    humanities->addChild(new CategoryNode("philosophy_religion", "哲学/宗教", humanities));
    humanities->addChild(new CategoryNode("psychology", "心理学", humanities));
    humanities->addChild(new CategoryNode("politics_military", "政治/军事", humanities));
    humanities->addChild(new CategoryNode("law", "法律", humanities));
    humanities->addChild(new CategoryNode("social_science", "社会科学", humanities));
    humanities->addChild(new CategoryNode("culture_anthropology", "文化/人类学", humanities));
    humanities->addChild(new CategoryNode("biography", "传记/回忆录", humanities));
    humanities->addChild(new CategoryNode("humanities_other", "其他", humanities));
    categoryRoot->addChild(humanities);

    // 3. 经济管理
    CategoryNode *economics = new CategoryNode("economics", "经济管理", categoryRoot);
    economics->addChild(new CategoryNode("economics_theory", "经济学理论", economics));
    economics->addChild(new CategoryNode("business_management", "企业管理", economics));
    economics->addChild(new CategoryNode("investment_finance", "投资理财", economics));
    economics->addChild(new CategoryNode("marketing", "市场营销", economics));
    economics->addChild(new CategoryNode("career_motivation", "职场励志", economics));
    economics->addChild(new CategoryNode("accounting_finance", "会计/金融", economics));
    economics->addChild(new CategoryNode("ecommerce", "电子商务", economics));
    economics->addChild(new CategoryNode("business_biography", "名人传记（商业）", economics));
    economics->addChild(new CategoryNode("economics_other", "其他", economics));
    categoryRoot->addChild(economics);

    // 4. 科学技术
    CategoryNode *science = new CategoryNode("science", "科学技术", categoryRoot);
    science->addChild(new CategoryNode("computer_internet", "计算机/互联网", science));
    science->addChild(new CategoryNode("popular_science", "科普读物", science));
    science->addChild(new CategoryNode("physics", "物理学", science));
    science->addChild(new CategoryNode("mathematics", "数学", science));
    science->addChild(new CategoryNode("chemistry", "化学", science));
    science->addChild(new CategoryNode("medicine_health", "医学/卫生", science));
    science->addChild(new CategoryNode("architecture_engineering", "建筑/工程", science));
    science->addChild(new CategoryNode("natural_science", "自然科学", science));
    science->addChild(new CategoryNode("science_other", "其他", science));
    categoryRoot->addChild(science);

    // 5. 教育考试
    CategoryNode *education = new CategoryNode("education", "教育考试", categoryRoot);
    education->addChild(new CategoryNode("k12_tutoring", "中小学教辅", education));
    education->addChild(new CategoryNode("foreign_language", "外语学习", education));
    education->addChild(new CategoryNode("exam_certification", "考试/考证（公考/考研）", education));
    education->addChild(new CategoryNode("textbook", "教材/课本", education));
    education->addChild(new CategoryNode("reference_dictionary", "工具书/字典", education));
    education->addChild(new CategoryNode("vocational_training", "职业培训", education));
    education->addChild(new CategoryNode("education_other", "其他", education));
    categoryRoot->addChild(education);

    // 6. 生活艺术
    CategoryNode *lifestyle = new CategoryNode("lifestyle", "生活艺术", categoryRoot);
    lifestyle->addChild(new CategoryNode("cooking_food", "烹饪/美食", lifestyle));
    lifestyle->addChild(new CategoryNode("travel_map", "旅游/地图", lifestyle));
    lifestyle->addChild(new CategoryNode("relationship_emotion", "两性/情感", lifestyle));
    lifestyle->addChild(new CategoryNode("home_garden", "家居/园艺", lifestyle));
    lifestyle->addChild(new CategoryNode("sports_fitness", "运动/健身", lifestyle));
    lifestyle->addChild(new CategoryNode("painting_calligraphy", "绘画/书法", lifestyle));
    lifestyle->addChild(new CategoryNode("photography_design", "摄影/设计", lifestyle));
    lifestyle->addChild(new CategoryNode("music_film", "音乐/影视", lifestyle));
    lifestyle->addChild(new CategoryNode("lifestyle_other", "其他", lifestyle));
    categoryRoot->addChild(lifestyle);

    // 7. 少儿童书
    CategoryNode *children = new CategoryNode("children", "少儿童书", categoryRoot);
    children->addChild(new CategoryNode("toddler_0_2", "0-2岁启蒙", children));
    children->addChild(new CategoryNode("picture_book_3_6", "3-6岁绘本", children));
    children->addChild(new CategoryNode("science_7_10", "7-10岁科普", children));
    children->addChild(new CategoryNode("children_literature", "儿童文学", children));
    children->addChild(new CategoryNode("children_english", "少儿英语", children));
    children->addChild(new CategoryNode("anime_cartoon", "动漫/卡通", children));
    children->addChild(new CategoryNode("children_other", "其他", children));
    categoryRoot->addChild(children);

    // 8. 其他
    CategoryNode *other = new CategoryNode("other", "其他", categoryRoot);
    categoryRoot->addChild(other);

    // 不再加载本地预设图书数据
    // 业务逻辑：买家初始没有图书，需要从服务器获取卖家上架的图书
    // loadLocalBooks();  // 已禁用

    // bookMap初始为空，等待从服务器加载图书
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
    loginLayout->setAlignment(Qt::AlignCenter);
    loginLayout->setContentsMargins(20, 20, 20, 20);

    // 创建登录卡片容器
    QWidget *loginCard = new QWidget();
    loginCard->setFixedWidth(480);
    loginCard->setStyleSheet(R"(
        QWidget {
            background-color: white;
            border-radius: 20px;
        }
    )");
    QVBoxLayout *cardLayout = new QVBoxLayout(loginCard);
    cardLayout->setContentsMargins(50, 50, 50, 50);
    cardLayout->setSpacing(25);

    // 标题
    QLabel *loginTitle = new QLabel("📚 图书购买系统");
    loginTitle->setAlignment(Qt::AlignCenter);
    QFont titleFont;
    titleFont.setFamily("Microsoft YaHei");
    titleFont.setBold(true);
    titleFont.setPointSize(25);
    loginTitle->setFont(titleFont);
    loginTitle->setStyleSheet("color: #2c3e50; margin-bottom: 10px;");
    cardLayout->addWidget(loginTitle);

    // 副标题
    QLabel *subTitle = new QLabel("欢迎回来，请登录您的账户");
    subTitle->setAlignment(Qt::AlignCenter);
    QFont subFont;
    subFont.setFamily("Microsoft YaHei");
    subFont.setPointSize(12);
    subTitle->setFont(subFont);
    subTitle->setStyleSheet("color: #7f8c8d; margin-bottom: 30px;");
    cardLayout->addWidget(subTitle);

    // 用户名输入框
    QLabel *usernameLabel = new QLabel("用户名");
    usernameLabel->setStyleSheet("color: #2c3e50; font-size: 14px; font-weight: 500; margin-bottom: 5px;");
    cardLayout->addWidget(usernameLabel);
    
    loginUsername = new QLineEdit();
    loginUsername->setPlaceholderText("请输入用户名");
    loginUsername->setMinimumHeight(50);
    loginUsername->setStyleSheet(R"(
        QLineEdit {
            border: 2px solid #e1e8ed;
            border-radius: 12px;
            padding: 12px 18px;
            font-size: 15px;
            background-color: #f8fafc;
            color: #2c3e50;
            font-family: 'Microsoft YaHei';
        }
        QLineEdit:focus {
            border: 2px solid #2980b9;
            background-color: white;
        }
    )");
    cardLayout->addWidget(loginUsername);

    // 密码输入框
    QLabel *passwordLabel = new QLabel("密码");
    passwordLabel->setStyleSheet("color: #2c3e50; font-size: 14px; font-weight: 500; margin-top: 15px; margin-bottom: 5px;");
    cardLayout->addWidget(passwordLabel);
    
    loginPassword = new QLineEdit();
    loginPassword->setPlaceholderText("请输入密码");
    loginPassword->setEchoMode(QLineEdit::Password);
    loginPassword->setMinimumHeight(50);
    loginPassword->setStyleSheet(R"(
        QLineEdit {
            border: 2px solid #e1e8ed;
            border-radius: 12px;
            padding: 12px 18px;
            font-size: 15px;
            background-color: #f8fafc;
            color: #2c3e50;
            font-family: 'Microsoft YaHei';
        }
        QLineEdit:focus {
            border: 2px solid #2980b9;
            background-color: white;
        }
    )");
    cardLayout->addWidget(loginPassword);

    // 状态标签
    loginStatusLabel = new QLabel();
    loginStatusLabel->setAlignment(Qt::AlignCenter);
    loginStatusLabel->setStyleSheet("color: #e74c3c; font-size: 12px; min-height: 20px;");
    loginStatusLabel->setObjectName("loginStatusLabel");
    cardLayout->addWidget(loginStatusLabel);

    // 按钮容器
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(15);

    loginButton = new QPushButton("登录");
    loginButton->setMinimumHeight(50);
    loginButton->setStyleSheet(R"(
        QPushButton {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #2980b9,
                stop:1 #3498db);
            color: white;
            border: none;
            border-radius: 12px;
            font-size: 16px;
            font-weight: bold;
            font-family: 'Microsoft YaHei';
        }
        QPushButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #3498db,
                stop:1 #2980b9);
        }
        QPushButton:pressed {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #1f5f8f,
                stop:1 #2471a3);
        }
    )");

    registerButton = new QPushButton("注册");
    registerButton->setMinimumHeight(50);
    registerButton->setStyleSheet(R"(
        QPushButton {
            background-color: white;
            color: #2980b9;
            border: 2px solid #2980b9;
            border-radius: 12px;
            font-size: 16px;
            font-weight: bold;
            font-family: 'Microsoft YaHei';
        }
        QPushButton:hover {
            background-color: #f0f8ff;
            border-color: #3498db;
        }
        QPushButton:pressed {
            background-color: #e8f4f8;
        }
    )");

    buttonLayout->addWidget(loginButton);
    buttonLayout->addWidget(registerButton);
    cardLayout->addLayout(buttonLayout);

    // 将卡片添加到主布局
    loginLayout->addStretch();
    loginLayout->addWidget(loginCard, 0, Qt::AlignCenter);
    loginLayout->addStretch();

    stackedWidget->addWidget(loginPage);

    // 2. 注册页面
    registerPage = new QWidget();
    QVBoxLayout *registerLayout = new QVBoxLayout(registerPage);
    registerLayout->setAlignment(Qt::AlignCenter);
    registerLayout->setContentsMargins(20, 20, 20, 20);

    // 创建注册卡片容器
    QWidget *registerCard = new QWidget();
    registerCard->setFixedWidth(480);
    registerCard->setStyleSheet(R"(
        QWidget {
            background-color: white;
            border-radius: 20px;
        }
    )");
    QVBoxLayout *regCardLayout = new QVBoxLayout(registerCard);
    regCardLayout->setContentsMargins(50, 50, 50, 50);
    regCardLayout->setSpacing(25);

    // 标题
    QLabel *registerTitle = new QLabel("📝 用户注册");
    registerTitle->setAlignment(Qt::AlignCenter);
    registerTitle->setFont(titleFont);
    registerTitle->setStyleSheet("color: #2c3e50; margin-bottom: 10px;");
    regCardLayout->addWidget(registerTitle);

    // 副标题
    QLabel *regSubTitle = new QLabel("创建新账户，开始您的购书之旅");
    regSubTitle->setAlignment(Qt::AlignCenter);
    regSubTitle->setFont(subFont);
    regSubTitle->setStyleSheet("color: #7f8c8d; margin-bottom: 30px;");
    regCardLayout->addWidget(regSubTitle);

    // 用户名输入框
    QLabel *regUsernameLabel = new QLabel("用户名");
    regUsernameLabel->setStyleSheet("color: #2c3e50; font-size: 14px; font-weight: 500; margin-bottom: 5px;");
    regCardLayout->addWidget(regUsernameLabel);
    
    regUsername = new QLineEdit();
    regUsername->setPlaceholderText("请输入用户名（3-20位字符）");
    regUsername->setMinimumHeight(50);
    regUsername->setStyleSheet(R"(
        QLineEdit {
            border: 2px solid #e1e8ed;
            border-radius: 12px;
            padding: 12px 18px;
            font-size: 15px;
            background-color: #f8fafc;
            color: #2c3e50;
            font-family: 'Microsoft YaHei';
        }
        QLineEdit:focus {
            border: 2px solid #2980b9;
            background-color: white;
        }
    )");
    regCardLayout->addWidget(regUsername);

    // 密码输入框
    QLabel *regPasswordLabel = new QLabel("密码");
    regPasswordLabel->setStyleSheet("color: #2c3e50; font-size: 14px; font-weight: 500; margin-top: 15px; margin-bottom: 5px;");
    regCardLayout->addWidget(regPasswordLabel);
    
    regPassword = new QLineEdit();
    regPassword->setPlaceholderText("请输入密码（至少6位）");
    regPassword->setEchoMode(QLineEdit::Password);
    regPassword->setMinimumHeight(50);
    regPassword->setStyleSheet(R"(
        QLineEdit {
            border: 2px solid #e1e8ed;
            border-radius: 12px;
            padding: 12px 18px;
            font-size: 15px;
            background-color: #f8fafc;
            color: #2c3e50;
            font-family: 'Microsoft YaHei';
        }
        QLineEdit:focus {
            border: 2px solid #2980b9;
            background-color: white;
        }
    )");
    regCardLayout->addWidget(regPassword);

    // 确认密码输入框
    QLabel *regConfirmPasswordLabel = new QLabel("确认密码");
    regConfirmPasswordLabel->setStyleSheet("color: #2c3e50; font-size: 14px; font-weight: 500; margin-top: 15px; margin-bottom: 5px;");
    regCardLayout->addWidget(regConfirmPasswordLabel);
    
    regConfirmPassword = new QLineEdit();
    regConfirmPassword->setPlaceholderText("请再次输入密码");
    regConfirmPassword->setEchoMode(QLineEdit::Password);
    regConfirmPassword->setMinimumHeight(50);
    regConfirmPassword->setStyleSheet(R"(
        QLineEdit {
            border: 2px solid #e1e8ed;
            border-radius: 12px;
            padding: 12px 18px;
            font-size: 15px;
            background-color: #f8fafc;
            color: #2c3e50;
            font-family: 'Microsoft YaHei';
        }
        QLineEdit:focus {
            border: 2px solid #2980b9;
            background-color: white;
        }
    )");
    regCardLayout->addWidget(regConfirmPassword);

    // 按钮容器
    QHBoxLayout *regButtonLayout = new QHBoxLayout();
    regButtonLayout->setSpacing(15);

    confirmRegisterBtn = new QPushButton("确认注册");
    confirmRegisterBtn->setMinimumHeight(50);
    confirmRegisterBtn->setStyleSheet(R"(
        QPushButton {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #2980b9,
                stop:1 #3498db);
            color: white;
            border: none;
            border-radius: 12px;
            font-size: 16px;
            font-weight: bold;
            font-family: 'Microsoft YaHei';
        }
        QPushButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #3498db,
                stop:1 #2980b9);
        }
        QPushButton:pressed {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #1f5f8f,
                stop:1 #2471a3);
        }
    )");

    backToLoginBtn = new QPushButton("返回登录");
    backToLoginBtn->setMinimumHeight(50);
    backToLoginBtn->setStyleSheet(R"(
        QPushButton {
            background-color: white;
            color: #2980b9;
            border: 2px solid #2980b9;
            border-radius: 12px;
            font-size: 16px;
            font-weight: bold;
            font-family: 'Microsoft YaHei';
        }
        QPushButton:hover {
            background-color: #f0f8ff;
            border-color: #3498db;
        }
        QPushButton:pressed {
            background-color: #e8f4f8;
        }
    )");

    regButtonLayout->addWidget(confirmRegisterBtn);
    regButtonLayout->addWidget(backToLoginBtn);
    regCardLayout->addLayout(regButtonLayout);

    // 将卡片添加到主布局
    registerLayout->addStretch();
    registerLayout->addWidget(registerCard, 0, Qt::AlignCenter);
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
    refreshButton = new QPushButton("刷新");
    refreshButton->setToolTip("刷新图书列表，查看最新上架的图书");
    cartButton = new QPushButton("购物车");
    ordersButton = new QPushButton("我的订单");
    profileButton = new QPushButton("个人中心");
    serviceButton = new QPushButton("客服");
    logoutButton = new QPushButton("退出登录");

    toolbarLayout->addWidget(searchInput);
    toolbarLayout->addWidget(searchButton);
    toolbarLayout->addWidget(refreshButton);
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
    recommendList->setSpacing(20);  // 增加间距，使卡片分布更均匀
    // 设置网格大小，确保卡片大小一致并能够均匀铺满
    // 加宽卡片到200px，增加高度到300px，确保能完整显示书名、价格、评分、收藏等信息
    recommendList->setGridSize(QSize(200, 300));
    // 禁用选中模式，避免未选中时显示蓝框
    recommendList->setSelectionMode(QAbstractItemView::NoSelection);
    // 设置统一item大小，确保均匀分布
    recommendList->setUniformItemSizes(true);
    // 注意：QListWidget在IconMode下对多行文本支持有限
    // 需要通过设置item的sizeHint和文本格式来确保完整显示
    // 设置卡片样式：背景色、边框、悬停效果
    recommendList->setStyleSheet(
        "QListWidget {"
        "    background-color: #ffffff;"
        "    border: none;"
        "}"
        "QListWidget::item {"
        "    background-color: #f8f9fa;"
        "    border: 1px solid #e0e0e0;"
        "    border-radius: 8px;"
        "    padding: 8px;"
        "    margin: 2px;"
        "    width: 200px;"
        "    height: 300px;"
        "    min-height: 300px;"
        "}"
        "QListWidget::item:hover {"
        "    background-color: #e3f2fd;"
        "    border: 2px solid #2196f3;"
        "    padding: 6px;"
        "}"
    );

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
    QHBoxLayout *bookInfoMainLayout = new QHBoxLayout(bookInfoWidget);
    
    // 封面图片标签
    bookCoverLabel = new QLabel();
    bookCoverLabel->setMinimumSize(200, 300);
    bookCoverLabel->setMaximumSize(200, 300);
    bookCoverLabel->setAlignment(Qt::AlignCenter);
    bookCoverLabel->setStyleSheet("border: 1px solid #ccc; background-color: #f5f5f5;");
    bookCoverLabel->setScaledContents(true);
    
    // 图书信息表单
    QWidget *bookInfoForm = new QWidget();
    QFormLayout *infoLayout = new QFormLayout(bookInfoForm);

    bookTitleLabel = new QLabel();
    QFont titleFont2 = bookTitleLabel->font();
    titleFont2.setPointSize(18);
    bookTitleLabel->setFont(titleFont2);

    bookAuthorLabel = new QLabel();
    bookPriceLabel = new QLabel();
    bookScoreLabel = new QLabel();
    bookFavoriteCountLabel = new QLabel();
    bookDescription = new QTextEdit();
    bookDescription->setReadOnly(true);

    infoLayout->addRow("书名:", bookTitleLabel);
    infoLayout->addRow("作者:", bookAuthorLabel);
    infoLayout->addRow("价格:", bookPriceLabel);
    infoLayout->addRow("评分:", bookScoreLabel);
    infoLayout->addRow("收藏量:", bookFavoriteCountLabel);
    infoLayout->addRow("描述:", bookDescription);
    
    bookInfoMainLayout->addWidget(bookCoverLabel);
    bookInfoMainLayout->addWidget(bookInfoForm, 1);

    QWidget *actionWidget = new QWidget();
    QHBoxLayout *actionLayout = new QHBoxLayout(actionWidget);

    quantitySpinBox = new QSpinBox();
    quantitySpinBox->setRange(1, 99);
    quantitySpinBox->setValue(1);

    addToCartBtn = new QPushButton("加入购物车");
    buyNowBtn = new QPushButton("立即购买");
    addToFavoriteBtn = new QPushButton("加入收藏");
    contactSellerBtn = new QPushButton("联系卖家");
    addReviewBtn = new QPushButton("评价商品");

    actionLayout->addWidget(new QLabel("数量:"));
    actionLayout->addWidget(quantitySpinBox);
    actionLayout->addStretch();
    actionLayout->addWidget(addToCartBtn);
    actionLayout->addWidget(buyNowBtn);
    actionLayout->addWidget(addToFavoriteBtn);
    actionLayout->addWidget(contactSellerBtn);
    actionLayout->addWidget(addReviewBtn);

    // 评论显示区域
    QWidget *reviewsWidget = new QWidget();
    QVBoxLayout *reviewsLayout = new QVBoxLayout(reviewsWidget);
    ratingStatsLabel = new QLabel("评分：暂无评分");
    reviewsDisplay = new QTextEdit();
    reviewsDisplay->setReadOnly(true);
    reviewsDisplay->setMaximumHeight(200);
    reviewsLayout->addWidget(new QLabel("商品评论："));
    reviewsLayout->addWidget(ratingStatsLabel);
    reviewsLayout->addWidget(reviewsDisplay);

    detailLayout->addWidget(detailHeader);
    detailLayout->addWidget(bookInfoWidget, 1);
    detailLayout->addWidget(actionWidget);
    detailLayout->addWidget(reviewsWidget);

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

    // 6. 支付页面
    paymentPage = new QWidget();
    QVBoxLayout *paymentLayout = new QVBoxLayout(paymentPage);
    paymentLayout->setSpacing(20);
    paymentLayout->setContentsMargins(30, 30, 30, 30);

    QLabel *paymentTitle = new QLabel("订单支付");
    QFont paymentTitleFont = paymentTitle->font();
    paymentTitleFont.setPointSize(20);
    paymentTitleFont.setBold(true);
    paymentTitle->setFont(paymentTitleFont);
    paymentTitle->setAlignment(Qt::AlignCenter);
    paymentLayout->addWidget(paymentTitle);

    // 订单信息
    QGroupBox *orderInfoGroup = new QGroupBox("订单信息");
    QVBoxLayout *orderInfoLayout = new QVBoxLayout(orderInfoGroup);
    
    paymentOrderIdLabel = new QLabel("订单号: -");
    paymentAmountLabel = new QLabel("支付金额: 0.00元");
    paymentOrderItems = new QTextEdit();
    paymentOrderItems->setReadOnly(true);
    paymentOrderItems->setMaximumHeight(150);
    
    orderInfoLayout->addWidget(paymentOrderIdLabel);
    orderInfoLayout->addWidget(paymentAmountLabel);
    orderInfoLayout->addWidget(new QLabel("订单明细:"));
    orderInfoLayout->addWidget(paymentOrderItems);
    paymentLayout->addWidget(orderInfoGroup);

    // 支付方式
    QGroupBox *paymentMethodGroup = new QGroupBox("支付方式");
    QVBoxLayout *paymentMethodLayout = new QVBoxLayout(paymentMethodGroup);
    
    paymentMethodCombo = new QComboBox();
    paymentMethodCombo->addItem("账户余额支付");
    paymentMethodCombo->addItem("其他支付方式（待实现）");
    paymentMethodLayout->addWidget(paymentMethodCombo);
    paymentLayout->addWidget(paymentMethodGroup);

    // 账户信息
    QGroupBox *accountInfoGroup = new QGroupBox("账户信息");
    QVBoxLayout *accountInfoLayout = new QVBoxLayout(accountInfoGroup);
    
    paymentBalanceLabel = new QLabel("账户余额: 0.00元");
    accountInfoLayout->addWidget(paymentBalanceLabel);
    
    // 优惠券信息
    paymentCoupon30Label = new QLabel("30元优惠券: 0张");
    paymentCoupon50Label = new QLabel("50元优惠券: 0张");
    accountInfoLayout->addWidget(paymentCoupon30Label);
    accountInfoLayout->addWidget(paymentCoupon50Label);
    
    // 优惠券选择
    QGroupBox *couponGroup = new QGroupBox("使用优惠券");
    QVBoxLayout *couponLayout = new QVBoxLayout(couponGroup);
    
    paymentCouponCombo = new QComboBox();
    paymentCouponCombo->addItem("不使用优惠券", "");
    paymentCouponCombo->addItem("使用30元优惠券", "30");
    paymentCouponCombo->addItem("使用50元优惠券", "50");
    couponLayout->addWidget(paymentCouponCombo);
    paymentLayout->addWidget(couponGroup);
    
    paymentLayout->addWidget(accountInfoGroup);

    // 按钮
    QHBoxLayout *paymentButtonLayout = new QHBoxLayout();
    confirmPaymentBtn = new QPushButton("确认支付");
    confirmPaymentBtn->setStyleSheet("background-color: #4CAF50; color: white; padding: 10px; font-size: 14px;");
    cancelPaymentBtn = new QPushButton("取消支付");
    cancelPaymentBtn->setStyleSheet("background-color: #f44336; color: white; padding: 10px; font-size: 14px;");
    
    paymentButtonLayout->addStretch();
    paymentButtonLayout->addWidget(cancelPaymentBtn);
    paymentButtonLayout->addWidget(confirmPaymentBtn);
    paymentLayout->addLayout(paymentButtonLayout);

    paymentLayout->addStretch();
    stackedWidget->addWidget(paymentPage);

    // 6. 订单页面
    ordersPage = new QWidget();
    QVBoxLayout *ordersLayout = new QVBoxLayout(ordersPage);

    QWidget *ordersHeader = new QWidget();
    QHBoxLayout *ordersHeaderLayout = new QHBoxLayout(ordersHeader);

    refreshOrdersBtn = new QPushButton("刷新订单");
    cancelOrderBtn = new QPushButton("取消订单");
    cancelOrderBtn->setStyleSheet("background-color: #e74c3c; color: white; font-weight: bold;");
    confirmReceiveBtn = new QPushButton("确认收货");
    confirmReceiveBtn->setStyleSheet("background-color: #2ecc71; color: white; font-weight: bold;");
    backFromOrdersBtn = new QPushButton("返回");
    
    ordersHeaderLayout->addWidget(refreshOrdersBtn);
    ordersHeaderLayout->addWidget(cancelOrderBtn);
    ordersHeaderLayout->addWidget(confirmReceiveBtn);
    ordersHeaderLayout->addStretch();
    ordersHeaderLayout->addWidget(backFromOrdersBtn);

    ordersTable = new QTableWidget();
    ordersTable->setColumnCount(6);
    ordersTable->setHorizontalHeaderLabels(QStringList() << "订单号" << "日期" << "商品" << "总金额" << "状态" << "发货时间");
    ordersTable->horizontalHeader()->setStretchLastSection(true);
    ordersTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ordersTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    // 订单表格显示优化：提升高度与行高，避免操作栏字体被裁剪
    ordersTable->setMinimumHeight(520);
    ordersTable->verticalHeader()->setDefaultSectionSize(52);
    ordersTable->setWordWrap(false);
    ordersTable->setAlternatingRowColors(true);
    selectedOrderRow = -1;

    ordersLayout->addWidget(ordersHeader);
    ordersLayout->addWidget(ordersTable, 1);

    stackedWidget->addWidget(ordersPage);

    // 7. 个人资料页面
    profilePage = new QWidget();
    QVBoxLayout *profileLayout = new QVBoxLayout(profilePage);
    profileLayout->setSpacing(0);
    profileLayout->setContentsMargins(0, 0, 0, 0);

    // 顶部Banner区域
    profileBanner = new QWidget();
    profileBanner->setMinimumHeight(200);  // 加高banner高度从120px到160px
    profileBanner->setMaximumHeight(200);
    profileBanner->setStyleSheet(
        "QWidget {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
        "        stop:0 #667eea, stop:1 #764ba2);"
        "    border: none;"
        "}"
    );
    QHBoxLayout *bannerLayout = new QHBoxLayout(profileBanner);
    bannerLayout->setContentsMargins(30, 25, 30, 25);  // 增加上下内边距，确保会员卡片完全显示
    bannerLayout->setSpacing(20);

    // 左侧欢迎语
    welcomeLabel = new QLabel("你好，jtr！");
    welcomeLabel->setStyleSheet(
        "QLabel {"
        "    color: white;"
        "    font-size: 28px;"
        "    font-weight: bold;"
        "    background: transparent;"
        "    border: none;"
        "}"
    );
    bannerLayout->addWidget(welcomeLabel);
    bannerLayout->addStretch();

    // 右侧会员卡片
    memberCard = new QWidget();
    memberCard->setStyleSheet(
        "QWidget {"
        "    background-color: rgba(255, 255, 255, 0.95);"
        "    border-radius: 12px;"
        "    padding: 18px 22px;"
        "    min-width: 290px;"
        "    max-width: 290px;"
        "    min-height: 130px;"
        "    max-height: 130px;"
        "}"
    );
    // 会员卡片主布局：左侧信息，右侧按钮
    QHBoxLayout *memberCardMainLayout = new QHBoxLayout(memberCard);
    memberCardMainLayout->setContentsMargins(18, 12, 18, 12);
    memberCardMainLayout->setSpacing(12);

    // 左侧：会员等级和累计充值信息（垂直布局）
    QVBoxLayout *memberInfoLayout = new QVBoxLayout();
    memberInfoLayout->setContentsMargins(0, 0, 0, 0);
    memberInfoLayout->setSpacing(0);  // 再次缩小行间距从2px到0px，使两行文字更紧凑

    memberCardLabel = new QLabel("金卡会员 9.0折");
    memberCardLabel->setStyleSheet(
        "QLabel {"
        "    color: #333333;"
        "    font-size: 18px;"
        "    font-weight: bold;"
        "    background: transparent;"
        "    border: none;"
        "    margin: 0px;"
        "    padding: 0px;"
        "}"
    );
    memberCardLabel->setWordWrap(false);
    memberCardLabel->setContentsMargins(0, 0, 0, 0);
    memberInfoLayout->addWidget(memberCardLabel);

    // 累计充值金额标签（小字显示在会员等级下方）
    memberCardRechargeLabel = new QLabel("累计充值: ¥0.00");
    memberCardRechargeLabel->setStyleSheet(
        "QLabel {"
        "    color: #666666;"
        "    font-size: 12px;"
        "    background: transparent;"
        "    border: none;"
        "    margin: 0px;"
        "    padding: 0px;"
        "    margin-top: -2px;"
        "}"
    );
    memberCardRechargeLabel->setWordWrap(false);
    memberCardRechargeLabel->setContentsMargins(0, 0, 0, 0);
    memberInfoLayout->addWidget(memberCardRechargeLabel);
    memberInfoLayout->addStretch();

    memberCardMainLayout->addLayout(memberInfoLayout);

    // 右侧：查看会员等级规则按钮
    levelInfoBtn = new QPushButton("查看会员等级规则");
    levelInfoBtn->setFixedHeight(20);  // 减小高度从28px到22px
    levelInfoBtn->setToolTip("点击查看会员等级说明");
    levelInfoBtn->setStyleSheet(
        "QPushButton {"
        "    background-color: #3498db;"
        "    color: white;"
        "    border-radius: 5px;"
        "    font-weight: normal;"
        "    font-size: 10px;"
        "    padding: 2px 6px;"
        "    border: 1px solid #2980b9;"
        "    min-width: 80px;"
        "    max-width: 120px;"
        "}"
        "QPushButton:hover {"
        "    background-color: #2980b9;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #21618c;"
        "}"
    );
    memberCardMainLayout->addWidget(levelInfoBtn);

    bannerLayout->addWidget(memberCard);

    profileLayout->addWidget(profileBanner);

    QWidget *profileHeader = new QWidget();
    QHBoxLayout *profileHeaderLayout = new QHBoxLayout(profileHeader);

    backFromProfileBtn = new QPushButton("返回");
    profileHeaderLayout->addWidget(backFromProfileBtn);
    profileHeaderLayout->addStretch();

    QTabWidget *profileTabs = new QTabWidget();

    // 基本信息标签页
    QWidget *basicInfoTab = new QWidget();
    QHBoxLayout *basicInfoMainLayout = new QHBoxLayout(basicInfoTab);
    basicInfoMainLayout->setSpacing(20);
    basicInfoMainLayout->setContentsMargins(20, 20, 20, 20);
    
    // 左侧：个人信息卡片
    QWidget *infoCard = new QWidget();
    infoCard->setStyleSheet(
        "QWidget {"
        "    background-color: #ffffff;"
        "    border: 1px solid #e0e0e0;"
        "    border-radius: 8px;"
        "    padding: 20px;"
        "}"
    );
    QVBoxLayout *infoCardLayout = new QVBoxLayout(infoCard);
    infoCardLayout->setSpacing(20);
    infoCardLayout->setContentsMargins(20, 20, 20, 20);
    infoCardLayout->setAlignment(Qt::AlignTop);  // 顶部对齐，避免内容被拉伸

    profileUsername = new QLineEdit();
    profileUsername->setReadOnly(true);  // 用户名不能修改
    profilePhone = new QLineEdit();
    profileEmail = new QLineEdit();
    profileAddress = new QLineEdit();

    // 设置输入框样式：底部单线条或浅灰色填充块
    QString inputBoxStyle = 
        "QLineEdit {"
        "    background-color: #f5f5f5;"
        "    border: none;"
        "    border-bottom: 2px solid #3498db;"
        "    padding: 10px 5px;"
        "    font-size: 14px;"
        "    border-radius: 0px;"
        "    margin: 0px;"
        "}"
        "QLineEdit:focus {"
        "    background-color: #ffffff;"
        "    border-bottom: 2px solid #2980b9;"
        "}";
    
    profilePhone->setStyleSheet(inputBoxStyle);
    profileEmail->setStyleSheet(inputBoxStyle);
    profileAddress->setStyleSheet(inputBoxStyle);
    
    // 电话字段：标签在上，输入框在下
    QVBoxLayout *phoneLayout = new QVBoxLayout();
    phoneLayout->setSpacing(8);
    phoneLayout->setContentsMargins(0, 0, 0, 0);
    QLabel *phoneLabel = new QLabel("电话");
    phoneLabel->setStyleSheet("QLabel { color: #666666; font-size: 13px; margin: 0px; padding: 0px; }");
    phoneLayout->addWidget(phoneLabel);
    profilePhone->setMinimumHeight(35);
    profilePhone->setMaximumHeight(35);
    phoneLayout->addWidget(profilePhone);
    infoCardLayout->addLayout(phoneLayout);
    
    // 邮箱字段：标签在上，输入框在下
    QVBoxLayout *emailLayout = new QVBoxLayout();
    emailLayout->setSpacing(8);
    emailLayout->setContentsMargins(0, 0, 0, 0);
    QLabel *emailLabel = new QLabel("邮箱");
    emailLabel->setStyleSheet("QLabel { color: #666666; font-size: 13px; margin: 0px; padding: 0px; }");
    emailLayout->addWidget(emailLabel);
    profileEmail->setMinimumHeight(35);
    profileEmail->setMaximumHeight(35);
    emailLayout->addWidget(profileEmail);
    infoCardLayout->addLayout(emailLayout);
    
    // 地址字段：标签在上，输入框在下
    QVBoxLayout *addressLayout = new QVBoxLayout();
    addressLayout->setSpacing(8);
    addressLayout->setContentsMargins(0, 0, 0, 0);
    QLabel *addressLabel = new QLabel("地址");
    addressLabel->setStyleSheet("QLabel { color: #666666; font-size: 13px; margin: 0px; padding: 0px; }");
    addressLayout->addWidget(addressLabel);
    profileAddress->setMinimumHeight(35);
    profileAddress->setMaximumHeight(35);
    addressLayout->addWidget(profileAddress);
    infoCardLayout->addLayout(addressLayout);
    
    // 添加间距，避免文本框重合
    infoCardLayout->addSpacing(20);

    // 修改密码区域
    QGroupBox *passwordGroup = new QGroupBox("修改密码");
    passwordGroup->setStyleSheet(
        "QGroupBox {"
        "    font-size: 14px;"
        "    font-weight: bold;"
        "    border: 1px solid #e0e0e0;"
        "    border-radius: 6px;"
        "    margin-top: 10px;"
        "    padding-top: 15px;"
        "}"
        "QGroupBox::title {"
        "    subcontrol-origin: margin;"
        "    left: 10px;"
        "    padding: 0 5px;"
        "}"
    );
    QVBoxLayout *passwordLayout = new QVBoxLayout(passwordGroup);
    passwordLayout->setSpacing(15);
    passwordLayout->setContentsMargins(15, 20, 15, 15);
    
    oldPasswordEdit = new QLineEdit();
    oldPasswordEdit->setEchoMode(QLineEdit::Password);
    oldPasswordEdit->setPlaceholderText("请输入当前密码");
    oldPasswordEdit->setStyleSheet(inputBoxStyle);
    
    newPasswordEdit = new QLineEdit();
    newPasswordEdit->setEchoMode(QLineEdit::Password);
    newPasswordEdit->setPlaceholderText("请输入新密码");
    newPasswordEdit->setStyleSheet(inputBoxStyle);
    
    confirmPasswordEdit = new QLineEdit();
    confirmPasswordEdit->setEchoMode(QLineEdit::Password);
    confirmPasswordEdit->setPlaceholderText("请再次输入新密码");
    confirmPasswordEdit->setStyleSheet(inputBoxStyle);
    
    // 密码字段：标签在上，输入框在下
    QVBoxLayout *oldPwdLayout = new QVBoxLayout();
    oldPwdLayout->setSpacing(8);
    oldPwdLayout->setContentsMargins(0, 0, 0, 0);
    QLabel *oldPwdLabel = new QLabel("当前密码");
    oldPwdLabel->setStyleSheet("QLabel { color: #666666; font-size: 13px; margin: 0px; padding: 0px; }");
    oldPwdLayout->addWidget(oldPwdLabel);
    oldPasswordEdit->setMinimumHeight(35);
    oldPasswordEdit->setMaximumHeight(35);
    oldPwdLayout->addWidget(oldPasswordEdit);
    passwordLayout->addLayout(oldPwdLayout);
    
    QVBoxLayout *newPwdLayout = new QVBoxLayout();
    newPwdLayout->setSpacing(8);
    newPwdLayout->setContentsMargins(0, 0, 0, 0);
    QLabel *newPwdLabel = new QLabel("新密码");
    newPwdLabel->setStyleSheet("QLabel { color: #666666; font-size: 13px; margin: 0px; padding: 0px; }");
    newPwdLayout->addWidget(newPwdLabel);
    newPasswordEdit->setMinimumHeight(35);
    newPasswordEdit->setMaximumHeight(35);
    newPwdLayout->addWidget(newPasswordEdit);
    passwordLayout->addLayout(newPwdLayout);
    
    QVBoxLayout *confirmPwdLayout = new QVBoxLayout();
    confirmPwdLayout->setSpacing(8);
    confirmPwdLayout->setContentsMargins(0, 0, 0, 0);
    QLabel *confirmPwdLabel = new QLabel("确认新密码");
    confirmPwdLabel->setStyleSheet("QLabel { color: #666666; font-size: 13px; margin: 0px; padding: 0px; }");
    confirmPwdLayout->addWidget(confirmPwdLabel);
    confirmPasswordEdit->setMinimumHeight(35);
    confirmPasswordEdit->setMaximumHeight(35);
    confirmPwdLayout->addWidget(confirmPasswordEdit);
    passwordLayout->addLayout(confirmPwdLayout);
    
    changePasswordBtn = new QPushButton("修改密码");
    changePasswordBtn->setStyleSheet(
        "QPushButton {"
        "    background-color: #3498db;"
        "    color: white;"
        "    padding: 10px;"
        "    font-size: 14px;"
        "    font-weight: bold;"
        "    border-radius: 6px;"
        "    border: none;"
        "}"
        "QPushButton:hover {"
        "    background-color: #2980b9;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #21618c;"
        "}"
    );
    changePasswordBtn->setFixedHeight(40);
    passwordLayout->addWidget(changePasswordBtn);
    
    infoCardLayout->addWidget(passwordGroup);

    // 更新信息按钮
    updateProfileBtn = new QPushButton("更新信息");
    updateProfileBtn->setStyleSheet(
        "QPushButton {"
        "    background-color: #3498db;"
        "    color: white;"
        "    padding: 10px 20px;"
        "    font-size: 14px;"
        "    font-weight: bold;"
        "    border-radius: 6px;"
        "    border: none;"
        "}"
        "QPushButton:hover {"
        "    background-color: #2980b9;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #21618c;"
        "}"
    );
    updateProfileBtn->setFixedHeight(40);
    infoCardLayout->addWidget(updateProfileBtn);
    
    basicInfoMainLayout->addWidget(infoCard, 1);
    
    // 右侧：账户资产区域
    QWidget *assetCard = new QWidget();
    assetCard->setStyleSheet(
        "QWidget {"
        "    background-color: #ffffff;"
        "    border: 1px solid #e0e0e0;"
        "    border-radius: 8px;"
        "    padding: 20px;"
        "}"
    );
    QVBoxLayout *assetLayout = new QVBoxLayout(assetCard);
    assetLayout->setSpacing(20);
    assetLayout->setContentsMargins(20, 20, 20, 20);
    assetLayout->setAlignment(Qt::AlignTop);  // 顶部对齐，避免内容被拉伸
    
    profileLevelLabel = new QLabel();
    profileBalanceLabel = new QLabel();
    
    // 账户余额：大数字显示
    QVBoxLayout *balanceLayout = new QVBoxLayout();
    balanceLayout->setSpacing(10);
    QLabel *balanceTitleLabel = new QLabel("账户余额");
    balanceTitleLabel->setStyleSheet("QLabel { color: #666666; font-size: 13px; }");
    balanceLayout->addWidget(balanceTitleLabel);
    
    profileBalanceLabel->setStyleSheet(
        "QLabel {"
        "    color: #2c3e50;"
        "    font-size: 32px;"
        "    font-weight: bold;"
        "    background: transparent;"
        "    border: none;"
        "    margin: 0px;"
        "    padding: 0px;"
        "}"
    );
    balanceLayout->addWidget(profileBalanceLabel);
    
    // 充值按钮：圆角样式
    rechargeBtn = new QPushButton("充值");
    rechargeBtn->setStyleSheet(
        "QPushButton {"
        "    background-color: #27ae60;"
        "    color: white;"
        "    padding: 10px 20px;"
        "    font-size: 14px;"
        "    font-weight: bold;"
        "    border-radius: 6px;"
        "    border: none;"
        "}"
        "QPushButton:hover {"
        "    background-color: #229954;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #1e8449;"
        "}"
    );
    rechargeBtn->setFixedHeight(40);
    balanceLayout->addWidget(rechargeBtn);
    assetLayout->addLayout(balanceLayout);
    
    // 积分显示：在余额下方
    QVBoxLayout *pointsLayout = new QVBoxLayout();
    pointsLayout->setSpacing(5);
    QLabel *pointsTitleLabel = new QLabel("积分");
    pointsTitleLabel->setStyleSheet("QLabel { color: #666666; font-size: 13px; }");
    pointsLayout->addWidget(pointsTitleLabel);
    profilePointsLabel = new QLabel("0");
    profilePointsLabel->setStyleSheet(
        "QLabel {"
        "    color: #2c3e50;"
        "    font-size: 20px;"
        "    font-weight: bold;"
        "    background: transparent;"
        "    border: none;"
        "    margin: 0px;"
        "    padding: 0px;"
        "}"
    );
    pointsLayout->addWidget(profilePointsLabel);
    assetLayout->addLayout(pointsLayout);
    
    // 参与抽奖按钮（无需修改功能，但优化样式）
    lotteryBtn = new QPushButton("参与抽奖（需3积分）");
    lotteryBtn->setEnabled(false);  // 默认禁用，积分满3后启用
    lotteryBtn->setStyleSheet(
        "QPushButton {"
        "    background-color: #95a5a6;"
        "    color: white;"
        "    padding: 10px;"
        "    font-size: 14px;"
        "    font-weight: bold;"
        "    border-radius: 6px;"
        "    border: none;"
        "}"
        "QPushButton:enabled {"
        "    background-color: #4CAF50;"
        "}"
        "QPushButton:enabled:hover {"
        "    background-color: #45a049;"
        "}"
        "QPushButton:enabled:pressed {"
        "    background-color: #3d8b40;"
        "}"
    );
    lotteryBtn->setFixedHeight(40);
    assetLayout->addWidget(lotteryBtn);
    
    basicInfoMainLayout->addWidget(assetCard, 1);

    // 收藏夹标签页
    QWidget *favoriteTab = new QWidget();
    QVBoxLayout *favoriteLayout = new QVBoxLayout(favoriteTab);

    favoriteList = new QListWidget();
    favoriteLayout->addWidget(favoriteList);

    // 卖家认证标签页
    QWidget *sellerCertTab = new QWidget();
    QVBoxLayout *sellerCertLayout = new QVBoxLayout(sellerCertTab);
    sellerCertLayout->setSpacing(15);
    sellerCertLayout->setContentsMargins(20, 20, 20, 20);

    // 认证状态显示
    QGroupBox *statusGroup = new QGroupBox("认证状态");
    QVBoxLayout *statusLayout = new QVBoxLayout(statusGroup);
    sellerStatusLabel = new QLabel("未认证");
    sellerStatusLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: #e74c3c;");
    statusLayout->addWidget(sellerStatusLabel);
    sellerCertLayout->addWidget(statusGroup);

    // 营业执照上传区域
    QGroupBox *licenseGroup = new QGroupBox("营业执照上传");
    QVBoxLayout *licenseLayout = new QVBoxLayout(licenseGroup);
    
    // 图片预览区域
    licenseImageLabel = new QLabel();
    licenseImageLabel->setMinimumHeight(200);
    licenseImageLabel->setMaximumHeight(300);
    licenseImageLabel->setAlignment(Qt::AlignCenter);
    licenseImageLabel->setStyleSheet("border: 2px dashed #bdc3c7; border-radius: 8px; background-color: #ecf0f1;");
    licenseImageLabel->setText("请选择营业执照图片\n(支持 JPG、PNG 格式)");
    licenseImageLabel->setWordWrap(true);
    licenseLayout->addWidget(licenseImageLabel);

    // 选择图片按钮
    selectLicenseBtn = new QPushButton("选择营业执照图片");
    selectLicenseBtn->setStyleSheet("background-color: #3498db; color: white; padding: 10px; font-size: 14px;");
    licenseLayout->addWidget(selectLicenseBtn);

    // 提示信息
    QLabel *tipLabel = new QLabel("提示：请先选择营业执照图片，然后点击提交按钮保存到数据库");
    tipLabel->setStyleSheet("color: #7f8c8d; font-size: 12px; padding: 10px;");
    tipLabel->setWordWrap(true);
    licenseLayout->addWidget(tipLabel);
    
    // 申请成为卖家按钮（提交图片到数据库）
    applySellerBtn = new QPushButton("提交营业执照");
    applySellerBtn->setStyleSheet("background-color: #27ae60; color: white; padding: 12px; font-size: 16px; font-weight: bold;");
    applySellerBtn->setEnabled(false);  // 初始状态禁用，选择图片后启用
    licenseLayout->addWidget(applySellerBtn);

    sellerCertLayout->addWidget(licenseGroup);
    sellerCertLayout->addStretch();

    profileTabs->addTab(basicInfoTab, "基本信息");
    profileTabs->addTab(favoriteTab, "我的收藏");
    profileTabs->addTab(sellerCertTab, "卖家认证");

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

// 应用统一样式（参考bookmerchant风格）
void Purchaser::applyStyle()
{
    // 定义颜色常量（与bookmerchant保持一致）
    const QColor PRIMARY_COLOR(41, 128, 185);      // 主蓝色
    const QColor SECONDARY_COLOR(52, 152, 219);    // 次要蓝色
    const QColor ACCENT_COLOR(46, 204, 113);        // 强调绿色
    const QColor WARNING_COLOR(231, 76, 60);       // 警告红色
    const QColor BG_COLOR(245, 247, 250);           // 背景色
    const QColor TEXT_COLOR(44, 62, 80);           // 文本色
    const QColor BORDER_COLOR(225, 232, 237);      // 边框色
    
    QString styleSheet = QString(R"(
        /* 主窗口背景 */
        QMainWindow {
            background-color: %1;
        }
        
        /* 堆叠窗口 */
        QStackedWidget {
            background-color: %1;
        }
        
        /* 输入框统一样式 */
        QLineEdit {
            border: 2px solid %2;
            border-radius: 10px;
            padding: 12px 20px;
            font-size: 15px;
            background: #f8fafc;
            color: %3;
            font-family: 'Microsoft YaHei', 'Segoe UI', Arial;
        }
        
        QLineEdit:focus {
            border-color: %4;
            background: white;
        }
        
        QLineEdit:disabled {
            background: #e9ecef;
            color: #6c757d;
        }
        
        /* 按钮统一样式 */
        QPushButton {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 %4,
                stop:1 %5);
            color: white;
            border: none;
            border-radius: 10px;
            font-size: 16px;
            font-weight: bold;
            padding: 12px 24px;
            font-family: 'Microsoft YaHei', 'Segoe UI', Arial;
        }
        
        QPushButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 %6,
                stop:1 %7);
        }
        
        QPushButton:pressed {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 %8,
                stop:1 %9);
        }
        
        QPushButton:disabled {
            background: #bdc3c7;
            color: #7f8c8d;
        }
        
        /* 表格统一样式 */
        QTableWidget {
            background-color: white;
            border: 1px solid %2;
            border-radius: 8px;
            gridline-color: %2;
            font-size: 14px;
            color: %3;
        }
        
        QTableWidget::item {
            padding: 8px;
            border: none;
        }
        
        QTableWidget::item:selected {
            background-color: %4;
            color: white;
        }
        
        QHeaderView::section {
            background-color: #f8f9fa;
            color: %3;
            padding: 10px;
            border: none;
            border-bottom: 2px solid %2;
            font-weight: bold;
            font-size: 14px;
        }
        
        /* 列表控件统一样式 */
        QListWidget {
            background-color: white;
            border: 1px solid %2;
            border-radius: 8px;
            padding: 5px;
            font-size: 14px;
            color: %3;
        }
        
        QListWidget::item {
            padding: 10px;
            border-radius: 5px;
            margin: 2px;
        }
        
        QListWidget::item:hover {
            background-color: #f1f8ff;
        }
        
        QListWidget::item:selected {
            background-color: %4;
            color: white;
        }
        
        /* 树形控件统一样式 */
        QTreeWidget {
            background-color: white;
            border: 1px solid %2;
            border-radius: 8px;
            font-size: 14px;
            color: %3;
        }
        
        QTreeWidget::item {
            padding: 8px;
        }
        
        QTreeWidget::item:hover {
            background-color: #f1f8ff;
        }
        
        QTreeWidget::item:selected {
            background-color: %4;
            color: white;
        }
        
        /* 文本编辑框统一样式 */
        QTextEdit {
            border: 2px solid %2;
            border-radius: 8px;
            padding: 10px;
            background: white;
            font-size: 14px;
            color: %3;
            font-family: 'Microsoft YaHei', 'Segoe UI', Arial;
        }
        
        QTextEdit:focus {
            border-color: %4;
        }
        
        /* 标签统一样式 */
        QLabel {
            color: %3;
            font-size: 14px;
            font-family: 'Microsoft YaHei', 'Segoe UI', Arial;
        }
        
        /* 组合框统一样式 */
        QComboBox {
            border: 2px solid %2;
            border-radius: 10px;
            padding: 10px 20px;
            background: #f8fafc;
            font-size: 14px;
            color: %3;
            font-family: 'Microsoft YaHei', 'Segoe UI', Arial;
        }
        
        QComboBox:focus {
            border-color: %4;
            background: white;
        }
        
        QComboBox::drop-down {
            border: none;
            width: 30px;
        }
        
        QComboBox::down-arrow {
            width: 12px;
            height: 12px;
        }
        
        /* 复选框统一样式 */
        QCheckBox {
            font-size: 14px;
            color: %3;
            spacing: 8px;
        }
        
        QCheckBox::indicator {
            width: 18px;
            height: 18px;
            border: 2px solid %2;
            border-radius: 4px;
        }
        
        QCheckBox::indicator:checked {
            background-color: %4;
            border-color: %4;
        }
        
        /* 微调框统一样式 */
        QSpinBox {
            border: 2px solid %2;
            border-radius: 8px;
            padding: 8px;
            background: #f8fafc;
            font-size: 14px;
            color: %3;
        }
        
        QSpinBox:focus {
            border-color: %4;
            background: white;
        }
        
        /* 分组框统一样式 */
        QGroupBox {
            border: 2px solid %2;
            border-radius: 10px;
            margin-top: 10px;
            padding-top: 15px;
            font-size: 16px;
            font-weight: bold;
            color: %3;
        }
        
        QGroupBox::title {
            subcontrol-origin: margin;
            subcontrol-position: top left;
            padding: 0 10px;
            background-color: %1;
        }
        
        /* 滚动条统一样式 */
        QScrollBar:vertical {
            background: #f1f1f1;
            width: 12px;
            border-radius: 6px;
        }
        
        QScrollBar::handle:vertical {
            background: %4;
            min-height: 30px;
            border-radius: 6px;
        }
        
        QScrollBar::handle:vertical:hover {
            background: %5;
        }
        
        QScrollBar:horizontal {
            background: #f1f1f1;
            height: 12px;
            border-radius: 6px;
        }
        
        QScrollBar::handle:horizontal {
            background: %4;
            min-width: 30px;
            border-radius: 6px;
        }
        
        QScrollBar::handle:horizontal:hover {
            background: %5;
        }
    )").arg(BG_COLOR.name())
      .arg(BORDER_COLOR.name())
      .arg(TEXT_COLOR.name())
      .arg(PRIMARY_COLOR.name())
      .arg(SECONDARY_COLOR.name())
      .arg(PRIMARY_COLOR.darker(120).name())
      .arg(SECONDARY_COLOR.darker(120).name())
      .arg(PRIMARY_COLOR.darker(130).name())
      .arg(SECONDARY_COLOR.darker(130).name());
    
    // 应用样式到主窗口
    this->setStyleSheet(styleSheet);
    
    // 登录页面特殊样式（渐变背景）
    loginPage->setStyleSheet(QString(R"(
        QWidget {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 %1,
                stop:0.5 %2,
                stop:1 %3);
        }
    )").arg(PRIMARY_COLOR.name())
      .arg(SECONDARY_COLOR.name())
      .arg(PRIMARY_COLOR.darker(110).name()));
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
    connect(refreshButton, &QPushButton::clicked, this, &Purchaser::onRefreshClicked);
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
    connect(addReviewBtn, &QPushButton::clicked, this, &Purchaser::onAddReviewClicked);
    
    // 联系卖家相关
    connect(contactSellerBtn, &QPushButton::clicked, this, &Purchaser::onContactSellerClicked);

    // 订单相关
    connect(ordersButton, &QPushButton::clicked, this, &Purchaser::onViewOrderClicked);

    // 个人信息相关
    connect(updateProfileBtn, &QPushButton::clicked, this, &Purchaser::onUpdateProfileClicked);
    connect(changePasswordBtn, &QPushButton::clicked, this, &Purchaser::onChangePasswordClicked);
    connect(profileButton, &QPushButton::clicked, this, &Purchaser::onViewProfileClicked);
    connect(rechargeBtn, &QPushButton::clicked, this, &Purchaser::onRechargeClicked);
    connect(levelInfoBtn, &QPushButton::clicked, this, &Purchaser::onLevelInfoClicked);
    connect(selectLicenseBtn, &QPushButton::clicked, this, &Purchaser::onSelectLicenseImageClicked);
    connect(applySellerBtn, &QPushButton::clicked, this, &Purchaser::onApplySellerClicked);
    connect(lotteryBtn, &QPushButton::clicked, this, &Purchaser::onLotteryClicked);

    // 支付相关（支付页按钮必须连接，否则点击无效）
    connect(confirmPaymentBtn, &QPushButton::clicked, this, &Purchaser::onConfirmPaymentClicked);
    connect(cancelPaymentBtn, &QPushButton::clicked, this, &Purchaser::onCancelPaymentClicked);

    // 客服相关
    connect(serviceButton, &QPushButton::clicked, this, &Purchaser::onCustomerServiceClicked);

    // 找到客服页面的发送按钮并连接
    QList<QPushButton*> serviceButtons = servicePage->findChildren<QPushButton*>();
    for (auto btn : serviceButtons) {
        if (btn->text() == "发送反馈") {
            connect(btn, &QPushButton::clicked, this, &Purchaser::onSendFeedbackClicked);
        }
    }

    // 订单操作
    connect(refreshOrdersBtn, &QPushButton::clicked, this, &Purchaser::onViewOrderClicked);
    connect(cancelOrderBtn, &QPushButton::clicked, this, &Purchaser::onCancelOrderClicked);
    connect(confirmReceiveBtn, &QPushButton::clicked, this, &Purchaser::onConfirmReceiveClicked);
    connect(ordersTable, &QTableWidget::cellClicked, this, &Purchaser::onOrderTableCellClicked);
    
    // 返回按钮
    connect(backToMainBtn, &QPushButton::clicked, this, &Purchaser::showMainPage);
    connect(backFromCartBtn, &QPushButton::clicked, this, &Purchaser::showMainPage);
    connect(backFromOrdersBtn, &QPushButton::clicked, this, &Purchaser::showMainPage);
    connect(backFromProfileBtn, &QPushButton::clicked, this, &Purchaser::showMainPage);
    connect(backFromServiceBtn, &QPushButton::clicked, this, &Purchaser::showMainPage);
}

void Purchaser::onRechargeClicked()
{
    if (!isLoggedIn || !currentUser) {
        QMessageBox::warning(this, "操作失败", "请先登录");
        showLoginPage();
        return;
    }

    bool ok = false;
    double amount = QInputDialog::getDouble(
        this,
        "充值",
        "请输入充值金额：",
        100.0,
        0.01,
        9999999.0,
        2,
        &ok
    );

    if (!ok) return;

    // 显示二维码支付对话框
    QDialog *paymentDialog = new QDialog(this);
    paymentDialog->setWindowTitle("微信支付");
    paymentDialog->setMinimumWidth(450);
    paymentDialog->setMinimumHeight(700);
    paymentDialog->setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);

    QVBoxLayout *dialogLayout = new QVBoxLayout(paymentDialog);
    dialogLayout->setAlignment(Qt::AlignCenter);

    // 显示二维码图片
    QLabel *qrCodeLabel = new QLabel();
    QString imagePath = "F:/Qt/project/bookmall (3)/bookmall/purchaser1/微信图片_20251220100916_21_74.jpg";

    // 检查图片文件是否存在，如果不存在尝试相对路径
    QFileInfo fileInfo(imagePath);
    if (!fileInfo.exists()) {
        qDebug() << "绝对路径图片文件不存在:" << imagePath;
        // 尝试使用相对路径
        imagePath = "./微信图片_20251220100916_21_74.jpg";
        fileInfo.setFile(imagePath);
        if (!fileInfo.exists()) {
            qDebug() << "相对路径图片文件也不存在:" << imagePath;
        }
    }

    QPixmap qrCodePixmap;
    bool imageLoaded = false;

    // 如果文件存在，尝试加载
    if (fileInfo.exists()) {
        imageLoaded = qrCodePixmap.load(imagePath);
        if (imageLoaded) {
            qDebug() << "二维码图片加载成功:" << imagePath;
        } else {
            qDebug() << "图片文件存在但加载失败:" << imagePath;
        }
    }

    if (imageLoaded && !qrCodePixmap.isNull()) {
        // 缩放图片以适应对话框，保持宽高比，最大宽度400px
        int maxWidth = 400;
        int maxHeight = 550;
        QPixmap originalPixmap = qrCodePixmap;
        if (originalPixmap.width() > maxWidth || originalPixmap.height() > maxHeight) {
            qrCodePixmap = originalPixmap.scaled(maxWidth, maxHeight, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }
        qrCodeLabel->setPixmap(qrCodePixmap);
        qrCodeLabel->setAlignment(Qt::AlignCenter);
        qrCodeLabel->setScaledContents(false);
        qrCodeLabel->setMinimumSize(qrCodePixmap.size());
        qDebug() << "图片显示尺寸:" << qrCodePixmap.size();
    } else {
        qrCodeLabel->setText(QString("二维码图片加载失败\n请确保图片文件存在\n路径: %1").arg(imagePath));
        qrCodeLabel->setAlignment(Qt::AlignCenter);
        qrCodeLabel->setStyleSheet("color: red; padding: 20px; font-size: 12px;");
        qrCodeLabel->setWordWrap(true);
        qDebug() << "二维码图片加载失败，尝试的路径:" << imagePath;
    }
    dialogLayout->addWidget(qrCodeLabel);

    // 显示充值金额
    QLabel *amountLabel = new QLabel(QString("充值金额：¥%1").arg(amount, 0, 'f', 2));
    amountLabel->setAlignment(Qt::AlignCenter);
    amountLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #27ae60; padding: 10px;");
    dialogLayout->addWidget(amountLabel);

    // 充值完成按钮
    QPushButton *completeBtn = new QPushButton("充值完成");
    completeBtn->setStyleSheet("background-color: #27ae60; color: white; padding: 12px; font-size: 16px; font-weight: bold;");
    completeBtn->setMinimumHeight(50);
    dialogLayout->addWidget(completeBtn);

    // 连接充值完成按钮
    connect(completeBtn, &QPushButton::clicked, paymentDialog, &QDialog::accept);

    // 显示对话框
    if (paymentDialog->exec() == QDialog::Accepted) {
        // 用户点击了"充值完成"按钮，执行实际充值
        performRecharge(amount);
    }

    delete paymentDialog;
}

void Purchaser::performRecharge(double amount)
{
    // 连接服务器
    if (!apiService->isConnected()) {
        if (!apiService->connectToServer(serverIp, serverPort)) {
            QMessageBox::warning(this, "连接失败", "无法连接到服务器");
            return;
        }
    }

    // 检查用户ID是否有效
    int userId = currentUser->getId();
    if (userId <= 0) {
        QMessageBox::warning(this, "错误", QString("用户ID无效（ID: %1），请重新登录").arg(userId));
        return;
    }
    
    qDebug() << "准备充值，用户ID:" << userId << "金额:" << amount;
    
    // 调用充值API
    QJsonObject response = apiService->rechargeBalance(QString::number(userId), amount);
    
    if (response["success"].toBool()) {
        // 更新本地余额
        if (response.contains("balance")) {
            currentUser->setBalance(response["balance"].toDouble());
        } else {
            currentUser->addBalance(amount);
        }
        
        // 更新会员等级信息
        if (response.contains("memberLevel")) {
            currentUser->setMemberLevel(response["memberLevel"].toString());
        }
        if (response.contains("totalRecharge")) {
            currentUser->setTotalRecharge(response["totalRecharge"].toDouble());
        }
        if (response.contains("memberDiscount")) {
            currentUser->setMemberDiscount(response["memberDiscount"].toDouble());
        }
        // 更新积分信息
        if (response.contains("points")) {
            currentUser->setPoints(response["points"].toInt());
        }
        
        updateProfileDisplay();
        
        QString memberLevel = currentUser->getMemberLevel();
        QString successMsg = QString("充值成功！\n本次充值：%1 元\n当前余额：%2 元")
                .arg(amount, 0, 'f', 2)
                .arg(currentUser->getBalance(), 0, 'f', 2);
        
        // 如果会员等级发生变化，显示提示
        if (memberLevel != "普通会员") {
            double discount = currentUser->getMemberDiscount() * 10;
            successMsg += QString("\n\n会员等级：%1\n享受折扣：%2折")
                    .arg(memberLevel)
                    .arg(discount, 0, 'f', 1);
        }
        
        QMessageBox::information(this, "充值成功", successMsg);
    } else {
        QMessageBox::warning(this, "充值失败", response["message"].toString());
    }
}

void Purchaser::onLevelInfoClicked()
{
    QString info = QString(
        "会员等级说明：\n\n"
        "• 普通会员：累计充值 0 元\n"
        "  享受折扣：无折扣\n\n"
        "• 银卡会员：累计充值 200 元\n"
        "  享受折扣：9.5折\n\n"
        "• 金卡会员：累计充值 1,000 元\n"
        "  享受折扣：9折\n\n"
        "• 铂金会员：累计充值 3,000 元\n"
        "  享受折扣：8.5折\n\n"
        "• 钻石会员：累计充值 8,000 元\n"
        "  享受折扣：8折\n\n"
        "• 黑钻会员：累计充值 20,000 元\n"
        "  享受折扣：7.5折\n\n"
        "注：会员等级根据累计充值总额自动升级"
    );

    QMessageBox::information(this, "会员等级说明", info);
}

QString Purchaser::truncateBookTitle(const QString &title, int maxLength)
{
    if (title.length() <= maxLength) {
        return title;
    }
    // 如果书名过长，截断并添加省略号
    return title.left(maxLength) + "...";
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
    // 清除选中状态，避免显示蓝框
    recommendList->clearSelection();

    QList<Book> recommended = PopularRecommend(10);
    for (const auto &book : recommended) {
        QListWidgetItem *item = new QListWidgetItem();
        // 优化评分显示：有评分显示"评分：X.X分"，无评分只显示"暂无评分"
        QString scoreText;
        if (book.getScore() > 0) {
            scoreText = QString("评分：%1分").arg(book.getScore(), 0, 'f', 1);
        } else {
            scoreText = "暂无评分";
        }
        // 取消书名截断，直接使用完整书名
        QString displayTitle = book.getTitle();
        // 确保文本格式正确，价格、评分、收藏信息正常显示
        // 使用换行符分隔，确保每行信息独立显示
        QString priceStr = QString::number(book.getPrice(), 'f', 2);
        QString favoriteStr = QString::number(book.getFavoriteCount());
        QString itemText = displayTitle + "\n¥" + priceStr + "\n" + scoreText + "\n收藏:" + favoriteStr;

        // 设置文本，确保所有信息都包含在内
        item->setText(itemText);
        // 设置文本对齐方式，确保文本居中显示
        item->setTextAlignment(Qt::AlignHCenter | Qt::AlignTop);
        // 强制设置文本，确保不会被截断
        item->setToolTip(itemText);  // 设置工具提示，确保信息可访问

        // 调试输出：验证文本内容是否正确设置
        if (book.getTitle().contains("深入理解计算机系统")) {
            qDebug() << "=== 深入理解计算机系统调试信息 ===";
            qDebug() << "原始书名:" << book.getTitle();
            qDebug() << "截断后书名:" << displayTitle;
            qDebug() << "完整文本内容:" << itemText;
            qDebug() << "价格:" << book.getPrice() << "->" << priceStr;
            qDebug() << "评分:" << scoreText;
            qDebug() << "收藏:" << book.getFavoriteCount() << "->" << favoriteStr;
            qDebug() << "文本长度:" << itemText.length();
            qDebug() << "文本行数:" << itemText.split('\n').size();
        }
        item->setData(Qt::UserRole, book.getId());
        
        // 设置封面图片
        QPixmap coverPixmap;
        if (!book.getCoverImage().isEmpty()) {
            QByteArray imageData = QByteArray::fromBase64(book.getCoverImage().toUtf8());
            coverPixmap.loadFromData(imageData);
        }
        if (coverPixmap.isNull()) {
            // 使用默认空白图片
            coverPixmap = QPixmap(100, 150);
            coverPixmap.fill(Qt::lightGray);
        }
        item->setIcon(QIcon(coverPixmap.scaled(100, 150, Qt::KeepAspectRatio, Qt::SmoothTransformation)));

        // 设置卡片大小，使其能够均匀铺满每行
        // 宽度200px，高度300px（图标150px + 文本约140px + 边距），确保能完整显示所有信息
        item->setSizeHint(QSize(200, 300));
        
        recommendList->addItem(item);
    }
}

void Purchaser::updateCartDisplay()
{
    cartTable->clearContents();
    cartTable->setRowCount(0);

    if (!currentUser) return;

    for (const auto &item : currentUser->getCartItems()) {
        int row = cartTable->rowCount();
        cartTable->insertRow(row);

        // 选择框
        QCheckBox *checkBox = new QCheckBox();
        checkBox->setProperty("bookId", item.bookId);
        checkBox->setChecked(true);  // 默认选中
        // 连接复选框状态改变信号，实时更新总金额
        connect(checkBox, &QCheckBox::stateChanged, this, &Purchaser::updateCartTotal);
        cartTable->setCellWidget(row, 0, checkBox);

        // 书名
        QTableWidgetItem *titleItem = new QTableWidgetItem(item.bookTitle);
        titleItem->setFlags(titleItem->flags() & ~Qt::ItemIsEditable);
        cartTable->setItem(row, 1, titleItem);

        // 单价
        QTableWidgetItem *priceItem = new QTableWidgetItem(QString::number(item.price, 'f', 2));
        priceItem->setFlags(priceItem->flags() & ~Qt::ItemIsEditable);
        cartTable->setItem(row, 2, priceItem);

        // 数量（使用QSpinBox以便用户调整）
        QSpinBox *quantitySpinBox = new QSpinBox();
        quantitySpinBox->setMinimum(1);
        quantitySpinBox->setMaximum(999);
        quantitySpinBox->setValue(item.quantity);
        quantitySpinBox->setProperty("bookId", item.bookId);
        quantitySpinBox->setProperty("price", item.price);
        // 连接数量改变信号，更新小计、总金额并保存到数据库
        connect(quantitySpinBox, QOverload<int>::of(&QSpinBox::valueChanged), 
                this, &Purchaser::onCartQuantityChanged);
        cartTable->setCellWidget(row, 3, quantitySpinBox);

        // 小计
        double subtotal = item.getTotal();
        QTableWidgetItem *subtotalItem = new QTableWidgetItem(QString::number(subtotal, 'f', 2));
        subtotalItem->setFlags(subtotalItem->flags() & ~Qt::ItemIsEditable);
        cartTable->setItem(row, 4, subtotalItem);
    }

    // 更新总金额（只计算选中的商品）
    updateCartTotal();
}

void Purchaser::updateCartTotal()
{
    if (!currentUser) return;

    double total = 0.0;
    for (int i = 0; i < cartTable->rowCount(); i++) {
        QCheckBox *checkBox = qobject_cast<QCheckBox*>(cartTable->cellWidget(i, 0));
        if (checkBox && checkBox->isChecked()) {
            // 获取小计（第5列，索引4）
            QTableWidgetItem *subtotalItem = cartTable->item(i, 4);
            if (subtotalItem) {
                double subtotal = subtotalItem->text().toDouble();
                total += subtotal;
            }
        }
    }

    cartTotalLabel->setText(QString("总计: %1元").arg(total, 0, 'f', 2));
}

void Purchaser::onCartQuantityChanged(int newQuantity)
{
    if (!currentUser) return;
    
    // 获取发送信号的QSpinBox
    QSpinBox *quantitySpinBox = qobject_cast<QSpinBox*>(sender());
    if (!quantitySpinBox) return;
    
    QString bookId = quantitySpinBox->property("bookId").toString();
    double price = quantitySpinBox->property("price").toDouble();
    
    if (bookId.isEmpty()) return;
    
    // 确保已连接到服务器
    if (!apiService->isConnected()) {
        if (!apiService->connectToServer(serverIp, serverPort)) {
            QMessageBox::warning(this, "连接失败", "无法连接到服务器");
            // 恢复原值
            quantitySpinBox->blockSignals(true);
            for (const auto &item : currentUser->getCartItems()) {
                if (item.bookId == bookId) {
                    quantitySpinBox->setValue(item.quantity);
                    break;
                }
            }
            quantitySpinBox->blockSignals(false);
            return;
        }
        // 移除阻塞延迟，连接后立即使用
        QCoreApplication::processEvents();  // 处理事件，确保连接完成
    }
    
    // 更新数据库中的数量
    QJsonObject response = apiService->updateCartQuantity(
        QString::number(currentUser->getId()),
        bookId,
        newQuantity
    );
    
    if (response.value("success").toBool()) {
        // 更新本地购物车数据
        for (auto &item : currentUser->getCartItems()) {
            if (item.bookId == bookId) {
                item.quantity = newQuantity;
                break;
            }
        }
        
        // 找到对应的行，更新小计
        for (int i = 0; i < cartTable->rowCount(); i++) {
            QSpinBox *spinBox = qobject_cast<QSpinBox*>(cartTable->cellWidget(i, 3));
            if (spinBox && spinBox->property("bookId").toString() == bookId) {
                // 更新小计（第5列，索引4）
                double subtotal = price * newQuantity;
                QTableWidgetItem *subtotalItem = cartTable->item(i, 4);
                if (subtotalItem) {
                    subtotalItem->setText(QString::number(subtotal, 'f', 2));
                }
                break;
            }
        }
        
        // 更新总金额
        updateCartTotal();
    } else {
        QString errorMsg = response.value("message").toString();
        QMessageBox::warning(this, "更新失败", errorMsg.isEmpty() ? "数量更新失败" : errorMsg);
        // 恢复原值
        quantitySpinBox->blockSignals(true);
        for (const auto &item : currentUser->getCartItems()) {
            if (item.bookId == bookId) {
                quantitySpinBox->setValue(item.quantity);
                break;
            }
        }
        quantitySpinBox->blockSignals(false);
    }
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
        const QList<OrderItem> &items = order.getItems();
        int itemCount = items.size();
        for (int i = 0; i < qMin(itemCount, 2); i++) {  // 最多显示2个商品
            const auto &item = items.at(i);
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
        // 覆盖全局按钮大字号样式，避免在表格里显示异常
        viewBtn->setMinimumHeight(36);
        viewBtn->setStyleSheet(
            "QPushButton{"
            "background-color:#3498db;"
            "color:white;"
            "border:none;"
            "border-radius:8px;"
            "font-size:14px;"
            "font-weight:600;"
            "padding:6px 12px;"
            "}"
            "QPushButton:hover{background-color:#2e86c1;}"
            "QPushButton:pressed{background-color:#2874a6;}"
        );
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

    // 不再更新用户名显示（已删除）
    // profileUsername->setText(currentUser->getUsername());
    profilePhone->setText(currentUser->getPhone());
    profileEmail->setText(currentUser->getEmail());
    profileAddress->setText(currentUser->getAddress());
    
    // 显示会员等级和折扣信息
    QString memberLevel = currentUser->getMemberLevel();
    if (memberLevel.isEmpty()) {
        memberLevel = "普通会员";
    }
    double memberDiscount = currentUser->getMemberDiscount();
    double totalRecharge = currentUser->getTotalRecharge();
    int points = currentUser->getPoints();
    bool canLottery = currentUser->canParticipateLottery();
    
    // 更新Banner中的欢迎语
    QString username = currentUser->getUsername();
    welcomeLabel->setText(QString("你好，%1！").arg(username));

    // 更新Banner中的会员卡片
    QString memberCardText = QString("%1 %2折")
                        .arg(memberLevel)
                        .arg(memberDiscount * 10, 0, 'f', 1);
    memberCardLabel->setText(memberCardText);

    // 更新会员卡片中的累计充值金额
    QString rechargeText = QString("累计充值: ¥%1")
                          .arg(totalRecharge, 0, 'f', 2);
    memberCardRechargeLabel->setText(rechargeText);

    // 不再更新详细信息中的会员等级显示（已删除）
    // QString levelText = QString("%1 (折扣: %2折)")
    //                     .arg(memberLevel)
    //                     .arg(memberDiscount * 10, 0, 'f', 1);
    // if (memberLevel != "普通会员") {
    //     levelText += QString("\n累计充值: ¥%1").arg(totalRecharge, 0, 'f', 2);
    // }
    // profileLevelLabel->setText(levelText);
    
    // 账户余额：大数字显示，带货币符号
    profileBalanceLabel->setText(QString("¥%1").arg(currentUser->getBalance(), 0, 'f', 2));
    
    // 显示积分信息（简化显示，只显示数字）
    QString pointsText = QString::number(points);
    profilePointsLabel->setText(pointsText);
    
    // 更新抽奖按钮状态（样式已在初始化时设置，这里只更新文本和启用状态）
    lotteryBtn->setEnabled(canLottery);
    if (canLottery) {
        lotteryBtn->setText("参与抽奖（消耗3积分）");
    } else {
        lotteryBtn->setText(QString("参与抽奖（需3积分，当前%1积分）").arg(points));
    }

    favoriteList->clear();
    for (const auto &bookId : currentUser->getFavoriteBooks()) {
        if (bookMap.contains(bookId)) {
            Book book = bookMap[bookId];
            // 优化评分显示：有评分显示"评分：X.X分"，无评分只显示"暂无评分"
            QString scoreText;
            if (book.getScore() > 0) {
                scoreText = QString("评分：%1分").arg(book.getScore(), 0, 'f', 1);
            } else {
                scoreText = "暂无评分";
            }
            // 收藏夹列表可以显示更长的书名，设置maxLength为20
            QString displayTitle = truncateBookTitle(book.getTitle(), 20);
            QListWidgetItem *item = new QListWidgetItem(
                QString("%1\n¥%2 - %3 收藏:%4")
                    .arg(displayTitle)
                    .arg(book.getPrice())
                    .arg(scoreText)
                    .arg(book.getFavoriteCount()));
            item->setData(Qt::UserRole, bookId);
            favoriteList->addItem(item);
        }
    }
    
    // 查询卖家认证状态（如果组件已初始化）
    if (sellerStatusLabel && apiService->isConnected() || apiService->connectToServer(serverIp, serverPort)) {
        QJsonObject response = apiService->getSellerCertStatus(QString::number(currentUser->getId()));
        if (response.value("success").toBool()) {
            QString status = response.value("status").toString();
            if (status == "已认证") {
                sellerStatusLabel->setText("✓ 已认证为卖家");
                sellerStatusLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: #27ae60;");
                if (selectLicenseBtn) selectLicenseBtn->setEnabled(false);
                if (applySellerBtn) {
                    applySellerBtn->setEnabled(false);
                    applySellerBtn->setText("已认证");
                }
            } else if (status == "审核中") {
                sellerStatusLabel->setText("⏳ 认证审核中，请耐心等待");
                sellerStatusLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: #f39c12;");
                if (selectLicenseBtn) selectLicenseBtn->setEnabled(false);
                if (applySellerBtn) {
                    applySellerBtn->setEnabled(false);
                    applySellerBtn->setText("审核中");
                }
            } else {
                sellerStatusLabel->setText("未认证");
                sellerStatusLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: #e74c3c;");
                if (selectLicenseBtn) selectLicenseBtn->setEnabled(true);
                if (applySellerBtn) {
                    applySellerBtn->setEnabled(!licenseImageBase64.isEmpty());
                    applySellerBtn->setText("提交营业执照");  // 重置按钮文本
                }
            }
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
    QSet<QString> addedBookIds;  // 用于去重，避免同一本书被添加多次

    qDebug() << "GetBooksByCategory - categoryId1:" << categoryId1 << "categoryId2:" << categoryId2;

    std::function<void(CategoryNode*)> searchCategory;
    searchCategory = [&](CategoryNode* node) {
        // 检查是否匹配一级分类或二级分类
        bool matchCategory1 = (categoryId2.isEmpty() && node->getId() == categoryId1);
        bool matchCategory2 = (!categoryId2.isEmpty() && node->getId() == categoryId2);
        
        if (matchCategory1 || matchCategory2) {
            qDebug() << "找到匹配的分类节点:" << node->getName() << "ID:" << node->getId() << "图书数量:" << node->getBookIds().size();
            
            // 找到分类，获取所有图书
            for (const auto &bookId : node->getBookIds()) {
                if (bookMap.contains(bookId) && !addedBookIds.contains(bookId)) {
                    result.append(bookMap[bookId]);
                    addedBookIds.insert(bookId);  // 标记已添加
                    qDebug() << "添加图书到结果:" << bookId << bookMap[bookId].getTitle();
                }
            }

            // 如果是父分类（选择一级分类时），还需要获取子分类的图书（但要去重）
            if (matchCategory1) {  // 只有选择一级分类时才获取子分类的图书
                qDebug() << "获取子分类图书，子分类数量:" << node->getChildren().size();
            for (auto child : node->getChildren()) {
                    qDebug() << "检查子分类:" << child->getName() << "ID:" << child->getId() << "图书数量:" << child->getBookIds().size();
                for (const auto &bookId : child->getBookIds()) {
                        if (bookMap.contains(bookId) && !addedBookIds.contains(bookId)) {
                        result.append(bookMap[bookId]);
                            addedBookIds.insert(bookId);  // 标记已添加
                            qDebug() << "从子分类添加图书:" << bookId << bookMap[bookId].getTitle();
                        }
                    }
                }
            }
        }

        // 递归查找子节点
        for (auto child : node->getChildren()) {
            searchCategory(child);
        }
    };

    searchCategory(categoryRoot);
    qDebug() << "GetBooksByCategory 返回结果数量:" << result.size();
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
    // #region agent log
    writeDebugLog("purchaser.cpp:1591", "注册函数入口", QJsonObject{{"username", username}, {"passwordLength", password.length()}}, "C");
    // #endregion
    
    // 首先确保已连接到服务器
    // #region agent log
    writeDebugLog("purchaser.cpp:1594", "检查TCP连接状态", QJsonObject{{"isConnected", apiService->isConnected()}, {"serverIp", serverIp}, {"serverPort", serverPort}}, "C");
    // #endregion
    if (!apiService->isConnected()) {
        qDebug() << "未连接到服务器，尝试连接...";
        // #region agent log
        writeDebugLog("purchaser.cpp:1596", "尝试连接服务器", QJsonObject{{"serverIp", serverIp}, {"serverPort", serverPort}}, "C");
        // #endregion
        if (!apiService->connectToServer(serverIp, serverPort)) {
            // #region agent log
            writeDebugLog("purchaser.cpp:1597", "连接服务器失败", QJsonObject{{"serverIp", serverIp}, {"serverPort", serverPort}}, "C");
            // #endregion
            qDebug() << "连接服务器失败";
            QMessageBox::warning(nullptr, "连接失败", 
                "无法连接到服务器，请检查:\n"
                "1. 服务器是否已启动\n"
                "2. 服务器地址: " + serverIp + ":" + QString::number(serverPort) + "\n"
                "3. 网络连接是否正常");
            return false;
        }
        qDebug() << "连接服务器成功";
        // #region agent log
        writeDebugLog("purchaser.cpp:1605", "连接服务器成功", QJsonObject{{"serverIp", serverIp}, {"serverPort", serverPort}}, "C");
        // #endregion
    }
    
    // 生成默认邮箱
    QString email = username + "@example.com";
    
    qDebug() << "========================================";
    qDebug() << "发送注册请求到服务器";
    qDebug() << "用户名:" << username;
    qDebug() << "密码:" << password;
    qDebug() << "邮箱:" << email;
    qDebug() << "========================================";
    
    // 调用API服务发送注册请求
    // #region agent log
    writeDebugLog("purchaser.cpp:1619", "发送注册请求前", QJsonObject{{"username", username}, {"email", email}}, "D");
    // #endregion
    QJsonObject response = apiService->registerUser(username, password, email);
    
    // #region agent log
    writeDebugLog("purchaser.cpp:1621", "收到注册响应", QJsonObject{{"success", response.value("success").toBool()}, {"message", response.value("message").toString()}, {"hasUserId", response.contains("userId")}}, "D");
    // #endregion
    qDebug() << "收到服务器响应:" << QJsonDocument(response).toJson(QJsonDocument::Compact);
    
    // 检查响应
    if (response.contains("success") && response["success"].toBool()) {
        qDebug() << "✅ 注册成功！用户信息已保存到数据库";
        qDebug() << "用户ID:" << response.value("userId").toInt();
        qDebug() << "用户名:" << response.value("username").toString();
        qDebug() << "邮箱:" << response.value("email").toString();
        
        // #region agent log
        writeDebugLog("purchaser.cpp:1630", "注册成功", QJsonObject{{"userId", response.value("userId").toInt()}, {"username", response.value("username").toString()}}, "D");
        // #endregion
        
        // 同时也在本地用户管理器中注册（保持本地数据一致）
        userManager.registerUser(username, password);
        
        return true;
    } else {
        QString errorMsg = response.value("message").toString("注册失败");
        // #region agent log
        writeDebugLog("purchaser.cpp:1635", "注册失败", QJsonObject{{"errorMsg", errorMsg}}, "D");
        // #endregion
        qDebug() << "❌ 注册失败:" << errorMsg;
        QMessageBox::warning(nullptr, "注册失败", errorMsg);
        return false;
    }
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
    if (!currentUser || !apiService->isConnected()) return false;
    
    // 调用服务器API添加到收藏
    QJsonObject response = apiService->addFavorite(QString::number(currentUser->getId()), bookId);
    if (response.value("success").toBool()) {
        // 如果服务器添加成功，更新本地用户对象
        if (!currentUser->favoriteBooks.contains(bookId)) {
            currentUser->favoriteBooks.append(bookId);
        }
        return true;
    }
    return false;
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
//void Purchaser::onLoginClicked()
//{
//    QString username = loginUsername->text().trimmed();
//    QString password = loginPassword->text().trimmed();

//    if (username.isEmpty() || password.isEmpty()) {
//        QLabel *statusLabel = loginPage->findChild<QLabel*>("loginStatusLabel");
//        if (statusLabel) {
//            statusLabel->setText("请输入用户名和密码");
//        }
//        return;
//    }

//    currentUser = Login(username, password);
//    if (currentUser != nullptr) {
//        isLoggedIn = true;
//        QLabel *statusLabel = loginPage->findChild<QLabel*>("loginStatusLabel");
//        if (statusLabel) {
//            statusLabel->clear();
//        }
//        loginUsername->clear();
//        loginPassword->clear();
//        showMainPage();
//        QMessageBox::information(this, "登录成功", QString("欢迎回来，%1！").arg(username));
//    } else {
//        QLabel *statusLabel = loginPage->findChild<QLabel*>("loginStatusLabel");
//        if (statusLabel) {
//            statusLabel->setText("用户名或密码错误");
//        }
//    }
//}

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
    // 停止自动刷新定时器
    if (autoRefreshTimer && autoRefreshTimer->isActive()) {
        autoRefreshTimer->stop();
        qDebug() << "已停止自动刷新定时器";
    }
    
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

    // 确保已连接到服务器
    if (!apiService->isConnected()) {
        qDebug() << "未连接服务器，正在连接...";
        if (!apiService->connectToServer(serverIp, serverPort)) {
            QMessageBox::warning(this, "连接失败", "无法连接到服务器，无法搜索");
            return;
        }
        // 移除阻塞延迟，连接后立即使用
        QCoreApplication::processEvents();  // 处理事件，确保连接完成
    }

    // 通过TCP请求搜索图书
    qDebug() << "通过TCP请求搜索图书，关键词:" << keyword;
    QJsonObject response = apiService->searchBooks(keyword);
    
    recommendList->clear();
    recommendList->clearSelection();

    if (response.value("success").toBool()) {
        QJsonArray booksArray = response.value("books").toArray();
        
        if (booksArray.isEmpty()) {
            QListWidgetItem *item = new QListWidgetItem("未找到相关图书");
            item->setTextAlignment(Qt::AlignCenter);
            recommendList->addItem(item);
            return;
        }

        // 更新本地图书数据
        QList<Book> searchResults;
        for (const QJsonValue &value : booksArray) {
            QJsonObject bookObj = value.toObject();
            Book book;
            book.bookId = bookObj.value("bookId").toString();
            book.title = bookObj.value("bookName").toString();
            // 优先使用category1和category2，如果没有则使用category和subCategory（向后兼容）
            book.categoryId1 = bookObj.value("category1").toString();
            if (book.categoryId1.isEmpty()) {
            book.categoryId1 = bookObj.value("category").toString();
            }
            book.categoryId2 = bookObj.value("category2").toString();
            if (book.categoryId2.isEmpty()) {
            book.categoryId2 = bookObj.value("subCategory").toString();
            }
            book.price = bookObj.value("price").toDouble();
            // 优先使用averageRating，如果没有则使用score，如果都没有则使用0.0
            if (bookObj.contains("averageRating")) {
                book.score = bookObj.value("averageRating").toDouble();
            } else if (bookObj.contains("score")) {
                book.score = bookObj.value("score").toDouble();
            } else {
                book.score = 0.0;  // 无评分
            }
            book.sales = bookObj.value("sales").toInt();
            book.author = bookObj.value("author").toString();
            book.coverImage = bookObj.value("coverImage").toString();
            searchResults.append(book);
        }

        // 显示搜索结果
        for (const auto &book : searchResults) {
            QListWidgetItem *item = new QListWidgetItem();
            // 优化评分显示：有评分显示"评分：X.X分"，无评分只显示"暂无评分"
            QString scoreText;
            if (book.getScore() > 0) {
                scoreText = QString("评分：%1分").arg(book.getScore(), 0, 'f', 1);
            } else {
                scoreText = "暂无评分";
            }
            // 取消书名截断，直接使用完整书名
            QString displayTitle = book.getTitle();
            // 确保文本格式正确，价格、评分、收藏信息正常显示
            // 使用字符串拼接确保所有信息都包含在内
            QString priceStr = QString::number(book.getPrice(), 'f', 2);
            QString favoriteStr = QString::number(book.getFavoriteCount());
            QString itemText = displayTitle + "\n¥" + priceStr + "\n" + scoreText + "\n收藏:" + favoriteStr;
            item->setText(itemText);
            // 设置文本对齐方式
            item->setTextAlignment(Qt::AlignHCenter | Qt::AlignTop);
            // 设置工具提示，确保信息可访问
            item->setToolTip(itemText);

            // 设置卡片大小，使其能够均匀铺满每行
            // 宽度200px，高度300px，确保能完整显示所有信息
            item->setSizeHint(QSize(200, 300));
            
            // 设置封面图片
            QPixmap coverPixmap;
            if (!book.getCoverImage().isEmpty()) {
                QByteArray imageData = QByteArray::fromBase64(book.getCoverImage().toUtf8());
                coverPixmap.loadFromData(imageData);
            }
            if (coverPixmap.isNull()) {
                // 使用默认空白图片
                coverPixmap = QPixmap(100, 150);
                coverPixmap.fill(Qt::lightGray);
            }
            item->setIcon(QIcon(coverPixmap.scaled(100, 150, Qt::KeepAspectRatio, Qt::SmoothTransformation)));
            item->setData(Qt::UserRole, book.getId());
            recommendList->addItem(item);
        }
    } else {
        QString errorMsg = response.value("message").toString();
        QListWidgetItem *item = new QListWidgetItem("搜索失败: " + errorMsg);
        item->setTextAlignment(Qt::AlignCenter);
        recommendList->addItem(item);
    }
}

void Purchaser::onCategoryItemClicked(QTreeWidgetItem *item, int column)
{
    QString categoryId = item->data(0, Qt::UserRole).toString();

    // 获取该分类及子分类的所有图书
    QList<Book> books = GetBooksByCategory(categoryId, "");

    recommendList->clear();
    recommendList->clearSelection();
    if (books.isEmpty()) {
        QListWidgetItem *noItem = new QListWidgetItem("该分类暂无图书");
        noItem->setTextAlignment(Qt::AlignCenter);
        recommendList->addItem(noItem);
        return;
    }

    for (const auto &book : books) {
        QListWidgetItem *listItem = new QListWidgetItem();
        // 优化评分显示：有评分显示"评分：X.X分"，无评分只显示"暂无评分"
        QString scoreText;
        if (book.getScore() > 0) {
            scoreText = QString("评分：%1分").arg(book.getScore(), 0, 'f', 1);
        } else {
            scoreText = "暂无评分";
        }
        // 取消书名截断，直接使用完整书名
        QString displayTitle = book.getTitle();
        // 确保文本格式正确，价格、评分、收藏信息正常显示
        // 使用字符串拼接确保所有信息都包含在内
        QString priceStr = QString::number(book.getPrice(), 'f', 2);
        QString favoriteStr = QString::number(book.getFavoriteCount());
        QString itemText = displayTitle + "\n¥" + priceStr + "\n" + scoreText + "\n收藏:" + favoriteStr;
        listItem->setText(itemText);
        // 设置文本对齐方式
        listItem->setTextAlignment(Qt::AlignHCenter | Qt::AlignTop);
        // 设置工具提示，确保信息可访问
        listItem->setToolTip(itemText);
        listItem->setData(Qt::UserRole, book.getId());

        // 设置卡片大小，使其能够均匀铺满每行
        // 宽度200px，高度300px，确保能完整显示所有信息
        listItem->setSizeHint(QSize(200, 300));
            
            // 设置封面图片
            QPixmap coverPixmap;
            if (!book.getCoverImage().isEmpty()) {
                QByteArray imageData = QByteArray::fromBase64(book.getCoverImage().toUtf8());
                coverPixmap.loadFromData(imageData);
            }
            if (coverPixmap.isNull()) {
                // 使用默认空白图片
                coverPixmap = QPixmap(100, 150);
                coverPixmap.fill(Qt::lightGray);
            }
            listItem->setIcon(QIcon(coverPixmap.scaled(100, 150, Qt::KeepAspectRatio, Qt::SmoothTransformation)));
            
        recommendList->addItem(listItem);
    }
}

void Purchaser::onBookItemClicked(QListWidgetItem *item)
{
    if (item->text() == "未找到相关图书" || item->text() == "该分类暂无图书") {
        return;
    }

    QString bookId = item->data(Qt::UserRole).toString();
    
    // 确保已连接到服务器
    if (!apiService->isConnected()) {
        qDebug() << "未连接服务器，正在连接...";
        if (!apiService->connectToServer(serverIp, serverPort)) {
            QMessageBox::warning(this, "连接失败", "无法连接到服务器");
            return;
        }
        // 移除阻塞延迟，连接后立即使用
        QCoreApplication::processEvents();  // 处理事件，确保连接完成
    }

    // 通过TCP请求获取图书详情
    QJsonObject response = apiService->getBook(bookId);
    
    if (response.value("success").toBool()) {
        // 解析图书详情
        QJsonObject bookObj = response;
        currentBook.bookId = bookObj.value("bookId").toString();
        currentBook.title = bookObj.value("bookName").toString();
        // 优先使用category1和category2，如果没有则使用category和subCategory（向后兼容）
        currentBook.categoryId1 = bookObj.value("category1").toString();
        if (currentBook.categoryId1.isEmpty()) {
            currentBook.categoryId1 = bookObj.value("category").toString();
        }
        currentBook.categoryId2 = bookObj.value("category2").toString();
        if (currentBook.categoryId2.isEmpty()) {
            currentBook.categoryId2 = bookObj.value("subCategory").toString();
        }
        currentBook.price = bookObj.value("price").toDouble();
        // 优先使用averageRating，如果没有则使用score，如果都没有则使用0.0
        if (bookObj.contains("averageRating")) {
            currentBook.score = bookObj.value("averageRating").toDouble();
        } else if (bookObj.contains("score")) {
        currentBook.score = bookObj.value("score").toDouble();
        } else {
            currentBook.score = 0.0;  // 无评分
        }
        currentBook.sales = bookObj.value("sales").toInt();
        currentBook.author = bookObj.value("author").toString();
        currentBook.coverImage = bookObj.value("coverImage").toString();
        currentBook.description = bookObj.value("description").toString();  // 书籍描述
        currentBook.merchantId = bookObj.value("merchantId").toInt();  // 商家ID
    } else {
        // 如果服务器获取失败，使用本地数据
        currentBook = ViewBookDetail(bookId);
    }

    // 更新图书详情显示
    bookTitleLabel->setText(currentBook.getTitle());
    bookAuthorLabel->setText(currentBook.getAuthor());
    bookPriceLabel->setText(QString("¥%1").arg(currentBook.getPrice()));
    // 显示评分，如果没有评分则显示"暂无评分"
    QString scoreText = (currentBook.getScore() > 0) ? QString::number(currentBook.getScore(), 'f', 1) : "暂无评分";
    bookScoreLabel->setText(scoreText);
    bookFavoriteCountLabel->setText(QString::number(currentBook.getFavoriteCount()));
    bookDescription->setText(currentBook.getDescription());
    
    // 显示封面图片
    QPixmap coverPixmap;
    if (!currentBook.getCoverImage().isEmpty()) {
        QByteArray imageData = QByteArray::fromBase64(currentBook.getCoverImage().toUtf8());
        coverPixmap.loadFromData(imageData);
    }
    if (coverPixmap.isNull()) {
        // 使用默认空白图片
        coverPixmap = QPixmap(200, 300);
        coverPixmap.fill(Qt::lightGray);
    }
    if (bookCoverLabel) {
        bookCoverLabel->setPixmap(coverPixmap.scaled(200, 300, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }

    // 加载评论和评分
    loadBookRatingStats();
    loadBookReviews();
    
    showBookDetailPage();
}

void Purchaser::onAddToCartClicked()
{
    if (!isLoggedIn || !currentUser) {
        QMessageBox::warning(this, "操作失败", "请先登录");
        showLoginPage();
        return;
    }

    // 确保已连接到服务器
    if (!apiService->isConnected()) {
        qDebug() << "未连接服务器，正在连接...";
        if (!apiService->connectToServer(serverIp, serverPort)) {
            QMessageBox::warning(this, "连接失败", "无法连接到服务器");
            return;
        }
        // 移除阻塞延迟，连接后立即使用
        QCoreApplication::processEvents();  // 处理事件，确保连接完成
    }

    // 通过TCP请求添加到购物车
    int quantity = quantitySpinBox->value();
    QJsonObject response = apiService->addToCart(
        QString::number(currentUser->getId()),
        currentBook.getId(),
        quantity
    );

    if (response.value("success").toBool()) {
        QMessageBox::information(this, "成功", "已添加到购物车");
        
        // 重新从服务器加载购物车以确保数据同步
        QJsonObject cartResponse = apiService->getCart(QString::number(currentUser->getId()));
        if (cartResponse.value("success").toBool()) {
            currentUser->clearCart();
            QJsonArray items = cartResponse.value("items").toArray();
            for (const QJsonValue &itemVal : items) {
                QJsonObject item = itemVal.toObject();
                QString bookId = item.value("bookId").toString();
                QString bookName = item.value("bookName").toString();
                double price = item.value("price").toDouble();
                int quantity = item.value("quantity").toInt();
                currentUser->addToCart(bookId, quantity, bookName, price);
            }
        }
        
        updateCartDisplay();
    } else {
        QString errorMsg = response.value("message").toString();
        QMessageBox::warning(this, "失败", errorMsg.isEmpty() ? "添加到购物车失败" : errorMsg);
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
        // 确保已连接到服务器
        if (!apiService->isConnected()) {
            if (!apiService->connectToServer(serverIp, serverPort)) {
                QMessageBox::warning(this, "连接失败", "无法连接到服务器");
                return;
            }
            // 移除阻塞延迟，连接后立即使用
        QCoreApplication::processEvents();  // 处理事件，确保连接完成
        }

        bool allSuccess = true;
        for (const auto &bookId : bookIdsToRemove) {
            // 调用服务器API从数据库删除记录
            QJsonObject response = apiService->removeFromCart(
                QString::number(currentUser->getId()),
                bookId
            );
            
            if (response.value("success").toBool()) {
                // 同时更新本地购物车
                RemoveFromCart(bookId);
            } else {
                allSuccess = false;
            }
        }

        if (allSuccess) {
            updateCartDisplay();
            QMessageBox::information(this, "成功", "已从购物车移除");
        } else {
            QMessageBox::warning(this, "部分失败", "部分商品移除失败，请重试");
            // 重新从服务器加载购物车以确保同步
            QJsonObject response = apiService->getCart(QString::number(currentUser->getId()));
            if (response.value("success").toBool()) {
                currentUser->clearCart();
                QJsonArray items = response.value("items").toArray();
                for (const QJsonValue &itemVal : items) {
                    QJsonObject item = itemVal.toObject();
                    QString bookId = item.value("bookId").toString();
                    QString bookName = item.value("bookName").toString();
                    double price = item.value("price").toDouble();
                    int quantity = item.value("quantity").toInt();
                    currentUser->addToCart(bookId, quantity, bookName, price);
                }
                updateCartDisplay();
            }
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

    // 使用TCP请求创建订单（待支付状态）
    if (!apiService->isConnected()) {
        if (!apiService->connectToServer(serverIp, serverPort)) {
            QMessageBox::warning(this, "连接失败", "无法连接到服务器");
            return;
        }
    }

    // 下单前校验收货信息（电话和地址）
    // 从currentUser获取用户名，而不是从UI控件（UI中已删除用户名显示）
    QString customerName = currentUser ? currentUser->getUsername() : "";
    QString phone = profilePhone ? profilePhone->text().trimmed() : "";
    QString address = profileAddress ? profileAddress->text().trimmed() : "";

    if (phone.isEmpty() || address.isEmpty()) {
        QMessageBox::warning(this,
                             "收货信息不完整",
                             "下单前请先在“个人中心”填写并保存联系电话和收货地址。");
        // 跳转到个人信息页面，方便用户填写
        showProfilePage();
        return;
    }

    // 获取勾选的商品
    QList<CartItem> selectedItems;
    double selectedTotal = 0.0;
    
    for (int i = 0; i < cartTable->rowCount(); i++) {
        QCheckBox *checkBox = qobject_cast<QCheckBox*>(cartTable->cellWidget(i, 0));
        if (checkBox && checkBox->isChecked()) {
            QString bookId = checkBox->property("bookId").toString();
            
            // 从表格中获取商品信息
            QTableWidgetItem *titleItem = cartTable->item(i, 1);
            QTableWidgetItem *priceItem = cartTable->item(i, 2);
            QSpinBox *quantitySpinBox = qobject_cast<QSpinBox*>(cartTable->cellWidget(i, 3));
            QTableWidgetItem *subtotalItem = cartTable->item(i, 4);
            
            if (titleItem && priceItem && quantitySpinBox && subtotalItem) {
                CartItem cartItem;
                cartItem.bookId = bookId;
                cartItem.bookTitle = titleItem->text();
                cartItem.price = priceItem->text().toDouble();
                cartItem.quantity = quantitySpinBox->value();
                
                selectedItems.append(cartItem);
                selectedTotal += subtotalItem->text().toDouble();
            }
        }
    }
    
    // 检查是否有选中的商品
    if (selectedItems.isEmpty()) {
        QMessageBox::warning(this, "提示", "请至少选择一件商品进行结算");
        return;
    }
    
    // 构建订单项（只包含勾选的商品）
    QJsonArray items;
    QString itemsText;
    pendingCartItems.clear();
    for (const auto &cartItem : selectedItems) {
        QJsonObject item;
        item["bookId"] = cartItem.bookId;
        item["bookName"] = cartItem.bookTitle;
        item["quantity"] = cartItem.quantity;
        item["price"] = cartItem.price;
        items.append(item);

        // 保存待支付订单商品快照（支付页展示/生成本地订单用）
        pendingCartItems.append(cartItem);
        
        itemsText += QString("%1 x%2 = %3元\n")
                    .arg(cartItem.bookTitle)
                    .arg(cartItem.quantity)
                    .arg(cartItem.getTotal(), 0, 'f', 2);
    }

    QJsonObject response = apiService->createOrder(
        QString::number(currentUser->getId()),
        items,
        customerName,
        phone,
        address
    );

    if (response.value("success").toBool()) {
        QString orderId = response.value("orderId").toString();
        double totalAmount = response.value("totalAmount").toDouble(0.0);
        
        // 如果服务器没有返回总金额，使用本地计算（只计算选中的商品）
        if (totalAmount <= 0) {
            totalAmount = selectedTotal;
        }
        
        // 保存待支付订单信息
        pendingOrderId = orderId;
        pendingAmount = totalAmount;

        // 将订单写入本地订单列表，状态=待支付（取消支付时也能在“我的订单”看到）
        {
            // 防止重复追加（同订单号只保留一份）
            bool exists = false;
            for (const auto &o : allOrders) {
                if (o.getOrderId() == pendingOrderId) { exists = true; break; }
            }
            if (!exists) {
                Order localOrder(pendingOrderId, currentUser->getId(), QDate::currentDate());
                for (const auto &ci : pendingCartItems) {
                    OrderItem oi;
                    oi.bookId = ci.bookId;
                    oi.bookTitle = ci.bookTitle;
                    oi.quantity = ci.quantity;
                    oi.price = ci.price;
                    oi.status = "待支付";
                    localOrder.addItem(oi);
                }
                localOrder.setTotalAmount(pendingAmount);
                localOrder.setStatus("待支付");
                allOrders.append(localOrder);
            }
        }

        // 购物车内直接余额支付（余额足够时一键支付）
        if (currentUser->getBalance() >= pendingAmount) {
            int payNow = QMessageBox::question(
                this,
                "余额支付",
                QString("订单已创建：%1\n应付金额：%2 元\n当前余额：%3 元\n\n是否立即使用余额支付？")
                    .arg(pendingOrderId)
                    .arg(pendingAmount, 0, 'f', 2)
                    .arg(currentUser->getBalance(), 0, 'f', 2),
                QMessageBox::Yes | QMessageBox::No
            );

            if (payNow == QMessageBox::Yes) {
                // 直接走余额支付，不依赖支付页面控件
                QJsonObject payResp = apiService->payOrder(pendingOrderId, "账户余额支付");
                if (payResp.value("success").toBool()) {
                    // 更新余额（从服务器响应中获取）
                    if (payResp.contains("balance")) {
                        currentUser->setBalance(payResp["balance"].toDouble());
                    } else {
                        currentUser->deductBalance(pendingAmount);
                    }
                    // 从数据库中删除已结算的商品
                    for (const auto &cartItem : pendingCartItems) {
                        QJsonObject removeResp = apiService->removeFromCart(
                            QString::number(currentUser->getId()),
                            cartItem.bookId
                        );
                        if (!removeResp.value("success").toBool()) {
                            qWarning() << "删除购物车商品失败:" << cartItem.bookId 
                                      << removeResp.value("message").toString();
                        }
                    }
                    
                    // 清空本地购物车
                    currentUser->clearCart();
                    updateCartDisplay();
                    updateProfileDisplay(); // 如果用户正好在资料页，也同步一下
                    
                    // 更新订单状态为已支付
                    for (auto &o : allOrders) {
                        if (o.getOrderId() == pendingOrderId) {
                            o.setStatus("已支付");
                            break;
                        }
                    }

                    QMessageBox::information(this, "支付成功",
                        QString("余额支付成功！\n订单号：%1\n支付金额：%2 元\n剩余余额：%3 元")
                            .arg(pendingOrderId)
                            .arg(pendingAmount, 0, 'f', 2)
                            .arg(currentUser->getBalance(), 0, 'f', 2));

                    pendingOrderId.clear();
                    pendingAmount = 0.0;
                    pendingCartItems.clear();
                    return;
                } else {
                    QString errorMsg = payResp.value("message").toString();
                    QMessageBox::warning(this, "支付失败", errorMsg.isEmpty() ? "支付失败，请重试" : errorMsg);
                    // 支付失败则进入支付页，允许重试/取消
                    showPaymentPage();
                    return;
                }
            }
        } else {
            int goRecharge = QMessageBox::question(
                this,
                "余额不足",
                QString("订单已创建：%1\n应付金额：%2 元\n当前余额：%3 元\n\n余额不足，是否前往充值？")
                    .arg(pendingOrderId)
                    .arg(pendingAmount, 0, 'f', 2)
                    .arg(currentUser->getBalance(), 0, 'f', 2),
                QMessageBox::Yes | QMessageBox::No
            );
            if (goRecharge == QMessageBox::Yes) {
                updateProfileDisplay();
                showProfilePage();
                return;
            }
        }

        // 默认：跳转到支付页面（可点“确认支付/取消支付”）
        showPaymentPage();
    } else {
        QString errorMsg = response.value("message").toString();
        QMessageBox::warning(this, "下单失败", errorMsg.isEmpty() ? "创建订单失败" : errorMsg);
    }
}

void Purchaser::onDirectBuyClicked()
{
    if (!isLoggedIn || !currentUser) {
        QMessageBox::warning(this, "操作失败", "请先登录");
        showLoginPage();
        return;
    }

    // 使用TCP请求创建订单（待支付状态）
    if (!apiService->isConnected()) {
        if (!apiService->connectToServer(serverIp, serverPort)) {
            QMessageBox::warning(this, "连接失败", "无法连接到服务器");
            return;
        }
    }

    // 下单前校验收货信息（电话和地址）
    // 从currentUser获取用户名，而不是从UI控件（UI中已删除用户名显示）
    QString customerName = currentUser ? currentUser->getUsername() : "";
    QString phone = profilePhone ? profilePhone->text().trimmed() : "";
    QString address = profileAddress ? profileAddress->text().trimmed() : "";

    if (phone.isEmpty() || address.isEmpty()) {
        QMessageBox::warning(this,
                             "收货信息不完整",
                             "下单前请先在\"个人中心\"填写并保存联系电话和收货地址。");
        // 跳转到个人信息页面，方便用户填写
        showProfilePage();
        return;
    }

    int quantity = quantitySpinBox->value();
    double total = currentBook.getPrice() * quantity;

    // 显示确认对话框
    int result = QMessageBox::question(this, "确认购买",
                                      QString("%1 × %2 = %3元\n确认购买吗？")
                                      .arg(currentBook.getTitle())
                                      .arg(quantity)
                                      .arg(total, 0, 'f', 2),
                                      QMessageBox::Yes | QMessageBox::No);

    if (result != QMessageBox::Yes) {
        return;
    }

    // 构建订单项
    QJsonArray items;
    QJsonObject item;
    item["bookId"] = currentBook.getId();
    item["bookName"] = currentBook.getTitle();
    item["quantity"] = quantity;
    item["price"] = currentBook.getPrice();
    items.append(item);

    // 调用API创建订单
    QJsonObject response = apiService->createOrder(
        QString::number(currentUser->getId()),
        items,
        customerName,
        phone,
        address
    );

    if (response.value("success").toBool()) {
        QString orderId = response.value("orderId").toString();
        double totalAmount = response.value("totalAmount").toDouble(0.0);
        
        // 如果服务器没有返回总金额，使用本地计算
        if (totalAmount <= 0) {
            totalAmount = total;
        }
        
        // 保存待支付订单信息
        pendingOrderId = orderId;
        pendingAmount = totalAmount;
        
        // 保存待支付订单商品快照
        pendingCartItems.clear();
        CartItem cartItem;
        cartItem.bookId = currentBook.getId();
        cartItem.bookTitle = currentBook.getTitle();
        cartItem.price = currentBook.getPrice();
        cartItem.quantity = quantity;
        pendingCartItems.append(cartItem);
        
        // 将订单写入本地订单列表，状态=待支付
        {
            // 防止重复追加（同订单号只保留一份）
            bool exists = false;
            for (const auto &o : allOrders) {
                if (o.getOrderId() == pendingOrderId) { exists = true; break; }
            }
            if (!exists) {
                Order localOrder(pendingOrderId, currentUser->getId(), QDate::currentDate());
                OrderItem oi;
                oi.bookId = currentBook.getId();
                oi.bookTitle = currentBook.getTitle();
                oi.quantity = quantity;
                oi.price = currentBook.getPrice();
                oi.status = "待支付";
                localOrder.addItem(oi);
                localOrder.setTotalAmount(pendingAmount);
                localOrder.setStatus("待支付");
                allOrders.append(localOrder);
            }
        }

        // 立即购买：如果余额足够，询问是否立即支付
        if (currentUser->getBalance() >= pendingAmount) {
            int payNow = QMessageBox::question(
                this,
                "余额支付",
                QString("订单已创建：%1\n应付金额：%2 元\n当前余额：%3 元\n\n是否立即使用余额支付？")
                    .arg(pendingOrderId)
                    .arg(pendingAmount, 0, 'f', 2)
                    .arg(currentUser->getBalance(), 0, 'f', 2),
                QMessageBox::Yes | QMessageBox::No
            );

            if (payNow == QMessageBox::Yes) {
                // 直接走余额支付
                QJsonObject payResp = apiService->payOrder(pendingOrderId, "账户余额支付");
                if (payResp.value("success").toBool()) {
                    // 更新余额（从服务器响应中获取）
                    if (payResp.contains("balance")) {
                        currentUser->setBalance(payResp["balance"].toDouble());
                    } else {
                        currentUser->deductBalance(pendingAmount);
                    }
                    
                    // 更新订单状态为已支付
                    for (auto &o : allOrders) {
                        if (o.getOrderId() == pendingOrderId) {
                            o.setStatus("已支付");
                            break;
                        }
                    }
                    
                    QMessageBox::information(this, "支付成功",
                        QString("订单支付成功！\n订单号: %1\n支付金额: %2元\n当前余额: %3元")
                        .arg(pendingOrderId)
                        .arg(pendingAmount, 0, 'f', 2)
                        .arg(currentUser->getBalance(), 0, 'f', 2));
                    
                    showMainPage();
                } else {
                    QMessageBox::warning(this, "支付失败", payResp.value("message").toString());
                }
            } else {
                // 用户选择稍后支付，跳转到支付页面
                showPaymentPage();
            }
        } else {
            // 余额不足，跳转到支付页面
            QMessageBox::information(this, "订单已创建",
                QString("订单创建成功！\n订单号: %1\n应付金额: %2元\n当前余额: %3元\n余额不足，请选择支付方式")
                .arg(pendingOrderId)
                .arg(pendingAmount, 0, 'f', 2)
                .arg(currentUser->getBalance(), 0, 'f', 2));
            showPaymentPage();
        }
    } else {
        QString errorMsg = response.value("message").toString();
        QMessageBox::warning(this, "订单创建失败", errorMsg.isEmpty() ? "订单创建失败，请稍后重试" : errorMsg);
    }
}
void Purchaser::onViewCartClicked()
{
    if (!isLoggedIn || !currentUser) {
        QMessageBox::warning(this, "操作失败", "请先登录");
        showLoginPage();
        return;
    }

    // 确保已连接到服务器
    if (!apiService->isConnected()) {
        if (!apiService->connectToServer(serverIp, serverPort)) {
            QMessageBox::warning(this, "连接失败", "无法连接到服务器");
            return;
        }
        // 移除阻塞延迟，连接后立即使用
        QCoreApplication::processEvents();  // 处理事件，确保连接完成
    }

    // 从数据库加载购物车数据
    QJsonObject response = apiService->getCart(QString::number(currentUser->getId()));
    if (response.value("success").toBool()) {
        // 清空本地购物车
        currentUser->clearCart();
        
        // 从服务器响应中加载购物车项
        QJsonArray items = response.value("items").toArray();
        for (const QJsonValue &itemVal : items) {
            QJsonObject item = itemVal.toObject();
            QString bookId = item.value("bookId").toString();
            QString bookName = item.value("bookName").toString();
            double price = item.value("price").toDouble();
            int quantity = item.value("quantity").toInt();
            
            // 添加到本地购物车
            currentUser->addToCart(bookId, quantity, bookName, price);
        }
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

    QString bookId = currentBook.getId();
    
    // 检查是否已经收藏
    if (currentUser->isFavorite(bookId)) {
        QMessageBox::information(this, "提示", "已经在收藏中了");
        return;
    }

    // 调用服务器API添加到收藏
    if (!apiService->isConnected()) {
        if (!apiService->connectToServer(serverIp, serverPort)) {
            QMessageBox::warning(this, "连接失败", "无法连接到服务器");
            return;
        }
    }
    
    QJsonObject response = apiService->addFavorite(QString::number(currentUser->getId()), bookId);
    if (response.value("success").toBool()) {
        // 如果服务器添加成功，更新本地用户对象
        if (!currentUser->favoriteBooks.contains(bookId)) {
            currentUser->favoriteBooks.append(bookId);
        }
        QMessageBox::information(this, "成功", "已添加到收藏");
        updateProfileDisplay();
    } else {
        QString errorMsg = response.value("message").toString();
        QMessageBox::warning(this, "失败", errorMsg.isEmpty() ? "添加到收藏失败" : errorMsg);
    }
}

void Purchaser::onViewOrderClicked()
{
    if (!isLoggedIn || !currentUser) {
        QMessageBox::warning(this, "操作失败", "请先登录");
        showLoginPage();
        return;
    }

    // 先显示订单页面，显示加载状态
    showOrdersPage();
    
    // 清空表格，准备加载新数据
    ordersTable->setRowCount(0);
    ordersTable->update();
    ordersTable->repaint();

    // 使用TCP请求获取订单列表
    if (!apiService->isConnected()) {
        if (!apiService->connectToServer(serverIp, serverPort)) {
            QMessageBox::warning(this, "连接失败", "无法连接到服务器");
            return;
        }
    }

    qDebug() << "onViewOrderClicked: 开始获取订单，用户ID:" << currentUser->getId();
    
    QJsonObject response = apiService->getUserOrders(QString::number(currentUser->getId()));
    
    // 调试：打印完整响应
    qDebug() << "onViewOrderClicked: 收到完整响应:" << QJsonDocument(response).toJson(QJsonDocument::Compact);
    
    // 检查响应是否有效
    if (response.isEmpty()) {
        qWarning() << "onViewOrderClicked: 响应为空";
        QMessageBox::warning(this, "获取订单失败", "服务器响应为空，请稍后重试");
        return;
    }
    
    if (response.value("success").toBool()) {
        QJsonArray orders = response.value("orders").toArray();
        int total = response.value("total").toInt();
        
        qDebug() << "onViewOrderClicked: 收到订单响应，订单数组大小:" << orders.size() << "总数:" << total;
        
        // 确保表格已清空
        if (ordersTable->rowCount() > 0) {
            ordersTable->setRowCount(0);
        }
        
        if (orders.isEmpty()) {
            qDebug() << "onViewOrderClicked: 订单列表为空";
            // 不显示消息框，让用户看到空表格
            // QMessageBox::information(this, "提示", "您还没有任何订单");
        } else {
            int validOrderCount = 0;
            int skippedOrderCount = 0;
            
            // 先禁用表格更新，提高性能
            ordersTable->setUpdatesEnabled(false);
            
            for (int i = 0; i < orders.size(); ++i) {
                const QJsonValue &orderVal = orders[i];
                QJsonObject order = orderVal.toObject();
                
                // 调试：打印每个订单的详细信息
                qDebug() << "onViewOrderClicked: 处理订单" << i << ":" << QJsonDocument(order).toJson(QJsonDocument::Compact);
                
                // 验证订单数据是否完整
                if (!order.contains("orderId")) {
                    qWarning() << "onViewOrderClicked: 跳过无效订单，缺少orderId字段。订单数据:" << QJsonDocument(order).toJson(QJsonDocument::Compact);
                    skippedOrderCount++;
                    continue;
                }
                
                QString orderId = order["orderId"].toString();
                if (orderId.isEmpty()) {
                    qWarning() << "onViewOrderClicked: 跳过无效订单，orderId为空。订单数据:" << QJsonDocument(order).toJson(QJsonDocument::Compact);
                    skippedOrderCount++;
                    continue;
                }
                
                validOrderCount++;
                
                int row = ordersTable->rowCount();
                ordersTable->insertRow(row);
                
                ordersTable->setItem(row, 0, new QTableWidgetItem(order["orderId"].toString()));
                ordersTable->setItem(row, 1, new QTableWidgetItem(order["orderDate"].toString()));
                
                // 商品信息
                QString itemsStr;
                QJsonArray items = order["items"].toArray();
                if (items.isEmpty()) {
                    itemsStr = "无商品信息";
                } else {
                    for (const QJsonValue &itemVal : items) {
                        QJsonObject item = itemVal.toObject();
                        if (!itemsStr.isEmpty()) itemsStr += ", ";
                        // 兼容不同的字段名：bookName 或 title
                        QString bookName = item.contains("bookName") ? item["bookName"].toString() : item["title"].toString();
                        if (bookName.isEmpty()) {
                            bookName = item.contains("bookId") ? item["bookId"].toString() : "未知商品";
                        }
                        itemsStr += QString("%1 x%2").arg(bookName).arg(item["quantity"].toInt());
                    }
                }
                ordersTable->setItem(row, 2, new QTableWidgetItem(itemsStr));
                
                ordersTable->setItem(row, 3, new QTableWidgetItem(QString::number(order["totalAmount"].toDouble(), 'f', 2)));
                ordersTable->setItem(row, 4, new QTableWidgetItem(order["status"].toString()));
                ordersTable->setItem(row, 5, new QTableWidgetItem(order["shipTime"].toString().isEmpty() ? "未发货" : order["shipTime"].toString()));
            }
            
            // 重新启用表格更新
            ordersTable->setUpdatesEnabled(true);
            
            qDebug() << "onViewOrderClicked: 成功加载了" << ordersTable->rowCount() << "个订单到表格";
            qDebug() << "onViewOrderClicked: 有效订单数:" << validOrderCount << "跳过订单数:" << skippedOrderCount;
            
            // 强制刷新表格显示
            ordersTable->resizeColumnsToContents();
            ordersTable->update();
            ordersTable->repaint();
            
            // 确保表格可见并刷新
            ordersTable->setVisible(true);
            ordersTable->viewport()->update();
            
            if (validOrderCount == 0 && skippedOrderCount > 0) {
                qWarning() << "onViewOrderClicked: 所有订单都被跳过，可能是数据格式问题";
                QMessageBox::warning(this, "数据格式错误", QString("收到%1个订单，但都因数据格式问题无法显示。请检查服务器日志。").arg(skippedOrderCount));
            } else if (ordersTable->rowCount() == 0 && orders.size() > 0) {
                qWarning() << "onViewOrderClicked: 订单数据存在但表格为空，可能是显示问题";
                QMessageBox::warning(this, "显示错误", QString("收到%1个订单，但表格未显示。请检查数据格式。").arg(orders.size()));
            } else if (ordersTable->rowCount() > 0) {
                qDebug() << "onViewOrderClicked: 订单表格已成功显示" << ordersTable->rowCount() << "个订单";
            }
        }
    } else {
        QString errorMsg = response.value("message").toString();
        qWarning() << "获取订单列表失败:" << errorMsg;
        QMessageBox::warning(this, "获取订单失败", errorMsg.isEmpty() ? "无法获取订单列表，请稍后重试" : errorMsg);
    }
}

void Purchaser::onCancelOrderClicked()
{
    if (selectedOrderRow < 0) {
        QMessageBox::warning(this, "提示", "请先选择要取消的订单！");
        return;
    }
    
    QTableWidgetItem *orderIdItem = ordersTable->item(selectedOrderRow, 0);
    QTableWidgetItem *statusItem = ordersTable->item(selectedOrderRow, 4);
    
    if (!orderIdItem || !statusItem) {
        QMessageBox::warning(this, "错误", "无法获取订单信息，请刷新订单列表后重试");
        return;
    }
    
    QString orderId = orderIdItem->text().trimmed();
    QString status = statusItem->text().trimmed();
    
    if (orderId.isEmpty()) {
        QMessageBox::warning(this, "错误", "订单ID为空，请刷新订单列表后重试");
        return;
    }
    
    // 只有待支付和已支付状态的订单可以取消
    if (status != "待支付" && status != "已支付") {
        QMessageBox::warning(this, "无法取消", 
            QString("只有【待支付】或【已支付】状态的订单才能取消！\n当前订单状态：%1").arg(status));
        return;
    }
    
    // 确认取消
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, 
        "确认取消订单", 
        QString("确定要取消订单 %1 吗？").arg(orderId),
        QMessageBox::Yes | QMessageBox::No
    );
    
    if (reply != QMessageBox::Yes) {
        return;
    }
    
    // 调用取消订单API
    qDebug() << "准备取消订单，订单ID:" << orderId << "用户ID:" << currentUser->getId();
    QJsonObject response = apiService->cancelOrder(orderId, QString::number(currentUser->getId()), "用户申请取消");
    
    if (response["success"].toBool()) {
        QMessageBox::information(this, "成功", "订单已取消！");
        onViewOrderClicked();  // 刷新订单列表
    } else {
        QMessageBox::warning(this, "错误", "取消订单失败：" + response["message"].toString());
    }
}

void Purchaser::onConfirmReceiveClicked()
{
    if (selectedOrderRow < 0) {
        QMessageBox::warning(this, "提示", "请先选择要确认收货的订单！");
        return;
    }
    
    QTableWidgetItem *orderIdItem = ordersTable->item(selectedOrderRow, 0);
    QTableWidgetItem *statusItem = ordersTable->item(selectedOrderRow, 4);
    
    if (!orderIdItem || !statusItem) {
        QMessageBox::warning(this, "错误", "无法获取订单信息，请刷新订单列表后重试");
        return;
    }
    
    QString orderId = orderIdItem->text().trimmed();
    QString status = statusItem->text().trimmed();
    
    if (orderId.isEmpty()) {
        QMessageBox::warning(this, "错误", "订单ID为空，请刷新订单列表后重试");
        return;
    }
    
    // 只有已发货状态的订单可以确认收货
    if (status != "已发货") {
        QMessageBox::warning(this, "无法确认收货", 
            QString("只有【已发货】状态的订单才能确认收货！\n当前订单状态：%1").arg(status));
        return;
    }
    
    // 确认收货
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, 
        "确认收货", 
        QString("确定要确认收货订单 %1 吗？\n确认后订单将变为已完成状态。").arg(orderId),
        QMessageBox::Yes | QMessageBox::No
    );
    
    if (reply != QMessageBox::Yes) {
        return;
    }
    
    // 调用确认收货API
    qDebug() << "准备确认收货，订单ID:" << orderId << "用户ID:" << currentUser->getId();
    QJsonObject response = apiService->confirmReceiveOrder(orderId, QString::number(currentUser->getId()));
    
    if (response["success"].toBool()) {
        QMessageBox::information(this, "成功", "确认收货成功！订单状态已更新为已完成。");
        onViewOrderClicked();  // 刷新订单列表
    } else {
        QMessageBox::warning(this, "错误", "确认收货失败：" + response["message"].toString());
    }
}

void Purchaser::onOrderTableCellClicked(int row, int column)
{
    selectedOrderRow = row;
}

void Purchaser::onUpdateProfileClicked()
{
    if (!currentUser) {
        QMessageBox::warning(this, "错误", "请先登录");
        return;
    }

    QString phone = profilePhone ? profilePhone->text().trimmed() : "";
    QString email = profileEmail ? profileEmail->text().trimmed() : "";
    QString address = profileAddress ? profileAddress->text().trimmed() : "";

    // 确保已连接到服务器
    if (!apiService->isConnected()) {
        if (!apiService->connectToServer(serverIp, serverPort)) {
            QMessageBox::warning(this, "连接失败", "无法连接到服务器");
            return;
        }
    }

    // 调用服务器API更新用户信息
    QJsonObject response = apiService->updateUserInfo(
        QString::number(currentUser->getId()),
        phone,
        email,
        address
    );

    if (response.value("success").toBool()) {
        // 更新本地用户对象
        ChangeInformation("phone", phone);
        ChangeInformation("email", email);
        ChangeInformation("address", address);
        
        QMessageBox::information(this, "成功", "个人信息已更新并保存到数据库");
        updateProfileDisplay();
    } else {
        QString errorMsg = response.value("message").toString();
        QMessageBox::warning(this, "更新失败", errorMsg.isEmpty() ? "更新用户信息失败" : errorMsg);
    }
}

// 修改密码
void Purchaser::onChangePasswordClicked()
{
    if (!currentUser) {
        QMessageBox::warning(this, "错误", "请先登录");
        return;
    }

    QString oldPassword = oldPasswordEdit->text();
    QString newPassword = newPasswordEdit->text();
    QString confirmPassword = confirmPasswordEdit->text();

    // 验证输入
    if (oldPassword.isEmpty()) {
        QMessageBox::warning(this, "错误", "请输入当前密码");
        return;
    }

    if (newPassword.isEmpty()) {
        QMessageBox::warning(this, "错误", "请输入新密码");
        return;
    }

    if (newPassword.length() < 6) {
        QMessageBox::warning(this, "错误", "新密码长度至少为6位");
        return;
    }

    if (newPassword != confirmPassword) {
        QMessageBox::warning(this, "错误", "两次输入的新密码不一致");
        return;
    }

    // 验证当前密码是否正确
    if (oldPassword != currentUser->password) {
        QMessageBox::warning(this, "错误", "当前密码不正确");
        oldPasswordEdit->clear();
        return;
    }

    // 调用API修改密码
    QJsonObject response = apiService->changePassword(
        QString::number(currentUser->getId()),
        oldPassword,
        newPassword
    );

    if (response.value("success").toBool()) {
        // 更新本地密码
        currentUser->password = newPassword;
        
        // 清空输入框
        oldPasswordEdit->clear();
        newPasswordEdit->clear();
        confirmPasswordEdit->clear();
        
        QMessageBox::information(this, "成功", "密码修改成功");
    } else {
        QString errorMsg = response.value("message").toString();
        if (errorMsg.isEmpty()) {
            errorMsg = "密码修改失败，请稍后重试";
        }
        QMessageBox::warning(this, "失败", errorMsg);
    }
}

// 选择营业执照图片
void Purchaser::onSelectLicenseImageClicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,
        "选择营业执照图片",
        "",
        "图片文件 (*.jpg *.jpeg *.png *.bmp);;所有文件 (*.*)");
    
    if (fileName.isEmpty()) {
        return;
    }
    
    // 加载图片
    QPixmap pixmap(fileName);
    if (pixmap.isNull()) {
        QMessageBox::warning(this, "错误", "无法加载图片文件");
        return;
    }
    
    // 缩放图片以适应预览区域（最大宽度600px）
    if (pixmap.width() > 600) {
        pixmap = pixmap.scaledToWidth(600, Qt::SmoothTransformation);
    }
    
    // 显示预览
    licenseImageLabel->setPixmap(pixmap);
    licenseImageLabel->setScaledContents(false);
    licenseImagePath = fileName;
    
    // 将图片转换为Base64编码
    QByteArray imageData;
    QBuffer buffer(&imageData);
    buffer.open(QIODevice::WriteOnly);
    pixmap.save(&buffer, "PNG");  // 保存为PNG格式
    licenseImageBase64 = imageData.toBase64();
    
    // 启用提交按钮
    applySellerBtn->setEnabled(true);
    
    qDebug() << "已选择营业执照图片:" << fileName << "大小:" << licenseImageBase64.size() << "字节";
}

// 申请成为卖家（提交营业执照图片到数据库）
void Purchaser::onApplySellerClicked()
{
    if (!currentUser) {
        QMessageBox::warning(this, "错误", "请先登录");
        return;
    }
    
    if (licenseImageBase64.isEmpty()) {
        QMessageBox::warning(this, "错误", "请先选择营业执照图片");
        return;
    }
    
    // 确保已连接到服务器
    if (!apiService->isConnected()) {
        if (!apiService->connectToServer(serverIp, serverPort)) {
            QMessageBox::warning(this, "连接失败", "无法连接到服务器");
            return;
        }
        // 移除阻塞延迟，连接后立即使用
        QCoreApplication::processEvents();  // 处理事件，确保连接完成
    }
    
    // 发送认证申请（将图片保存到数据库）
    qDebug() << "准备提交营业执照图片到数据库";
    qDebug() << "用户ID:" << currentUser->getId();
    qDebug() << "用户名:" << currentUser->getUsername();
    qDebug() << "密码长度:" << currentUser->getPassword().length();
    qDebug() << "邮箱:" << currentUser->getEmail();
    qDebug() << "图片Base64大小:" << licenseImageBase64.size() << "字节";
    
    // 获取密码
    QString userPassword = currentUser->getPassword();
    if (userPassword.isEmpty()) {
        QMessageBox::warning(this, "错误", "无法获取用户密码，请重新登录");
        qDebug() << "错误：用户密码为空";
        return;
    }
    
    QJsonObject response = apiService->applySellerCertification(
        QString::number(currentUser->getId()),
        currentUser->getUsername(),
        userPassword,
        currentUser->getEmail(),
        licenseImageBase64
    );
    
    qDebug() << "服务器响应:" << QJsonDocument(response).toJson(QJsonDocument::Compact);
    
    if (response.value("success").toBool()) {
        // 根据需求：提示"已提交，等待管理员审核"
        QMessageBox::information(this, "提交成功",
            "已提交，等待管理员审核");
        
        // 更新状态 - 显示"审核中"（因为role已经改为0）
        sellerStatusLabel->setText("⏳ 认证审核中，请耐心等待");
        sellerStatusLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: #f39c12;");
        selectLicenseBtn->setEnabled(false);
        applySellerBtn->setEnabled(false);
        applySellerBtn->setText("审核中");
        
        // 清空图片预览
        licenseImageLabel->setText("营业执照已提交\n等待管理员审核");
        licenseImagePath.clear();
        licenseImageBase64.clear();

        // 刷新状态显示，以便下次打开页面时能正确显示"审核中"状态
        // 这里不立即查询，因为数据库更新可能有延迟，让用户下次打开时再查询
    } else {
        QString errorMsg = response.value("message").toString();
        QMessageBox::warning(this, "提交失败", errorMsg.isEmpty() ? "提交失败，请重试" : errorMsg);
    }
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
    if (!isLoggedIn || !currentUser) {
        QMessageBox::warning(this, "操作失败", "请先登录");
        showLoginPage();
        return;
    }
    
    // 显示客服页面
    showServicePage();
    
    // 加载聊天历史
    loadChatHistory();
}

void Purchaser::showServicePage()
{
    stackedWidget->setCurrentWidget(servicePage);
    
    // 启动聊天刷新定时器
    if (!chatRefreshTimer->isActive()) {
        chatRefreshTimer->start();
    }
}

void Purchaser::loadChatHistory()
{
    if (!isLoggedIn || !currentUser) {
        return;
    }
    
    // 连接服务器
    if (!apiService->isConnected()) {
        if (!apiService->connectToServer(serverIp, serverPort)) {
            QMessageBox::warning(this, "连接失败", "无法连接到服务器");
            return;
        }
    }
    
    // 获取聊天历史（发送给客服，receiverId为空表示发送给管理员）
    QJsonObject response = apiService->getChatHistory(
        QString::number(currentUser->getId()), 
        "buyer"
    );
    
    if (response["success"].toBool()) {
        chatDisplay->clear();
        QJsonArray messages = response["messages"].toArray();
        
        for (const QJsonValue &msgVal : messages) {
            QJsonObject msg = msgVal.toObject();
            QString senderType = msg["senderType"].toString();
            QString content = msg["content"].toString();
            QString sendTime = msg["sendTime"].toString();
            
            // 格式化时间
            QDateTime dateTime = QDateTime::fromString(sendTime, "yyyy-MM-dd hh:mm:ss");
            if (!dateTime.isValid()) {
                dateTime = QDateTime::fromString(sendTime, Qt::ISODate);
            }
            QString timeStr = dateTime.isValid() ? dateTime.toString("yyyy-MM-dd hh:mm") : sendTime;
            
            // 显示消息
            QString senderName;
            if (senderType == "buyer") {
                senderName = "我";
            } else if (senderType == "admin") {
                senderName = "客服";
            } else if (senderType == "seller") {
                senderName = "卖家";
            } else {
                senderName = "未知";
            }
            
            chatDisplay->append(QString("[%1] %2: %3").arg(timeStr).arg(senderName).arg(content));
        }
        
        // 滚动到底部
        QTextCursor cursor = chatDisplay->textCursor();
        cursor.movePosition(QTextCursor::End);
        chatDisplay->setTextCursor(cursor);
    }
}

void Purchaser::onSendFeedbackClicked()
{
    if (!isLoggedIn || !currentUser) {
        QMessageBox::warning(this, "操作失败", "请先登录");
        showLoginPage();
        return;
    }
    
    QString message = feedbackInput->toPlainText().trimmed();
    if (message.isEmpty()) {
        QMessageBox::warning(this, "发送失败", "请输入消息内容");
        return;
    }

    // 连接服务器
    if (!apiService->isConnected()) {
        if (!apiService->connectToServer(serverIp, serverPort)) {
            QMessageBox::warning(this, "连接失败", "无法连接到服务器");
            return;
        }
    }
    
    // 发送消息给客服（receiverId为空表示发送给管理员/客服）
    QJsonObject response = apiService->sendChatMessage(
        QString::number(currentUser->getId()),
        "buyer",
        "",  // 发送给客服，receiverId为空
        "",  // 接收者是管理员，但receiverId为空时表示发送给所有管理员
        message  // 消息内容
    );
    
    if (response["success"].toBool()) {
        feedbackInput->clear();
        
        // 不在这里直接显示消息，避免与定时器刷新时重复显示
        // 立即触发一次聊天历史加载，让服务器返回的消息被正确显示
        loadChatHistory();
    } else {
        QMessageBox::warning(this, "发送失败", response["message"].toString());
    }
}

// 页面切换函数
void Purchaser::showLoginPage()
{
    stackedWidget->setCurrentWidget(loginPage);
}

void Purchaser::showMainPage()
{
    // 停止聊天刷新定时器
    if (chatRefreshTimer && chatRefreshTimer->isActive()) {
        chatRefreshTimer->stop();
    }
    
    // 从服务器加载卖家上架的图书
    loadBooks();
    updateRecommendations();
    stackedWidget->setCurrentWidget(mainPage);
}

void Purchaser::showBookDetailPage()
{
    // 加载评论和评分
    loadBookRatingStats();
    loadBookReviews();
    
    stackedWidget->setCurrentWidget(bookDetailPage);
}

// 加载商品评分统计
void Purchaser::loadBookRatingStats()
{
    if (currentBook.getId().isEmpty()) {
        return;
    }
    
    if (!apiService->isConnected()) {
        if (!apiService->connectToServer(serverIp, serverPort)) {
            return;
        }
    }
    
    QJsonObject response = apiService->getBookRatingStats(currentBook.getId());
    if (response.value("success").toBool()) {
        double avgRating = response.value("averageRating").toDouble();
        int reviewCount = response.value("reviewCount").toInt();
        bool hasRating = response.value("hasRating").toBool();
        
        if (hasRating && reviewCount > 0) {
            ratingStatsLabel->setText(QString("评分：%1分（%2条评论）").arg(avgRating, 0, 'f', 1).arg(reviewCount));
            bookScoreLabel->setText(QString::number(avgRating, 'f', 1));
        } else {
            ratingStatsLabel->setText("评分：暂无评分");
            bookScoreLabel->setText("暂无");
        }
    }
}

// 加载商品评论
void Purchaser::loadBookReviews()
{
    if (currentBook.getId().isEmpty()) {
        reviewsDisplay->clear();
        return;
    }
    
    if (!apiService->isConnected()) {
        if (!apiService->connectToServer(serverIp, serverPort)) {
            return;
        }
    }
    
    QJsonObject response = apiService->getBookReviews(currentBook.getId());
    if (response.value("success").toBool()) {
        QJsonArray reviews = response.value("reviews").toArray();
        
        QString reviewsText;
        if (reviews.isEmpty()) {
            reviewsText = "暂无评论";
        } else {
            for (const QJsonValue &reviewVal : reviews) {
                QJsonObject review = reviewVal.toObject();
                QString username = review.value("username").toString();
                int rating = review.value("rating").toInt();
                QString comment = review.value("comment").toString();
                QString reviewTime = review.value("reviewTime").toString();
                
                reviewsText += QString("【%1】%2分 - %3\n时间：%4\n\n")
                    .arg(username)
                    .arg(rating)
                    .arg(comment)
                    .arg(reviewTime);
            }
        }
        
        reviewsDisplay->setPlainText(reviewsText);
    }
}

// 添加评论
void Purchaser::onAddReviewClicked()
{
    if (!isLoggedIn || !currentUser) {
        QMessageBox::warning(this, "操作失败", "请先登录");
        showLoginPage();
        return;
    }
    
    if (currentBook.getId().isEmpty()) {
        QMessageBox::warning(this, "操作失败", "请先选择商品");
        return;
    }
    
    // 创建评论对话框
    QDialog *reviewDialog = new QDialog(this);
    reviewDialog->setWindowTitle("评价商品");
    reviewDialog->setModal(true);
    
    QVBoxLayout *dialogLayout = new QVBoxLayout(reviewDialog);
    
    QLabel *bookLabel = new QLabel(QString("商品：%1").arg(currentBook.getTitle()));
    dialogLayout->addWidget(bookLabel);
    
    QLabel *ratingLabel = new QLabel("评分（1-5分）：");
    dialogLayout->addWidget(ratingLabel);
    
    QSpinBox *ratingSpinBox = new QSpinBox();
    ratingSpinBox->setRange(1, 5);
    ratingSpinBox->setValue(5);
    dialogLayout->addWidget(ratingSpinBox);
    
    QLabel *commentLabel = new QLabel("评论内容：");
    dialogLayout->addWidget(commentLabel);
    
    QTextEdit *commentEdit = new QTextEdit();
    commentEdit->setMaximumHeight(150);
    dialogLayout->addWidget(commentEdit);
    
    QPushButton *submitBtn = new QPushButton("提交");
    QPushButton *cancelBtn = new QPushButton("取消");
    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->addStretch();
    btnLayout->addWidget(submitBtn);
    btnLayout->addWidget(cancelBtn);
    dialogLayout->addLayout(btnLayout);
    
    connect(submitBtn, &QPushButton::clicked, reviewDialog, &QDialog::accept);
    connect(cancelBtn, &QPushButton::clicked, reviewDialog, &QDialog::reject);
    
    if (reviewDialog->exec() == QDialog::Accepted) {
        int rating = ratingSpinBox->value();
        QString comment = commentEdit->toPlainText().trimmed();
        
        if (comment.isEmpty()) {
            QMessageBox::warning(this, "评论失败", "请输入评论内容");
            return;
        }
        
        if (!apiService->isConnected()) {
            if (!apiService->connectToServer(serverIp, serverPort)) {
                QMessageBox::warning(this, "连接失败", "无法连接到服务器");
                return;
            }
        }
        
        QJsonObject response = apiService->addReview(
            QString::number(currentUser->getId()),
            currentBook.getId(),
            rating,
            comment
        );
        
        if (response.value("success").toBool()) {
            QMessageBox::information(this, "成功", "评论提交成功");
            // 重新加载评论和评分
            loadBookRatingStats();
            loadBookReviews();
        } else {
            QString errorMsg = response.value("message").toString();
            QMessageBox::warning(this, "评论失败", errorMsg.isEmpty() ? "评论提交失败，请稍后重试" : errorMsg);
        }
    }
    
    delete reviewDialog;
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
    if (currentUser) {
        updateProfileDisplay();
    }
    stackedWidget->setCurrentWidget(profilePage);
}

void Purchaser::onLotteryClicked()
{
    if (!isLoggedIn || !currentUser) {
        QMessageBox::warning(this, "操作失败", "请先登录");
        showLoginPage();
        return;
    }
    
    // 检查积分是否足够
    if (!currentUser->canParticipateLottery()) {
        QMessageBox::warning(this, "积分不足", 
            QString("您的积分不足，无法参与抽奖。\n当前积分：%1，需要3积分才能参与抽奖。\n每充值100元可获得1积分。")
                .arg(currentUser->getPoints()));
        return;
    }
    
    // 确认对话框
    int ret = QMessageBox::question(this, "确认抽奖", 
        QString("参与抽奖将消耗3积分。\n当前积分：%1\n确定要参与抽奖吗？").arg(currentUser->getPoints()),
        QMessageBox::Yes | QMessageBox::No);
    
    if (ret != QMessageBox::Yes) {
        return;
    }
    
    // 连接服务器
    if (!apiService->isConnected()) {
        if (!apiService->connectToServer(serverIp, serverPort)) {
            QMessageBox::warning(this, "连接失败", "无法连接到服务器");
            return;
        }
    }
    
    // 调用抽奖API
    QJsonObject response = apiService->participateLottery(QString::number(currentUser->getId()));
    
    if (response["success"].toBool()) {
        QString prize = response["prize"].toString();
        int remainingPoints = response["remainingPoints"].toInt();
        
        // 更新本地积分
        currentUser->setPoints(remainingPoints);
        
        // 更新优惠券数量（如果抽中了优惠券）
        if (response.contains("coupon30")) {
            currentUser->setCoupon30(response["coupon30"].toInt());
        }
        if (response.contains("coupon50")) {
            currentUser->setCoupon50(response["coupon50"].toInt());
        }
        
        updateProfileDisplay();
        
        QString message = QString("抽奖结果：%1\n\n剩余积分：%2").arg(prize).arg(remainingPoints);
        if (prize != "谢谢参与") {
            message += "\n\n恭喜您获得奖品！";
            if (prize == "30元优惠券") {
                message += QString("\n当前30元优惠券数量：%1张").arg(currentUser->getCoupon30());
            } else if (prize == "50元优惠券") {
                message += QString("\n当前50元优惠券数量：%1张").arg(currentUser->getCoupon50());
            }
        } else {
            message += "\n\n很遗憾，下次再来！";
        }
        
        QMessageBox::information(this, "抽奖结果", message);
    } else {
        QMessageBox::warning(this, "抽奖失败", response["message"].toString());
    }
}

void Purchaser::showPaymentPage()
{
    if (!currentUser || pendingOrderId.isEmpty()) {
        QMessageBox::warning(this, "错误", "订单信息无效");
        showCartPage();
        return;
    }

    // 更新支付页面信息
    paymentOrderIdLabel->setText(QString("订单号: %1").arg(pendingOrderId));
    paymentAmountLabel->setText(QString("支付金额: %1元").arg(pendingAmount, 0, 'f', 2));
    paymentBalanceLabel->setText(QString("账户余额: %1元").arg(currentUser->getBalance(), 0, 'f', 2));
    
    // 从currentUser中获取优惠券信息（登录时已保存）
    int coupon30 = currentUser ? currentUser->getCoupon30() : 0;
    int coupon50 = currentUser ? currentUser->getCoupon50() : 0;
    paymentCoupon30Label->setText(QString("30元优惠券: %1张").arg(coupon30));
    paymentCoupon50Label->setText(QString("50元优惠券: %1张").arg(coupon50));
    
    // 更新优惠券下拉框状态（根据优惠券数量启用/禁用选项）
    QStandardItemModel *model = qobject_cast<QStandardItemModel*>(paymentCouponCombo->model());
    if (model) {
        QStandardItem *item30 = model->item(1);
        QStandardItem *item50 = model->item(2);
        if (item30) {
            item30->setEnabled(coupon30 > 0);
            if (coupon30 == 0) {
                item30->setText("使用30元优惠券（无）");
            } else {
                item30->setText(QString("使用30元优惠券（%1张）").arg(coupon30));
            }
        }
        if (item50) {
            item50->setEnabled(coupon50 > 0);
            if (coupon50 == 0) {
                item50->setText("使用50元优惠券（无）");
            } else {
                item50->setText(QString("使用50元优惠券（%1张）").arg(coupon50));
            }
        }
    }
    // 默认选择"不使用优惠券"
    paymentCouponCombo->setCurrentIndex(0);
    
    // 显示订单明细：优先使用待支付快照（避免依赖购物车）
    QString itemsText = "订单明细:\n";
    const QList<CartItem> &displayItems = pendingCartItems.isEmpty()
        ? currentUser->getCartItems()
        : pendingCartItems;

    for (const auto &cartItem : displayItems) {
        itemsText += QString("  %1 x%2 = %3元\n")
                    .arg(cartItem.bookTitle)
                    .arg(cartItem.quantity)
                    .arg(cartItem.getTotal(), 0, 'f', 2);
    }
    paymentOrderItems->setPlainText(itemsText);
    
    // 检查余额是否充足
    if (currentUser->getBalance() < pendingAmount) {
        paymentBalanceLabel->setStyleSheet("color: red; font-weight: bold;");
        paymentBalanceLabel->setText(QString("账户余额: %1元 (余额不足)").arg(currentUser->getBalance(), 0, 'f', 2));
    } else {
        paymentBalanceLabel->setStyleSheet("color: green;");
    }
    
    stackedWidget->setCurrentWidget(paymentPage);
}

// showServicePage() 函数已在第3110行定义，删除此重复定义

// purchaser.cpp（实现槽函数）

// 处理登录结果
void Purchaser::onLoginResult(bool success, const User &user)
{
    if (success) {
        currentUser = new User(user);  // 更新当前登录用户
        isLoggedIn = true;
        // 清空营业执照相关变量，避免显示上个用户的数据
        licenseImagePath.clear();
        licenseImageBase64.clear();
        if (licenseImageLabel) {
            licenseImageLabel->clear();
            licenseImageLabel->setText("请选择营业执照图片\n(支持 JPG、PNG 格式)");
        }
        if (applySellerBtn) {
            applySellerBtn->setText("提交营业执照");
            applySellerBtn->setEnabled(false);
        }
        showMainPage();  // 登录成功，跳转到主页面
        QMessageBox::information(this, "登录成功", "欢迎回来，" + user.getUsername() + "!");
    } else {
        QMessageBox::warning(this, "登录失败", "用户名或密码错误");
    }
}

// 处理下单结果
void Purchaser::onOrderResult(bool success, const QString &orderId)
{
    if (success) {
        QMessageBox::information(this, "下单成功", "订单创建成功！订单号：" + orderId);
//        clearCart();  // 清空购物车
        updateCartDisplay();  // 刷新购物车UI
    } else {
        QMessageBox::warning(this, "下单失败", "服务器未能处理订单，请重试");
    }
}

// 处理网络错误
void Purchaser::onNetworkError(const QString &errMsg)
{
    QMessageBox::critical(this, "网络错误", errMsg);
}

// 进入支付页面（如果已有待支付订单）
void Purchaser::onPaymentClicked()
{
    showPaymentPage();
}

// 确认支付
void Purchaser::onConfirmPaymentClicked()
{
    if (!currentUser || pendingOrderId.isEmpty()) {
        QMessageBox::warning(this, "错误", "订单信息无效");
        return;
    }

    QString paymentMethod = paymentMethodCombo->currentText();
    
    // 获取选择的优惠券
    QString useCoupon = "";
    if (paymentCouponCombo->currentIndex() == 1) {
        useCoupon = "30";
    } else if (paymentCouponCombo->currentIndex() == 2) {
        useCoupon = "50";
    }
    
    // 检查余额（需要考虑优惠券折扣）
    double finalAmount = pendingAmount;
    if (useCoupon == "30") {
        finalAmount = qMax(0.0, pendingAmount - 30.0);
    } else if (useCoupon == "50") {
        finalAmount = qMax(0.0, pendingAmount - 50.0);
    }
    
    if (paymentMethod == "账户余额支付") {
        if (currentUser->getBalance() < finalAmount) {
            QMessageBox::warning(this, "支付失败", QString("账户余额不足，请充值\n需要支付: %1元\n当前余额: %2元").arg(finalAmount, 0, 'f', 2).arg(currentUser->getBalance(), 0, 'f', 2));
            return;
        }
    }

    // 确保已连接到服务器
    if (!apiService->isConnected()) {
        if (!apiService->connectToServer(serverIp, serverPort)) {
            QMessageBox::warning(this, "连接失败", "无法连接到服务器");
            return;
        }
        // 移除阻塞延迟，连接后立即使用
        QCoreApplication::processEvents();  // 处理事件，确保连接完成
    }

    // 通过TCP请求支付订单（传递优惠券信息）
    QJsonObject response = apiService->payOrder(pendingOrderId, paymentMethod, useCoupon);

    if (response.value("success").toBool()) {
        // 更新余额（从服务器响应中获取）
        if (paymentMethod == "账户余额支付" && response.contains("balance")) {
            currentUser->setBalance(response["balance"].toDouble());
        } else if (paymentMethod == "账户余额支付") {
            // 如果没有返回余额，则本地扣除
            currentUser->deductBalance(finalAmount);
        }
        
        // 更新优惠券数量（如果使用了优惠券）
        if (useCoupon == "30") {
            int currentCoupon30 = currentUser->getCoupon30();
            if (currentCoupon30 > 0) {
                currentUser->setCoupon30(currentCoupon30 - 1);
            }
        } else if (useCoupon == "50") {
            int currentCoupon50 = currentUser->getCoupon50();
            if (currentCoupon50 > 0) {
                currentUser->setCoupon50(currentCoupon50 - 1);
            }
        }

        // 更新订单状态为已支付
        for (auto &o : allOrders) {
            if (o.getOrderId() == pendingOrderId) {
                o.setStatus("已支付");
                break;
            }
        }
        
        QString balanceInfo = "";
        if (paymentMethod == "账户余额支付") {
            balanceInfo = QString("\n剩余余额: %1元").arg(currentUser->getBalance(), 0, 'f', 2);
        }
        
        QString couponInfo = "";
        if (useCoupon == "30") {
            couponInfo = "\n使用优惠券: 30元";
        } else if (useCoupon == "50") {
            couponInfo = "\n使用优惠券: 50元";
        }
        
        QMessageBox::information(this, "支付成功",
            QString("支付成功！\n订单号: %1\n支付金额: %2元\n支付方式: %3%4%5")
            .arg(pendingOrderId)
            .arg(finalAmount, 0, 'f', 2)
            .arg(paymentMethod)
            .arg(couponInfo)
            .arg(balanceInfo));
        
        // 从数据库中删除已结算的商品
        for (const auto &cartItem : pendingCartItems) {
            QJsonObject removeResp = apiService->removeFromCart(
                QString::number(currentUser->getId()),
                cartItem.bookId
            );
            if (!removeResp.value("success").toBool()) {
                qWarning() << "删除购物车商品失败:" << cartItem.bookId 
                          << removeResp.value("message").toString();
            }
        }
        
        // 清空本地购物车
        currentUser->clearCart();
        
        // 清空待支付订单信息
        pendingOrderId.clear();
        pendingAmount = 0.0;
        pendingCartItems.clear();
        
        // 返回购物车页面
        showCartPage();
    } else {
        QString errorMsg = response.value("message").toString();
        QMessageBox::warning(this, "支付失败", errorMsg.isEmpty() ? "支付失败，请重试" : errorMsg);
    }
}

// 取消支付
void Purchaser::onCancelPaymentClicked()
{
    int result = QMessageBox::question(this, "确认取消", 
        "确定要取消支付吗？订单将被取消。",
        QMessageBox::Yes | QMessageBox::No);
    
    if (result == QMessageBox::Yes) {
        // 取消支付：不删除订单，保留为“待支付”
        // 清空待支付状态（订单已在allOrders里）
        pendingOrderId.clear();
        pendingAmount = 0.0;
        pendingCartItems.clear();
        
        updateOrderDisplay();
        showOrdersPage();
    }
}

// 登录按钮点击事件 - 通过TCP请求到服务器
void Purchaser::onLoginClicked()
{
    QString username = loginUsername->text().trimmed();
    QString password = loginPassword->text().trimmed();

    // #region agent log
    writeDebugLog("purchaser.cpp:2651", "登录函数入口", QJsonObject{{"username", username}, {"passwordLength", password.length()}}, "A");
    // #endregion

    if (username.isEmpty() || password.isEmpty()) {
        // #region agent log
        writeDebugLog("purchaser.cpp:2654", "输入验证失败", QJsonObject{{"reason", "用户名或密码为空"}}, "A");
        // #endregion
        QMessageBox::warning(this, "输入错误", "请输入用户名和密码");
        return;
    }

    // 确保已连接到服务器
    // #region agent log
    writeDebugLog("purchaser.cpp:2660", "检查TCP连接状态", QJsonObject{{"isConnected", apiService->isConnected()}, {"serverIp", serverIp}, {"serverPort", serverPort}}, "A");
    // #endregion
    if (!apiService->isConnected()) {
        qDebug() << "未连接服务器，正在连接...";
        // #region agent log
        writeDebugLog("purchaser.cpp:2662", "尝试连接服务器", QJsonObject{{"serverIp", serverIp}, {"serverPort", serverPort}}, "A");
        // #endregion
        if (!apiService->connectToServer(serverIp, serverPort)) {
            // #region agent log
            writeDebugLog("purchaser.cpp:2663", "连接服务器失败", QJsonObject{{"serverIp", serverIp}, {"serverPort", serverPort}}, "A");
            // #endregion
            QMessageBox::warning(this, "连接失败", 
                QString("无法连接到服务器 %1:%2\n请确保服务器正在运行").arg(serverIp).arg(serverPort));
            return;
        }
        // 等待连接稳定
        // 移除阻塞延迟，连接后立即使用
        QCoreApplication::processEvents();  // 处理事件，确保连接完成
        // #region agent log
        writeDebugLog("purchaser.cpp:2668", "连接服务器成功", QJsonObject{{"serverIp", serverIp}, {"serverPort", serverPort}}, "A");
        // #endregion
    }

    // 通过TCP请求登录
    qDebug() << "通过TCP请求登录...";
    // #region agent log
    writeDebugLog("purchaser.cpp:2673", "发送登录请求前", QJsonObject{{"username", username}}, "B");
    // #endregion
    QJsonObject response = apiService->login(username, password);
    
    // #region agent log
    writeDebugLog("purchaser.cpp:2675", "收到登录响应", QJsonObject{{"success", response.value("success").toBool()}, {"message", response.value("message").toString()}, {"hasUserId", response.contains("userId")}}, "B");
    // #endregion
    
    if (response.value("success").toBool()) {
        // 登录成功
        int userId = response.value("userId").toInt();
        QString respUsername = response.value("username").toString();
        
        // 验证用户ID是否有效
        if (userId <= 0) {
            qWarning() << "登录失败：服务器返回的用户ID无效，userId:" << userId;
            QMessageBox::warning(this, "登录失败", "服务器返回的用户ID无效，请重试");
            return;
        }
        
        // #region agent log
        writeDebugLog("purchaser.cpp:2681", "登录成功，创建用户对象", QJsonObject{{"userId", userId}, {"username", respUsername}}, "B");
        // #endregion
        
        // 创建用户对象（需要保存密码以便后续使用）
        // 注意：这里需要保存登录时使用的密码，因为服务器响应中不包含密码
        // 我们需要在登录时保存密码
        currentUser = new User();
        currentUser->setId(userId);
        qDebug() << "用户登录成功，用户ID:" << userId << "用户名:" << respUsername;
        currentUser->username = respUsername;
        currentUser->password = password;  // 保存登录时使用的密码
        
        // 设置用户的其他信息
        if (response.contains("email")) {
            currentUser->setEmail(response.value("email").toString());
        }
        if (response.contains("phone")) {
            currentUser->setPhone(response.value("phone").toString());
        }
        // 保存优惠券信息（如果响应中包含）
        if (response.contains("coupon30")) {
            currentUser->setCoupon30(response.value("coupon30").toInt());
        }
        if (response.contains("coupon50")) {
            currentUser->setCoupon50(response.value("coupon50").toInt());
        }
        if (response.contains("address")) {
            currentUser->setAddress(response.value("address").toString());
        }
        if (response.contains("balance")) {
            currentUser->setBalance(response.value("balance").toDouble());
        }
        // 设置会员等级（1-5）- 保留向后兼容
        int membershipLevel = 1;
        if (response.contains("membershipLevel")) {
            membershipLevel = response.value("membershipLevel").toInt();
            if (membershipLevel < 1 || membershipLevel > 5) {
                membershipLevel = 1;  // 确保值在有效范围内
            }
        }
        currentUser->setMembershipLevel(membershipLevel);
        
        // 设置会员等级字符串和相关信息
        QString memberLevel = "普通会员";
        if (response.contains("memberLevel")) {
            memberLevel = response.value("memberLevel").toString();
            if (memberLevel.isEmpty()) {
                memberLevel = "普通会员";
            }
        }
        currentUser->setMemberLevel(memberLevel);
        
        // 设置累计充值总额
        double totalRecharge = 0.0;
        if (response.contains("totalRecharge")) {
            totalRecharge = response.value("totalRecharge").toDouble();
        }
        currentUser->setTotalRecharge(totalRecharge);
        
        // 设置会员折扣率
        double memberDiscount = 1.0;
        if (response.contains("memberDiscount")) {
            memberDiscount = response.value("memberDiscount").toDouble();
            if (memberDiscount <= 0 || memberDiscount > 1.0) {
                memberDiscount = 1.0;
            }
        }
        currentUser->setMemberDiscount(memberDiscount);
        
        // 设置积分
        int points = 0;
        if (response.contains("points")) {
            points = response.value("points").toInt();
        }
        currentUser->setPoints(points);
        
        // 加载收藏书籍列表
        if (response.contains("favoriteBooks")) {
            QJsonArray favoriteBooksArray = response.value("favoriteBooks").toArray();
            currentUser->favoriteBooks.clear();
            for (const QJsonValue &val : favoriteBooksArray) {
                QString bookId = val.toString();
                if (!bookId.isEmpty()) {
                    currentUser->favoriteBooks.append(bookId);
                }
            }
            qDebug() << "已加载收藏书籍" << currentUser->favoriteBooks.size() << "本";
        }
        
        isLoggedIn = true;
        
        loginUsername->clear();
        loginPassword->clear();
        
        // 跳转到主界面
        showMainPage();
        QMessageBox::information(this, "登录成功", "欢迎回来，" + respUsername + "!");
        
        // 登录成功后，先显示主页面，然后异步加载图书列表（避免阻塞）
        showMainPage();
        
        // 启动自动刷新定时器（每30秒自动刷新一次）
        if (autoRefreshTimer && !autoRefreshTimer->isActive()) {
            autoRefreshTimer->start();
            qDebug() << "已启动自动刷新定时器，刷新间隔:" << AUTO_REFRESH_INTERVAL / 1000 << "秒";
        }
        
        // 异步加载图书列表（使用QTimer延迟加载，避免阻塞登录流程）
        QTimer::singleShot(100, this, [this]() {
            loadBooks();
            updateRecommendations();
        });
    } else {
        QString errorMsg = response.value("message").toString();
        // #region agent log
        writeDebugLog("purchaser.cpp:2696", "登录失败", QJsonObject{{"errorMsg", errorMsg}}, "B");
        // #endregion
        QMessageBox::warning(this, "登录失败", errorMsg.isEmpty() ? "用户名或密码错误" : errorMsg);
    }
}

// 连接按钮点击事件（如果存在connectButton按钮）
void Purchaser::onConnectClicked()
{
    // 如果确实需要连接按钮的功能，可以在这里实现
    // 目前暂时为空实现，避免链接错误
    Q_UNUSED(this);
}

// 加载本地预设图书数据（不等待服务器）
void Purchaser::loadLocalBooks()
{
    allBooks.clear();
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

    // 更新图书映射表
    bookMap.clear();
    for (const auto &book : allBooks) {
        bookMap[book.getId()] = book;
    }
    
    qDebug() << "已加载本地图书数据，共" << allBooks.size() << "本";
}

// 从服务器加载图书 - 通过TCP请求
void Purchaser::loadBooks()
{
    // 确保已连接到服务器
    if (!apiService->isConnected()) {
        qDebug() << "未连接服务器，正在连接...";
        if (!apiService->connectToServer(serverIp, serverPort)) {
            qDebug() << "连接服务器失败，无法加载图书";
            return;
        }
        // 移除阻塞延迟，连接后立即使用
        QCoreApplication::processEvents();  // 处理事件，确保连接完成
    }

    // 通过TCP请求获取图书列表
    qDebug() << "通过TCP请求获取图书列表...";
    QJsonObject response = apiService->getAllBooks();
    
    if (response.value("success").toBool()) {
        QJsonArray booksArray = response.value("books").toArray();
        QList<Book> books;
        
        for (const QJsonValue &value : booksArray) {
            QJsonObject bookObj = value.toObject();
            Book book;
            book.bookId = bookObj.value("bookId").toString();
            book.title = bookObj.value("bookName").toString();
            // 优先使用category1和category2，如果没有则使用category和subCategory（向后兼容）
            book.categoryId1 = bookObj.value("category1").toString();
            if (book.categoryId1.isEmpty()) {
            book.categoryId1 = bookObj.value("category").toString();
            }
            book.categoryId2 = bookObj.value("category2").toString();
            if (book.categoryId2.isEmpty()) {
            book.categoryId2 = bookObj.value("subCategory").toString();
            }
            book.price = bookObj.value("price").toDouble();
            // 优先使用averageRating，如果没有则使用score，如果都没有则使用0.0
            if (bookObj.contains("averageRating")) {
                book.score = bookObj.value("averageRating").toDouble();
            } else if (bookObj.contains("score")) {
            book.score = bookObj.value("score").toDouble();
            } else {
                book.score = 0.0;  // 无评分
            }
            book.sales = bookObj.value("sales").toInt();
            book.author = bookObj.value("author").toString();
            book.coverImage = bookObj.value("coverImage").toString();
            book.description = bookObj.value("description").toString();  // 书籍描述
            book.merchantId = bookObj.value("merchantId").toInt();  // 商家ID
            // 保存收藏量（如果服务器返回了）
            if (bookObj.contains("favoriteCount")) {
                book.favoriteCount = bookObj.value("favoriteCount").toInt();
            } else {
                book.favoriteCount = 0;
            }
            books.append(book);
        }
        
        // 更新本地数据
        allBooks = books;
        bookMap.clear();
        for (const auto &book : allBooks) {
            bookMap[book.getId()] = book;
        }
        
        qDebug() << "已从服务器加载图书数据，共" << allBooks.size() << "本";
        
        // 将图书添加到分类树（重要！）
        updateBooksToCategories();
        
        onBooksLoaded(books);  // 更新UI
        updateRecommendations();  // 更新推荐列表
    } else {
        QString errorMsg = response.value("message").toString();
        qDebug() << "获取图书失败:" << errorMsg;
    }
}

void Purchaser::updateBooksToCategories()
{
    // 先清空所有分类的图书ID
    clearCategoryBooks(categoryRoot);

    // 使用映射表来匹配分类
    QMap<QString, CategoryNode*> category1Map;  // 一级分类名称 -> 节点映射
    QMap<QString, CategoryNode*> category2Map;   // 二级分类名称 -> 节点映射

    // 遍历所有图书，收集所有分类
    QSet<QString> category1Set;
    QMap<QString, QSet<QString>> category2SetMap;  // 一级分类 -> 二级分类集合
    
    for (const auto &book : allBooks) {
        QString cat1 = book.getCategory1().trimmed();
        QString cat2 = book.getCategory2().trimmed();
        
        if (!cat1.isEmpty()) {
            category1Set.insert(cat1);
            if (!cat2.isEmpty()) {
                category2SetMap[cat1].insert(cat2);
            }
        }
    }

    // 匹配或创建一级分类节点
    for (const QString &cat1Name : category1Set) {
        CategoryNode *cat1Node = findCategoryNodeByName(categoryRoot, cat1Name);
        if (!cat1Node) {
            // 如果预设分类中不存在，动态创建新节点
            QString cat1Id = cat1Name.toLower().replace(" ", "_").replace("/", "_");
            cat1Node = new CategoryNode(cat1Id, cat1Name, categoryRoot);
            categoryRoot->addChild(cat1Node);
            qDebug() << "动态创建一级分类:" << cat1Name << "ID:" << cat1Id;
        } else {
            qDebug() << "找到预设一级分类:" << cat1Name << "ID:" << cat1Node->getId();
        }
        category1Map[cat1Name] = cat1Node;
    }

    // 匹配或创建二级分类节点
    // 使用"一级分类名称|二级分类名称"作为key，避免不同一级分类下的同名二级分类互相覆盖
    QMap<QString, CategoryNode*> category2MapWithParent;  // "cat1|cat2" -> 节点映射
    for (auto it = category2SetMap.begin(); it != category2SetMap.end(); ++it) {
        QString cat1Name = it.key();
        CategoryNode *cat1Node = category1Map[cat1Name];
        if (cat1Node) {
            for (const QString &cat2Name : it.value()) {
                CategoryNode *cat2Node = findCategoryNodeByName(cat1Node, cat2Name);
                if (!cat2Node) {
                    // 如果预设分类中不存在，动态创建新节点
                    QString cat2Id = cat2Name.toLower().replace(" ", "_").replace("/", "_");
                    cat2Node = new CategoryNode(cat2Id, cat2Name, cat1Node);
                    cat1Node->addChild(cat2Node);
                    qDebug() << "动态创建二级分类:" << cat1Name << "->" << cat2Name;
                }
                // 使用组合key避免冲突
                QString combinedKey = cat1Name + "|" + cat2Name;
                category2MapWithParent[combinedKey] = cat2Node;
                // 同时保留旧的映射方式（仅二级分类名称）用于向后兼容，但只在当前一级分类下有效
                if (!category2Map.contains(cat2Name)) {
                    category2Map[cat2Name] = cat2Node;
                }
            }
        }
    }

    // 重新加载分类树到UI（显示所有分类，包括预设和动态创建的）
    loadCategories();

    // 遍历所有图书，添加到对应分类
    for (const auto &book : allBooks) {
        QString cat1 = book.getCategory1().trimmed();
        QString cat2 = book.getCategory2().trimmed();
        
        // 添加到一级分类
        if (!cat1.isEmpty() && category1Map.contains(cat1)) {
            CategoryNode *targetNode = category1Map[cat1];
            targetNode->addBook(book.getId());
            qDebug() << "添加图书到一级分类:" << cat1 << "(ID:" << targetNode->getId() << ")->" << book.getId() << book.getTitle();
            
            // 验证：确保图书被添加到了正确的分类
            if (cat1 == "少儿童书" && targetNode->getId() != "children") {
                qDebug() << "错误：少儿童书分类ID不匹配！期望:children, 实际:" << targetNode->getId();
            }
            if (cat1 == "生活艺术" && targetNode->getId() != "lifestyle") {
                qDebug() << "错误：生活艺术分类ID不匹配！期望:lifestyle, 实际:" << targetNode->getId();
            }
            if (cat1 == "其他" && targetNode->getId() != "other") {
                qDebug() << "错误：其他分类ID不匹配！期望:other, 实际:" << targetNode->getId();
            }
        } else if (!cat1.isEmpty()) {
            qDebug() << "警告：未找到一级分类节点:" << cat1 << "图书:" << book.getId() << book.getTitle();
        }

        // 添加到二级分类 - 使用组合key确保找到正确的分类节点
        if (!cat2.isEmpty() && !cat1.isEmpty()) {
            QString combinedKey = cat1 + "|" + cat2;
            if (category2MapWithParent.contains(combinedKey)) {
                category2MapWithParent[combinedKey]->addBook(book.getId());
                qDebug() << "添加图书到二级分类:" << cat1 << "->" << cat2 << "->" << book.getId() << book.getTitle();
            } else if (category2Map.contains(cat2)) {
                // 向后兼容：如果组合key不存在，尝试使用旧的映射方式
                category2Map[cat2]->addBook(book.getId());
                qDebug() << "添加图书到二级分类(兼容模式):" << cat2 << "->" << book.getId() << book.getTitle();
            } else {
                qDebug() << "警告：未找到二级分类节点:" << cat1 << "->" << cat2 << "图书:" << book.getId() << book.getTitle();
            }
        }
    }
    
    qDebug() << "已将" << allBooks.size() << "本图书添加到分类树";
    qDebug() << "一级分类数量:" << category1Map.size() << "二级分类数量:" << category2Map.size();
    
    // 调试：打印"其他"分类的图书数量
    CategoryNode *otherNode = findCategoryNodeByName(categoryRoot, "其他");
    if (otherNode) {
        qDebug() << "\"其他\"分类节点找到，ID:" << otherNode->getId() << "图书数量:" << otherNode->getBookIds().size();
        for (const QString &bookId : otherNode->getBookIds()) {
            if (bookMap.contains(bookId)) {
                qDebug() << "  -" << bookId << bookMap[bookId].getTitle();
            }
        }
    } else {
        qDebug() << "警告：未找到\"其他\"分类节点";
    }
}

// 手动刷新图书列表
void Purchaser::onRefreshClicked()
{
    qDebug() << "手动刷新图书列表...";
    
    // 显示刷新提示
    refreshButton->setText("刷新中...");
    refreshButton->setEnabled(false);
    
    // 重新加载图书列表
    loadBooks();
    
    // 如果当前在主页面，更新推荐列表
    if (stackedWidget->currentWidget() == mainPage) {
        updateRecommendations();
    }
    
    // 恢复按钮状态
    refreshButton->setText("刷新");
    refreshButton->setEnabled(true);
    
    QMessageBox::information(this, "刷新完成", QString("已刷新图书列表，共 %1 本图书").arg(allBooks.size()));
}

// 自动刷新图书列表（定时器触发）
void Purchaser::onAutoRefresh()
{
    // 只在已登录且当前在主页面时自动刷新
    if (!isLoggedIn || stackedWidget->currentWidget() != mainPage) {
        return;
    }
    
    qDebug() << "自动刷新图书列表...";
    
    // 重新加载图书列表
    loadBooks();
    
    // 更新推荐列表
    updateRecommendations();
    
    qDebug() << "自动刷新完成，共" << allBooks.size() << "本图书";
}

// 递归查找分类节点（按ID）
CategoryNode* Purchaser::findCategoryNode(CategoryNode *node, const QString &categoryId)
{
    if (!node) return nullptr;
    
    // 检查当前节点
    if (node->getId() == categoryId) {
        return node;
    }
    
    // 递归查找子节点
    for (auto child : node->getChildren()) {
        CategoryNode *found = findCategoryNode(child, categoryId);
        if (found) {
            return found;
        }
    }
    
    return nullptr;
}

// 递归查找分类节点（按名称）- 只在直接子节点中查找，不递归查找整个树
CategoryNode* Purchaser::findCategoryNodeByName(CategoryNode *node, const QString &categoryName)
{
    if (!node) return nullptr;
    
    // 只在直接子节点中查找，避免找到错误的节点（比如找到二级分类而不是一级分类）
    for (auto child : node->getChildren()) {
        if (child->getName() == categoryName) {
            return child;
        }
    }
    
    return nullptr;
}

// 递归清空分类下的图书
void Purchaser::clearCategoryBooks(CategoryNode *node)
{
    if (!node) return;

    // 清空当前节点的所有图书ID
    QList<QString> bookIds = node->getBookIds();
    for (const QString &bookId : bookIds) {
        node->removeBook(bookId);
    }
    
    // 递归清空子节点
    for (auto child : node->getChildren()) {
        clearCategoryBooks(child);
    }
}

void Purchaser::onBooksLoaded(const QList<Book>& books)
{
    // 保存服务器返回的图书列表
    allBooks = books;
    // 更新图书映射表
    for (const auto &book : allBooks) {
        bookMap[book.getId()] = book;
    }

    // 将图书添加到对应分类
    updateBooksToCategories();

    // 如果已登录，刷新图书展示界面
//    if (isLoggedIn) {
//        refreshBookList();
//    }
}



// 处理服务器返回的图书列表
//void Purchaser::onBooksLoaded(const QList<Book> &books)
//{
//    allBooks = books;  // 更新本地图书数据
//    bookMap.clear();
//    for (const auto &book : allBooks) {
//        bookMap[book.getId()] = book;
//    }
////    loadBooks();  // 刷新UI显示图书
//    updateRecommendations();  // 更新推荐列表
//}

//// 结账按钮点击事件
//void Purchaser::onCheckoutClicked()
//{
//    if (!currentUser) return;

//    // 遍历购物车，批量下单（或按服务器要求的格式发送）
//    for (const auto &item : currentUser->getCartItems()) {
//            item.bookId,
//            item.quantity,
//            QString::number(currentUser->getId())
//        );
//    }
//}

//// 在Purchaser类中添加连接状态检查函数
//bool Purchaser::ensureConnected()
//{
//    if (!networkController->isConnected()) {
//        networkController->connectToServer(serverIp, serverPort);
//        // 等待连接（简单处理，实际可通过信号判断）
//        return networkController->waitForConnected(3000);  // 等待3秒
//    }
//    return true;
//}

//// 发送请求前调用
//void Purchaser::onSearchClicked()
//{
//    if (!ensureConnected()) {
//        QMessageBox::warning(this, "连接失败", "无法连接到服务器");
//        return;
//    }

// 联系卖家功能实现
void Purchaser::onContactSellerClicked()
{
    if (!isLoggedIn || !currentUser) {
        QMessageBox::warning(this, "操作失败", "请先登录");
        showLoginPage();
        return;
    }
    
    // 检查当前书籍是否有商家ID
    if (currentBook.getMerchantId() <= 0) {
        QMessageBox::warning(this, "提示", "该书籍没有关联的卖家信息");
        return;
    }
    
    // 保存当前卖家ID
    currentSellerId = currentBook.getMerchantId();
    
    // 创建或显示卖家聊天对话框
    if (!sellerChatDialog) {
        sellerChatDialog = new QDialog(this);
        sellerChatDialog->setWindowTitle(QString("与卖家聊天 - 商品: %1").arg(currentBook.getTitle()));
        sellerChatDialog->setMinimumSize(500, 400);
        
        QVBoxLayout *dialogLayout = new QVBoxLayout(sellerChatDialog);
        
        // 聊天显示区域
        sellerChatDisplay = new QTextEdit();
        sellerChatDisplay->setReadOnly(true);
        dialogLayout->addWidget(sellerChatDisplay);
        
        // 消息输入区域
        QHBoxLayout *inputLayout = new QHBoxLayout();
        sellerMessageInput = new QTextEdit();
        sellerMessageInput->setMaximumHeight(80);
        sellerMessageInput->setPlaceholderText("输入消息...");
        sendSellerMessageBtn = new QPushButton("发送");
        inputLayout->addWidget(sellerMessageInput, 1);
        inputLayout->addWidget(sendSellerMessageBtn);
        
        dialogLayout->addLayout(inputLayout);
        
        // 连接发送按钮
        connect(sendSellerMessageBtn, &QPushButton::clicked, this, &Purchaser::onSendSellerMessageClicked);
    } else {
        sellerChatDialog->setWindowTitle(QString("与卖家聊天 - 商品: %1").arg(currentBook.getTitle()));
    }
    
    // 重置最后消息时间，确保首次加载显示所有消息
    lastMessageTime = QDateTime();
    
    // 加载聊天历史
    loadSellerChatHistory();
    
    // 启动聊天刷新定时器
    if (sellerChatRefreshTimer && !sellerChatRefreshTimer->isActive()) {
        sellerChatRefreshTimer->start();
    }
    
    // 连接对话框关闭信号，停止定时器
    connect(sellerChatDialog, &QDialog::finished, this, [this]() {
        if (sellerChatRefreshTimer && sellerChatRefreshTimer->isActive()) {
            sellerChatRefreshTimer->stop();
        }
        // 重置最后消息时间
        lastMessageTime = QDateTime();
    });
    
    // 显示对话框
    sellerChatDialog->show();
    sellerChatDialog->raise();
    sellerChatDialog->activateWindow();
}

void Purchaser::onSendSellerMessageClicked()
{
    if (!isLoggedIn || !currentUser || currentSellerId <= 0) {
        QMessageBox::warning(this, "发送失败", "请先登录或选择卖家");
        return;
    }
    
    QString message = sellerMessageInput->toPlainText().trimmed();
    if (message.isEmpty()) {
        QMessageBox::warning(this, "发送失败", "请输入消息内容");
        return;
    }
    
    // 连接服务器
    if (!apiService->isConnected()) {
        if (!apiService->connectToServer(serverIp, serverPort)) {
            QMessageBox::warning(this, "连接失败", "无法连接到服务器");
            return;
        }
    }
    
    // 发送消息给卖家
    QJsonObject response = apiService->sendChatMessage(
        QString::number(currentUser->getId()),
        "buyer",
        QString::number(currentSellerId),
        "seller",
        message
    );
    
    if (response["success"].toBool()) {
        sellerMessageInput->clear();
        
        // 不在这里直接显示消息，避免与定时器刷新时重复显示
        // 立即触发一次聊天历史加载，让服务器返回的消息被正确显示
        // loadSellerChatHistory()会正确更新lastMessageTime
        loadSellerChatHistory();
    } else {
        QMessageBox::warning(this, "发送失败", response["message"].toString());
    }
}

void Purchaser::loadSellerChatHistory()
{
    if (!isLoggedIn || !currentUser || currentSellerId <= 0) {
        return;
    }
    
    // 连接服务器
    if (!apiService->isConnected()) {
        if (!apiService->connectToServer(serverIp, serverPort)) {
            return;
        }
    }
    
    // 获取与卖家的聊天历史
    QJsonObject response = apiService->getChatHistory(
        QString::number(currentUser->getId()),
        "buyer",
        QString::number(currentSellerId),
        "seller"
    );
    
    if (response["success"].toBool()) {
        QJsonArray messages = response["messages"].toArray();
        
        // 获取当前显示的最后一条消息时间（用于增量更新）
        QDateTime currentLastTime = lastMessageTime;
        bool hasNewMessages = false;
        
        // 如果lastMessageTime无效，说明是首次加载，清空显示
        if (!lastMessageTime.isValid()) {
            sellerChatDisplay->clear();
        }
        
        for (const QJsonValue &msgVal : messages) {
            QJsonObject msg = msgVal.toObject();
            QString senderType = msg["senderType"].toString();
            QString content = msg["content"].toString();
            QString sendTime = msg["sendTime"].toString();
            
            // 格式化时间
            QDateTime dateTime = QDateTime::fromString(sendTime, "yyyy-MM-dd hh:mm:ss");
            if (!dateTime.isValid()) {
                dateTime = QDateTime::fromString(sendTime, Qt::ISODate);
            }
            
            // 如果是增量更新，只显示新消息
            if (lastMessageTime.isValid() && dateTime <= lastMessageTime) {
                continue;
            }
            
            QString timeStr = dateTime.isValid() ? dateTime.toString("yyyy-MM-dd hh:mm") : sendTime;
            
            // 显示消息
            QString senderName;
            if (senderType == "buyer") {
                senderName = "我";
            } else if (senderType == "seller") {
                senderName = "卖家";
            } else {
                senderName = "未知";
            }
            
            sellerChatDisplay->append(QString("[%1] %2: %3").arg(timeStr).arg(senderName).arg(content));
            
            // 更新最后一条消息时间
            if (dateTime.isValid() && (!lastMessageTime.isValid() || dateTime > lastMessageTime)) {
                lastMessageTime = dateTime;
                hasNewMessages = true;
            }
        }
        
        // 如果有新消息，滚动到底部
        if (hasNewMessages || !currentLastTime.isValid()) {
            QTextCursor cursor = sellerChatDisplay->textCursor();
            cursor.movePosition(QTextCursor::End);
            sellerChatDisplay->setTextCursor(cursor);
        }
    }
}
//}
