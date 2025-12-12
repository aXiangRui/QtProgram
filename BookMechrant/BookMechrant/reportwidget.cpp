#include "ReportWidget.h"
#include <QHeaderView>
#include <QGroupBox>
#include <QFormLayout>
#include <QTextCodec>
#include<QApplication>

// 初始化静态常量
const QColor ReportWidget::COLOR_PRIMARY = QColor(41, 128, 185);
const QColor ReportWidget::COLOR_SUCCESS = QColor(39, 174, 96);
const QColor ReportWidget::COLOR_WARNING = QColor(241, 196, 15);
const QColor ReportWidget::COLOR_DANGER = QColor(231, 76, 60);

// 样式常量
namespace {
    const QString TABLE_STYLE =
        "QTableWidget {"
        "    border: 1px solid #ddd;"
        "    border-radius: 4px;"
        "    background-color: white;"
        "    font-size: 13px;"
        "    selection-background-color: #3498db;"
        "    selection-color: white;"
        "}"
        "QTableWidget::item {"
        "    padding: 10px 8px;"
        "    border-bottom: 1px solid #f0f0f0;"
        "}"
        "QHeaderView::section {"
        "    background-color: #f8f9fa;"
        "    padding: 12px 8px;"
        "    border: none;"
        "    border-right: 1px solid #ddd;"
        "    border-bottom: 2px solid #3498db;"
        "    font-weight: bold;"
        "    color: #333;"
        "    font-size: 13px;"
        "}"
        "QHeaderView::section:last {"
        "    border-right: none;"
        "}";
}

ReportWidget::ReportWidget(QWidget *parent)
    : QWidget(parent)
{
    // 设置窗口属性 - 自适应大小
    setMinimumSize(800, 500);

    // 创建主布局
    mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(15, 15, 15, 15);
    mainLayout->setSpacing(15);

    // 设置背景
    setStyleSheet("background-color: #f8f9fa;");

    // 创建界面组件
    createControlPanel();
    createReportArea();

    // 初始化随机种子
    initRandomSeed();

    // 初始化状态 - 但不立即生成报表
    statusLabel->setText("就绪 - 请选择日期范围并点击生成报表");
}

void ReportWidget::initRandomSeed()
{
    QTime time = QTime::currentTime();
    qsrand(static_cast<uint>(time.msec() + time.second() * 1000));
}

// ========== 界面创建函数 ==========

