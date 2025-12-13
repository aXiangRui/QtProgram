#ifndef REPORTWIDGET_H
#define REPORTWIDGET_H

#include <QWidget>
#include <QTabWidget>
#include <QTableWidget>
#include <QPushButton>
#include <QDateEdit>
#include <QComboBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QGroupBox>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QFileDialog>
#include <QDateTime>
#include <QTime>
#include <QProgressDialog>

class ReportWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ReportWidget(QWidget *parent = 0);

private slots:
    void onGenerateReport();
    void onExportXML();
    void onRefresh();
    void onDateRangeChanged();
    void onReportTypeChanged(int index);
    void onSalesTableDoubleClick(int row, int column);
    void onInventoryTableDoubleClick(int row, int column);
    void onMemberTableDoubleClick(int row, int column);

private:
    // 界面创建函数
    void createControlPanel();
    void createReportArea();
    void createSalesTab();
    void createInventoryTab();
    void createMemberTab();

    // 报表生成函数
    void generateSalesReport();
    void generateInventoryReport();
    void generateMemberReport();

    // XML导出函数
    bool exportToXML(const QString &filename);
    void writeSalesXML(QTextStream &stream);
    void writeInventoryXML(QTextStream &stream);
    void writeMemberXML(QTextStream &stream);
    QString escapeXML(const QString &text);

    // 辅助函数
    void clearAllTables();
    QString getDefaultFilename();
    void updateDateRangeLabel();
    void updateRecordCount();
    void initRandomSeed();

    // 数据模拟函数
    QList<QStringList> generateSalesData();
    QList<QStringList> generateInventoryData();
    QList<QStringList> generateMemberData();

    // 界面控件
    QWidget *controlPanel;
    QComboBox *reportTypeCombo;
    QDateEdit *startDateEdit;
    QDateEdit *endDateEdit;
    QPushButton *generateButton;
    QPushButton *exportButton;
    QPushButton *refreshButton;
    QLabel *dateRangeLabel;
    QLabel *recordCountLabel;
    QLabel *statusLabel;

    QTabWidget *reportTabs;
    QWidget *salesTab;
    QWidget *inventoryTab;
    QWidget *memberTab;

    QTableWidget *salesTable;
    QTableWidget *inventoryTable;
    QTableWidget *memberTable;

    QVBoxLayout *mainLayout;

    // 颜色常量
    static const QColor COLOR_PRIMARY;
    static const QColor COLOR_SUCCESS;
    static const QColor COLOR_WARNING;
    static const QColor COLOR_DANGER;
};

#endif // REPORTWIDGET_H
