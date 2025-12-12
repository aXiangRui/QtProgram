#ifndef DASHBOARDWIDGET_H
#define DASHBOARDWIDGET_H

#include <QWidget>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QTimer>
#include <QProgressBar>
#include <QTableWidget>

// Qt Charts 头文件
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QtCharts/QDateTimeAxis>

QT_CHARTS_USE_NAMESPACE

class DashboardWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DashboardWidget(QWidget *parent = 0);

signals:
    void navigateToBooks();
    void navigateToOrders();
    void navigateToReports();
    void navigateToInventory();

private slots:
    void refreshData();
    void onPrintReport();
    void onInventoryCheck();
    void updateRecentOrders();
    void updateInventoryAlerts();
    void updateChartData();

private:
    void createStatsCards();
    void createChartArea();
    void createRecentOrders();
    void createInventoryAlerts();
    void createActionButtons();
    void setupLayout();

    // 颜色常量
    const QColor PRIMARY_COLOR = QColor(41, 128, 185);      // 主蓝
    const QColor SUCCESS_COLOR = QColor(39, 174, 96);       // 成功绿
    const QColor WARNING_COLOR = QColor(241, 196, 15);      // 警告黄
    const QColor DANGER_COLOR = QColor(231, 76, 60);        // 危险红
    const QColor CHART_COLOR = QColor(52, 152, 219);        // 图表蓝
    const QColor BG_COLOR = QColor(245, 247, 250);          // 背景灰

    // 统计卡片
    QFrame *statsCardToday;
    QFrame *statsCardWeek;
    QFrame *statsCardMonth;
    QFrame *statsCardInventory;

    // 统计标签
    QLabel *todaySalesLabel;
    QLabel *todayOrdersLabel;
    QLabel *weekSalesLabel;
    QLabel *weekTrendLabel;
    QLabel *monthSalesLabel;
    QLabel *monthTargetLabel;
    QLabel *inventoryWarningLabel;
    QProgressBar *todayProgressBar;

    // 图表区域
    QWidget *chartWidget;
    QChartView *chartView;
    QChart *salesChart;
    QLineSeries *salesSeries;
    QDateTimeAxis *axisX;
    QValueAxis *axisY;

    // 最近订单
    QWidget *recentOrdersWidget;
    QLabel *recentOrdersTitle;

    // 库存预警
    QWidget *inventoryWidget;
    QTableWidget *inventoryAlertTable;

    // 操作按钮
    QPushButton *printReportButton;
    QPushButton *inventoryCheckButton;

    // 布局
    QGridLayout *mainLayout;
    QTimer *refreshTimer;

    // 模拟数据
    double todayTarget;
    QList<double> weeklySales;  // 存储最近7天数据
};

#endif // DASHBOARDWIDGET_H