void ReportWidget::createControlPanel()
{
    // 控制面板容器
    controlPanel = new QWidget;
    controlPanel->setStyleSheet(
        "background-color: white;"
        "border-radius: 8px;"
        "border: 1px solid #e0e0e0;"
        "padding: 15px;"
    );

    QVBoxLayout *mainControlLayout = new QVBoxLayout(controlPanel);
    mainControlLayout->setContentsMargins(0, 0, 0, 0);
    mainControlLayout->setSpacing(12);

    // 第一行：筛选条件
    QHBoxLayout *filterLayout = new QHBoxLayout;
    filterLayout->setSpacing(15);

    QLabel *typeLabel = new QLabel("报表类型:");
    typeLabel->setStyleSheet("font-weight: bold; color: #555; font-size: 13px;");

    reportTypeCombo = new QComboBox;
    reportTypeCombo->addItem("📊 销售报表");
    reportTypeCombo->addItem("📦 库存报表");
    reportTypeCombo->addItem("👥 会员报表");
    reportTypeCombo->setFixedWidth(150);
    reportTypeCombo->setStyleSheet(
        "QComboBox {"
        "    border: 1px solid #ccc;"
        "    border-radius: 4px;"
        "    padding: 8px 12px;"
        "    background-color: white;"
        "    font-size: 13px;"
        "}"
        "QComboBox:hover {"
        "    border-color: #3498db;"
        "}"
        "QComboBox::drop-down {"
        "    border: none;"
        "}"
    );

    QLabel *dateLabel = new QLabel("日期范围:");
    dateLabel->setStyleSheet("font-weight: bold; color: #555; font-size: 13px;");

    startDateEdit = new QDateEdit;
    endDateEdit = new QDateEdit;

    QDate today = QDate::currentDate();
    startDateEdit->setDate(today.addDays(-7));  // 默认显示最近7天
    endDateEdit->setDate(today);

    startDateEdit->setDisplayFormat("yyyy-MM-dd");
    endDateEdit->setDisplayFormat("yyyy-MM-dd");
    startDateEdit->setCalendarPopup(true);
    endDateEdit->setCalendarPopup(true);

    startDateEdit->setFixedWidth(110);
    endDateEdit->setFixedWidth(110);
    startDateEdit->setStyleSheet("padding: 6px;");
    endDateEdit->setStyleSheet("padding: 6px;");

    QLabel *toLabel = new QLabel("至");
    toLabel->setStyleSheet("color: #777; font-size: 13px;");

    dateRangeLabel = new QLabel;
    dateRangeLabel->setStyleSheet("color: #666; font-size: 12px;");
    updateDateRangeLabel();

    filterLayout->addWidget(typeLabel);
    filterLayout->addWidget(reportTypeCombo);
    filterLayout->addSpacing(30);
    filterLayout->addWidget(dateLabel);
    filterLayout->addWidget(startDateEdit);
    filterLayout->addWidget(toLabel);
    filterLayout->addWidget(endDateEdit);
    filterLayout->addWidget(dateRangeLabel);
    filterLayout->addStretch();

    // 第二行：操作按钮和状态
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->setSpacing(12);

    // 状态标签
    statusLabel = new QLabel("状态: 就绪");
    statusLabel->setStyleSheet("color: #666; font-size: 12px; padding: 5px 0;");

    // 记录数标签
    recordCountLabel = new QLabel("记录数: 0");
    recordCountLabel->setStyleSheet("color: #666; font-size: 12px; padding: 5px 0;");

    // 操作按钮
    generateButton = new QPushButton("🔄 生成报表");
    exportButton = new QPushButton("📤 导出XML");
    refreshButton = new QPushButton("刷新");

    // 按钮样式
    QString buttonStyle =
        "QPushButton {"
        "    background-color: #3498db;"
        "    color: white;"
        "    border: none;"
        "    border-radius: 4px;"
        "    padding: 10px 20px;"
        "    font-size: 13px;"
        "    font-weight: bold;"
        "    min-height: 36px;"
        "}"
        "QPushButton:hover {"
        "    background-color: #2980b9;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #1c638e;"
        "}"
        "QPushButton:disabled {"
        "    background-color: #95a5a6;"
        "}";

    generateButton->setStyleSheet(buttonStyle);
    exportButton->setStyleSheet(buttonStyle);
    refreshButton->setStyleSheet(buttonStyle);

    // 初始禁用导出按钮
    exportButton->setEnabled(false);

    buttonLayout->addWidget(statusLabel);
    buttonLayout->addStretch();
    buttonLayout->addWidget(recordCountLabel);
    buttonLayout->addSpacing(20);
    buttonLayout->addWidget(refreshButton);
    buttonLayout->addWidget(generateButton);
    buttonLayout->addWidget(exportButton);

    // 添加到主控制布局
    mainControlLayout->addLayout(filterLayout);
    mainControlLayout->addLayout(buttonLayout);

    // 添加到主布局
    mainLayout->addWidget(controlPanel);

    // 连接信号
    connect(reportTypeCombo, SIGNAL(currentIndexChanged(int)),
            this, SLOT(onReportTypeChanged(int)));
    connect(generateButton, SIGNAL(clicked()),
            this, SLOT(onGenerateReport()));
    connect(exportButton, SIGNAL(clicked()),
            this, SLOT(onExportXML()));
    connect(refreshButton, SIGNAL(clicked()),
            this, SLOT(onRefresh()));
    connect(startDateEdit, SIGNAL(dateChanged(QDate)),
            this, SLOT(onDateRangeChanged()));
    connect(endDateEdit, SIGNAL(dateChanged(QDate)),
            this, SLOT(onDateRangeChanged()));
}

void ReportWidget::createReportArea()
{
    // 创建标签页容器
    reportTabs = new QTabWidget;
    reportTabs->setObjectName("reportTabs");

    // 创建各报表页
    createSalesTab();
    createInventoryTab();
    createMemberTab();

    // 添加到标签页
    reportTabs->addTab(salesTab, "📊 销售分析");
    reportTabs->addTab(inventoryTab, "📦 库存分析");
    reportTabs->addTab(memberTab, "👥 会员分析");

    // 设置标签页样式
    reportTabs->setStyleSheet(
        "#reportTabs::pane {"
        "    border: 1px solid #ddd;"
        "    border-top: none;"
        "    border-radius: 0 0 8px 8px;"
        "    background-color: white;"
        "}"
        "QTabBar::tab {"
        "    background-color: #f5f5f5;"
        "    border: 1px solid #ddd;"
        "    border-bottom: none;"
        "    padding: 12px 25px;"
        "    margin-right: 2px;"
        "    border-top-left-radius: 6px;"
        "    border-top-right-radius: 6px;"
        "    font-size: 13px;"
        "}"
        "QTabBar::tab:selected {"
        "    background-color: white;"
        "    font-weight: bold;"
        "    color: #3498db;"
        "}"
        "QTabBar::tab:hover {"
        "    background-color: #e9e9e9;"
        "}"
    );

    // 设置拉伸策略
    mainLayout->addWidget(reportTabs, 1);
}

