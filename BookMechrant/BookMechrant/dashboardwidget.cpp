#include "DashboardWidget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QTimer>
#include <QDate>
#include <QTime>
#include <QHeaderView>
#include <QMessageBox>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QGraphicsDropShadowEffect>
#include <QLinearGradient>
#include <QBrush>
#include<QTextEdit>

// 样式常量
namespace {
    const int WIDGET_MARGIN = 20;
    const int WIDGET_SPACING = 15;
    const int CARD_SPACING = 10;
    const int CHART_HEIGHT = 220;
    const int ORDERS_HEIGHT = 220;
    const int TABLE_HEIGHT = 200;
}

DashboardWidget::DashboardWidget(QWidget *parent)
    : QWidget(parent), todayTarget(2000.0)
{
    // 设置窗口背景
    setStyleSheet(QString("background-color: %1;").arg(BG_COLOR.name()));

    // 主布局
    mainLayout = new QGridLayout(this);
    mainLayout->setContentsMargins(WIDGET_MARGIN, WIDGET_MARGIN, WIDGET_MARGIN, WIDGET_MARGIN);
    mainLayout->setSpacing(WIDGET_SPACING);

    // 设置列比例
    mainLayout->setColumnStretch(0, 1);
    mainLayout->setColumnStretch(1, 1);
    mainLayout->setColumnStretch(2, 1);
    mainLayout->setColumnStretch(3, 1);

    // 创建各个组件
    createStatsCards();
    createChartArea();
    createRecentOrders();
    createInventoryAlerts();
    createActionButtons();

    // 初始加载数据
    refreshData();

    // 定时刷新数据
    refreshTimer = new QTimer(this);
    connect(refreshTimer, SIGNAL(timeout()), this, SLOT(refreshData()));
    refreshTimer->start(60000); // 60秒刷新一次
}

