#ifndef ORDERMANAGEWIDGET_H
#define ORDERMANAGEWIDGET_H

#include <QWidget>
#include <QTabWidget>
#include <QTableWidget>
#include <QPushButton>
#include <QComboBox>
#include <QDateEdit>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QGroupBox>
#include <QSpinBox>
#include <QTextEdit>
#include <QHeaderView>
#include <QRadioButton>
#include <QCheckBox>
#include <QProgressBar>
#include <QString>
#include <QDateTime>
#include <QDialog>

class OrderManageWidget : public QWidget
{
    Q_OBJECT

public:
    explicit OrderManageWidget(QWidget *parent = 0);

private slots:
    void onTabChanged(int index);
    void onOpenNewOrderDialog(); // 打开新建订单对话框
    void onExportOrders(); // 导出订单数据
    void onRefresh(); // 刷新数据
    void onSearch(); // 搜索订单
    void onShowOrderDetails(); // 显示订单详情
    void onTableSelectionChanged(); // 表格选择变化
    void onFilterChanged(); // 筛选条件变化
    void onAmountRangeChanged(); // 金额范围变化
    void onExportCurrentTab(); // 导出当前标签页
    void onClearFilters(); // 清除筛选条件
    void onDeleteOrder(); // 删除订单
    void onclearCart();
    // 新建订单相关槽函数
    void onAddToCart(); // 添加到购物车
    void onRemoveFromCart(); // 从购物车移除
    void onUpdateCartQuantity(); // 更新购物车数量检查
    void onConfirmNewOrder(); // 确认新建订单
    void onCancelNewOrder(); // 取消新建订单
    void onSelectPaymentMethod(); // 选择支付方式（占位函数）

private:
    void createToolbar(); // 创建工具栏
    void createTabs(); // 创建标签页
    void createFilterPanel(); // 创建筛选面板
    void createOrderDetails(); // 创建订单详情面板
    void createNewOrderDialog(); // 创建新建订单对话框
    void setupTabLayout(QWidget *tab, const QString &status);
    void setupOrderTable(QTableWidget *table);
    void loadOrders(const QString &status);
    void updateStatistics(); // 更新统计信息
    void applyTableStyle(QTableWidget *table);
    QString getStatusColor(const QString &status);
    void saveOrdersToFile(const QString &filename, const QString &format);
    void generateSampleData(); // 生成模拟数据
    void loadProductsForNewOrder(); // 加载商品数据用于新建订单
    void updateCartTotal(); // 更新购物车总价
    void clearNewOrderForm(); // 清空新建订单表单

    // 布局
    QVBoxLayout *mainLayout;

    // 左侧筛选面板
    QWidget *filterPanel;
    QGroupBox *timeFilterGroup;
    QGroupBox *amountFilterGroup;
    QGroupBox *paymentFilterGroup;
    QGroupBox *statsGroup;

    // 筛选控件
    QRadioButton *timeToday;
    QRadioButton *timeWeek;
    QRadioButton *timeMonth;
    QRadioButton *timeAll;
    QSpinBox *minAmountEdit;
    QSpinBox *maxAmountEdit;
    QCheckBox *paymentAll;
    QCheckBox *paymentCash;
    QCheckBox *paymentWechat;
    QCheckBox *paymentAlipay;
    QCheckBox *paymentCard;
    QLineEdit *searchEdit;
    QPushButton *searchButton;
    QPushButton *clearFilterButton;

    // 统计信息
    QLabel *totalOrdersLabel;
    QLabel *totalAmountLabel;
    QLabel *pendingOrdersLabel;
    QProgressBar *completionBar;

    // 标签页
    QTabWidget *tabWidget;
    QWidget *tabAll;
    QWidget *tabPending;
    QWidget *tabPaid;
    QWidget *tabShipped;
    QWidget *tabCompleted;
    QWidget *tabCancelled;

    // 订单表格
    QTableWidget *orderTableAll;
    QTableWidget *orderTablePending;
    QTableWidget *orderTablePaid;
    QTableWidget *orderTableShipped;
    QTableWidget *orderTableCompleted;
    QTableWidget *orderTableCancelled;

    // 工具栏
    QWidget *toolbar;
    QPushButton *createButton;
    QPushButton *exportButton;
    QPushButton *refreshButton;
    QPushButton *exportTabButton;
    QPushButton *deleteButton;

    // 订单详情面板
    QWidget *detailPanel;
    QGroupBox *orderInfoGroup;
    QGroupBox *orderItemsGroup;

    // 订单详情控件
    QLabel *orderNoLabel;
    QLabel *customerLabel;
    QLabel *phoneLabel;       // 新增：手机号码
    QLabel *addressLabel;     // 新增：收货地址
    QLabel *statusLabel;
    QLabel *amountLabel;
    QLabel *paymentLabel;
    QLabel *timeLabel;
    QLabel *operatorLabel;
    QLabel *remarkLabel;
    QTableWidget *orderItemsTable;
    QLabel *subtotalLabel;
    QLabel *discountLabel;
    QLabel *totalLabel;

    // 模拟数据
    QList<QStringList> orderData;
    int currentOrderRow;

    // 新建订单相关成员
    QDialog *newOrderDialog;
    QComboBox *productCombo;
    QSpinBox *quantitySpin;
    QTableWidget *cartTable;
    QLabel *cartTotalLabel;
    QComboBox *paymentCombo;
    QLineEdit *customerNameEdit;
    QLineEdit *customerPhoneEdit;
    QLineEdit *customerAddressEdit;  // 新增：收货地址输入框
    QTextEdit *remarkEdit;
    QPushButton *confirmOrderButton;

    // 商品数据
    QList<QStringList> productData;
    QList<QStringList> cartItems;
    double cartTotal;
};

#endif // ORDERMANAGEWIDGET_H