void ReportWidget::createSalesTab()
{
    salesTab = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(salesTab);
    layout->setContentsMargins(0, 0, 0, 0);

    salesTable = new QTableWidget;
    salesTable->setObjectName("salesTable");
    salesTable->setColumnCount(6);
    salesTable->setHorizontalHeaderLabels(
        QStringList() << "日期" << "订单数" << "销售额" << "成本" << "毛利" << "毛利率"
    );

    // 设置表格属性
    salesTable->verticalHeader()->setVisible(false);
    salesTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    salesTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    salesTable->setAlternatingRowColors(true);
    salesTable->setStyleSheet(TABLE_STYLE);

    // 设置列宽和拉伸策略
    QHeaderView *header = salesTable->horizontalHeader();
    header->setStretchLastSection(true);  // 最后一列拉伸

    // 设置初始列宽（百分比方式）
    salesTable->setColumnWidth(0, 120);  // 日期
    salesTable->setColumnWidth(1, 100);  // 订单数
    salesTable->setColumnWidth(2, 120);  // 销售额
    salesTable->setColumnWidth(3, 120);  // 成本
    salesTable->setColumnWidth(4, 120);  // 毛利
    // 第5列自动拉伸

    // 设置行高
    salesTable->verticalHeader()->setDefaultSectionSize(42);

    // 连接双击信号
    connect(salesTable, SIGNAL(cellDoubleClicked(int, int)),
            this, SLOT(onSalesTableDoubleClick(int, int)));

    layout->addWidget(salesTable);
}

