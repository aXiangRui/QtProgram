#include "OrderManageWidget.h"

#include <QHeaderView>
#include <QTabBar>
#include <QDate>
#include <QMessageBox>
#include <QTime>
#include <QFileDialog>
#include <QTextStream>
#include <QFile>
#include <QDateTime>
#include <QColor>
#include <QFont>
#include <QPalette>
#include <QInputDialog>
#include <QDebug>
#include <QFormLayout>
#include<QVBoxLayout>
#include<QHBoxLayout>

// 本地配置常量
namespace {
    const QColor PRIMARY_COLOR(41, 128, 185);    // 主蓝色
    const QColor SUCCESS_COLOR(39, 174, 96);     // 成功绿色
    const QColor WARNING_COLOR(241, 196, 15);    // 警告黄色
    const QColor DANGER_COLOR(231, 76, 60);      // 危险红色
    const QColor INFO_COLOR(52, 152, 219);       // 信息蓝色
    const QColor BG_COLOR(245, 247, 250);        // 背景灰色
    const int FILTER_PANEL_WIDTH = 280;          // 增加筛选面板宽度
    const int DETAIL_PANEL_WIDTH = 380;          // 增加详情面板宽度
    const int TABLE_ROW_HEIGHT = 42;
}

OrderManageWidget::OrderManageWidget(QWidget *parent)
    : QWidget(parent), currentOrderRow(-1), cartTotal(0.0)
{
    // 设置窗口背景
    setStyleSheet(QString("background-color: %1;").arg(BG_COLOR.name()));

    // 初始化随机种子
    QTime time = QTime::currentTime();
    qsrand(time.msec() + time.second() * 1000);

    // ========== 关键修改：改为垂直布局 ==========
    mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(10);

    // 1. 创建工具栏（会加到顶部）
    createToolbar();

    // 2. 创建三栏内容容器
    QWidget *contentContainer = new QWidget;
    QHBoxLayout *contentLayout = new QHBoxLayout(contentContainer);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(10);

    // 创建各个组件（这些函数内部会创建控件，但不会添加到mainLayout）
    createFilterPanel();
    createTabs();
    createOrderDetails();
    createNewOrderDialog();

    // 将三栏添加到内容布局
    contentLayout->addWidget(filterPanel);
    contentLayout->addWidget(tabWidget, 1);  // 中间弹性扩展
    contentLayout->addWidget(detailPanel);

    // 将内容容器添加到主垂直布局
    mainLayout->addWidget(toolbar);          // 顶部工具栏
    mainLayout->addWidget(contentContainer, 1);  // 三栏内容（弹性扩展）

    // 初始加载数据
    generateSampleData();
    loadProductsForNewOrder();
    onTabChanged(0);
    updateStatistics();

    // 设置初始状态
    onTableSelectionChanged();
}

void OrderManageWidget::createToolbar()
{
    toolbar = new QWidget;
    toolbar->setObjectName("toolbar");
    toolbar->setFixedHeight(65);
    toolbar->setStyleSheet("background-color: white; border-radius: 10px; border: 1px solid #ddd;");

    QHBoxLayout *toolLayout = new QHBoxLayout(toolbar);
    toolLayout->setContentsMargins(20, 12, 20, 12);
    toolLayout->setSpacing(20);

    // 创建按钮
    createButton = new QPushButton("📝 新建订单");
    exportButton = new QPushButton("📤 导出数据");
    refreshButton = new QPushButton("🔄 刷新");
    exportTabButton = new QPushButton("📋 导出本页");
    deleteButton = new QPushButton("🗑️ 删除订单");

    // 设置按钮样式
    QString buttonStyle = QString(
        "QPushButton {"
        "    background-color: %1;"
        "    color: white;"
        "    border: none;"
        "    border-radius: 8px;"
        "    padding: 12px 24px;"
        "    font-size: 14px;"
        "    font-weight: bold;"
        "    min-width: 120px;"
        "}"
        "QPushButton:hover {"
        "    background-color: %2;"
        "    transform: translateY(-1px);"
        "    box-shadow: 0 4px 12px rgba(41, 128, 185, 0.3);"
        "}"
        "QPushButton:pressed {"
        "    background-color: %3;"
        "    transform: translateY(0);"
        "}"
    ).arg(PRIMARY_COLOR.name())
     .arg(PRIMARY_COLOR.darker(120).name())
     .arg(PRIMARY_COLOR.darker(150).name());

    QString exportStyle = QString(
        "QPushButton {"
        "    background-color: %1;"
        "    color: white;"
        "    border: none;"
        "    border-radius: 8px;"
        "    padding: 12px 24px;"
        "    font-size: 14px;"
        "    font-weight: bold;"
        "    min-width: 120px;"
        "}"
        "QPushButton:hover {"
        "    background-color: %2;"
        "    transform: translateY(-1px);"
        "    box-shadow: 0 4px 12px rgba(39, 174, 96, 0.3);"
        "}"
    ).arg(SUCCESS_COLOR.name())
     .arg(SUCCESS_COLOR.darker(120).name());

    QString deleteStyle = QString(
        "QPushButton {"
        "    background-color: %1;"
        "    color: white;"
        "    border: none;"
        "    border-radius: 8px;"
        "    padding: 12px 24px;"
        "    font-size: 14px;"
        "    font-weight: bold;"
        "    min-width: 120px;"
        "}"
        "QPushButton:hover {"
        "    background-color: %2;"
        "    transform: translateY(-1px);"
        "    box-shadow: 0 4px 12px rgba(231, 76, 60, 0.3);"
        "}"
    ).arg(DANGER_COLOR.name())
     .arg(DANGER_COLOR.darker(120).name());

    createButton->setStyleSheet(buttonStyle);
    exportButton->setStyleSheet(exportStyle);
    refreshButton->setStyleSheet(buttonStyle);
    exportTabButton->setStyleSheet(exportStyle);
    deleteButton->setStyleSheet(deleteStyle);

    // 添加到布局
    toolLayout->addWidget(createButton);
    toolLayout->addWidget(exportButton);
    toolLayout->addWidget(exportTabButton);
    toolLayout->addWidget(deleteButton);
    toolLayout->addStretch();
    toolLayout->addWidget(refreshButton);

    // 将工具栏放在主布局顶部
   // mainLayout->addWidget(toolbar, 0, Qt::AlignTop);

    // 连接信号
    connect(createButton, SIGNAL(clicked()), this, SLOT(onOpenNewOrderDialog()));
    connect(exportButton, SIGNAL(clicked()), this, SLOT(onExportOrders()));
    connect(refreshButton, SIGNAL(clicked()), this, SLOT(onRefresh()));
    connect(exportTabButton, SIGNAL(clicked()), this, SLOT(onExportCurrentTab()));
    connect(deleteButton, SIGNAL(clicked()), this, SLOT(onDeleteOrder()));
}