void DashboardWidget::createStatsCards()
{
    // ========== 今日销售卡片 ==========
    statsCardToday = new QFrame;
    statsCardToday->setObjectName("statsCardToday");
    statsCardToday->setMinimumHeight(120);
    statsCardToday->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    // 添加阴影效果
    QGraphicsDropShadowEffect *shadowToday = new QGraphicsDropShadowEffect;
    shadowToday->setBlurRadius(15);
    shadowToday->setColor(QColor(0, 0, 0, 25));
    shadowToday->setOffset(0, 2);
    statsCardToday->setGraphicsEffect(shadowToday);

    QVBoxLayout *todayLayout = new QVBoxLayout(statsCardToday);
    todayLayout->setContentsMargins(20, 15, 20, 15);
    todayLayout->setSpacing(8);

    // 图标和标题
    QHBoxLayout *titleLayout = new QHBoxLayout;
    QLabel *todayIcon = new QLabel("💰");
    todayIcon->setStyleSheet("font-size: 20px;");
    QLabel *todayTitle = new QLabel("今日销售");
    todayTitle->setStyleSheet("color: #555; font-size: 14px; font-weight: 600;");
    titleLayout->addWidget(todayIcon);
    titleLayout->addWidget(todayTitle);
    titleLayout->addStretch();

    // 销售额
    todaySalesLabel = new QLabel("¥ 0.00");
    todaySalesLabel->setStyleSheet(QString(
        "color: %1; font-size: 24px; font-weight: bold; margin: 5px 0;"
    ).arg(PRIMARY_COLOR.name()));
    todaySalesLabel->setAlignment(Qt::AlignLeft);

    // 进度条
    todayProgressBar = new QProgressBar;
    todayProgressBar->setRange(0, 100);
    todayProgressBar->setValue(0);
    todayProgressBar->setTextVisible(true);
    todayProgressBar->setFormat("%p% 完成");
    todayProgressBar->setStyleSheet(QString(
        "QProgressBar {"
        "  border: 1px solid #e0e0e0;"
        "  border-radius: 4px;"
        "  text-align: center;"
        "  height: 10px;"
        "  background-color: #f5f5f5;"
        "}"
        "QProgressBar::chunk {"
        "  background-color: %1;"
        "  border-radius: 4px;"
        "}"
    ).arg(PRIMARY_COLOR.name()));

    // 订单数
    todayOrdersLabel = new QLabel("订单: 0 笔");
    todayOrdersLabel->setStyleSheet("color: #777; font-size: 13px;");
    todayOrdersLabel->setAlignment(Qt::AlignLeft);

    todayLayout->addLayout(titleLayout);
    todayLayout->addWidget(todaySalesLabel);
    todayLayout->addWidget(todayProgressBar);
    todayLayout->addWidget(todayOrdersLabel);

    // ========== 本周销售卡片 ==========
    statsCardWeek = new QFrame;
    statsCardWeek->setObjectName("statsCardWeek");
    statsCardWeek->setMinimumHeight(120);
    statsCardWeek->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    QGraphicsDropShadowEffect *shadowWeek = new QGraphicsDropShadowEffect;
    shadowWeek->setBlurRadius(15);
    shadowWeek->setColor(QColor(0, 0, 0, 25));
    shadowWeek->setOffset(0, 2);
    statsCardWeek->setGraphicsEffect(shadowWeek);

    QVBoxLayout *weekLayout = new QVBoxLayout(statsCardWeek);
    weekLayout->setContentsMargins(20, 15, 20, 15);
    weekLayout->setSpacing(8);

    QLabel *weekIcon = new QLabel("📅");
    weekIcon->setStyleSheet("font-size: 20px;");
    weekIcon->setAlignment(Qt::AlignLeft);

    QLabel *weekTitle = new QLabel("本周销售");
    weekTitle->setStyleSheet("color: #555; font-size: 14px; font-weight: 600;");
    weekTitle->setAlignment(Qt::AlignLeft);

    weekSalesLabel = new QLabel("¥ 0.00");
    weekSalesLabel->setStyleSheet(QString(
        "color: %1; font-size: 24px; font-weight: bold; margin: 5px 0;"
    ).arg(SUCCESS_COLOR.name()));
    weekSalesLabel->setAlignment(Qt::AlignLeft);

    weekTrendLabel = new QLabel("环比: +0%");
    weekTrendLabel->setStyleSheet("color: #27ae60; font-size: 13px; font-weight: 500;");
    weekTrendLabel->setAlignment(Qt::AlignLeft);

    weekLayout->addWidget(weekIcon);
    weekLayout->addWidget(weekTitle);
    weekLayout->addWidget(weekSalesLabel);
    weekLayout->addWidget(weekTrendLabel);

    // ========== 本月销售卡片 ==========
    statsCardMonth = new QFrame;
    statsCardMonth->setObjectName("statsCardMonth");
    statsCardMonth->setMinimumHeight(120);
    statsCardMonth->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    QGraphicsDropShadowEffect *shadowMonth = new QGraphicsDropShadowEffect;
    shadowMonth->setBlurRadius(15);
    shadowMonth->setColor(QColor(0, 0, 0, 25));
    shadowMonth->setOffset(0, 2);
    statsCardMonth->setGraphicsEffect(shadowMonth);

    QVBoxLayout *monthLayout = new QVBoxLayout(statsCardMonth);
    monthLayout->setContentsMargins(20, 15, 20, 15);
    monthLayout->setSpacing(8);

    QLabel *monthIcon = new QLabel("📊");
    monthIcon->setStyleSheet("font-size: 20px;");
    monthIcon->setAlignment(Qt::AlignLeft);

    QLabel *monthTitle = new QLabel("本月销售");
    monthTitle->setStyleSheet("color: #555; font-size: 14px; font-weight: 600;");
    monthTitle->setAlignment(Qt::AlignLeft);

    monthSalesLabel = new QLabel("¥ 0.00");
    monthSalesLabel->setStyleSheet(QString(
        "color: %1; font-size: 24px; font-weight: bold; margin: 5px 0;"
    ).arg(WARNING_COLOR.darker(120).name()));
    monthSalesLabel->setAlignment(Qt::AlignLeft);

    monthTargetLabel = new QLabel("目标: ¥10,000");
    monthTargetLabel->setStyleSheet("color: #777; font-size: 13px;");
    monthTargetLabel->setAlignment(Qt::AlignLeft);

    monthLayout->addWidget(monthIcon);
    monthLayout->addWidget(monthTitle);
    monthLayout->addWidget(monthSalesLabel);
    monthLayout->addWidget(monthTargetLabel);

        // ========== 库存预警卡片 ==========
        statsCardInventory = new QFrame;
        statsCardInventory->setObjectName("statsCardInventory");
        statsCardInventory->setMinimumHeight(120);
        statsCardInventory->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

        QGraphicsDropShadowEffect *shadowInv = new QGraphicsDropShadowEffect;
        shadowInv->setBlurRadius(15);
        shadowInv->setColor(QColor(0, 0, 0, 25));
        shadowInv->setOffset(0, 2);
        statsCardInventory->setGraphicsEffect(shadowInv);

        QVBoxLayout *invLayout = new QVBoxLayout(statsCardInventory);
        invLayout->setContentsMargins(20, 15, 20, 15);
        invLayout->setSpacing(8);

        QLabel *invIcon = new QLabel("⚠️");
        invIcon->setStyleSheet("font-size: 20px;");
        invIcon->setAlignment(Qt::AlignLeft);

        QLabel *invTitle = new QLabel("库存预警");
        invTitle->setStyleSheet("color: #555; font-size: 14px; font-weight: 600;");
        invTitle->setAlignment(Qt::AlignLeft);

        inventoryWarningLabel = new QLabel("0 种商品");
        inventoryWarningLabel->setStyleSheet(QString(
            "color: %1; font-size: 24px; font-weight: bold; margin: 5px 0;"
        ).arg(DANGER_COLOR.name()));
        inventoryWarningLabel->setAlignment(Qt::AlignLeft);

        QLabel *invSub = new QLabel("需要立即补货");
        invSub->setStyleSheet("color: #777; font-size: 13px;");
        invSub->setAlignment(Qt::AlignLeft);

        invLayout->addWidget(invIcon);
        invLayout->addWidget(invTitle);
        invLayout->addWidget(inventoryWarningLabel);
        invLayout->addWidget(invSub);

        // ========== 添加到主布局 ==========
        mainLayout->addWidget(statsCardToday, 0, 0);
        mainLayout->addWidget(statsCardWeek, 0, 1);
        mainLayout->addWidget(statsCardMonth, 0, 2);
        mainLayout->addWidget(statsCardInventory, 0, 3);

        // ========== 设置卡片样式 ==========
        QString cardStyle = QString(
            "#statsCardToday, #statsCardWeek, #statsCardMonth, #statsCardInventory {"
            "  background-color: white;"
            "  border-radius: 12px;"
            "  border: 1px solid #e0e0e0;"
            "}"
            "#statsCardToday:hover, #statsCardWeek:hover, #statsCardMonth:hover, #statsCardInventory:hover {"
            "  border-color: %1;"
            "}"
        ).arg(PRIMARY_COLOR.name());

        statsCardToday->setStyleSheet(cardStyle);
        statsCardWeek->setStyleSheet(cardStyle);
        statsCardMonth->setStyleSheet(cardStyle);
        statsCardInventory->setStyleSheet(cardStyle);
    }

    void DashboardWidget::createChartArea()
    {
        // 创建图表容器
        chartWidget = new QWidget;
        chartWidget->setObjectName("chartWidget");
        chartWidget->setMinimumHeight(CHART_HEIGHT);

        QGraphicsDropShadowEffect *shadowChart = new QGraphicsDropShadowEffect;
        shadowChart->setBlurRadius(15);
        shadowChart->setColor(QColor(0, 0, 0, 25));
        shadowChart->setOffset(0, 2);
        chartWidget->setGraphicsEffect(shadowChart);

        QVBoxLayout *chartContainerLayout = new QVBoxLayout(chartWidget);
        chartContainerLayout->setContentsMargins(0, 0, 0, 0);

        // 图表标题
        QWidget *titleWidget = new QWidget;
        QHBoxLayout *titleLayout = new QHBoxLayout(titleWidget);
        titleLayout->setContentsMargins(20, 15, 20, 10);

        QLabel *chartTitle = new QLabel("📈 七日销售趋势");
        chartTitle->setStyleSheet("font-size: 16px; font-weight: bold; color: #333;");

        titleLayout->addWidget(chartTitle);
        titleLayout->addStretch();

        // 创建图表
        salesChart = new QChart;
        salesChart->setBackgroundBrush(QBrush(Qt::white));
        salesChart->setMargins(QMargins(10, 10, 10, 10));
        salesChart->setAnimationOptions(QChart::SeriesAnimations);

        // 创建折线序列
        salesSeries = new QLineSeries;
        salesSeries->setName("日销售额");

        // 设置线条样式
        QPen pen(CHART_COLOR);
        pen.setWidth(3);
        pen.setCapStyle(Qt::RoundCap);
        pen.setJoinStyle(Qt::RoundJoin);
        salesSeries->setPen(pen);

        // 设置数据点样式
        salesSeries->setPointsVisible(true);
        salesSeries->setPointLabelsVisible(false);

        // 添加序列到图表
        salesChart->addSeries(salesSeries);
        // 创建X轴（时间轴）
           axisX = new QDateTimeAxis;
           axisX->setFormat("MM-dd");
           axisX->setTitleText("日期");
           axisX->setLabelsAngle(-45);
           axisX->setGridLineColor(QColor(240, 240, 240));
           axisX->setLinePenColor(QColor(200, 200, 200));
           axisX->setLabelsColor(QColor(100, 100, 100));

           // 创建Y轴（数值轴）
           axisY = new QValueAxis;
           axisY->setTitleText("销售额 (元)");
           axisY->setLabelFormat("%.0f");
           axisY->setGridLineColor(QColor(240, 240, 240));
           axisY->setLinePenColor(QColor(200, 200, 200));
           axisY->setLabelsColor(QColor(100, 100, 100));

           // 将坐标轴附加到图表
           salesChart->addAxis(axisX, Qt::AlignBottom);
           salesChart->addAxis(axisY, Qt::AlignLeft);
           salesSeries->attachAxis(axisX);
           salesSeries->attachAxis(axisY);

           // 创建图表视图
           chartView = new QChartView(salesChart);
           chartView->setRenderHint(QPainter::Antialiasing);
           chartView->setStyleSheet("border: none; background: transparent;");

           chartContainerLayout->addWidget(titleWidget);
           chartContainerLayout->addWidget(chartView);

           // 设置容器样式
           chartWidget->setStyleSheet(
               "#chartWidget {"
               "  background-color: white;"
               "  border-radius: 12px;"
               "  border: 1px solid #e0e0e0;"
               "}"
           );

           // 添加到主布局（占据3列）
           mainLayout->addWidget(chartWidget, 1, 0, 1, 3);
       }

       void DashboardWidget::createRecentOrders()
       {
           // 创建最近订单容器
           recentOrdersWidget = new QWidget;
           recentOrdersWidget->setObjectName("recentOrdersWidget");
           recentOrdersWidget->setMinimumHeight(ORDERS_HEIGHT);

           QGraphicsDropShadowEffect *shadowOrders = new QGraphicsDropShadowEffect;
           shadowOrders->setBlurRadius(15);
           shadowOrders->setColor(QColor(0, 0, 0, 25));
           shadowOrders->setOffset(0, 2);
           recentOrdersWidget->setGraphicsEffect(shadowOrders);

           QVBoxLayout *ordersLayout = new QVBoxLayout(recentOrdersWidget);
           ordersLayout->setContentsMargins(0, 0, 0, 0);
           ordersLayout->setSpacing(0);

           // 标题
           QWidget *titleWidget = new QWidget;
           QHBoxLayout *titleLayout = new QHBoxLayout(titleWidget);
           titleLayout->setContentsMargins(20, 15, 20, 10);

           recentOrdersTitle = new QLabel("📋 最近订单");
           recentOrdersTitle->setStyleSheet("font-size: 16px; font-weight: bold; color: #333;");

           titleLayout->addWidget(recentOrdersTitle);
           titleLayout->addStretch();

           // 订单列表容器
           QWidget *ordersListWidget = new QWidget;
           ordersListWidget->setObjectName("ordersList");
           QVBoxLayout *listLayout = new QVBoxLayout(ordersListWidget);
           listLayout->setContentsMargins(15, 0, 15, 15);
           listLayout->setSpacing(8);

           ordersLayout->addWidget(titleWidget);
           ordersLayout->addWidget(ordersListWidget, 1);

           // 设置样式
           recentOrdersWidget->setStyleSheet(
               "#recentOrdersWidget {"
               "  background-color: white;"
               "  border-radius: 12px;"
               "  border: 1px solid #e0e0e0;"
               "}"
               "#ordersList {"
               "  background-color: transparent;"
               "}"
           );

           // 添加到主布局（占据1列）
           mainLayout->addWidget(recentOrdersWidget, 1, 3);
       }

       void DashboardWidget::createActionButtons()
       {
           QWidget *buttonWidget = new QWidget;
           buttonWidget->setObjectName("buttonWidget");
           buttonWidget->setFixedHeight(80);  // 确保足够高度

           QHBoxLayout *buttonLayout = new QHBoxLayout(buttonWidget);
           buttonLayout->setContentsMargins(0, 15, 0, 15);
           buttonLayout->setSpacing(30);

           // 打印报表按钮 - 明确设置所有样式属性
           printReportButton = new QPushButton("📊 打印报表");
           printReportButton->setObjectName("printReportButton");
           printReportButton->setFixedSize(180, 50);  // 更大的按钮

           // 库存盘点按钮
           inventoryCheckButton = new QPushButton("📦 库存盘点");
           inventoryCheckButton->setObjectName("inventoryCheckButton");
           inventoryCheckButton->setFixedSize(180, 50);

           buttonLayout->addStretch();
           buttonLayout->addWidget(printReportButton);
           buttonLayout->addWidget(inventoryCheckButton);
           buttonLayout->addStretch();

           // 关键修复：使用明确的颜色定义，确保字体可见
           QString buttonStyle = QString(
               "QPushButton {"
               "  background-color: %1;"
               "  color: white !important;"  // 使用!important强制白色字体
               "  border: 2px solid %2;"
               "  border-radius: 10px;"
               "  font-size: 16px;"
               "  font-weight: bold;"
               "  padding: 12px 30px;"
               "  min-height: 50px;"  // 确保最小高度
               "  min-width: 160px;"  // 确保最小宽度
               "}"
               "QPushButton:hover {"
               "  background-color: %2;"
               "  border-color: %2;"
               "}"
               "QPushButton:pressed {"
               "  background-color: %3;"
               "}"
           ).arg(PRIMARY_COLOR.name())               // 主蓝色背景
            .arg(PRIMARY_COLOR.darker(120).name())   // 悬停时更深的蓝色
            .arg(PRIMARY_COLOR.darker(150).name());  // 按下时最深的蓝色

           // 应用样式
           printReportButton->setStyleSheet(buttonStyle);
           inventoryCheckButton->setStyleSheet(buttonStyle);

           // 还可以在代码层面强制设置字体颜色
           QPalette palette = printReportButton->palette();
           palette.setColor(QPalette::ButtonText, Qt::white);  // 按钮文字设为白色
           printReportButton->setPalette(palette);
           inventoryCheckButton->setPalette(palette);

           // 设置字体
           QFont buttonFont = printReportButton->font();
           buttonFont.setPointSize(14);
           buttonFont.setBold(true);
           printReportButton->setFont(buttonFont);
           inventoryCheckButton->setFont(buttonFont);

           // 连接到信号
           connect(printReportButton, SIGNAL(clicked()), this, SLOT(onPrintReport()));
           connect(inventoryCheckButton, SIGNAL(clicked()), this, SLOT(onInventoryCheck()));

           // 添加到主布局
           mainLayout->addWidget(buttonWidget, 2, 0, 1, 4);
       }
       void DashboardWidget::createInventoryAlerts()
       {
           // 创建库存预警容器
           inventoryWidget = new QWidget;
           inventoryWidget->setObjectName("inventoryWidget");

           // 设置为可扩展
           inventoryWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

           // 垂直布局
           QVBoxLayout *inventoryLayout = new QVBoxLayout(inventoryWidget);
           inventoryLayout->setContentsMargins(0, 0, 0, 0);
           inventoryLayout->setSpacing(0);

           // ========== 标题行 ==========
           QWidget *titleWidget = new QWidget;
           titleWidget->setFixedHeight(50);
           titleWidget->setObjectName("inventoryTitleWidget");

           QHBoxLayout *titleLayout = new QHBoxLayout(titleWidget);
           titleLayout->setContentsMargins(20, 0, 20, 0);

           QLabel *tableTitle = new QLabel("⚠️ 库存预警");
           tableTitle->setStyleSheet(
               "font-size: 16px;"
               "font-weight: bold;"
               "color: #333;"
               "font-family: 'Microsoft YaHei', Arial, sans-serif;"
           );

           titleLayout->addWidget(tableTitle);
           titleLayout->addStretch();

           inventoryLayout->addWidget(titleWidget);

           // ========== 预警表格 ==========
           inventoryAlertTable = new QTableWidget;
           inventoryAlertTable->setObjectName("inventoryTable");

           // 设置为可扩展
           inventoryAlertTable->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

           // 5列：商品名称、当前库存、安全库存、状态、建议操作
           inventoryAlertTable->setColumnCount(5);

           // 设置表头
           QStringList headers;
           headers << "商品名称" << "当前库存" << "安全库存" << "状态" << "建议操作";
           inventoryAlertTable->setHorizontalHeaderLabels(headers);

           // 设置表格属性
           inventoryAlertTable->verticalHeader()->setVisible(false);
           inventoryAlertTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
           inventoryAlertTable->setSelectionBehavior(QAbstractItemView::SelectRows);
           inventoryAlertTable->setAlternatingRowColors(true);
           inventoryAlertTable->setShowGrid(false);

           // 关键：设置表头拉伸策略
           QHeaderView *horizontalHeader = inventoryAlertTable->horizontalHeader();

           // 设置初始列宽
           inventoryAlertTable->setColumnWidth(0, 180);  // 商品名称
           inventoryAlertTable->setColumnWidth(1, 90);   // 当前库存
           inventoryAlertTable->setColumnWidth(2, 90);   // 安全库存
           inventoryAlertTable->setColumnWidth(3, 90);   // 状态
           inventoryAlertTable->setColumnWidth(4, 120);  // 建议操作

           // 设置列拉伸模式（关键解决右边空白问题）
           horizontalHeader->setStretchLastSection(true);  // 最后一列拉伸

           // 设置行高
           inventoryAlertTable->verticalHeader()->setDefaultSectionSize(42);

           inventoryLayout->addWidget(inventoryAlertTable, 1);  // 表格填充剩余空间

           // ========== 设置样式 ==========
           inventoryWidget->setStyleSheet(QString(
               "#inventoryWidget {"
               "  background-color: white;"
               "  border-radius: 12px;"
               "  border: 1px solid #e0e0e0;"
               "}"
               "#inventoryTitleWidget {"
               "  border-bottom: 1px solid #f0f0f0;"
               "}"
               "#inventoryTable {"
               "  border: none;"
               "  background-color: white;"
               "  alternate-background-color: #f9f9f9;"
               "  font-size: 14px;"
               "  font-family: 'Microsoft YaHei', Arial, sans-serif;"
               "  selection-background-color: %1;"
               "}"
               "#inventoryTable::item {"
               "  padding: 12px 8px;"
               "  border-bottom: 1px solid #f0f0f0;"
               "}"
               "#inventoryTable::item:selected {"
               "  color: white;"
               "}"
               "QHeaderView::section {"
               "  background-color: #f8f9fa;"
               "  padding: 14px 8px;"
               "  border: none;"
               "  border-right: 1px solid #e0e0e0;"
               "  border-bottom: 2px solid %1;"
               "  font-weight: bold;"
               "  color: #333;"
               "  font-size: 14px;"
               "  font-family: 'Microsoft YaHei', Arial, sans-serif;"
               "}"
           ).arg(PRIMARY_COLOR.name()));

           // 添加到主布局
           mainLayout->addWidget(inventoryWidget, 3, 0, 1, 4);
       }


           void DashboardWidget::refreshData()
           {
               // 初始化随机种子
               QTime time = QTime::currentTime();
               qsrand(time.msec() + time.second() * 1000);

               // ========== 模拟今日数据 ==========
               double todaySales = 800.0 + (qrand() % 1201); // 800-2000
               int todayOrders = 8 + qrand() % 13; // 8-20
               double progress = (todaySales / todayTarget) * 100;

               todaySalesLabel->setText(QString("¥%1").arg(todaySales, 0, 'f', 2));
               todayOrdersLabel->setText(QString("订单: %1 笔").arg(todayOrders));
               todayProgressBar->setValue(qMin(100, (int)progress));

               // ========== 模拟本周数据 ==========
               double weekSales = todaySales * 7 + (qrand() % 5001) - 2500;
               double weekTrend = 5.0 + (qrand() % 200) / 10.0 - 10.0; // -5%到+15%

               weekSalesLabel->setText(QString("¥%1").arg(weekSales, 0, 'f', 2));

               if (weekTrend >= 0) {
                   weekTrendLabel->setText(QString("↑%1%").arg(weekTrend, 0, 'f', 1));
                   weekTrendLabel->setStyleSheet("color: #27ae60; font-size: 13px; font-weight: 500;");
               } else {
                   weekTrendLabel->setText(QString("↓%1%").arg(-weekTrend, 0, 'f', 1));
                   weekTrendLabel->setStyleSheet("color: #e74c3c; font-size: 13px; font-weight: 500;");
               }
               // ========== 模拟本月数据 ==========
               double monthSales = weekSales * 4.3 + (qrand() % 10001) - 5000;
               monthSalesLabel->setText(QString("¥%1").arg(monthSales, 0, 'f', 2));
               monthTargetLabel->setText(QString("目标: ¥10,000"));

               // ========== 模拟库存预警 ==========
               int warningCount = 2 + qrand() % 8; // 2-9
               inventoryWarningLabel->setText(QString("%1 种商品").arg(warningCount));

               // ========== 更新最近订单 ==========
               updateRecentOrders();

               // ========== 更新库存预警 ==========
               updateInventoryAlerts();

               // ========== 更新图表数据 ==========
               updateChartData();
           }

           void DashboardWidget::updateChartData()
           {
               // 清空现有数据
               salesSeries->clear();
               weeklySales.clear();

               // 生成最近7天的数据
               QDateTime currentDateTime = QDateTime::currentDateTime();
               double baseValue = 500.0;

               for (int i = 6; i >= 0; i--) {
                   QDateTime dateTime = currentDateTime.addDays(-i);
                   double value = baseValue + (qrand() % 1501); // 500-2000
                   weeklySales.append(value);

                   salesSeries->append(dateTime.toMSecsSinceEpoch(), value);
               }

               // 更新坐标轴范围
               QDateTime minDate = currentDateTime.addDays(-6);
               QDateTime maxDate = currentDateTime;

               axisX->setRange(minDate, maxDate);

               // 计算Y轴范围（给数据一些上下空间）
               double minY = *std::min_element(weeklySales.begin(), weeklySales.end()) * 0.9;
               double maxY = *std::max_element(weeklySales.begin(), weeklySales.end()) * 1.1;
               axisY->setRange(minY, maxY);

               // 更新图表
               salesChart->update();
           }

           void DashboardWidget::updateRecentOrders()
           {
               QWidget *ordersList = recentOrdersWidget->findChild<QWidget*>("ordersList");
               if (!ordersList) return;

               // 清空现有订单
               QLayoutItem* item;
               QVBoxLayout *layout = qobject_cast<QVBoxLayout*>(ordersList->layout());
               if (!layout) return;

               while ((item = layout->takeAt(0)) != 0) {
                   delete item->widget();
                   delete item;
               }

               // 生成3-5个最近订单
               QStringList statusList = QStringList() << "待付款" << "已付款" << "已发货" << "已完成";
               QStringList payMethods = QStringList() << "现金" << "微信" << "支付宝" << "会员卡";

               int orderCount = 3 + qrand() % 3; // 3-5个订单

               for (int i = 0; i < orderCount; i++) {
                   QWidget *orderItem = new QWidget;
                   orderItem->setFixedHeight(50);
                   orderItem->setObjectName("orderItem");

                   QHBoxLayout *itemLayout = new QHBoxLayout(orderItem);
                   itemLayout->setContentsMargins(15, 0, 15, 0);
                   itemLayout->setSpacing(10);

                   // 生成订单号
                   QString orderNo = QString("#%1%2")
                       .arg(QDate::currentDate().toString("yyyyMMdd"))
                       .arg(1000 + qrand() % 9000, 4, 10, QChar('0'));

                   // 随机金额
                   double amount = 30.0 + (qrand() % 971); // 30-1000

                   // 随机状态
                   QString status = statusList.at(qrand() % statusList.size());
                   QString payMethod = payMethods.at(qrand() % payMethods.size());

                   QLabel *orderNoLabel = new QLabel(orderNo);
                   orderNoLabel->setStyleSheet("color: #333; font-weight: bold; font-size: 13px;");

                   QLabel *amountLabel = new QLabel(QString("¥%1").arg(amount, 0, 'f', 2));
                   amountLabel->setStyleSheet("color: #e74c3c; font-weight: bold; font-size: 13px;");

                   QLabel *statusLabel = new QLabel(status);

                   // 设置状态颜色
                   QString statusColor, bgColor;
                   if (status == "待付款") {
                       statusColor = "#e67e22";
                       bgColor = "#fef5e7";
                   } else if (status == "已付款") {
                       statusColor = "#3498db";
                       bgColor = "#ebf5fb";
                   } else if (status == "已发货") {
                       statusColor = "#9b59b6";
                       bgColor = "#f4ecf7";
                   } else {
                       statusColor = "#27ae60";
                       bgColor = "#eafaf1";
                   }

                   statusLabel->setStyleSheet(QString(
                       "padding: 4px 12px; border-radius: 12px; "
                       "font-size: 11px; color: %1; "
                       "background-color: %2; font-weight: bold;"
                   ).arg(statusColor).arg(bgColor));

                   itemLayout->addWidget(orderNoLabel);
                   itemLayout->addStretch();
                   itemLayout->addWidget(amountLabel);
                   itemLayout->addSpacing(10);
                   itemLayout->addWidget(statusLabel);

                   layout->addWidget(orderItem);

                   // 添加分隔线（除了最后一个）
                   if (i < orderCount - 1) {
                       QFrame *separator = new QFrame;
                       separator->setFrameShape(QFrame::HLine);
                       separator->setFrameShadow(QFrame::Sunken);
                       separator->setStyleSheet("background-color: #f0f0f0; border: none; height: 1px; margin: 0 15px;");
                       layout->addWidget(separator);
                   }
               }

               // 更新订单计数
               recentOrdersTitle->setText(QString("📋 最近订单 (%1)").arg(orderCount));
           }

           void DashboardWidget::updateInventoryAlerts()
           {
               // 清空表格
               inventoryAlertTable->setRowCount(0);

               // 模拟库存预警数据
               QStringList bookNames = QStringList()
                   << "时间简史" << "艺术的故事" << "人类简史" << "经济学原理"
                   << "心理学与生活" << "红楼梦" << "三国演义" << "西游记"
                   << "百年孤独" << "追风筝的人" << "活着" << "围城"
                   << "挪威的森林" << "白夜行" << "解忧杂货店" << "小王子";

               // 生成10-12条记录
               int recordCount = 10 + qrand() % 3;

               for (int i = 0; i < recordCount && i < bookNames.size(); i++) {
                   int row = inventoryAlertTable->rowCount();
                   inventoryAlertTable->insertRow(row);

                   int stock = 1 + qrand() % 30;       // 当前库存 1-30
                   int safeStock = 10 + qrand() % 21;  // 安全库存 10-30
                   QString status, suggestion, statusColor;

                   // 确定状态和建议
                   if (stock < 5) {
                       status = "紧急";
                       suggestion = "⚡立即补货";
                       statusColor = "#e74c3c";
                   } else if (stock < safeStock) {
                       status = "预警";
                       suggestion = "📝采购计划";
                       statusColor = "#f39c12";
                   } else {
                       status = "正常";
                       suggestion = "✅库存充足";
                       statusColor = "#27ae60";
                   }

                   // 商品名称
                   QTableWidgetItem *nameItem = new QTableWidgetItem(bookNames[i]);
                   inventoryAlertTable->setItem(row, 0, nameItem);

                   // 当前库存
                   QTableWidgetItem *stockItem = new QTableWidgetItem(QString::number(stock));
                   stockItem->setTextAlignment(Qt::AlignCenter);
                   inventoryAlertTable->setItem(row, 1, stockItem);

                   // 安全库存
                   QTableWidgetItem *safeItem = new QTableWidgetItem(QString::number(safeStock));
                   safeItem->setTextAlignment(Qt::AlignCenter);
                   inventoryAlertTable->setItem(row, 2, safeItem);

                   // 状态
                   QTableWidgetItem *statusItem = new QTableWidgetItem(status);
                   statusItem->setTextAlignment(Qt::AlignCenter);
                   statusItem->setForeground(QBrush(QColor(statusColor)));
                   statusItem->setFont(QFont("", -1, QFont::Bold));
                   inventoryAlertTable->setItem(row, 3, statusItem);

                   // 建议操作
                   QTableWidgetItem *suggestionItem = new QTableWidgetItem(suggestion);
                   suggestionItem->setTextAlignment(Qt::AlignCenter);

                   // 根据建议类型设置颜色
                   if (suggestion.contains("立即补货")) {
                       suggestionItem->setForeground(QBrush(QColor("#e74c3c")));
                       suggestionItem->setFont(QFont("", -1, QFont::Bold));
                   } else if (suggestion.contains("采购计划")) {
                       suggestionItem->setForeground(QBrush(QColor("#f39c12")));
                   } else {
                       suggestionItem->setForeground(QBrush(QColor("#27ae60")));
                   }

                   inventoryAlertTable->setItem(row, 4, suggestionItem);
               }

               // 更新库存预警卡片的数据
               int warningCount = qrand() % 5 + 3;  // 3-7个预警
               inventoryWarningLabel->setText(QString("%1 种商品").arg(warningCount));
           }
           void DashboardWidget::onPrintReport()
           {
               // 创建打印/导出对话框
               QDialog exportDialog(this);
               exportDialog.setWindowTitle("报表导出");
               exportDialog.resize(500, 400);

               QVBoxLayout *layout = new QVBoxLayout(&exportDialog);
               layout->setContentsMargins(25, 25, 25, 25);
               layout->setSpacing(20);

               // 标题
               QLabel *titleLabel = new QLabel("销售报表导出");
               titleLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #333;");
               titleLabel->setAlignment(Qt::AlignCenter);

               // 报表内容预览
               QTextEdit *previewEdit = new QTextEdit;
               previewEdit->setReadOnly(true);
               previewEdit->setStyleSheet(
                   "border: 1px solid #ddd; border-radius: 6px; padding: 10px;"
                   "font-size: 12px;"
               );

               // 生成报表内容 - 使用Qt 4.x兼容方法
               QString reportText;

               // 使用QString的fill方法创建分隔线
               QString separator = QString().fill('=', 50);

               reportText += separator + "\n";
               reportText += "       图书商家销售报表\n";
               reportText += separator + "\n\n";

               reportText += QString("生成时间: %1\n")
                   .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));

               reportText += QString("今日销售: %1\n").arg(todaySalesLabel->text());
               reportText += QString("今日订单: %1\n").arg(todayOrdersLabel->text());
               reportText += QString("本周销售: %1 (%2)\n")
                   .arg(weekSalesLabel->text())
                   .arg(weekTrendLabel->text());
               reportText += QString("本月销售: %1\n\n").arg(monthSalesLabel->text());

               // 七日销售趋势数据
               reportText += "七日销售趋势:\n";
               if (!weeklySales.isEmpty()) {
                   for (int i = 0; i < weeklySales.size(); i++) {
                       QDateTime date = QDateTime::currentDateTime().addDays(i - 6);
                       reportText += QString("  %1: ¥%2\n")
                           .arg(date.toString("MM-dd"))
                           .arg(weeklySales[i], 0, 'f', 2);
                   }
               } else {
                   reportText += "  暂无数据\n";
               }

               reportText += "\n" + separator + "\n";
               reportText += "      --- 报告结束 ---\n";
               reportText += separator + "\n";

               previewEdit->setText(reportText);

               // 按钮区域
               QHBoxLayout *btnLayout = new QHBoxLayout;
               QPushButton *saveBtn = new QPushButton("保存为文本文件");
               QPushButton *cancelBtn = new QPushButton("取消");

               // 按钮样式
               saveBtn->setStyleSheet(
                   "QPushButton {"
                   "  background-color: #3498db;"
                   "  color: white;"
                   "  border: none;"
                   "  border-radius: 6px;"
                   "  padding: 10px 20px;"
                   "  font-size: 14px;"
                   "}"
                   "QPushButton:hover {"
                   "  background-color: #2980b9;"
                   "}"
               );

               cancelBtn->setStyleSheet(
                   "QPushButton {"
                   "  background-color: #95a5a6;"
                   "  color: white;"
                   "  border: none;"
                   "  border-radius: 6px;"
                   "  padding: 10px 20px;"
                   "  font-size: 14px;"
                   "}"
                   "QPushButton:hover {"
                   "  background-color: #7f8c8d;"
                   "}"
               );

               btnLayout->addStretch();
               btnLayout->addWidget(saveBtn);
               btnLayout->addWidget(cancelBtn);

               layout->addWidget(titleLabel);
               layout->addWidget(new QLabel("报表预览:"));
               layout->addWidget(previewEdit, 1);
               layout->addLayout(btnLayout);

               // 连接按钮
               connect(saveBtn, SIGNAL(clicked()), &exportDialog, SLOT(accept()));
               connect(cancelBtn, SIGNAL(clicked()), &exportDialog, SLOT(reject()));

               if (exportDialog.exec() == QDialog::Accepted) {
                   // 保存为文本文件
                   QString defaultName = QString("销售报表_%1.txt")
                       .arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss"));

                   QString fileName = QFileDialog::getSaveFileName(
                       this, "保存报表", defaultName, "文本文件 (*.txt)"
                   );

                   if (!fileName.isEmpty()) {
                       QFile file(fileName);
                       if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                           QTextStream stream(&file);
                           // Qt 4.x使用setCodec
                           stream.setCodec("UTF-8");
                           stream << reportText;
                           file.close();

                           QMessageBox::information(this, "保存成功",
                               QString("报表已保存到:\n%1").arg(fileName));
                       } else {
                           QMessageBox::warning(this, "保存失败", "无法保存文件");
                       }
                   }
               }
           }


              void DashboardWidget::onInventoryCheck()
              {
                  // 创建库存盘点对话框
                  QDialog inventoryDialog(this);
                  inventoryDialog.setWindowTitle("📦 库存盘点");
                  inventoryDialog.resize(500, 400);
                  inventoryDialog.setStyleSheet("background-color: white;");

                  QVBoxLayout *layout = new QVBoxLayout(&inventoryDialog);
                  layout->setContentsMargins(25, 25, 25, 25);
                  layout->setSpacing(20);

                  // 标题
                  QLabel *titleLabel = new QLabel("📋 库存盘点报告");
                  titleLabel->setStyleSheet("font-size: 20px; font-weight: bold; color: #333;");
                  titleLabel->setAlignment(Qt::AlignCenter);

                  // 统计信息
                  QWidget *statsWidget = new QWidget;
                  QHBoxLayout *statsLayout = new QHBoxLayout(statsWidget);
                  statsLayout->setContentsMargins(0, 0, 0, 0);
                  statsLayout->setSpacing(15);

                  // 总商品数
                  QFrame *totalFrame = new QFrame;
                  totalFrame->setStyleSheet(
                      "background-color: #f8f9fa; border-radius: 8px; border: 1px solid #e9ecef;"
                  );
                  QVBoxLayout *totalLayout = new QVBoxLayout(totalFrame);
                  totalLayout->setContentsMargins(20, 15, 20, 15);
                  QLabel *totalLabel = new QLabel("总商品数");
                  totalLabel->setStyleSheet("color: #6c757d; font-size: 13px;");
                  totalLabel->setAlignment(Qt::AlignCenter);
                  QLabel *totalValue = new QLabel("156");
                  totalValue->setStyleSheet("color: #333; font-size: 24px; font-weight: bold;");
                  totalValue->setAlignment(Qt::AlignCenter);
                  totalLayout->addWidget(totalLabel);
                  totalLayout->addWidget(totalValue);

                  // 预警商品
                  QFrame *warningFrame = new QFrame;
                  warningFrame->setStyleSheet(
                      "background-color: #fff8e1; border-radius: 8px; border: 2px solid #ffd54f;"
                  );
                  QVBoxLayout *warningLayout = new QVBoxLayout(warningFrame);
                  warningLayout->setContentsMargins(20, 15, 20, 15);
                  QLabel *warningLabel = new QLabel("预警商品");
                  warningLabel->setStyleSheet("color: #f39c12; font-size: 13px; font-weight: bold;");
                  warningLabel->setAlignment(Qt::AlignCenter);
                  QLabel *warningValue = new QLabel(inventoryWarningLabel->text());
                  warningValue->setStyleSheet("color: #f39c12; font-size: 24px; font-weight: bold;");
                  warningValue->setAlignment(Qt::AlignCenter);
                  warningLayout->addWidget(warningLabel);
                  warningLayout->addWidget(warningValue);

                  statsLayout->addWidget(totalFrame, 1);
                  statsLayout->addWidget(warningFrame, 1);
                  // 预警列表
                      QTextEdit *alertList = new QTextEdit;
                      alertList->setReadOnly(true);
                      alertList->setMinimumHeight(150);
                      alertList->setStyleSheet(
                          "border: 1px solid #ddd; border-radius: 6px; padding: 10px;"
                          "font-size: 13px;"
                      );

                      // 生成预警列表
                      QString alertText;
                      for (int i = 0; i < inventoryAlertTable->rowCount(); i++) {
                          QString name = inventoryAlertTable->item(i, 0)->text();
                          QString stock = inventoryAlertTable->item(i, 1)->text();
                          QString safe = inventoryAlertTable->item(i, 2)->text();
                          QString status = inventoryAlertTable->item(i, 3)->text();

                          QString prefix = "⚠️ ";
                          if (status == "紧急") prefix = "🚨 ";

                          alertText += QString("%1%2 库存: %3/%4 状态: %5\n")
                              .arg(prefix).arg(name).arg(stock).arg(safe).arg(status);
                      }

                      alertList->setText(alertText);

                      // 按钮
                      QHBoxLayout *btnLayout = new QHBoxLayout;
                      QPushButton *exportBtn = new QPushButton("📥 导出盘点单");
                      QPushButton *closeBtn = new QPushButton("关闭");

                      exportBtn->setStyleSheet(QString(
                          "QPushButton {"
                          "  background: linear-gradient(135deg, %1, %2);"
                          "  color: white;"
                          "  border: none;"
                          "  border-radius: 6px;"
                          "  padding: 10px 25px;"
                          "  font-size: 14px;"
                          "}"
                          "QPushButton:hover {"
                          "  background: linear-gradient(135deg, %2, %3);"
                          "}"
                      ).arg(PRIMARY_COLOR.lighter(120).name())
                       .arg(PRIMARY_COLOR.name())
                       .arg(PRIMARY_COLOR.darker(120).name()));

                      closeBtn->setStyleSheet(
                          "QPushButton {"
                          "  background-color: #95a5a6;"
                          "  color: white;"
                          "  border: none;"
                          "  border-radius: 6px;"
                          "  padding: 10px 25px;"
                          "  font-size: 14px;"
                          "}"
                          "QPushButton:hover {"
                          "  background-color: #7f8c8d;"
                          "}"
                      );

                      btnLayout->addStretch();
                      btnLayout->addWidget(exportBtn);
                      btnLayout->addWidget(closeBtn);

                      layout->addWidget(titleLabel);
                      layout->addWidget(statsWidget);
                      layout->addWidget(new QLabel("库存预警列表:"));
                      layout->addWidget(alertList, 1);
                      layout->addLayout(btnLayout);

                      // 连接按钮
                      connect(exportBtn, SIGNAL(clicked()), &inventoryDialog, SLOT(accept()));
                      connect(closeBtn, SIGNAL(clicked()), &inventoryDialog, SLOT(reject()));

                      if (inventoryDialog.exec() == QDialog::Accepted) {
                          // 保存盘点单
                          QString defaultName = QString("库存盘点单_%1.txt")
                              .arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss"));

                          QString fileName = QFileDialog::getSaveFileName(
                              this, "保存盘点单", defaultName, "文本文件 (*.txt)"
                          );

                          if (!fileName.isEmpty()) {
                              QFile file(fileName);
                              if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                                  QTextStream stream(&file);
                                  stream.setCodec("UTF-8");
                                  stream << alertText;
                                  file.close();

                                  QMessageBox::information(this, "保存成功",
                                      QString("盘点单已保存到:\n%1").arg(fileName));
                              } else {
                                  QMessageBox::warning(this, "保存失败", "无法保存文件");
                              }
                          }
                      }
                  }
