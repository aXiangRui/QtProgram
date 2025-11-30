#include "homewidget.h"
#include "ui_homewidget.h"

HomeWidget::HomeWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::HomeWidget)
{
    ui->setupUi(this);
    // ========== 全局样式（匹配封面风格） ==========
    setStyleSheet(
        "QWidget { background-color: #ffffff; }"
        "QFrame { border: 2px solid #000000; border-radius: 8px; }"
        "QPushButton { background-color: #e0e0e0; border: none; border-radius: 4px; }"
        "QLabel { font-family: Arial; }"
        "QLabel#priceLabel { color: #ff0000; font-weight: bold; }"
    );

    // ========== 主框架 ==========
    QFrame *mainFrame = new QFrame(this);
    QHBoxLayout *mainLayout = new QHBoxLayout(mainFrame);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // ========== 左侧导航栏 ==========
    QWidget *navWidget = new QWidget(this);
    navWidget->setStyleSheet("background-color: #4a90e2;");
    QVBoxLayout *navLayout = new QVBoxLayout(navWidget);
    navLayout->setContentsMargins(20, 30, 20, 0);
    navLayout->setSpacing(30);
    navLayout->setAlignment(Qt::AlignTop);

    // 导航图标按钮（用Label模拟）
    QString navIconStyle = "color: white; font-size: 24px;";
    QLabel *userIcon = new QLabel("👤", this);
    userIcon->setStyleSheet(navIconStyle);
    QLabel *homeIcon = new QLabel("🏠", this);
    homeIcon->setStyleSheet(navIconStyle);
    QLabel *cartIcon = new QLabel("🛒", this);
    cartIcon->setStyleSheet(navIconStyle);
    QLabel *orderIcon = new QLabel("📦", this);
    orderIcon->setStyleSheet(navIconStyle);
    QLabel *settingIcon = new QLabel("⚙️", this);
    settingIcon->setStyleSheet(navIconStyle);

    navLayout->addWidget(userIcon);
    navLayout->addWidget(homeIcon);
    navLayout->addWidget(cartIcon);
    navLayout->addWidget(orderIcon);
    navLayout->addStretch();
    navLayout->addWidget(settingIcon);

    // ========== 右侧内容区 ==========
    QWidget *contentWidget = new QWidget(this);
    QVBoxLayout *contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setContentsMargins(20, 20, 20, 20);
    contentLayout->setSpacing(20);

    // 顶部搜索栏
    QWidget *searchBar = new QWidget(this);
    QHBoxLayout *searchLayout = new QHBoxLayout(searchBar);
    searchLayout->setSpacing(10);

    QLineEdit *searchEdit = new QLineEdit(this);
    searchEdit->setPlaceholderText("搜索图书...");
    searchEdit->setStyleSheet("padding: 6px; border: 1px solid #ccc; border-radius: 4px;");

    QPushButton *searchBtn = new QPushButton("🔍", this);
    searchBtn->setFixedSize(30, 30);

    // 标签栏（Label01/02/03）
    QWidget *labelBar = new QWidget(this);
    QHBoxLayout *labelLayout = new QHBoxLayout(labelBar);
    labelLayout->setSpacing(5);

    QString labelStyle = "background-color: #e0e0e0; padding: 4px 8px; border: 1px solid #000; border-radius: 4px;";
    QLabel *label01 = new QLabel("Label01", this);
    label01->setStyleSheet(labelStyle);
    QLabel *label02 = new QLabel("Label02", this);
    label02->setStyleSheet(labelStyle);
    QLabel *label03 = new QLabel("Label03", this);
    label03->setStyleSheet(labelStyle);

    labelLayout->addWidget(label01);
    labelLayout->addWidget(label02);
    labelLayout->addWidget(label03);
    labelLayout->addStretch();

    searchLayout->addWidget(labelBar);
    searchLayout->addStretch();
    searchLayout->addWidget(searchEdit);
    searchLayout->addWidget(searchBtn);

    // 图书展示区（2行3列）
    QWidget *bookArea = new QWidget(this);
    QGridLayout *bookLayout = new QGridLayout(bookArea);
    bookLayout->setSpacing(20);

    // 生成6本示例图书
    for (int row = 0; row < 2; ++row) {
        for (int col = 0; col < 3; ++col) {
            QWidget *bookCard = new QWidget(this);
            QVBoxLayout *cardLayout = new QVBoxLayout(bookCard);
            cardLayout->setContentsMargins(0, 0, 0, 0);
            cardLayout->setSpacing(5);

            // 图书封面占位
            QLabel *coverLabel = new QLabel(this);
            coverLabel->setFixedSize(120, 150);
            coverLabel->setStyleSheet("background-color: #f0f0f0; border: 1px solid #ccc;");

            // 图书名称
            QLabel *nameLabel = new QLabel("samplebook", this);
            nameLabel->setAlignment(Qt::AlignCenter);

            // 价格标签
            QLabel *priceLabel = new QLabel("100.00", this);
            priceLabel->setObjectName("priceLabel");
            priceLabel->setAlignment(Qt::AlignCenter);

            cardLayout->addWidget(coverLabel);
            cardLayout->addWidget(nameLabel);
            cardLayout->addWidget(priceLabel);

            bookLayout->addWidget(bookCard, row, col);
        }
    }

    // 组装内容区
    contentLayout->addWidget(searchBar);
    contentLayout->addWidget(bookArea);

    // ========== 主布局组装 ==========
    mainLayout->addWidget(navWidget, 1);
    mainLayout->addWidget(contentWidget, 4);

    QVBoxLayout *rootLayout = new QVBoxLayout(this);
    rootLayout->addWidget(mainFrame);

    // ========== 窗口设置 ==========
    setWindowTitle("图书商城");
    resize(800, 600);
    setMinimumSize(800, 600);
}

HomeWidget::~HomeWidget()
{
    delete ui;
}