void OrderManageWidget::createFilterPanel()
{
    // 筛选面板容器
    filterPanel = new QWidget;
    filterPanel->setMinimumWidth(FILTER_PANEL_WIDTH);
    filterPanel->setMaximumWidth(FILTER_PANEL_WIDTH);
    filterPanel->setObjectName("filterPanel");
    filterPanel->setStyleSheet("#filterPanel { background-color: white; border-radius: 12px; border: 1px solid #ddd; }");

    QVBoxLayout *filterLayout = new QVBoxLayout(filterPanel);
    filterLayout->setContentsMargins(15, 20, 15, 20);
    filterLayout->setSpacing(20);

    // ========== 时间筛选 ==========
    timeFilterGroup = new QGroupBox("📅 时间筛选");
    timeFilterGroup->setStyleSheet(
        "QGroupBox {"
        "    font-weight: bold;"
        "    color: #2c3e50;"
        "    border: 2px solid #ecf0f1;"
        "    border-radius: 10px;"
        "    margin-top: 14px;"
        "    background-color: #f8f9fa;"
        "}"
        "QGroupBox::title {"
        "    subcontrol-origin: margin;"
        "    left: 12px;"
        "    padding: 0 8px 0 8px;"
        "    color: #3498db;"
        "    font-size: 14px;"
        "}"
    );

    QVBoxLayout *timeLayout = new QVBoxLayout(timeFilterGroup);
    timeLayout->setContentsMargins(18, 30, 18, 18);
    timeLayout->setSpacing(12);

    timeToday = new QRadioButton("今天");
    timeWeek = new QRadioButton("本周");
    timeMonth = new QRadioButton("本月");
    timeAll = new QRadioButton("全部");
    timeAll->setChecked(true);

    QString radioStyle = QString(
        "QRadioButton {"
        "    color: #34495e;"
        "    font-size: 14px;"
        "    padding: 8px 0;"
        "}"
        "QRadioButton::indicator {"
        "    width: 18px;"
        "    height: 18px;"
        "}"
        "QRadioButton::indicator:checked {"
                "    background-color: %1;"
                "    border: 3px solid white;"
                "    border-radius: 9px;"
                "    box-shadow: 0 0 0 2px %1;"
                "}"
            ).arg(PRIMARY_COLOR.name());

            timeToday->setStyleSheet(radioStyle);
            timeWeek->setStyleSheet(radioStyle);
            timeMonth->setStyleSheet(radioStyle);
            timeAll->setStyleSheet(radioStyle);

            timeLayout->addWidget(timeToday);
            timeLayout->addWidget(timeWeek);
            timeLayout->addWidget(timeMonth);
            timeLayout->addWidget(timeAll);

            // ========== 金额筛选 ==========
            amountFilterGroup = new QGroupBox("💰 金额筛选");
            amountFilterGroup->setStyleSheet(timeFilterGroup->styleSheet());
            QFormLayout *amountLayout = new QFormLayout(amountFilterGroup);
            amountLayout->setContentsMargins(18, 30, 18, 18);
            amountLayout->setSpacing(15);
            amountLayout->setLabelAlignment(Qt::AlignRight);

            minAmountEdit = new QSpinBox;
            maxAmountEdit = new QSpinBox;
            minAmountEdit->setRange(0, 99999);
            maxAmountEdit->setRange(0, 99999);
            minAmountEdit->setValue(0);
            maxAmountEdit->setValue(9999);
            minAmountEdit->setPrefix("¥ ");
            maxAmountEdit->setPrefix("¥ ");

            QString spinStyle = QString(
                "QSpinBox {"
                "    padding: 10px;"
                "    border: 2px solid #ecf0f1;"
                "    border-radius: 6px;"
                "    font-size: 14px;"
                "    min-height: 36px;"
                "}"
                "QSpinBox:hover {"
                "    border-color: #bdc3c7;"
                "}"
                "QSpinBox:focus {"
                "    border-color: %1;"
                "    outline: none;"
                "}"
            ).arg(PRIMARY_COLOR.name());

            minAmountEdit->setStyleSheet(spinStyle);
            maxAmountEdit->setStyleSheet(spinStyle);

            amountLayout->addRow("最小金额:", minAmountEdit);
            amountLayout->addRow("最大金额:", maxAmountEdit);

            // ========== 快速搜索 ==========
            QGroupBox *searchGroup = new QGroupBox("🔍 快速搜索");
            searchGroup->setStyleSheet(timeFilterGroup->styleSheet());
            QVBoxLayout *searchLayout = new QVBoxLayout(searchGroup);
            searchLayout->setContentsMargins(18, 30, 18, 18);
            searchLayout->setSpacing(15);

            searchEdit = new QLineEdit;
            searchEdit->setPlaceholderText("搜索订单号/客户名/手机号...");
            searchEdit->setStyleSheet(QString(
                "QLineEdit {"
                "    border: 2px solid #ecf0f1;"
                "    border-radius: 8px;"
                "    padding: 12px 15px;"
                "    font-size: 14px;"
                "    background-color: white;"
                "}"
                "QLineEdit:focus {"
                "    border-color: %1;"
                "    outline: none;"
                "    box-shadow: 0 0 0 3px rgba(52, 152, 219, 0.1);"
                "}"
            ).arg(PRIMARY_COLOR.name()));

            QHBoxLayout *searchButtonLayout = new QHBoxLayout;
            searchButton = new QPushButton("搜索");
            clearFilterButton = new QPushButton("清除");

            searchButton->setStyleSheet(QString(
                "QPushButton {"
                "    background-color: %1;"
                "    color: white;"
                "    border: none;"
                "    border-radius: 8px;"
                "    padding: 12px 20px;"
                "    font-size: 14px;"
                "    font-weight: bold;"
                "    min-width: 80px;"
                "}"
                "QPushButton:hover {"
                "    background-color: %2;"
                "    transform: translateY(-1px);"
                "}"
            ).arg(PRIMARY_COLOR.name())
             .arg(PRIMARY_COLOR.darker(120).name()));

            clearFilterButton->setStyleSheet(
                "QPushButton {"
                "    background-color: #95a5a6;"
                "    color: white;"
                "    border: none;"
                "    border-radius: 8px;"
                "    padding: 12px 20px;"
                "    font-size: 14px;"
                "    font-weight: bold;"
                "    min-width: 80px;"
                "}"
                "QPushButton:hover {"
                "    background-color: #7f8c8d;"
                "    transform: translateY(-1px);"
                "}"
            );

            searchButtonLayout->addWidget(searchButton);
            searchButtonLayout->addWidget(clearFilterButton);
            searchButtonLayout->setSpacing(10);

            searchLayout->addWidget(searchEdit);
            searchLayout->addLayout(searchButtonLayout);

            // ========== 支付方式 ==========
            paymentFilterGroup = new QGroupBox("💳 支付方式");
            paymentFilterGroup->setStyleSheet(timeFilterGroup->styleSheet());
            QVBoxLayout *paymentLayout = new QVBoxLayout(paymentFilterGroup);
            paymentLayout->setContentsMargins(18, 30, 18, 18);
            paymentLayout->setSpacing(12);

            paymentAll = new QCheckBox("全部");
            paymentCash = new QCheckBox("现金");
            paymentWechat = new QCheckBox("微信支付");
            paymentAlipay = new QCheckBox("支付宝");
            paymentCard = new QCheckBox("会员卡");
            paymentAll->setChecked(true);

            QString checkStyle = QString(
                "QCheckBox {"
                "    color: #34495e;"
                "    font-size: 14px;"
                "    padding: 8px 0;"
                "}"
                "QCheckBox::indicator {"
                "    width: 20px;"
                "    height: 20px;"
                "    border: 2px solid #bdc3c7;"
                "    border-radius: 4px;"
                "}"
                "QCheckBox::indicator:checked {"
                "    background-color: %1;"
                "    border-color: %1;"
                "    image: url(:/icons/check.svg);"
                "}"
                "QCheckBox::indicator:hover {"
                "    border-color: %1;"
                "}"
            ).arg(PRIMARY_COLOR.name());

            paymentAll->setStyleSheet(checkStyle);
            paymentCash->setStyleSheet(checkStyle);
            paymentWechat->setStyleSheet(checkStyle);
            paymentAlipay->setStyleSheet(checkStyle);
            paymentCard->setStyleSheet(checkStyle);

            paymentLayout->addWidget(paymentAll);
            paymentLayout->addWidget(paymentCash);
            paymentLayout->addWidget(paymentWechat);
            paymentLayout->addWidget(paymentAlipay);
            paymentLayout->addWidget(paymentCard);

            // ========== 统计信息 ==========
            statsGroup = new QGroupBox("📊 统计信息");
            statsGroup->setStyleSheet(timeFilterGroup->styleSheet());
            QVBoxLayout *statsLayout = new QVBoxLayout(statsGroup);
            statsLayout->setContentsMargins(18, 30, 18, 18);
            statsLayout->setSpacing(15);

            totalOrdersLabel = new QLabel("总计: 0 单");
            totalAmountLabel = new QLabel("金额: ¥0.00");
            pendingOrdersLabel = new QLabel("待处理: 0 单");
            completionBar = new QProgressBar;

            totalOrdersLabel->setStyleSheet("font-size: 14px; color: #2c3e50; font-weight: 500; padding: 5px 0;");
            totalAmountLabel->setStyleSheet("font-size: 14px; color: #27ae60; font-weight: 500; padding: 5px 0;");
            pendingOrdersLabel->setStyleSheet("font-size: 14px; color: #e74c3c; font-weight: 500; padding: 5px 0;");

            completionBar->setRange(0, 100);
            completionBar->setValue(0);
            completionBar->setTextVisible(true);
            completionBar->setFormat("订单完成度: %p%");
            completionBar->setStyleSheet(QString(
                "QProgressBar {"
                "    border: 2px solid #ecf0f1;"
                "    border-radius: 8px;"
                "    text-align: center;"
                "    height: 24px;"
                "    font-size: 12px;"
                "    font-weight: bold;"
                "    color: #2c3e50;"
                "    background-color: white;"
                "}"
                "QProgressBar::chunk {"
                "    background-color: qlineargradient(x1:0, y1:0, x2:1, y2:0,"
                "        stop:0 %1, stop:1 #2ecc71);"
                "    border-radius: 6px;"
                "}"
            ).arg(PRIMARY_COLOR.name()));

            statsLayout->addWidget(totalOrdersLabel);
            statsLayout->addWidget(totalAmountLabel);
            statsLayout->addWidget(pendingOrdersLabel);
            statsLayout->addSpacing(10);
            statsLayout->addWidget(completionBar);

            // ========== 添加到筛选面板 ==========
            filterLayout->addWidget(timeFilterGroup);
            filterLayout->addWidget(amountFilterGroup);
            filterLayout->addWidget(searchGroup);
            filterLayout->addWidget(paymentFilterGroup);
            filterLayout->addWidget(statsGroup);
            filterLayout->addStretch();

            // 添加到主布局
          //  mainLayout->addWidget(filterPanel);

            // 连接信号
            connect(timeToday, SIGNAL(clicked()), this, SLOT(onFilterChanged()));
            connect(timeWeek, SIGNAL(clicked()), this, SLOT(onFilterChanged()));
            connect(timeMonth, SIGNAL(clicked()), this, SLOT(onFilterChanged()));
            connect(timeAll, SIGNAL(clicked()), this, SLOT(onFilterChanged()));
            connect(minAmountEdit, SIGNAL(valueChanged(int)), this, SLOT(onAmountRangeChanged()));
            connect(maxAmountEdit, SIGNAL(valueChanged(int)), this, SLOT(onAmountRangeChanged()));
            connect(paymentAll, SIGNAL(clicked()), this, SLOT(onFilterChanged()));
            connect(paymentCash, SIGNAL(clicked()), this, SLOT(onFilterChanged()));
            connect(paymentWechat, SIGNAL(clicked()), this, SLOT(onFilterChanged()));
            connect(paymentAlipay, SIGNAL(clicked()), this, SLOT(onFilterChanged()));
            connect(paymentCard, SIGNAL(clicked()), this, SLOT(onFilterChanged()));
            connect(searchButton, SIGNAL(clicked()), this, SLOT(onSearch()));
            connect(searchEdit, SIGNAL(returnPressed()), this, SLOT(onSearch()));
            connect(clearFilterButton, SIGNAL(clicked()), this, SLOT(onClearFilters()));
        }

        void OrderManageWidget::createTabs()
        {
            // 创建标签页容器
            QWidget *centerContainer = new QWidget;
            centerContainer->setObjectName("centerContainer");
            QVBoxLayout *centerLayout = new QVBoxLayout(centerContainer);
            centerLayout->setContentsMargins(0, 0, 0, 0);
            centerLayout->setSpacing(0);

            // 创建标签页控件
            tabWidget = new QTabWidget;
            tabWidget->setObjectName("orderTabs");
            tabWidget->setDocumentMode(true);

            // 创建各标签页
            tabAll = new QWidget;
            tabPending = new QWidget;
            tabPaid = new QWidget;
            tabShipped = new QWidget;
            tabCompleted = new QWidget;
            tabCancelled = new QWidget;

            // 设置标签页布局
            setupTabLayout(tabAll, "全部");
            setupTabLayout(tabPending, "待付款");
            setupTabLayout(tabPaid, "已付款");
            setupTabLayout(tabShipped, "已发货");
            setupTabLayout(tabCompleted, "已完成");
            setupTabLayout(tabCancelled, "已取消");

            // 添加到标签页控件
            tabWidget->addTab(tabAll, "📋 全部订单");
            tabWidget->addTab(tabPending, "⏳ 待付款");
            tabWidget->addTab(tabPaid, "💳 已付款");
            tabWidget->addTab(tabShipped, "🚚 已发货");
            tabWidget->addTab(tabCompleted, "✅ 已完成");
            tabWidget->addTab(tabCancelled, "❌ 已取消");

            // 设置标签页样式
            tabWidget->setStyleSheet(QString(
                "#orderTabs::pane {"
                "    border: 2px solid #ecf0f1;"
                "    border-radius: 12px;"
                "    background-color: white;"
                "    top: -2px;"
                "    padding: 0px;"
                "}"
                "QTabBar::tab {"
                "    background-color: #f8f9fa;"
                "    border: 2px solid #ecf0f1;"
                "    border-bottom: none;"
                "    padding: 14px 28px;"
                "    margin-right: 4px;"
                "    font-size: 14px;"
                "    font-weight: bold;"
                "    color: #7f8c8d;"
                "    border-top-left-radius: 10px;"
                "    border-top-right-radius: 10px;"
                "    min-width: 120px;"
                "}"
                "QTabBar::tab:selected {"
                "    background-color: white;"
                "    color: %1;"
                "    border-bottom-color: white;"
                "    border-top: 3px solid %1;"
                "}"
                "QTabBar::tab:hover:!selected {"
                "    background-color: #e9ecef;"
                "    color: #34495e;"
                "}"
                "QTabBar::tab:first {"
                "    margin-left: 10px;"
                "}"
            ).arg(PRIMARY_COLOR.name()));

            centerLayout->addWidget(tabWidget);
           // mainLayout->addWidget(centerContainer, 1); // 中间容器占最大空间
            // 连接标签切换信号
            connect(tabWidget, SIGNAL(currentChanged(int)), this, SLOT(onTabChanged(int)));
        }

        void OrderManageWidget::setupTabLayout(QWidget *tab, const QString &status)
        {
            QVBoxLayout *layout = new QVBoxLayout(tab);
            layout->setContentsMargins(0, 0, 0, 0);

            // 创建对应的表格
            QTableWidget *table = new QTableWidget;
            table->setObjectName("orderTable");

            // 设置表格属性
            setupOrderTable(table);

            // 存储表格指针
            if (status == "全部") orderTableAll = table;
            else if (status == "待付款") orderTablePending = table;
            else if (status == "已付款") orderTablePaid = table;
            else if (status == "已发货") orderTableShipped = table;
            else if (status == "已完成") orderTableCompleted = table;
            else if (status == "已取消") orderTableCancelled = table;

            layout->addWidget(table);
        }

        void OrderManageWidget::setupOrderTable(QTableWidget *table)
        {
            // 设置表格属性 - 增加列以显示更多信息
            table->setColumnCount(9); // 增加列数
            table->setHorizontalHeaderLabels(QStringList()
                << "订单号" << "客户" << "手机号" << "金额" << "状态"
                << "支付方式" << "创建时间" << "地址" << "操作");

            // 设置表头
            QHeaderView *header = table->horizontalHeader();
            header->setStretchLastSection(true);
            header->setDefaultAlignment(Qt::AlignLeft);
            header->setMinimumHeight(48);
            header->setDefaultSectionSize(120);
            header->setSectionResizeMode(QHeaderView::Interactive);

            // 设置表格属性
            table->setAlternatingRowColors(true);
            table->setSelectionBehavior(QAbstractItemView::SelectRows);
            table->setSelectionMode(QAbstractItemView::SingleSelection);
            table->setEditTriggers(QAbstractItemView::NoEditTriggers);
            table->verticalHeader()->setVisible(false);
            table->verticalHeader()->setDefaultSectionSize(80);

            // 设置列宽 - 优化列宽分配
            table->setColumnWidth(0, 160); // 订单号
            table->setColumnWidth(1, 100); // 客户
            table->setColumnWidth(2, 130); // 手机号
            table->setColumnWidth(3, 100); // 金额
            table->setColumnWidth(4, 90);  // 状态
            table->setColumnWidth(5, 90);  // 支付方式
            table->setColumnWidth(6, 170); // 创建时间
            table->setColumnWidth(7, 180); // 地址（新增）
            // 操作列自动拉伸

            // 应用样式
            applyTableStyle(table);

            // 连接选择变化信号
            connect(table, SIGNAL(itemSelectionChanged()), this, SLOT(onTableSelectionChanged()));
        }

        void OrderManageWidget::createOrderDetails()
        {
            // 详情面板容器
            detailPanel = new QWidget;
            detailPanel->setMinimumWidth(DETAIL_PANEL_WIDTH);
            detailPanel->setMaximumWidth(DETAIL_PANEL_WIDTH);
            detailPanel->setObjectName("detailPanel");
            detailPanel->setStyleSheet("#detailPanel { background-color: white; border-radius: 12px; border: 1px solid #ddd; }");

            QVBoxLayout *detailLayout = new QVBoxLayout(detailPanel);
            detailLayout->setContentsMargins(20, 25, 20, 25);
            detailLayout->setSpacing(20);

            // ========== 订单基本信息 ==========
            orderInfoGroup = new QGroupBox("📋 订单详情");
            orderInfoGroup->setStyleSheet(
                "QGroupBox {"
                "    font-weight: bold;"
                "    color: #2c3e50;"
                "    border: 2px solid #ecf0f1;"
                "    border-radius: 10px;"
                "    margin-top: 14px;"
                "    background-color: #f8f9fa;"
                "}"
                "QGroupBox::title {"
                "    subcontrol-origin: margin;"
                "    left: 12px;"
                "    padding: 0 8px 0 8px;"
                "    color: #3498db;"
                "    font-size: 15px;"
                "}"
            );

            QFormLayout *infoLayout = new QFormLayout(orderInfoGroup);
            infoLayout->setContentsMargins(20, 35, 20, 25);
            infoLayout->setSpacing(16);
            infoLayout->setLabelAlignment(Qt::AlignRight);

            // 创建详情标签
            orderNoLabel = new QLabel("-");
            customerLabel = new QLabel("-");
            phoneLabel = new QLabel("-");      // 新增：手机号码
            addressLabel = new QLabel("-");    // 新增：收货地址
            statusLabel = new QLabel("-");
            amountLabel = new QLabel("-");
            paymentLabel = new QLabel("-");
            timeLabel = new QLabel("-");
            operatorLabel = new QLabel("-");
            remarkLabel = new QLabel("-");

            // 设置标签样式
            QString labelStyle =
                "QLabel {"
                "    color: #34495e;"
                "    font-size: 14px;"
                "    padding: 6px 0;"
                "    min-height: 24px;"
                "    border-bottom: 1px solid #f0f0f0;"
                "}";

            QString valueStyle =
                "QLabel {"
                "    color: #2c3e50;"
                "    font-size: 14px;"
                "    padding: 6px 0;"
                "    min-height: 24px;"
                "    font-weight: 500;"
                "    border-bottom: 1px solid #f0f0f0;"
                "}";

            // 设置标签和值样式
            orderNoLabel->setStyleSheet(valueStyle);
            customerLabel->setStyleSheet(valueStyle);
            phoneLabel->setStyleSheet(valueStyle);
            addressLabel->setStyleSheet(valueStyle + "QLabel { max-height: 40px; }"); // 地址可能较长
            statusLabel->setStyleSheet(valueStyle);
            amountLabel->setStyleSheet(valueStyle);
            paymentLabel->setStyleSheet(valueStyle);
            timeLabel->setStyleSheet(valueStyle);
            operatorLabel->setStyleSheet(valueStyle);
            remarkLabel->setStyleSheet(valueStyle + "QLabel { max-height: 60px; }");

            // 添加到表单
            infoLayout->addRow("订单号:", orderNoLabel);
            infoLayout->addRow("客户姓名:", customerLabel);
            infoLayout->addRow("手机号码:", phoneLabel);
            infoLayout->addRow("收货地址:", addressLabel);
            infoLayout->addRow("订单状态:", statusLabel);
            infoLayout->addRow("订单金额:", amountLabel);
            infoLayout->addRow("支付方式:", paymentLabel);
            infoLayout->addRow("创建时间:", timeLabel);
            infoLayout->addRow("操作员:", operatorLabel);
            infoLayout->addRow("订单备注:", remarkLabel);

            // ========== 订单商品列表 ==========
            orderItemsGroup = new QGroupBox("🛒 商品清单");
            orderItemsGroup->setStyleSheet(orderInfoGroup->styleSheet());
            QVBoxLayout *itemsLayout = new QVBoxLayout(orderItemsGroup);
            itemsLayout->setContentsMargins(15, 35, 15, 20);
            itemsLayout->setSpacing(15);

            // 创建商品表格
            orderItemsTable = new QTableWidget;
            orderItemsTable->setColumnCount(4);
            orderItemsTable->setHorizontalHeaderLabels(QStringList()
                << "商品名称" << "单价" << "数量" << "小计");

            // 设置表格属性
            orderItemsTable->verticalHeader()->setVisible(false);
            orderItemsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
            orderItemsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
            orderItemsTable->setAlternatingRowColors(true);
            orderItemsTable->setShowGrid(false);

            QHeaderView *itemsHeader = orderItemsTable->horizontalHeader();
            itemsHeader->setStretchLastSection(true);
            itemsHeader->setDefaultSectionSize(100);
            itemsHeader->setMinimumHeight(40);

            orderItemsTable->setStyleSheet(
                "QTableWidget {"
                "    border: 2px solid #ecf0f1;"
                "    border-radius: 8px;"
                "    background-color: white;"
                "    font-size: 13px;"
                "    selection-background-color: #d6eaf8;"
                "}"
                "QTableWidget::item {"
                "    padding: 12px 8px;"
                "    border-bottom: 1px solid #f8f9fa;"
                "}"
                "QHeaderView::section {"
                "    background-color: #f1f8ff;"
                "    padding: 14px 8px;"
                "    border: none;"
                "    font-weight: bold;"
                "    font-size: 13px;"
                "    color: #3498db;"
                "    border-bottom: 2px solid #3498db;"
                "}"
            );

            // 总计区域
            QWidget *totalWidget = new QWidget;
            QVBoxLayout *totalLayout = new QVBoxLayout(totalWidget);
            totalLayout->setContentsMargins(0, 15, 0, 0);
            totalLayout->setSpacing(10);

            subtotalLabel = new QLabel("商品总额: ¥0.00");
            discountLabel = new QLabel("优惠金额: ¥0.00");
            totalLabel = new QLabel("实付金额: ¥0.00");

            QString totalStyle =
                "QLabel {"
                "    color: #2c3e50;"
                "    font-size: 14px;"
                "    padding: 8px 0;"
                "    border-bottom: 1px dashed #ecf0f1;"
                "}"
                "QLabel:last-child {"
                "    color: #e74c3c;"
                "    font-weight: bold;"
                "    font-size: 16px;"
                "    border-bottom: 2px solid #e74c3c;"
                "}";

            subtotalLabel->setStyleSheet(totalStyle);
            discountLabel->setStyleSheet(totalStyle);
            totalLabel->setStyleSheet(totalStyle);

            totalLayout->addWidget(subtotalLabel);
            totalLayout->addWidget(discountLabel);
            totalLayout->addWidget(totalLabel);

            itemsLayout->addWidget(orderItemsTable, 1);
            itemsLayout->addWidget(totalWidget);
            // ========== 操作按钮 ==========
                QWidget *actionWidget = new QWidget;
                QHBoxLayout *actionLayout = new QHBoxLayout(actionWidget);
                actionLayout->setContentsMargins(0, 0, 0, 0);
                actionLayout->setSpacing(15);

                QPushButton *viewDetailButton = new QPushButton("🔍 查看详情");
                QPushButton *exportSingleButton = new QPushButton("📄 导出本单");

                viewDetailButton->setStyleSheet(QString(
                    "QPushButton {"
                    "    background-color: %1;"
                    "    color: white;"
                    "    border: none;"
                    "    border-radius: 8px;"
                    "    padding: 12px 24px;"
                    "    font-size: 14px;"
                    "    font-weight: bold;"
                    "    min-width: 120px;"
                    "}"
                    "QPushButton:hover {"
                    "    background-color: %2;"
                    "    transform: translateY(-1px);"
                    "    box-shadow: 0 4px 12px rgba(52, 152, 219, 0.3);"
                    "}"
                ).arg(INFO_COLOR.name())
                 .arg(INFO_COLOR.darker(120).name()));

                exportSingleButton->setStyleSheet(QString(
                    "QPushButton {"
                    "    background-color: %1;"
                    "    color: white;"
                    "    border: none;"
                    "    border-radius: 8px;"
                    "    padding: 12px 24px;"
                    "    font-size: 14px;"
                    "    font-weight: bold;"
                    "    min-width: 120px;"
                    "}"
                    "QPushButton:hover {"
                    "    background-color: %2;"
                    "    transform: translateY(-1px);"
                    "    box-shadow: 0 4px 12px rgba(39, 174, 96, 0.3);"
                    "}"
                ).arg(SUCCESS_COLOR.name())
                 .arg(SUCCESS_COLOR.darker(120).name()));

                actionLayout->addWidget(viewDetailButton);
                actionLayout->addWidget(exportSingleButton);

                // ========== 添加到详情面板 ==========
                detailLayout->addWidget(orderInfoGroup);
                detailLayout->addWidget(orderItemsGroup, 1);
                detailLayout->addWidget(actionWidget);

                // 添加到主布局
             //   mainLayout->addWidget(detailPanel);

                // 连接信号
                connect(viewDetailButton, SIGNAL(clicked()), this, SLOT(onShowOrderDetails()));
                connect(exportSingleButton, SIGNAL(clicked()), this, SLOT(onExportCurrentTab()));
            }

            void OrderManageWidget::createNewOrderDialog()
            {
                newOrderDialog = new QDialog(this);
                newOrderDialog->setWindowTitle("🛒 新建图书订单");
                newOrderDialog->setModal(true);
                newOrderDialog->resize(900, 700);
                newOrderDialog->setStyleSheet(
                    "QDialog {"
                    "    background-color: #f8f9fa;"
                    "    border-radius: 12px;"
                    "}"
                );

                QVBoxLayout *dialogLayout = new QVBoxLayout(newOrderDialog);
                dialogLayout->setContentsMargins(25, 25, 25, 25);
                dialogLayout->setSpacing(20);

                // 标题
                QLabel *titleLabel = new QLabel("新建图书订单");
                titleLabel->setStyleSheet(
                    "font-size: 24px;"
                    "font-weight: bold;"
                    "color: #2c3e50;"
                    "padding-bottom: 15px;"
                    "border-bottom: 3px solid #3498db;"
                    "margin-bottom: 10px;"
                    "background-color: white;"
                    "border-radius: 10px;"
                    "padding: 20px;"
                );
                titleLabel->setAlignment(Qt::AlignCenter);

                // 客户信息区域
                QGroupBox *customerGroup = new QGroupBox("👤 客户信息");
                customerGroup->setStyleSheet(
                    "QGroupBox {"
                    "    font-weight: bold;"
                    "    color: #2c3e50;"
                    "    border: 2px solid #ecf0f1;"
                    "    border-radius: 10px;"
                    "    margin-top: 14px;"
                    "    background-color: white;"
                    "}"
                    "QGroupBox::title {"
                    "    subcontrol-origin: margin;"
                    "    left: 15px;"
                    "    padding: 0 10px 0 10px;"
                    "    color: #3498db;"
                    "    font-size: 15px;"
                    "}"
                );

                QFormLayout *customerLayout = new QFormLayout(customerGroup);
                customerLayout->setContentsMargins(20, 35, 20, 25);
                customerLayout->setSpacing(15);
                customerLayout->setLabelAlignment(Qt::AlignRight);

                customerNameEdit = new QLineEdit;
                customerPhoneEdit = new QLineEdit;
                customerAddressEdit = new QLineEdit;  // 新增：收货地址输入框

                customerNameEdit->setPlaceholderText("请输入客户姓名");
                customerPhoneEdit->setPlaceholderText("请输入手机号码");
                customerAddressEdit->setPlaceholderText("请输入收货地址");
                QString inputStyle =
                        "QLineEdit {"
                        "    padding: 12px 15px;"
                        "    border: 2px solid #ecf0f1;"
                        "    border-radius: 8px;"
                        "    font-size: 14px;"
                        "    background-color: white;"
                        "}"
                        "QLineEdit:focus {"
                        "    border-color: #3498db;"
                        "    outline: none;"
                        "    box-shadow: 0 0 0 3px rgba(52, 152, 219, 0.1);"
                        "}";

                    customerNameEdit->setStyleSheet(inputStyle);
                    customerPhoneEdit->setStyleSheet(inputStyle);
                    customerAddressEdit->setStyleSheet(inputStyle);

                    customerLayout->addRow("客户姓名:", customerNameEdit);
                    customerLayout->addRow("手机号码:", customerPhoneEdit);
                    customerLayout->addRow("收货地址:", customerAddressEdit);

                    // 商品选择区域
                    QGroupBox *productGroup = new QGroupBox("📚 选择图书");
                    productGroup->setStyleSheet(customerGroup->styleSheet());

                    QHBoxLayout *productSelectLayout = new QHBoxLayout;
                    productSelectLayout->setSpacing(15);

                    QLabel *productLabel = new QLabel("选择图书:");
                    productLabel->setStyleSheet("font-weight: bold; font-size: 14px; color: #2c3e50;");

                    productCombo = new QComboBox;
                    productCombo->setMinimumWidth(350);
                    productCombo->setStyleSheet(
                        "QComboBox {"
                        "    padding: 12px 15px;"
                        "    border: 2px solid #ecf0f1;"
                        "    border-radius: 8px;"
                        "    font-size: 14px;"
                        "    background-color: white;"
                        "}"
                        "QComboBox:focus {"
                        "    border-color: #3498db;"
                        "    outline: none;"
                        "}"
                        "QComboBox::drop-down {"
                        "    border: none;"
                        "}"
                        "QComboBox::down-arrow {"
                        "    image: none;"
                        "    border-left: 5px solid transparent;"
                        "    border-right: 5px solid transparent;"
                        "    border-top: 5px solid #7f8c8d;"
                        "}"
                    );

                    QLabel *quantityLabel = new QLabel("数量:");
                    quantityLabel->setStyleSheet("font-weight: bold; font-size: 14px; color: #2c3e50;");

                    quantitySpin = new QSpinBox;
                    quantitySpin->setRange(1, 999);
                    quantitySpin->setValue(1);
                    quantitySpin->setStyleSheet(inputStyle + "QSpinBox { min-width: 80px; max-width: 100px; }");

                    QPushButton *addButton = new QPushButton("➕ 添加到购物车");
                    addButton->setStyleSheet(QString(
                        "QPushButton {"
                        "    background-color: %1;"
                        "    color: white;"
                        "    border: none;"
                        "    border-radius: 8px;"
                        "    padding: 14px 28px;"
                        "    font-size: 14px;"
                        "    font-weight: bold;"
                        "    min-width: 150px;"
                        "}"
                        "QPushButton:hover {"
                        "    background-color: %2;"
                        "    transform: translateY(-1px);"
                        "    box-shadow: 0 4px 12px rgba(41, 128, 185, 0.3);"
                        "}"
                    ).arg(PRIMARY_COLOR.name())
                     .arg(PRIMARY_COLOR.darker(120).name()));

                    productSelectLayout->addWidget(productLabel);
                    productSelectLayout->addWidget(productCombo, 1);
                    productSelectLayout->addWidget(quantityLabel);
                    productSelectLayout->addWidget(quantitySpin);
                    productSelectLayout->addWidget(addButton);

                    QVBoxLayout *productLayout = new QVBoxLayout(productGroup);
                    productLayout->setContentsMargins(20, 35, 20, 25);
                    productLayout->addLayout(productSelectLayout);

                    // 购物车区域
                    QGroupBox *cartGroup = new QGroupBox("🛍️ 购物车清单");
                    cartGroup->setStyleSheet(customerGroup->styleSheet());

                    QVBoxLayout *cartLayout = new QVBoxLayout(cartGroup);
                    cartLayout->setContentsMargins(20, 35, 20, 25);
                    cartLayout->setSpacing(15);

                    cartTable = new QTableWidget;
                    cartTable->setColumnCount(6);
                    cartTable->setHorizontalHeaderLabels(QStringList()
                        << "图书名称" << "ISBN" << "单价" << "数量" << "小计" << "操作");

                    cartTable->verticalHeader()->setVisible(false);
                    cartTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
                    cartTable->setSelectionBehavior(QAbstractItemView::SelectRows);
                    cartTable->setAlternatingRowColors(true);
                    cartTable->setShowGrid(false);

                    QHeaderView *cartHeader = cartTable->horizontalHeader();
                    cartHeader->setStretchLastSection(true);
                    cartHeader->setDefaultSectionSize(120);
                    cartHeader->setMinimumHeight(45);

                    cartTable->setStyleSheet(
                        "QTableWidget {"
                        "    border: 2px solid #ecf0f1;"
                        "    border-radius: 8px;"
                        "    background-color: white;"
                        "    font-size: 14px;"
                        "    selection-background-color: #d6eaf8;"
                        "}"
                        "QTableWidget::item {"
                        "    padding: 14px 10px;"
                        "    border-bottom: 1px solid #f8f9fa;"
                        "}"
                        "QHeaderView::section {"
                        "    background-color: #f1f8ff;"
                        "    padding: 16px 10px;"
                        "    border: none;"
                        "    font-weight: bold;"
                        "    font-size: 14px;"
                        "    color: #3498db;"
                        "    border-bottom: 2px solid #3498db;"
                        "}"
                    );
                    // 购物车操作按钮
                       QHBoxLayout *cartButtonLayout = new QHBoxLayout;
                       cartButtonLayout->setSpacing(15);

                       QPushButton *removeButton = new QPushButton("➖ 移除选中");
                       QPushButton *clearCartButton = new QPushButton("🗑️ 清空购物车");

                       removeButton->setStyleSheet(QString(
                           "QPushButton {"
                           "    background-color: %1;"
                           "    color: white;"
                           "    border: none;"
                           "    border-radius: 8px;"
                           "    padding: 12px 24px;"
                           "    font-size: 14px;"
                           "    font-weight: bold;"
                           "    min-width: 120px;"
                           "}"
                           "QPushButton:hover {"
                           "    background-color: %2;"
                           "    transform: translateY(-1px);"
                           "    box-shadow: 0 4px 12px rgba(231, 76, 60, 0.3);"
                           "}"
                       ).arg(DANGER_COLOR.name())
                        .arg(DANGER_COLOR.darker(120).name()));

                       clearCartButton->setStyleSheet(
                           "QPushButton {"
                           "    background-color: #95a5a6;"
                           "    color: white;"
                           "    border: none;"
                           "    border-radius: 8px;"
                           "    padding: 12px 24px;"
                           "    font-size: 14px;"
                           "    font-weight: bold;"
                           "    min-width: 120px;"
                           "}"
                           "QPushButton:hover {"
                           "    background-color: #7f8c8d;"
                           "    transform: translateY(-1px);"
                           "    box-shadow: 0 4px 12px rgba(149, 165, 166, 0.3);"
                           "}"
                       );

                       cartButtonLayout->addWidget(removeButton);
                       cartButtonLayout->addWidget(clearCartButton);
                       cartButtonLayout->addStretch();

                       // 总计区域
                       QWidget *totalWidget = new QWidget;
                       QHBoxLayout *totalLayout = new QHBoxLayout(totalWidget);
                       cartTotalLabel = new QLabel("总计: ¥0.00");
                       cartTotalLabel->setStyleSheet(
                           "font-size: 22px;"
                           "font-weight: bold;"
                           "color: #e74c3c;"
                           "padding: 15px 20px;"
                           "background-color: #fdf2f2;"
                           "border-radius: 8px;"
                           "border: 2px solid #fadbd8;"
                       );

                       totalLayout->addStretch();
                       totalLayout->addWidget(cartTotalLabel);

                       cartLayout->addWidget(cartTable, 1);
                       cartLayout->addLayout(cartButtonLayout);
                       cartLayout->addWidget(totalWidget);

                       // 支付和备注区域
                       QGroupBox *paymentGroup = new QGroupBox("💳 支付信息");
                       paymentGroup->setStyleSheet(customerGroup->styleSheet());

                       QFormLayout *paymentLayout = new QFormLayout(paymentGroup);
                       paymentLayout->setContentsMargins(20, 35, 20, 25);
                       paymentLayout->setSpacing(15);
                       paymentLayout->setLabelAlignment(Qt::AlignRight);

                       paymentCombo = new QComboBox;
                       paymentCombo->addItems(QStringList() << "现金" << "微信支付" << "支付宝" << "会员卡" << "银行卡");
                       paymentCombo->setStyleSheet(productCombo->styleSheet());

                       QLabel *remarkLabel = new QLabel("订单备注:");
                       remarkLabel->setStyleSheet("font-weight: bold; font-size: 14px; color: #2c3e50;");

                       remarkEdit = new QTextEdit;
                       remarkEdit->setMaximumHeight(100);
                       remarkEdit->setPlaceholderText("请输入订单备注信息（可选）...");
                       remarkEdit->setStyleSheet(
                           "QTextEdit {"
                           "    border: 2px solid #ecf0f1;"
                           "    border-radius: 8px;"
                           "    padding: 12px;"
                           "    font-size: 14px;"
                           "    background-color: white;"
                           "}"
                           "QTextEdit:focus {"
                           "    border-color: #3498db;"
                           "    outline: none;"
                           "    box-shadow: 0 0 0 3px rgba(52, 152, 219, 0.1);"
                           "}"
                       );

                       paymentLayout->addRow("支付方式:", paymentCombo);
                       paymentLayout->addRow(remarkLabel, remarkEdit);
                       // 按钮区域
                           QHBoxLayout *buttonLayout = new QHBoxLayout;
                           buttonLayout->setSpacing(25);

                           confirmOrderButton = new QPushButton("✅ 确认下单");
                           QPushButton *cancelButton = new QPushButton("取消订单");

                           confirmOrderButton->setStyleSheet(QString(
                               "QPushButton {"
                               "    background-color: %1;"
                               "    color: white;"
                               "    border: none;"
                               "    border-radius: 10px;"
                               "    padding: 18px 50px;"
                               "    font-size: 16px;"
                               "    font-weight: bold;"
                               "    min-width: 160px;"
                               "}"
                               "QPushButton:hover {"
                               "    background-color: %2;"
                               "    transform: translateY(-2px);"
                               "    box-shadow: 0 6px 20px rgba(39, 174, 96, 0.4);"
                               "}"
                               "QPushButton:disabled {"
                               "    background-color: #bdc3c7;"
                               "    cursor: not-allowed;"
                               "    transform: none;"
                               "    box-shadow: none;"
                               "}"
                           ).arg(SUCCESS_COLOR.name())
                            .arg(SUCCESS_COLOR.darker(120).name()));

                           cancelButton->setStyleSheet(
                               "QPushButton {"
                               "    background-color: #95a5a6;"
                               "    color: white;"
                               "    border: none;"
                               "    border-radius: 10px;"
                               "    padding: 18px 50px;"
                               "    font-size: 16px;"
                               "    font-weight: bold;"
                               "    min-width: 160px;"
                               "}"
                               "QPushButton:hover {"
                               "    background-color: #7f8c8d;"
                               "    transform: translateY(-2px);"
                               "    box-shadow: 0 6px 20px rgba(149, 165, 166, 0.4);"
                               "}"
                           );

                           confirmOrderButton->setEnabled(false);

                           buttonLayout->addStretch();
                           buttonLayout->addWidget(cancelButton);
                           buttonLayout->addWidget(confirmOrderButton);

                           // 添加到对话框布局
                           dialogLayout->addWidget(titleLabel);
                           dialogLayout->addWidget(customerGroup);
                           dialogLayout->addWidget(productGroup);
                           dialogLayout->addWidget(cartGroup, 1);
                           dialogLayout->addWidget(paymentGroup);
                           dialogLayout->addLayout(buttonLayout);

                           // 连接信号
                           connect(addButton, SIGNAL(clicked()), this, SLOT(onAddToCart()));
                           connect(removeButton, SIGNAL(clicked()), this, SLOT(onRemoveFromCart()));
                           connect(clearCartButton, SIGNAL(clicked()), this, SLOT(onclearCart()));
                           connect(customerNameEdit, SIGNAL(textChanged(const QString&)), this, SLOT(onUpdateCartQuantity()));
                           connect(customerPhoneEdit, SIGNAL(textChanged(const QString&)), this, SLOT(onUpdateCartQuantity()));
                           connect(customerAddressEdit, SIGNAL(textChanged(const QString&)), this, SLOT(onUpdateCartQuantity()));
                           connect(confirmOrderButton, SIGNAL(clicked()), this, SLOT(onConfirmNewOrder()));
                           connect(cancelButton, SIGNAL(clicked()), this, SLOT(onCancelNewOrder()));
                           connect(productCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(onUpdateCartQuantity()));
                           connect(quantitySpin, SIGNAL(valueChanged(int)), this, SLOT(onUpdateCartQuantity()));
                       }

                       void OrderManageWidget::applyTableStyle(QTableWidget *table)
                       {
                           table->setStyleSheet(QString(
                               "#orderTable {"
                               "    border: 2px solid #ecf0f1;"
                               "    border-radius: 0 0 12px 12px;"
                               "    background-color: white;"
                               "    font-size: 14px;"
                               "    selection-background-color: %1;"
                               "    selection-color: white;"
                               "    gridline-color: #f8f9fa;"
                               "}"
                               "#orderTable::item {"
                               "    padding: 14px 10px;"
                               "    border-bottom: 1px solid #f8f9fa;"
                               "}"
                               "#orderTable::item:selected {"
                               "    color: white;"
                               "    font-weight: bold;"
                               "}"
                               "QHeaderView::section {"
                               "    background-color: #f1f8ff;"
                               "    padding: 16px 10px;"
                               "    border: none;"
                               "    border-right: 1px solid #e3f2fd;"
                               "    border-bottom: 3px solid %1;"
                               "    font-weight: bold;"
                               "    color: #2c3e50;"
                               "    font-size: 14px;"
                               "}"
                               "QHeaderView::section:last {"
                               "    border-right: none;"
                               "}"
                           ).arg(PRIMARY_COLOR.name()));
                       }

                       QString OrderManageWidget::getStatusColor(const QString &status)
                       {
                           if (status == "待付款") return "#e67e22";      // 橙色
                           else if (status == "已付款") return "#3498db"; // 蓝色
                           else if (status == "已发货") return "#9b59b6"; // 紫色
                           else if (status == "已完成") return "#27ae60"; // 绿色
                           else if (status == "已取消") return "#95a5a6"; // 灰色
                           else if (status == "退款中") return "#e74c3c"; // 红色
                           else return "#2c3e50";                       // 深蓝色
                       }
                       void OrderManageWidget::generateSampleData()
                       {
                           // 清空现有数据
                           orderData.clear();

                           // 模拟订单数据
                           QStringList statuses = QStringList() << "待付款" << "已付款" << "已发货" << "已完成" << "已取消";
                           QStringList payments = QStringList() << "现金" << "微信支付" << "支付宝" << "会员卡";
                           QStringList customers = QStringList() << "张三" << "李四" << "王五" << "赵六" << "钱七" << "孙八" << "周九";
                           QStringList phones = QStringList() << "13800138000" << "13900139000" << "13600136000"
                                                             << "13700137000" << "13500135000" << "13400134000";
                           QStringList addresses = QStringList()
                               << "北京市海淀区中关村大街1号"
                               << "上海市浦东新区陆家嘴环路100号"
                               << "广州市天河区珠江新城华穗路1号"
                               << "深圳市南山区科技园科技中一路"
                               << "杭州市西湖区文三路398号"
                               << "南京市鼓楼区中山北路1号";
                           QStringList operators = QStringList() << "王经理" << "店员A" << "店员B" << "系统" << "自动";

                           // 生成50个测试订单
                           for (int i = 0; i < 50; i++) {
                               QStringList order;

                               // 生成订单号
                               QString orderNo = QString("BOOK%1").arg(202312000 + i);
                               order << orderNo;

                               // 客户
                               order << customers.at(i % customers.size());

                               // 手机号码
                               order << phones.at(i % phones.size());

                               // 金额 (30-500之间的随机数)
                               double amount = 30.0 + (qrand() % 471);
                               order << QString("¥%1").arg(amount, 0, 'f', 2);

                               // 状态
                               QString status = statuses.at(i % statuses.size());
                               order << status;

                               // 支付方式
                               QString payment = payments.at(i % payments.size());
                               order << payment;

                               // 创建时间 (最近30天内)
                               int daysAgo = qrand() % 30;
                               QDateTime createTime = QDateTime::currentDateTime().addDays(-daysAgo);
                               createTime = createTime.addSecs(qrand() % 86400);
                               order << createTime.toString("yyyy-MM-dd hh:mm:ss");

                               // 收货地址
                               order << addresses.at(i % addresses.size());

                               // 操作员
                               order << operators.at(i % operators.size());

                               // 备注（部分订单有备注）
                               if (i % 3 == 0) {
                                   QStringList remarks = QStringList()
                                       << "需要发票"
                                       << "急件，请尽快发货"
                                       << "送货前请电话联系"
                                       << "包装要精美"
                                       << "送到小区门口即可";
                                   order << remarks.at(i % remarks.size());
                               }

                               orderData.append(order);
                           }
                       }

                       void OrderManageWidget::loadProductsForNewOrder()
                       {
                           // 清空商品数据
                           productData.clear();

                           // 模拟商品数据
                           QStringList products = QStringList()
                               << "红楼梦|9787020002207|文学|59.80"
                               << "三国演义|9787020008728|文学|49.90"
                               << "西游记|9787500601593|文学|45.00"
                               << "时间简史|9787532744306|科技|38.00"
                               << "人类简史|9787505738968|科技|68.00"
                               << "教育学原理|9787561772045|教育|39.80"
                               << "艺术的故事|9787301127606|艺术|280.00"
                               << "追风筝的人|9787208061644|文学|36.00"
                               << "百年孤独|9787020043270|文学|39.50"
                               << "活着|9787506365437|文学|28.00"
                               << "围城|9787020008729|文学|32.00"
                               << "挪威的森林|9787544258607|文学|29.80"
                               << "小王子|9787532761853|文学|22.00"
                               << "三体|9787536692930|科幻|23.00"
                               << "明朝那些事儿|9787506344791|历史|29.80";

                           // 添加到下拉框
                           productCombo->clear();
                           for (int i = 0; i < products.size(); i++) {
                               QStringList fields = products[i].split('|');
                               if (fields.size() >= 4) {
                                   productData.append(fields);
                                   QString displayText = QString("%1 (¥%2)").arg(fields[0]).arg(fields[3]);
                                   productCombo->addItem(displayText);
                               }
                           }
                       }
                       void OrderManageWidget::loadOrders(const QString &status)
                       {
                           QTableWidget *table = nullptr;

                           // 根据状态选择表格
                           if (status == "全部") table = orderTableAll;
                           else if (status == "待付款") table = orderTablePending;
                           else if (status == "已付款") table = orderTablePaid;
                           else if (status == "已发货") table = orderTableShipped;
                           else if (status == "已完成") table = orderTableCompleted;
                           else if (status == "已取消") table = orderTableCancelled;

                           if (!table) return;

                           // 清空表格
                           table->setRowCount(0);

                           // 填充表格
                           int visibleCount = 0;
                           for (int i = 0; i < orderData.size(); i++) {
                               const QStringList &order = orderData[i];

                               // 过滤状态
                               QString orderStatus = order[4]; // 注意索引变化
                               if (status != "全部" && orderStatus != status) {
                                   continue;
                               }

                               // 应用其他筛选条件
                               bool passedFilter = true;

                               // 金额筛选
                               double amount = order[3].mid(1).toDouble();
                               if (amount < minAmountEdit->value() || amount > maxAmountEdit->value()) {
                                   passedFilter = false;
                               }

                               // 支付方式筛选
                               QString payment = order[5];
                               if (!paymentAll->isChecked()) {
                                   bool paymentMatch = false;
                                   if (paymentCash->isChecked() && payment == "现金") paymentMatch = true;
                                   if (paymentWechat->isChecked() && payment == "微信支付") paymentMatch = true;
                                   if (paymentAlipay->isChecked() && payment == "支付宝") paymentMatch = true;
                                   if (paymentCard->isChecked() && payment == "会员卡") paymentMatch = true;
                                   if (!paymentMatch) passedFilter = false;
                               }

                               // 关键词搜索
                               QString keyword = searchEdit->text().trimmed();
                               if (!keyword.isEmpty() && passedFilter) {
                                   bool found = false;
                                   if (order[0].contains(keyword) ||    // 订单号
                                       order[1].contains(keyword) ||    // 客户名
                                       order[2].contains(keyword)) {    // 手机号
                                       found = true;
                                   }
                                   if (!found) passedFilter = false;
                               }

                               if (!passedFilter) continue;

                               int row = table->rowCount();
                               table->insertRow(row);

                               // 填充数据 - 注意索引变化
                               for (int col = 0; col < 8; col++) { // 现在有8列数据
                                   int orderIndex = col;
                                   if (col >= 2) orderIndex = col; // 调整索引映射
                                   if (orderIndex < order.size()) {
                                       QTableWidgetItem *item = new QTableWidgetItem(order[orderIndex]);
                                       table->setItem(row, col, item);

                                       // 设置状态颜色
                                       if (col == 4) {
                                           QString color = getStatusColor(order[orderIndex]);
                                           item->setForeground(QColor(color));
                                           item->setFont(QFont("", -1, QFont::Bold));
                                       }

                                       // 金额列居右对齐
                                       if (col == 3) {
                                           item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
                                       }

                                       // 时间列居中对齐
                                       if (col == 6) {
                                           item->setTextAlignment(Qt::AlignCenter);
                                       }
                                   }
                               }

                               // 操作列
                               QWidget *actionWidget = new QWidget;
                               QHBoxLayout *actionLayout = new QHBoxLayout(actionWidget);
                               actionLayout->setContentsMargins(5, 2, 5, 2);
                               actionLayout->setSpacing(8);

                               QPushButton *viewButton = new QPushButton("查看详情");
                               viewButton->setProperty("row", i);
                               viewButton->setFixedSize(90, 60);
                               viewButton->setStyleSheet(
                                   "QPushButton {"
                                   "    background-color: #3498db;"
                                   "    color: white;"
                                   "    border: none;"
                                   "    border-radius: 6px;"
                                   "    font-size: 13px;"
                                   "    font-weight: 500;"
                                   "}"
                                   "QPushButton:hover {"
                                   "    background-color: #2980b9;"
                                   "    transform: translateY(-1px);"
                                   "}"
                               );

                               actionLayout->addWidget(viewButton);
                               actionLayout->addStretch();

                               connect(viewButton, SIGNAL(clicked()), this, SLOT(onShowOrderDetails()));

                               table->setCellWidget(row, 8, actionWidget);

                               visibleCount++;
                           }

                           // 更新统计信息
                           updateStatistics();
                       }
                       void OrderManageWidget::updateStatistics()
                       {
                           // 计算统计数据
                           int totalOrders = 0;
                           int pendingOrders = 0;
                           double totalAmount = 0.0;

                           for (int i = 0; i < orderData.size(); i++) {
                               const QStringList &order = orderData[i];
                               totalOrders++;
                               if (order[4] == "待付款") {
                                   pendingOrders++;
                               }
                               totalAmount += order[3].mid(1).toDouble();
                           }

                           // 更新统计标签
                           totalOrdersLabel->setText(QString("总计: %1 单").arg(totalOrders));
                           totalAmountLabel->setText(QString("金额: ¥%1").arg(totalAmount, 0, 'f', 2));
                           pendingOrdersLabel->setText(QString("待处理: %1 单").arg(pendingOrders));

                           // 更新进度条
                           int completedPercent = totalOrders > 0 ? (totalOrders - pendingOrders) * 100 / totalOrders : 0;
                           completionBar->setValue(completedPercent);
                       }

                       // ==================== 槽函数实现 ====================

                       void OrderManageWidget::onTabChanged(int index)
                       {
                           QString status;
                           switch (index) {
                               case 0: status = "全部"; break;
                               case 1: status = "待付款"; break;
                               case 2: status = "已付款"; break;
                               case 3: status = "已发货"; break;
                               case 4: status = "已完成"; break;
                               case 5: status = "已取消"; break;
                               default: status = "全部";
                           }
                           loadOrders(status);
                       }

                       void OrderManageWidget::onOpenNewOrderDialog()
                       {
                           // 重置表单
                           clearNewOrderForm();
                           // 显示对话框
                           newOrderDialog->exec();
                       }

                       void OrderManageWidget::onAddToCart()
                       {
                           int index = productCombo->currentIndex();
                           if (index < 0 || index >= productData.size()) {
                               QMessageBox::warning(newOrderDialog, "警告", "请选择图书商品");
                               return;
                           }

                           QStringList product = productData[index];
                           QString productName = product[0];
                           QString isbn = product[1];
                           double price = product[3].toDouble();
                           int quantity = quantitySpin->value();
                           double subtotal = price * quantity;

                           // 检查是否已存在相同商品
                           for (int i = 0; i < cartItems.size(); i++) {
                               if (cartItems[i][1] == isbn) {
                                   // 更新数量
                                   int oldQty = cartItems[i][3].toInt();
                                   int newQty = oldQty + quantity;
                                   double newSubtotal = price * newQty;
                                   cartItems[i][3] = QString::number(newQty);
                                   cartItems[i][4] = QString::number(newSubtotal, 'f', 2);
                                   updateCartTotal();
                                   return;
                               }
                           }

                           // 添加到购物车数据
                           QStringList cartItem;
                           cartItem << productName
                                    << isbn
                                    << QString::number(price, 'f', 2)
                                    << QString::number(quantity)
                                    << QString::number(subtotal, 'f', 2);
                           cartItems.append(cartItem);

                           // 更新购物车显示
                           updateCartTotal();

                           // 重置数量
                           quantitySpin->setValue(1);
                       }

                       void OrderManageWidget::onRemoveFromCart()
                       {
                           int currentRow = cartTable->currentRow();
                           if (currentRow < 0 || currentRow >= cartItems.size()) {
                               QMessageBox::warning(newOrderDialog, "警告", "请选择要移除的商品");
                               return;
                           }
                           cartItems.removeAt(currentRow);
                           updateCartTotal();
                       }

                       void OrderManageWidget::onUpdateCartQuantity()
                       {
                           // 检查客户信息是否填写完整
                           QString customerName = customerNameEdit->text().trimmed();
                           QString customerPhone = customerPhoneEdit->text().trimmed();
                           QString customerAddress = customerAddressEdit->text().trimmed();

                           bool hasCustomerInfo = !customerName.isEmpty() && !customerPhone.isEmpty() && !customerAddress.isEmpty();
                           confirmOrderButton->setEnabled(hasCustomerInfo && !cartItems.isEmpty());
                       }

                       void OrderManageWidget::updateCartTotal()
                       {
                           // 更新购物车表格
                           cartTable->setRowCount(0);
                           cartTotal = 0.0;

                           for (int i = 0; i < cartItems.size(); i++) {
                               int row = cartTable->rowCount();
                               cartTable->insertRow(row);

                               const QStringList &item = cartItems[i];
                               for (int col = 0; col < item.size(); col++) {
                                   QTableWidgetItem *tableItem = new QTableWidgetItem(item[col]);
                                   cartTable->setItem(row, col, tableItem);

                                   // 价格和小计列居右对齐
                                   if (col >= 2) {
                                       tableItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
                                   }
                               }

                               // 添加操作按钮
                               QPushButton *removeBtn = new QPushButton("移除");
                               removeBtn->setProperty("row", i);
                               removeBtn->setFixedSize(70, 32);
                               removeBtn->setStyleSheet(
                                   "QPushButton {"
                                   "    background-color: #e74c3c;"
                                   "    color: white;"
                                   "    border: none;"
                                   "    border-radius: 6px;"
                                   "    font-size: 12px;"
                                   "}"
                                   "QPushButton:hover {"
                                   "    background-color: #c0392b;"
                                   "    transform: translateY(-1px);"
                                   "}"
                               );
                               connect(removeBtn, SIGNAL(clicked()), this, SLOT(onRemoveFromCart()));
                               cartTable->setCellWidget(row, 5, removeBtn);

                               cartTotal += item[4].toDouble();
                           }

                           // 更新总计
                           cartTotalLabel->setText(QString("总计: ¥%1").arg(cartTotal, 0, 'f', 2));

                           // 更新确认按钮状态
                           onUpdateCartQuantity();
                       }

                       void OrderManageWidget::clearNewOrderForm()
                       {
                           customerNameEdit->clear();
                           customerPhoneEdit->clear();
                           customerAddressEdit->clear();
                           cartItems.clear();
                           cartTotal = 0.0;
                           paymentCombo->setCurrentIndex(0);
                           remarkEdit->clear();
                           quantitySpin->setValue(1);
                           updateCartTotal();
                       }

                       void OrderManageWidget::onConfirmNewOrder()
                       {
                           QString customerName = customerNameEdit->text().trimmed();
                           QString customerPhone = customerPhoneEdit->text().trimmed();
                           QString customerAddress = customerAddressEdit->text().trimmed();

                           if (customerName.isEmpty()) {
                               QMessageBox::warning(newOrderDialog, "警告", "请输入客户姓名");
                               return;
                           }

                           if (customerPhone.isEmpty()) {
                               QMessageBox::warning(newOrderDialog, "警告", "请输入手机号码");
                               return;
                           }

                           if (customerAddress.isEmpty()) {
                               QMessageBox::warning(newOrderDialog, "警告", "请输入收货地址");
                               return;
                           }

                           if (cartItems.isEmpty()) {
                               QMessageBox::warning(newOrderDialog, "警告", "请至少添加一件商品");
                               return;
                           }

                           // 生成订单号
                           QString orderNo = QString("BOOK%1")
                               .arg(QDateTime::currentDateTime().toString("yyyyMMddhhmmss"));

                           // 获取支付方式
                           QString paymentMethod = paymentCombo->currentText();
                           QString remark = remarkEdit->toPlainText().trimmed();

                           // 创建新订单数据
                           QStringList newOrder;
                           newOrder << orderNo
                                    << customerName
                                    << customerPhone
                                    << QString("¥%1").arg(cartTotal, 0, 'f', 2)
                                    << "待付款"
                                    << paymentMethod
                                    << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")
                                    << customerAddress
                                    << "当前用户";

                           if (!remark.isEmpty()) {
                               newOrder << remark;
                           }

                           // 添加到订单数据
                           orderData.append(newOrder);

                           // 关闭对话框
                           newOrderDialog->accept();

                           // 刷新订单列表
                           onTabChanged(tabWidget->currentIndex());

                           // 更新统计信息
                           updateStatistics();

                           // 显示成功消息
                           QMessageBox::information(this, "订单创建成功",
                               QString("订单创建成功！\n\n"
                                       "订单号: %1\n"
                                       "客户: %2\n"
                                       "手机: %3\n"
                                       "地址: %4\n"
                                       "金额: ¥%5\n"
                                       "支付方式: %6\n"
                                       "状态: 待付款")
                                   .arg(orderNo)
                                   .arg(customerName)
                                   .arg(customerPhone)
                                   .arg(customerAddress)
                                   .arg(cartTotal, 0, 'f', 2)
                                   .arg(paymentMethod));
                       }

                       void OrderManageWidget::onCancelNewOrder()
                       {
                           if (!cartItems.isEmpty()) {
                               QMessageBox::StandardButton reply;
                               reply = QMessageBox::question(newOrderDialog, "确认取消",
                                   "购物车中有商品，确定要取消订单吗？",
                                   QMessageBox::Yes | QMessageBox::No);
                               if (reply == QMessageBox::No) {
                                   return;
                               }
                           }
                           newOrderDialog->reject();
                       }

                       void OrderManageWidget::onDeleteOrder()
                       {
                           // 获取当前选中的表格和行
                           QTableWidget *currentTable = nullptr;
                           int tabIndex = tabWidget->currentIndex();
                           switch (tabIndex) {
                               case 0: currentTable = orderTableAll; break;
                               case 1: currentTable = orderTablePending; break;
                               case 2: currentTable = orderTablePaid; break;
                               case 3: currentTable = orderTableShipped; break;
                               case 4: currentTable = orderTableCompleted; break;
                               case 5: currentTable = orderTableCancelled; break;
                               default: currentTable = orderTableAll;
                           }

                           if (!currentTable) {
                               QMessageBox::warning(this, "错误", "无法获取当前表格");
                               return;
                           }

                           int currentRow = currentTable->currentRow();
                           if (currentRow < 0 || currentRow >= currentTable->rowCount()) {
                               QMessageBox::warning(this, "警告", "请先选择要删除的订单");
                               return;
                           }

                           // 获取订单信息
                           QString orderNo = currentTable->item(currentRow, 0)->text();
                           QString customer = currentTable->item(currentRow, 1)->text();
                           QString phone = currentTable->item(currentRow, 2)->text();
                           QString amount = currentTable->item(currentRow, 3)->text();
                           QString status = currentTable->item(currentRow, 4)->text();

                           // 确认删除对话框
                           QMessageBox::StandardButton reply;
                           reply = QMessageBox::question(this, "确认删除",
                               QString("确定要删除订单吗？\n\n"
                                       "订单号: %1\n"
                                       "客户: %2\n"
                                       "手机: %3\n"
                                       "金额: %4\n"
                                       "状态: %5")
                                   .arg(orderNo)
                                   .arg(customer)
                                   .arg(phone)
                                   .arg(amount)
                                   .arg(status),
                               QMessageBox::Yes | QMessageBox::No);

                           if (reply == QMessageBox::Yes) {
                               // 从表格中删除
                               currentTable->removeRow(currentRow);

                               // 从模拟数据中删除
                               for (int i = 0; i < orderData.size(); i++) {
                                   if (orderData[i][0] == orderNo) {
                                       orderData.removeAt(i);
                                       break;
                                   }
                               }

                               // 更新统计信息
                               updateStatistics();

                               // 清空详情面板
                               onTableSelectionChanged();

                               QMessageBox::information(this, "删除成功", "订单已删除");
                           }
                       }

                       void OrderManageWidget::onExportOrders()
                       {
                           QString defaultName = QString("图书订单数据_%1.txt")
                               .arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss"));

                           QString fileName = QFileDialog::getSaveFileName(this,
                               "导出订单数据", defaultName,
                               "文本文件 (*.txt);;CSV文件 (*.csv);;所有文件 (*.*)");

                           if (!fileName.isEmpty()) {
                               // 创建文件并写入数据
                               QFile file(fileName);
                               if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                                   QTextStream stream(&file);
                                   stream.setCodec("UTF-8");

                                   // 写入表头
                                   stream << "订单号,客户姓名,手机号码,订单金额,订单状态,支付方式,创建时间,收货地址,操作员\n";

                                   // 写入数据
                                   for (int i = 0; i < orderData.size(); i++) {
                                       const QStringList &order = orderData[i];
                                       QString line;
                                       for (int j = 0; j < qMin(9, order.size()); j++) { // 现在最多9列
                                           if (j > 0) line += ",";
                                           line += order[j];
                                       }
                                       stream << line << "\n";
                                   }
                                   file.close();

                                   QMessageBox::information(this, "导出成功",
                                       QString("已导出 %1 笔订单数据到：\n%2")
                                           .arg(orderData.size()).arg(fileName));
                               } else {
                                   QMessageBox::warning(this, "导出失败", "无法创建文件");
                               }
                           }
                       }

                       void OrderManageWidget::onRefresh()
                       {
                           // 重新生成模拟数据
                           generateSampleData();

                           // 重新加载当前标签页
                           onTabChanged(tabWidget->currentIndex());

                           // 更新统计信息
                           updateStatistics();

                           QMessageBox::information(this, "刷新完成", "订单数据已刷新！");
                       }

                       void OrderManageWidget::onSearch()
                       {
                           // 重新加载当前标签页，应用筛选条件
                           onTabChanged(tabWidget->currentIndex());
                           // 显示搜索结果统计
                               QTableWidget *currentTable = nullptr;
                               int tabIndex = tabWidget->currentIndex();
                               switch (tabIndex) {
                                   case 0: currentTable = orderTableAll; break;
                                   case 1: currentTable = orderTablePending; break;
                                   case 2: currentTable = orderTablePaid; break;
                                   case 3: currentTable = orderTableShipped; break;
                                   case 4: currentTable = orderTableCompleted; break;
                                   case 5: currentTable = orderTableCancelled; break;
                               }

                               if (currentTable) {
                                   int visibleCount = currentTable->rowCount();
                                   QString keyword = searchEdit->text().trimmed();
                                   if (!keyword.isEmpty() || !paymentAll->isChecked() ||
                                       minAmountEdit->value() > 0 || maxAmountEdit->value() < 9999) {
                                       QMessageBox::information(this, "搜索结果",
                                           QString("找到 %1 条匹配的订单").arg(visibleCount));
                                   }
                               }
                           }

                           void OrderManageWidget::onShowOrderDetails()
                           {
                               QPushButton *senderBtn = qobject_cast<QPushButton*>(sender());
                               int dataIndex = -1;

                               if (senderBtn) {
                                   dataIndex = senderBtn->property("row").toInt();
                               } else {
                                   // 从当前选中行获取
                                   QTableWidget *currentTable = nullptr;
                                   int tabIndex = tabWidget->currentIndex();
                                   switch (tabIndex) {
                                       case 0: currentTable = orderTableAll; break;
                                       case 1: currentTable = orderTablePending; break;
                                       case 2: currentTable = orderTablePaid; break;
                                       case 3: currentTable = orderTableShipped; break;
                                       case 4: currentTable = orderTableCompleted; break;
                                       case 5: currentTable = orderTableCancelled; break;
                                   }

                                   if (currentTable && currentTable->currentRow() >= 0) {
                                       // 在实际项目中，这里应该通过订单号查找数据
                                       // 这里简化处理，使用当前行索引
                                       dataIndex = currentTable->currentRow();
                                   }
                               }

                               if (dataIndex < 0 || dataIndex >= orderData.size()) {
                                   // 如果没有有效数据，清空详情面板
                                   onTableSelectionChanged();
                                   return;
                               }

                               currentOrderRow = dataIndex;
                               const QStringList &order = orderData[dataIndex];

                               // 更新订单基本信息
                               orderNoLabel->setText(order[0]);
                               customerLabel->setText(order[1]);
                               phoneLabel->setText(order[2]);      // 手机号码
                               addressLabel->setText(order[7]);    // 收货地址（注意索引变化）
                               amountLabel->setText(order[3]);

                               // 状态标签（带颜色）
                               QString status = order[4];
                               QString statusColor = getStatusColor(status);
                               QString statusText = QString("<span style='color: %1; font-weight: bold;'>%2</span>")
                                   .arg(statusColor).arg(status);
                               statusLabel->setText(statusText);

                               paymentLabel->setText(order[5]);
                               timeLabel->setText(order[6]);
                               operatorLabel->setText(order[8]);   // 操作员（注意索引变化）

                               // 备注（如果有）
                               if (order.size() > 9) {
                                   remarkLabel->setText(order[9]);
                               } else {
                                   remarkLabel->setText("无备注");
                               }

                               // 更新商品清单（模拟数据）
                               orderItemsTable->setRowCount(0);

                               // 随机生成一些商品
                               int itemCount = 1 + qrand() % 4; // 1-4个商品
                               double subtotal = 0.0;

                               for (int i = 0; i < itemCount; i++) {
                                   int row = orderItemsTable->rowCount();
                                   orderItemsTable->insertRow(row);

                                   // 商品名
                                   QStringList products = QStringList()
                                       << "红楼梦" << "三国演义" << "西游记" << "时间简史" << "人类简史"
                                       << "追风筝的人" << "百年孤独" << "活着" << "围城";
                                   QString productName = products.at(i % products.size());
                                   QTableWidgetItem *nameItem = new QTableWidgetItem(productName);
                                   orderItemsTable->setItem(row, 0, nameItem);

                                   // 单价
                                   double price = 30.0 + (qrand() % 70);
                                   QTableWidgetItem *priceItem = new QTableWidgetItem(QString("¥%1").arg(price, 0, 'f', 2));
                                   priceItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
                                   orderItemsTable->setItem(row, 1, priceItem);

                                   // 数量
                                   int quantity = 1 + (qrand() % 3);
                                   QTableWidgetItem *qtyItem = new QTableWidgetItem(QString::number(quantity));
                                   qtyItem->setTextAlignment(Qt::AlignCenter);
                                   orderItemsTable->setItem(row, 2, qtyItem);

                                   // 小计
                                   double itemTotal = price * quantity;
                                   QTableWidgetItem *totalItem = new QTableWidgetItem(QString("¥%1").arg(itemTotal, 0, 'f', 2));
                                   totalItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
                                   orderItemsTable->setItem(row, 3, totalItem);

                                   subtotal += itemTotal;
                               }
                               // 计算总计
                                   double discount = subtotal * 0.1; // 10%折扣
                                   double total = subtotal - discount;

                                   subtotalLabel->setText(QString("商品总额: ¥%1").arg(subtotal, 0, 'f', 2));
                                   discountLabel->setText(QString("优惠金额: -¥%1").arg(discount, 0, 'f', 2));
                                   totalLabel->setText(QString("实付金额: ¥%1").arg(total, 0, 'f', 2));
                               }

                               void OrderManageWidget::onTableSelectionChanged()
                               {
                                   // 获取当前选中的订单（如果有的话）
                                   QTableWidget *currentTable = nullptr;
                                   int tabIndex = tabWidget->currentIndex();
                                   switch (tabIndex) {
                                       case 0: currentTable = orderTableAll; break;
                                       case 1: currentTable = orderTablePending; break;
                                       case 2: currentTable = orderTablePaid; break;
                                       case 3: currentTable = orderTableShipped; break;
                                       case 4: currentTable = orderTableCompleted; break;
                                       case 5: currentTable = orderTableCancelled; break;
                                   }

                                   if (currentTable && currentTable->currentRow() >= 0) {
                                       // 有选中行，启用删除按钮
                                       deleteButton->setEnabled(true);
                                   } else {
                                       // 无选中行，禁用删除按钮并清空详情面板
                                       deleteButton->setEnabled(false);
                                       orderNoLabel->setText("-");
                                       customerLabel->setText("-");
                                       phoneLabel->setText("-");       // 手机号码
                                       addressLabel->setText("-");     // 收货地址
                                       statusLabel->setText("-");
                                       amountLabel->setText("-");
                                       paymentLabel->setText("-");
                                       timeLabel->setText("-");
                                       operatorLabel->setText("-");
                                       remarkLabel->setText("-");
                                       orderItemsTable->setRowCount(0);
                                       subtotalLabel->setText("商品总额: ¥0.00");
                                       discountLabel->setText("优惠金额: ¥0.00");
                                       totalLabel->setText("实付金额: ¥0.00");
                                   }
                               }

                               void OrderManageWidget::onFilterChanged()
                               {
                                   // 重新加载数据应用筛选条件
                                   onTabChanged(tabWidget->currentIndex());
                               }

                               void OrderManageWidget::onAmountRangeChanged()
                               {
                                   // 确保最小值不大于最大值
                                   if (minAmountEdit->value() > maxAmountEdit->value()) {
                                       minAmountEdit->setValue(maxAmountEdit->value());
                                   }
                                   // 应用筛选
                                   onFilterChanged();
                               }

                               void OrderManageWidget::onExportCurrentTab()
                               {
                                   int tabIndex = tabWidget->currentIndex();
                                   QString tabName = tabWidget->tabText(tabIndex);

                                   QString defaultName = QString("%1_%2.txt")
                                       .arg(tabName)
                                       .arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss"));

                                   QString fileName = QFileDialog::getSaveFileName(this,
                                       QString("导出%1数据").arg(tabName), defaultName,
                                       "文本文件 (*.txt);;CSV文件 (*.csv)");

                                   if (!fileName.isEmpty()) {
                                       // 获取当前标签页的数据
                                       QString status;
                                       switch (tabIndex) {
                                           case 0: status = "全部"; break;
                                           case 1: status = "待付款"; break;
                                           case 2: status = "已付款"; break;
                                           case 3: status = "已发货"; break;
                                           case 4: status = "已完成"; break;
                                           case 5: status = "已取消"; break;
                                           default: status = "全部";
                                       }

                                       // 创建文件并写入数据
                                       QFile file(fileName);
                                       if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                                           QTextStream stream(&file);
                                           stream.setCodec("UTF-8");

                                           // 写入表头
                                           stream << "订单号,客户姓名,手机号码,订单金额,订单状态,支付方式,创建时间,收货地址,操作员\n";

                                           // 写入数据
                                           int count = 0;
                                           for (int i = 0; i < orderData.size(); i++) {
                                               const QStringList &order = orderData[i];

                                               // 过滤状态
                                               if (status != "全部" && order[4] != status) {
                                                   continue;
                                               }

                                               QString line;
                                               for (int j = 0; j < qMin(9, order.size()); j++) {
                                                   if (j > 0) line += ",";
                                                   line += order[j];
                                               }
                                               stream << line << "\n";
                                               count++;
                                           }
                                           file.close();

                                           QMessageBox::information(this, "导出成功",
                                               QString("已导出【%1】共%2笔订单数据到：\n%3")
                                                   .arg(tabName).arg(count).arg(fileName));
                                       } else {
                                           QMessageBox::warning(this, "导出失败", "无法创建文件");
                                       }
                                   }
                               }

                               void OrderManageWidget::onClearFilters()
                               {
                                   // 重置所有筛选条件
                                   timeAll->setChecked(true);
                                   minAmountEdit->setValue(0);
                                   maxAmountEdit->setValue(9999);
                                   searchEdit->clear();
                                   paymentAll->setChecked(true);
                                   paymentCash->setChecked(false);
                                   paymentWechat->setChecked(false);
                                   paymentAlipay->setChecked(false);
                                   paymentCard->setChecked(false);

                                   // 重新加载数据
                                   onTabChanged(tabWidget->currentIndex());

                                   QMessageBox::information(this, "清除筛选", "所有筛选条件已重置");
                               }

                               void OrderManageWidget::saveOrdersToFile(const QString &filename, const QString &format)
                               {
                                   Q_UNUSED(format);
                                   Q_UNUSED(filename);
                                   // 在实际项目中，这里应该实现不同格式的导出
                               }

                               // 添加缺失的槽函数实现
                               void OrderManageWidget::onSelectPaymentMethod()
                               {
                                   // 处理支付方式选择
                               }
                               void OrderManageWidget::onclearCart()
                               {
                                   if (cartItems.isEmpty()) {
                                       QMessageBox::information(newOrderDialog, "提示", "购物车已经是空的");
                                       return;
                                   }

                                   QMessageBox::StandardButton reply;
                                   reply = QMessageBox::question(newOrderDialog, "确认清空",
                                       "确定要清空购物车中的所有商品吗？",
                                       QMessageBox::Yes | QMessageBox::No);

                                   if (reply == QMessageBox::Yes) {
                                       cartItems.clear();
                                       cartTotal = 0.0;
                                       updateCartTotal();
                                       QMessageBox::information(newOrderDialog, "清空成功", "购物车已清空");
                                   }
                               }