void ReportWidget::createInventoryTab()
{
    inventoryTab = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(inventoryTab);
    layout->setContentsMargins(0, 0, 0, 0);

    inventoryTable = new QTableWidget;
    inventoryTable->setObjectName("inventoryTable");
    inventoryTable->setColumnCount(6);
    inventoryTable->setHorizontalHeaderLabels(
        QStringList() << "商品名称" << "当前库存" << "安全库存" << "状态" << "周转天数" << "建议"
    );

    // 设置表格属性
    inventoryTable->verticalHeader()->setVisible(false);
    inventoryTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    inventoryTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    inventoryTable->setAlternatingRowColors(true);
    inventoryTable->setStyleSheet(TABLE_STYLE);

    // 设置列宽和拉伸策略
    QHeaderView *header = inventoryTable->horizontalHeader();
    header->setStretchLastSection(true);

    // 设置初始列宽
    inventoryTable->setColumnWidth(0, 200);  // 商品名称
    inventoryTable->setColumnWidth(1, 100);  // 当前库存
    inventoryTable->setColumnWidth(2, 100);  // 安全库存
    inventoryTable->setColumnWidth(3, 80);   // 状态
    inventoryTable->setColumnWidth(4, 100);  // 周转天数
    // 第5列自动拉伸

    // 设置行高
    inventoryTable->verticalHeader()->setDefaultSectionSize(42);
    // 连接双击信号
        connect(inventoryTable, SIGNAL(cellDoubleClicked(int, int)),
                this, SLOT(onInventoryTableDoubleClick(int, int)));

        layout->addWidget(inventoryTable);
    }

    void ReportWidget::createMemberTab()
    {
        memberTab = new QWidget;
        QVBoxLayout *layout = new QVBoxLayout(memberTab);
        layout->setContentsMargins(0, 0, 0, 0);

        memberTable = new QTableWidget;
        memberTable->setObjectName("memberTable");
        memberTable->setColumnCount(5);
        memberTable->setHorizontalHeaderLabels(
            QStringList() << "会员等级" << "会员数" << "消费总额" << "平均消费" << "复购率"
        );

        // 设置表格属性
        memberTable->verticalHeader()->setVisible(false);
        memberTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
        memberTable->setSelectionBehavior(QAbstractItemView::SelectRows);
        memberTable->setAlternatingRowColors(true);
        memberTable->setStyleSheet(TABLE_STYLE);

        // 设置列宽和拉伸策略
        QHeaderView *header = memberTable->horizontalHeader();
        header->setStretchLastSection(true);

        // 设置初始列宽
        memberTable->setColumnWidth(0, 120);  // 会员等级
        memberTable->setColumnWidth(1, 100);  // 会员数
        memberTable->setColumnWidth(2, 150);  // 消费总额
        memberTable->setColumnWidth(3, 120);  // 平均消费
        // 第4列自动拉伸

        // 设置行高
        memberTable->verticalHeader()->setDefaultSectionSize(42);

        // 连接双击信号
        connect(memberTable, SIGNAL(cellDoubleClicked(int, int)),
                this, SLOT(onMemberTableDoubleClick(int, int)));

        layout->addWidget(memberTable);
    }

    // ========== 功能实现函数 ==========

    void ReportWidget::onGenerateReport()
    {
        // 清空所有表格
        clearAllTables();

        // 更新状态
        statusLabel->setText("正在生成报表...");
        statusLabel->setStyleSheet("color: #3498db; font-size: 12px; padding: 5px 0;");

        // 根据选择的报表类型生成数据
        int reportType = reportTypeCombo->currentIndex();

        switch (reportType) {
            case 0:  // 销售报表
                generateSalesReport();
                break;
            case 1:  // 库存报表
                generateInventoryReport();
                break;
            case 2:  // 会员报表
                generateMemberReport();
                break;
        }

        // 切换到对应的标签页
        reportTabs->setCurrentIndex(reportType);

        // 更新记录数和状态
        updateRecordCount();
        statusLabel->setText("报表生成完成");
        statusLabel->setStyleSheet("color: #27ae60; font-size: 12px; padding: 5px 0;");

        // 启用导出按钮
        exportButton->setEnabled(true);

        // 不需要弹出消息框，状态标签已经显示
    }

    void ReportWidget::generateSalesReport()
    {
        salesTable->setRowCount(0);

        QList<QStringList> salesData = generateSalesData();

        // 填充表格
        for (int i = 0; i < salesData.size(); i++) {
            int row = salesTable->rowCount();
            salesTable->insertRow(row);

            const QStringList &rowData = salesData[i];
            for (int col = 0; col < rowData.size() && col < 6; col++) {
                QTableWidgetItem *item = new QTableWidgetItem(rowData[col]);

                // 设置对齐方式
                if (col >= 1) {  // 数字列右对齐
                    item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
                } else {
                    item->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);
                }

                // 设置颜色
                if (col == 5) {  // 毛利率列
                    QString rateText = rowData[col];
                    rateText.replace("%", "");
                    double rate = rateText.toDouble();

                    if (rate > 40) {
                        item->setForeground(COLOR_SUCCESS);
                    } else if (rate > 30) {
                        item->setForeground(COLOR_WARNING);
                    } else {
                        item->setForeground(COLOR_DANGER);
                    }
                    item->setFont(QFont("", -1, QFont::Bold));
                }

                salesTable->setItem(row, col, item);
            }
        }
    }

    void ReportWidget::generateInventoryReport()
    {
        inventoryTable->setRowCount(0);

        QList<QStringList> inventoryData = generateInventoryData();

        // 填充表格
        for (int i = 0; i < inventoryData.size(); i++) {
            int row = inventoryTable->rowCount();
            inventoryTable->insertRow(row);

            const QStringList &rowData = inventoryData[i];
            for (int col = 0; col < rowData.size() && col < 6; col++) {
                QTableWidgetItem *item = new QTableWidgetItem(rowData[col]);

                // 设置对齐方式
                if (col == 1 || col == 2 || col == 4) {
                    item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
                } else if (col == 0) {
                    item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
                } else {
                    item->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);
                }

                // 设置状态颜色
                if (col == 3) {
                    QString status = rowData[col];
                    if (status == "紧急") {
                        item->setForeground(COLOR_DANGER);
                        item->setFont(QFont("", -1, QFont::Bold));
                    } else if (status == "预警") {
                        item->setForeground(COLOR_WARNING);
                        item->setFont(QFont("", -1, QFont::Bold));
                    } else {
                        item->setForeground(COLOR_SUCCESS);
                    }
                }

                inventoryTable->setItem(row, col, item);
            }
        }
    }

    void ReportWidget::generateMemberReport()
    {
        memberTable->setRowCount(0);

        QList<QStringList> memberData = generateMemberData();

        // 填充表格
        for (int i = 0; i < memberData.size(); i++) {
            int row = memberTable->rowCount();
            memberTable->insertRow(row);

            const QStringList &rowData = memberData[i];
            for (int col = 0; col < rowData.size() && col < 5; col++) {
                QTableWidgetItem *item = new QTableWidgetItem(rowData[col]);

                // 设置对齐方式
                if (col >= 1) {
                    item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
                } else {
                    item->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);
                }

                // 设置等级颜色
                if (col == 0) {
                    QString level = rowData[col];
                    if (level == "钻石") {
                        item->setForeground(QColor("#9b59b6"));  // 紫色
                    } else if (level == "黄金") {
                        item->setForeground(COLOR_WARNING);
                    } else if (level == "白银") {
                        item->setForeground(QColor("#95a5a6"));  // 灰色
                    } else {
                        item->setForeground(QColor("#34495e"));  // 深灰色
                    }
                    item->setFont(QFont("", -1, QFont::Bold));
                }

                memberTable->setItem(row, col, item);
            }
        }
    }
    // ========== XML导出函数 ==========

    void ReportWidget::onExportXML()
    {
        QString defaultName = getDefaultFilename();

        QString fileName = QFileDialog::getSaveFileName(
            this,
            "导出XML报表",
            defaultName,
            "XML文件 (*.xml)"
        );

        if (fileName.isEmpty()) {
            statusLabel->setText("导出已取消");
            statusLabel->setStyleSheet("color: #666; font-size: 12px; padding: 5px 0;");
            return;
        }

        if (!fileName.endsWith(".xml", Qt::CaseInsensitive)) {
            fileName += ".xml";
        }

        // 显示导出进度
        QProgressDialog progress("正在导出报表...", "取消", 0, 100, this);
        progress.setWindowModality(Qt::WindowModal);
        progress.setWindowTitle("导出进度");
        progress.show();

        progress.setValue(30);
        QApplication::processEvents();

        if (exportToXML(fileName)) {
            progress.setValue(100);

            statusLabel->setText(QString("报表已导出到: %1").arg(QFileInfo(fileName).fileName()));
            statusLabel->setStyleSheet("color: #27ae60; font-size: 12px; padding: 5px 0; font-weight: bold;");

            // 只在状态栏显示，不弹窗
        } else {
            progress.setValue(0);
            statusLabel->setText("导出失败");
            statusLabel->setStyleSheet("color: #e74c3c; font-size: 12px; padding: 5px 0;");
        }
    }

    bool ReportWidget::exportToXML(const QString &filename)
    {
        QFile file(filename);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            return false;
        }

        QTextStream stream(&file);
    #if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
        stream.setCodec("UTF-8");
    #endif

        // 写入XML头部
        stream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
        stream << "<!-- 图书商家管理系统报表 -->\n";
        stream << "<!-- 生成时间: " << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") << " -->\n\n";

        // 根据报表类型写入数据
        int reportType = reportTypeCombo->currentIndex();
        switch (reportType) {
            case 0: writeSalesXML(stream); break;
            case 1: writeInventoryXML(stream); break;
            case 2: writeMemberXML(stream); break;
        }

        stream << "\n</BookMerchantReport>\n";

        file.close();
        return true;
    }

    QString ReportWidget::escapeXML(const QString &text)
    {
        QString escaped = text;
        escaped.replace("&", "&amp;");
        escaped.replace("<", "&lt;");
        escaped.replace(">", "&gt;");
        escaped.replace("\"", "&quot;");
        escaped.replace("'", "&apos;");
        return escaped;
    }

    void ReportWidget::writeSalesXML(QTextStream &stream)
    {
        stream << "<BookMerchantReport type=\"销售报表\" ";
        stream << "startDate=\"" << startDateEdit->date().toString("yyyy-MM-dd") << "\" ";
        stream << "endDate=\"" << endDateEdit->date().toString("yyyy-MM-dd") << "\" ";
        stream << "recordCount=\"" << salesTable->rowCount() << "\" ";
        stream << "generateTime=\"" << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") << "\">\n\n";

        // 汇总信息
        stream << "  <Summary>\n";

        double totalSales = 0;
        double totalCost = 0;
        double totalProfit = 0;
        int totalOrders = 0;

        for (int row = 0; row < salesTable->rowCount(); row++) {
            if (QTableWidgetItem *ordersItem = salesTable->item(row, 1)) {
                totalOrders += ordersItem->text().toInt();
            }
            if (QTableWidgetItem *salesItem = salesTable->item(row, 2)) {
                QString salesText = salesItem->text();
                salesText.remove("¥");
                totalSales += salesText.toDouble();
            }
            if (QTableWidgetItem *costItem = salesTable->item(row, 3)) {
                QString costText = costItem->text();
                costText.remove("¥");
                totalCost += costText.toDouble();
            }
            if (QTableWidgetItem *profitItem = salesTable->item(row, 4)) {
                QString profitText = profitItem->text();
                profitText.remove("¥");
                totalProfit += profitText.toDouble();
            }
        }

        stream << "    <TotalOrders>" << totalOrders << "</TotalOrders>\n";
        stream << "    <TotalSales>¥" << QString::number(totalSales, 'f', 2) << "</TotalSales>\n";
        stream << "    <TotalCost>¥" << QString::number(totalCost, 'f', 2) << "</TotalCost>\n";
        stream << "    <TotalProfit>¥" << QString::number(totalProfit, 'f', 2) << "</TotalProfit>\n";

        double profitRate = totalSales > 0 ? (totalProfit / totalSales * 100) : 0;
        stream << "    <ProfitRate>" << QString::number(profitRate, 'f', 1) << "%</ProfitRate>\n";

        stream << "  </Summary>\n\n";

        // 详细数据
        stream << "  <Details>\n";

        for (int row = 0; row < salesTable->rowCount(); row++) {
            stream << "    <Record row=\"" << (row + 1) << "\">\n";

            for (int col = 0; col < salesTable->columnCount(); col++) {
                if (QTableWidgetItem *item = salesTable->item(row, col)) {
                    QString header = salesTable->horizontalHeaderItem(col)->text();
                    QString value = escapeXML(item->text());
                    stream << "      <" << header << ">" << value << "</" << header << ">\n";
                }
            }

            stream << "    </Record>\n";
        }

        stream << "  </Details>\n";
    }

    void ReportWidget::writeInventoryXML(QTextStream &stream)
    {
        stream << "<BookMerchantReport type=\"库存报表\" ";
        stream << "recordCount=\"" << inventoryTable->rowCount() << "\" ";
        stream << "generateTime=\"" << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") << "\">\n\n";
        // 汇总信息
        stream << "  <Summary>\n";

        int warningCount = 0;
        int urgentCount = 0;
        int normalCount = 0;
        int totalStock = 0;

        for (int row = 0; row < inventoryTable->rowCount(); row++) {
            if (QTableWidgetItem *stockItem = inventoryTable->item(row, 1)) {
                totalStock += stockItem->text().toInt();
            }
            if (QTableWidgetItem *statusItem = inventoryTable->item(row, 3)) {
                QString status = statusItem->text();
                if (status == "紧急") urgentCount++;
                else if (status == "预警") warningCount++;
                else if (status == "正常") normalCount++;
            }
        }

        stream << "    <TotalProducts>" << inventoryTable->rowCount() << "</TotalProducts>\n";
        stream << "    <TotalStock>" << totalStock << "</TotalStock>\n";
        stream << "    <UrgentCount>" << urgentCount << "</UrgentCount>\n";
        stream << "    <WarningCount>" << warningCount << "</WarningCount>\n";
        stream << "    <NormalCount>" << normalCount << "</NormalCount>\n";

        stream << "  </Summary>\n\n";

        // 详细数据
        stream << "  <Details>\n";

        for (int row = 0; row < inventoryTable->rowCount(); row++) {
            stream << "    <Product row=\"" << (row + 1) << "\">\n";

            for (int col = 0; col < inventoryTable->columnCount(); col++) {
                if (QTableWidgetItem *item = inventoryTable->item(row, col)) {
                    QString header = inventoryTable->horizontalHeaderItem(col)->text();
                    QString value = escapeXML(item->text());
                    stream << "      <" << header << ">" << value << "</" << header << ">\n";
                }
            }

            stream << "    </Product>\n";
        }

        stream << "  </Details>\n";
    }

    void ReportWidget::writeMemberXML(QTextStream &stream)
    {
        stream << "<BookMerchantReport type=\"会员报表\" ";
        stream << "recordCount=\"" << memberTable->rowCount() << "\" ";
        stream << "generateTime=\"" << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") << "\">\n\n";

        // 汇总信息
        stream << "  <Summary>\n";

        int totalMembers = 0;
        double totalConsumption = 0;

        for (int row = 0; row < memberTable->rowCount(); row++) {
            if (QTableWidgetItem *countItem = memberTable->item(row, 1)) {
                totalMembers += countItem->text().toInt();
            }
            if (QTableWidgetItem *consumptionItem = memberTable->item(row, 2)) {
                QString consumptionText = consumptionItem->text();
                consumptionText.remove("¥");
                totalConsumption += consumptionText.toDouble();
            }
        }

        stream << "    <TotalMembers>" << totalMembers << "</TotalMembers>\n";
        stream << "    <TotalConsumption>¥" << QString::number(totalConsumption, 'f', 2) << "</TotalConsumption>\n";
        stream << "    <LevelCount>" << memberTable->rowCount() << "</LevelCount>\n";

        stream << "  </Summary>\n\n";

        // 详细数据
        stream << "  <LevelDetails>\n";

        for (int row = 0; row < memberTable->rowCount(); row++) {
            stream << "    <Level ";

            for (int col = 0; col < memberTable->columnCount(); col++) {
                if (QTableWidgetItem *item = memberTable->item(row, col)) {
                    QString header = memberTable->horizontalHeaderItem(col)->text();
                    QString value = escapeXML(item->text());
                    stream << header << "=\"" << value << "\" ";
                }
            }

            stream << "/>\n";
        }

        stream << "  </LevelDetails>\n";
    }

    // ========== 辅助函数 ==========

    void ReportWidget::onRefresh()
    {
        onGenerateReport();
    }

    void ReportWidget::onDateRangeChanged()
    {
        updateDateRangeLabel();
    }

    void ReportWidget::onReportTypeChanged(int index)
    {
        reportTabs->setCurrentIndex(index);
        onGenerateReport();
    }

    void ReportWidget::clearAllTables()
    {
        salesTable->setRowCount(0);
        inventoryTable->setRowCount(0);
        memberTable->setRowCount(0);
    }

    QString ReportWidget::getDefaultFilename()
    {
        QString reportType = reportTypeCombo->currentText();
        reportType.replace(" ", "").replace("📊", "").replace("📦", "").replace("👥", "");

        QString dateRange = QString("%1_%2")
            .arg(startDateEdit->date().toString("yyyyMMdd"))
            .arg(endDateEdit->date().toString("yyyyMMdd"));

        return QString("报表_%1_%2.xml")
            .arg(reportType)
            .arg(dateRange);
    }

    void ReportWidget::updateDateRangeLabel()
    {
        QDate start = startDateEdit->date();
        QDate end = endDateEdit->date();

        int days = start.daysTo(end) + 1;
        if (days < 0) {
            dateRangeLabel->setText("（日期错误）");
            dateRangeLabel->setStyleSheet("color: #e74c3c; font-size: 12px;");
            endDateEdit->setDate(start);
        } else {
            dateRangeLabel->setText(QString("（共%1天）").arg(days));
            dateRangeLabel->setStyleSheet("color: #666; font-size: 12px;");
        }
    }

    void ReportWidget::updateRecordCount()
    {
        int count = 0;
        int currentTab = reportTabs->currentIndex();

        switch (currentTab) {
            case 0: count = salesTable->rowCount(); break;
            case 1: count = inventoryTable->rowCount(); break;
            case 2: count = memberTable->rowCount(); break;
        }

        recordCountLabel->setText(QString("记录数: %1").arg(count));
    }
    // ========== 数据模拟函数 ==========

    QList<QStringList> ReportWidget::generateSalesData()
    {
        QList<QStringList> data;
        QDate currentDate = startDateEdit->date();
        QDate endDate = endDateEdit->date();

        int days = 0;
        int maxDays = 30;  // 最多显示30天

        while (currentDate <= endDate && days < maxDays) {
            int orders = 5 + qrand() % 16;
            double sales = 500.0 + (qrand() % 2501);
            double costRate = 0.55 + (qrand() % 16) / 100.0;
            double cost = sales * costRate;
            double profit = sales - cost;
            double profitRate = (profit / sales) * 100;

            QStringList row;
            row << currentDate.toString("MM-dd")
                << QString::number(orders)
                << QString("¥%1").arg(sales, 0, 'f', 2)
                << QString("¥%1").arg(cost, 0, 'f', 2)
                << QString("¥%1").arg(profit, 0, 'f', 2)
                << QString("%1%").arg(profitRate, 0, 'f', 1);

            data.append(row);
            currentDate = currentDate.addDays(1);
            days++;
        }

        return data;
    }

    QList<QStringList> ReportWidget::generateInventoryData()
    {
        QList<QStringList> data;

        QStringList products;
        products << "红楼梦" << "三国演义" << "西游记" << "史记"
                 << "时间简史" << "人类简史" << "教育学原理" << "艺术的故事"
                 << "经济学原理" << "心理学与生活";

        for (int i = 0; i < products.size(); i++) {
            int stock = 5 + qrand() % 96;
            int safeStock = 20;
            int turnDays = 15 + qrand() % 46;

            QString status, suggestion;

            if (stock == 0) {
                status = "紧急";
                suggestion = "立即采购";
            } else if (stock < 5) {
                status = "紧急";
                suggestion = "紧急补货";
            } else if (stock < safeStock) {
                status = "预警";
                suggestion = "建议采购";
            } else {
                status = "正常";
                suggestion = "库存充足";
            }

            QStringList row;
            row << products[i]
                << QString::number(stock)
                << QString::number(safeStock)
                << status
                << QString("%1天").arg(turnDays)
                << suggestion;

            data.append(row);
        }

        return data;
    }

    QList<QStringList> ReportWidget::generateMemberData()
    {
        QList<QStringList> data;

        // 钻石会员
        QStringList diamond;
        diamond << "钻石"
                << QString::number(25 + qrand() % 26)
                << QString("¥%1").arg(50000 + qrand() % 50001, 0, 'f', 2)
                << QString("¥%1").arg(1500 + qrand() % 1501, 0, 'f', 2)
                << QString("%1%").arg(75 + qrand() % 16, 0, 'f', 1);
        data.append(diamond);

        // 黄金会员
        QStringList gold;
        gold << "黄金"
             << QString::number(50 + qrand() % 51)
             << QString("¥%1").arg(30000 + qrand() % 30001, 0, 'f', 2)
             << QString("¥%1").arg(800 + qrand() % 801, 0, 'f', 2)
             << QString("%1%").arg(60 + qrand() % 21, 0, 'f', 1);
        data.append(gold);

        // 白银会员
        QStringList silver;
        silver << "白银"
               << QString::number(100 + qrand() % 101)
               << QString("¥%1").arg(20000 + qrand() % 20001, 0, 'f', 2)
               << QString("¥%1").arg(400 + qrand() % 401, 0, 'f', 2)
               << QString("%1%").arg(45 + qrand() % 21, 0, 'f', 1);
        data.append(silver);
        // 普通会员
          QStringList normal;
          normal << "普通"
                 << QString::number(200 + qrand() % 201)
                 << QString("¥%1").arg(10000 + qrand() % 10001, 0, 'f', 2)
                 << QString("¥%1").arg(150 + qrand() % 151, 0, 'f', 2)
                 << QString("%1%").arg(30 + qrand() % 21, 0, 'f', 1);
          data.append(normal);

          return data;
      }

      // ========== 表格双击事件 ==========

      void ReportWidget::onSalesTableDoubleClick(int row, int column)
      {
          Q_UNUSED(column);
          if (row >= 0 && row < salesTable->rowCount()) {
              QString date = salesTable->item(row, 0)->text();
              QString sales = salesTable->item(row, 2)->text();
              QString profit = salesTable->item(row, 4)->text();
              QString rate = salesTable->item(row, 5)->text();

              QMessageBox::information(this, "销售详情",
                  QString("日期: %1\n销售额: %2\n利润: %3\n毛利率: %4")
                      .arg(date).arg(sales).arg(profit).arg(rate));
          }
      }

      void ReportWidget::onInventoryTableDoubleClick(int row, int column)
      {
          Q_UNUSED(column);
          if (row >= 0 && row < inventoryTable->rowCount()) {
              QString product = inventoryTable->item(row, 0)->text();
              QString stock = inventoryTable->item(row, 1)->text();
              QString status = inventoryTable->item(row, 3)->text();
              QString suggestion = inventoryTable->item(row, 5)->text();

              QMessageBox::information(this, "库存详情",
                  QString("商品: %1\n当前库存: %2\n状态: %3\n建议: %4")
                      .arg(product).arg(stock).arg(status).arg(suggestion));
          }
      }

      void ReportWidget::onMemberTableDoubleClick(int row, int column)
      {
          Q_UNUSED(column);
          if (row >= 0 && row < memberTable->rowCount()) {
              QString level = memberTable->item(row, 0)->text();
              QString count = memberTable->item(row, 1)->text();
              QString total = memberTable->item(row, 2)->text();
              QString average = memberTable->item(row, 3)->text();

              QMessageBox::information(this, "会员详情",
                  QString("等级: %1\n会员数: %2\n消费总额: %3\n平均消费: %4")
                      .arg(level).arg(count).arg(total).arg(average));
          }
      }
