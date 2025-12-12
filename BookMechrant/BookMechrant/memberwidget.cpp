#include "MemberWidget.h"
#include <QHeaderView>
#include <QFormLayout>
#include <QMessageBox>
#include <QFileDialog>
#include <QDate>
#include <QDateTime>
//#include <QRandomGenerator>
#include <QRadioButton>
#include <QButtonGroup>
#include<QTextEdit>
#include<QProgressDialog>
#include<QCoreApplication>
#include<QThread>
// 颜色常量
namespace {
    const QColor PRIMARY_COLOR(41, 128, 185);   // 主蓝
    const QColor DIAMOND_COLOR(231, 76, 60);    // 钻石红
    const QColor GOLD_COLOR(241, 196, 15);      // 黄金黄
    const QColor SILVER_COLOR(189, 195, 199);   // 白银灰
    const QColor NORMAL_COLOR(52, 152, 219);    // 普通蓝
    const QColor SUCCESS_COLOR(39, 174, 96);    // 成功绿
}

MemberWidget::MemberWidget(QWidget *parent)
    : QWidget(parent), isEditing(false), editingRow(-1), rechargeDialog(nullptr)
{
    // 设置最小尺寸适应大界面
    setMinimumSize(1200, 700);

    // 主布局 - 三栏布局
    mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(15, 15, 15, 15);
    mainLayout->setSpacing(15);

    // 创建各个组件
    createToolbar();
    createTable();
    createForm();

    // 初始加载数据
    loadMembers();

    // 初始状态
    onTableSelectionChanged();
}

MemberWidget::~MemberWidget()
{
    if (rechargeDialog) {
        delete rechargeDialog;
    }
}

void MemberWidget::loadMembers()
{
    // 清空表格
    memberTable->setRowCount(0);

    // 设置表格行高，确保按钮能完整显示
    memberTable->verticalHeader()->setDefaultSectionSize(80); // 增加行高

    // 使用固定的模拟会员数据
    QList<QStringList> membersData;

    // 钻石会员
    QStringList diamond1;
    diamond1 << "M20230001" << "张三" << "138-0013-8001" << "钻石" << "¥1,500.00"
             << "3800" << "1985-03-15" << "2023-01-10";
    membersData.append(diamond1);

    QStringList diamond2;
    diamond2 << "M20230002" << "李四" << "139-0024-9002" << "钻石" << "¥2,300.00"
             << "5200" << "1990-07-22" << "2023-02-15";
    membersData.append(diamond2);

    // 黄金会员
    QStringList gold1;
    gold1 << "M20230003" << "王五" << "136-0035-6003" << "黄金" << "¥800.00"
          << "2100" << "1992-11-08" << "2023-03-20";
    membersData.append(gold1);

    QStringList gold2;
    gold2 << "M20230004" << "赵六" << "137-0046-7004" << "黄金" << "¥650.00"
          << "1750" << "1988-05-30" << "2023-04-05";
    membersData.append(gold2);

    // 白银会员
    QStringList silver1;
    silver1 << "M20230005" << "钱七" << "135-0057-8005" << "白银" << "¥300.00"
            << "850" << "1979-09-12" << "2023-05-18";
    membersData.append(silver1);

    // 普通会员
    QStringList normal1;
    normal1 << "M20230006" << "孙八" << "134-0068-9006" << "普通" << "¥50.00"
            << "120" << "1995-12-25" << "2023-06-22";
    membersData.append(normal1);

    QStringList normal2;
    normal2 << "M20230007" << "周九" << "133-0079-1007" << "普通" << "¥120.00"
            << "300" << "1993-08-14" << "2023-07-30";
    membersData.append(normal2);

    // 填充表格
    for (int i = 0; i < membersData.size(); i++) {
        const QStringList &fields = membersData[i];
        if (fields.size() >= 8) {
            int row = memberTable->rowCount();
            memberTable->insertRow(row);

            // 填充前8列数据
            for (int col = 0; col < 8; col++) {
                QTableWidgetItem *item = new QTableWidgetItem(fields[col]);
                memberTable->setItem(row, col, item);

                // 设置等级颜色
                if (col == 3) {
                    QString level = fields[col];
                    if (level == "钻石") {
                        item->setForeground(QBrush(DIAMOND_COLOR));
                        item->setFont(QFont("", -1, QFont::Bold));
                    } else if (level == "黄金") {
                        item->setForeground(QBrush(GOLD_COLOR));
                        item->setFont(QFont("", -1, QFont::Bold));
                    } else if (level == "白银") {
                        item->setForeground(QBrush(SILVER_COLOR));
                    } else {
                        item->setForeground(QBrush(NORMAL_COLOR));
                    }
                    item->setTextAlignment(Qt::AlignCenter);
                }

                // 数字列居右对齐
                if (col == 4) { // 余额列
                    item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
                    item->setForeground(QBrush(SUCCESS_COLOR));
                    item->setFont(QFont("", -1, QFont::Bold));
                }

                if (col == 5) { // 积分列
                    item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
                }
            }

            // 操作列 - 创建按钮容器
            // 在 loadMembers() 函数中，修改按钮创建部分：

            // 操作列 - 创建按钮容器
            QWidget *actionWidget = new QWidget;
            actionWidget->setObjectName("actionWidget");

            QHBoxLayout *actionLayout = new QHBoxLayout(actionWidget);
            actionLayout->setContentsMargins(8, 4, 8, 4); // 和新建的一样
            actionLayout->setSpacing(10);

            // 编辑按钮 - 和新建用户一样的设置
            QPushButton *editBtn = new QPushButton("编辑");
            editBtn->setProperty("row", row);
            editBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
            editBtn->setMinimumHeight(28); // 和新建用户一样的28像素
            editBtn->setStyleSheet(
                "QPushButton {"
                "  background-color: #0d6efd;"
                "  color: white;"
                "  border: none;"
                "  border-radius: 4px;"
                "  font-size: 12px;"
                "  font-weight: bold;"
                "  padding: 6px 12px;" // 6px垂直，12px水平
                "  min-width: 60px;" // 60像素宽度
                "}"
                "QPushButton:hover {"
                "  background-color: #0b5ed7;"
                "}"
                "QPushButton:pressed {"
                "  background-color: #0a58ca;"
                "}"
            );

            // 删除按钮 - 和新建用户一样的设置
            QPushButton *deleteBtn = new QPushButton("删除");
            deleteBtn->setProperty("row", row);
            deleteBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
            deleteBtn->setMinimumHeight(28); // 和新建用户一样的28像素
            deleteBtn->setStyleSheet(
                "QPushButton {"
                "  background-color: #dc3545;"
                "  color: white;"
                "  border: none;"
                "  border-radius: 4px;"
                "  font-size: 12px;"
                "  font-weight: bold;"
                "  padding: 6px 12px;" // 6px垂直，12px水平
                "  min-width: 60px;" // 60像素宽度
                "}"
                "QPushButton:hover {"
                "  background-color: #bb2d3b;"
                "}"
                "QPushButton:pressed {"
                "  background-color: #b02a37;"
                "}"
            );

            // 设置按钮容器样式
            actionWidget->setStyleSheet(
                "#actionWidget {"
                "  background-color: transparent;"
                "  border: none;"
                "}"
            );

            actionLayout->addWidget(editBtn);
            actionLayout->addWidget(deleteBtn);
            actionLayout->addStretch();

            connect(editBtn, SIGNAL(clicked()), this, SLOT(onEditMember()));
            connect(deleteBtn, SIGNAL(clicked()), this, SLOT(onDeleteMember()));

            memberTable->setCellWidget(row, 8, actionWidget);
        }
    }

    // 更新按钮状态
    onTableSelectionChanged();
}

// ==================== 界面创建函数 ====================

void MemberWidget::createToolbar()
{
    // 工具栏容器
    toolbar = new QWidget;
    toolbar->setFixedWidth(220);
    toolbar->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);

    QVBoxLayout *toolLayout = new QVBoxLayout(toolbar);
    toolLayout->setContentsMargins(0, 10, 0, 10);
    toolLayout->setSpacing(15);

    // ========== 搜索区域 ==========
    QGroupBox *searchGroup = new QGroupBox("🔍 搜索");
    searchGroup->setStyleSheet(
        "QGroupBox {"
        "  font-weight: bold;"
        "  color: #555;"
        "  border: 1px solid #dee2e6;"
        "  border-radius: 8px;"
        "  margin-top: 10px;"
        "}"
        "QGroupBox::title {"
        "  subcontrol-origin: margin;"
        "  left: 10px;"
        "  padding: 0 5px 0 5px;"
        "}"
    );

    QVBoxLayout *searchLayout = new QVBoxLayout(searchGroup);
    searchLayout->setContentsMargins(15, 25, 15, 15);
    searchLayout->setSpacing(10);

    QLabel *searchLabel = new QLabel("快速搜索:");
    searchLabel->setStyleSheet("color: #495057; font-weight: bold;");

    searchEdit = new QLineEdit;
    searchEdit->setPlaceholderText("姓名、电话、卡号...");
    searchEdit->setStyleSheet(
        "QLineEdit {"
        "  border: 1px solid #ced4da;"
        "  border-radius: 4px;"
        "  padding: 8px;"
        "  font-size: 14px;"
        "}"
        "QLineEdit:focus {"
        "  border-color: #86b7fe;"
        "  outline: none;"
        "}"
    );

    QPushButton *searchButton = new QPushButton("搜索");
    searchButton->setFixedHeight(36);
    searchButton->setStyleSheet(
        "QPushButton {"
        "  background-color: #0d6efd;"
        "  color: white;"
        "  border: none;"
        "  border-radius: 6px;"
        "  font-size: 14px;"
        "  font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "  background-color: #0b5ed7;"
        "}"
    );

    searchLayout->addWidget(searchLabel);
    searchLayout->addWidget(searchEdit);
    searchLayout->addWidget(searchButton);

    // ========== 等级筛选 ==========
    QGroupBox *filterGroup = new QGroupBox("⚖️ 等级筛选");
    filterGroup->setStyleSheet(searchGroup->styleSheet());

    QVBoxLayout *filterLayout = new QVBoxLayout(filterGroup);
    filterLayout->setContentsMargins(15, 25, 15, 15);
    filterLayout->setSpacing(10);

    QLabel *filterLabel = new QLabel("按会员等级:");
    filterLabel->setStyleSheet("color: #495057; font-weight: bold;");

    levelFilter = new QComboBox;
    levelFilter->addItems(QStringList() << "全部" << "普通" << "白银" << "黄金" << "钻石");
    levelFilter->setStyleSheet(
        "QComboBox {"
        "  border: 1px solid #ced4da;"
        "  border-radius: 4px;"
        "  padding: 8px;"
        "  font-size: 14px;"
        "}"
        "QComboBox:focus {"
        "  border-color: #86b7fe;"
        "}"
        "QComboBox::drop-down {"
        "  border: none;"
        "}"
    );

    filterLayout->addWidget(filterLabel);
    filterLayout->addWidget(levelFilter);

    // ========== 操作按钮 ==========
    QGroupBox *actionGroup = new QGroupBox("🛠️ 会员操作");
    actionGroup->setStyleSheet(searchGroup->styleSheet());

    QVBoxLayout *actionLayout = new QVBoxLayout(actionGroup);
    actionLayout->setContentsMargins(15, 25, 15, 15);
    actionLayout->setSpacing(8);

    // 创建操作按钮
    addButton = new QPushButton("➕ 添加会员");
    editButton = new QPushButton("✏️ 编辑会员");
    deleteButton = new QPushButton("🗑️ 删除会员");
    rechargeButton = new QPushButton("💳 会员充值");
    messageButton = new QPushButton("📨 发送消息");
    exportButton = new QPushButton("📤 导出数据");
    refreshButton = new QPushButton("🔄 刷新数据");

    // 设置按钮样式
    QString baseButtonStyle =
        "QPushButton {"
        "  padding: 12px 15px;"
        "  border-radius: 8px;"
        "  font-size: 14px;"
        "  text-align: left;"
        "  border: 1px solid #dee2e6;"
        "  background-color: white;"
        "  margin-bottom: 5px;"
        "}"
        "QPushButton:hover {"
        "  background-color: #f8f9fa;"
        "  border-color: #adb5bd;"
        "  transform: translateY(-1px);"
        "}"
        "QPushButton:pressed {"
        "  background-color: #e9ecef;"
        "  transform: translateY(0);"
        "}";

    addButton->setStyleSheet(baseButtonStyle +
        "color: #0d6efd;"
        "font-weight: bold;"
        "border-left: 4px solid #0d6efd;"
    );

    editButton->setStyleSheet(baseButtonStyle +
        "color: #6c757d;"
        "border-left: 4px solid #6c757d;"
    );

    deleteButton->setStyleSheet(baseButtonStyle +
        "color: #dc3545;"
        "border-left: 4px solid #dc3545;"
    );

    rechargeButton->setStyleSheet(baseButtonStyle +
        "color: #198754;"
        "border-left: 4px solid #198754;"
    );

    messageButton->setStyleSheet(baseButtonStyle +
        "color: #6f42c1;"
        "border-left: 4px solid #6f42c1;"
    );

    exportButton->setStyleSheet(baseButtonStyle +
        "color: #fd7e14;"
        "border-left: 4px solid #fd7e14;"
    );

    refreshButton->setStyleSheet(baseButtonStyle +
        "color: #17a2b8;"
        "border-left: 4px solid #17a2b8;"
    );

    // 添加到布局
    actionLayout->addWidget(addButton);
    actionLayout->addWidget(editButton);
    actionLayout->addWidget(deleteButton);
    actionLayout->addWidget(rechargeButton);
    actionLayout->addWidget(messageButton);
    actionLayout->addWidget(exportButton);
    actionLayout->addWidget(refreshButton);

    // ========== 添加到工具栏 ==========
    toolLayout->addWidget(searchGroup);
    toolLayout->addWidget(filterGroup);
    toolLayout->addWidget(actionGroup);
    toolLayout->addStretch();

    // 添加到主布局
    mainLayout->addWidget(toolbar);

    // ========== 连接信号 ==========
        connect(addButton, SIGNAL(clicked()), this, SLOT(onAddMember()));
        connect(editButton, SIGNAL(clicked()), this, SLOT(onEditMember()));
        connect(deleteButton, SIGNAL(clicked()), this, SLOT(onDeleteMember()));
        connect(rechargeButton, SIGNAL(clicked()), this, SLOT(onRecharge()));
        connect(messageButton, SIGNAL(clicked()), this, SLOT(onSendMessage()));
        connect(exportButton, SIGNAL(clicked()), this, SLOT(onExportMembers()));
        connect(refreshButton, SIGNAL(clicked()), this, SLOT(onRefresh()));
        connect(searchButton, SIGNAL(clicked()), this, SLOT(onSearch()));
        connect(searchEdit, SIGNAL(returnPressed()), this, SLOT(onSearch()));
        connect(levelFilter, SIGNAL(currentIndexChanged(int)), this, SLOT(onLevelFilterChanged(int)));
    }

    void MemberWidget::createTable()
    {
        // 表格容器
        QWidget *tableContainer = new QWidget;
        tableContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        QVBoxLayout *tableLayout = new QVBoxLayout(tableContainer);
        tableLayout->setContentsMargins(0, 0, 0, 0);

        // 创建表格
        memberTable = new QTableWidget;
        memberTable->setObjectName("memberTable");
        memberTable->setColumnCount(9);

        // 设置表头
        QStringList headers;
        headers << "会员卡号" << "姓名" << "电话" << "等级" << "余额"
                << "积分" << "生日" << "注册时间" << "操作";
        memberTable->setHorizontalHeaderLabels(headers);

        // 获取表头
        QHeaderView *header = memberTable->horizontalHeader();
        header->setStretchLastSection(true); // 最后一列拉伸

        // 设置表格属性
        memberTable->setAlternatingRowColors(true);
        memberTable->setSelectionBehavior(QAbstractItemView::SelectRows);
        memberTable->setSelectionMode(QAbstractItemView::SingleSelection);
        memberTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
        memberTable->verticalHeader()->setVisible(false);
        memberTable->setSortingEnabled(true);

        // 设置列宽

            // ... 前面的代码保持不变 ...

            // 设置列宽
            memberTable->setColumnWidth(0, 120);  // 会员卡号
            memberTable->setColumnWidth(1, 100);  // 姓名
            memberTable->setColumnWidth(2, 130);  // 电话
            memberTable->setColumnWidth(3, 80);   // 等级
            memberTable->setColumnWidth(4, 90);   // 余额
            memberTable->setColumnWidth(5, 80);   // 积分
            memberTable->setColumnWidth(6, 100);  // 生日
            memberTable->setColumnWidth(7, 120);  // 注册时间
            // 注意：操作列不要设置固定宽度，让最后一列自动拉伸

            // 设置表头样式
            header->setMinimumHeight(45);
            header->setDefaultAlignment(Qt::AlignCenter);

            // 设置表格样式 - 修复选择样式
            memberTable->setStyleSheet(
                "#memberTable {"
                "  border: 1px solid #dee2e6;"
                "  border-radius: 8px;"
                "  background-color: white;"
                "  font-size: 13px;"
                "  gridline-color: #f0f0f0;"
                "}"
                "#memberTable::item {"
                "  padding: 12px 8px;"
                "  border-bottom: 1px solid #f0f0f0;"
                "}"
                "QTableWidget::item:selected {"
                "  background-color: #e3f2fd;"
                "  color: #000;"
                "}"
                "QHeaderView::section {"
                "  background-color: #f8f9fa;"
                "  padding: 12px 8px;"
                "  border: none;"
                "  border-right: 1px solid #dee2e6;"
                "  border-bottom: 2px solid #0d6efd;"
                "  font-weight: bold;"
                "  color: #495057;"
                "  font-size: 12px;"
                "  font-family: 'Microsoft YaHei', Arial, sans-serif;"
                "}"
                "QHeaderView::section:last {"
                "  border-right: none;"
                "}"
            );

            // ... 后面的代码保持不变 ...

        tableLayout->addWidget(memberTable);
        mainLayout->addWidget(tableContainer, 1);

        // 连接选择变化信号
        connect(memberTable, SIGNAL(itemSelectionChanged()),
                this, SLOT(onTableSelectionChanged()));
    }

    void MemberWidget::createForm()
    {
        // 编辑表单组
        formGroup = new QGroupBox("📝 会员信息编辑");
        formGroup->setFixedWidth(350);
        formGroup->setObjectName("formGroup");

        QVBoxLayout *formLayout = new QVBoxLayout(formGroup);
        formLayout->setContentsMargins(20, 25, 20, 20);
        formLayout->setSpacing(18);

        // ========== 表单字段 ==========
        QFormLayout *fieldLayout = new QFormLayout;
        fieldLayout->setSpacing(15);
        fieldLayout->setContentsMargins(0, 0, 0, 0);
        fieldLayout->setLabelAlignment(Qt::AlignRight);

        // 创建表单控件
        cardEdit = new QLineEdit;
        cardEdit->setPlaceholderText("自动生成");
        cardEdit->setReadOnly(true);
        cardEdit->setStyleSheet("background-color: #f8f9fa;");

        nameEdit = new QLineEdit;
        nameEdit->setPlaceholderText("请输入会员姓名");
        nameEdit->setMaxLength(20);

        phoneEdit = new QLineEdit;
        phoneEdit->setPlaceholderText("例如：138-0000-0000");
        phoneEdit->setInputMask("000-0000-0000");

        birthdayEdit = new QDateEdit;
        birthdayEdit->setDisplayFormat("yyyy-MM-dd");
        birthdayEdit->setDate(QDate::currentDate().addYears(-25));
        birthdayEdit->setCalendarPopup(true);
        birthdayEdit->setMaximumDate(QDate::currentDate());

        levelCombo = new QComboBox;
        levelCombo->addItems(QStringList() << "普通" << "白银" << "黄金" << "钻石");

        balanceEdit = new QDoubleSpinBox;
        balanceEdit->setRange(0, 999999.99);
        balanceEdit->setDecimals(2);
        balanceEdit->setPrefix("¥ ");
        balanceEdit->setValue(0);

        pointsEdit = new QSpinBox;
        pointsEdit->setRange(0, 999999);
        pointsEdit->setValue(0);
        // 添加到表单布局
           fieldLayout->addRow("会员卡号:", cardEdit);
           fieldLayout->addRow("姓名*:", nameEdit);
           fieldLayout->addRow("电话*:", phoneEdit);
           fieldLayout->addRow("生日:", birthdayEdit);
           fieldLayout->addRow("等级:", levelCombo);
           fieldLayout->addRow("余额:", balanceEdit);
           fieldLayout->addRow("积分:", pointsEdit);

           // 设置标签样式
           for (int i = 0; i < fieldLayout->rowCount(); ++i) {
               QLabel *label = qobject_cast<QLabel*>(fieldLayout->itemAt(i, QFormLayout::LabelRole)->widget());
               if (label) {
                   label->setStyleSheet("color: #495057; font-weight: bold; min-width: 70px;");
               }
           }

           // ========== 按钮区域 ==========
           QHBoxLayout *buttonLayout = new QHBoxLayout;
           buttonLayout->setSpacing(15);

           saveButton = new QPushButton("💾 保存");
           cancelButton = new QPushButton("取消");

           saveButton->setFixedWidth(120);
           cancelButton->setFixedWidth(100);

           saveButton->setStyleSheet(
               "QPushButton {"
               "  background-color: #0d6efd;"
               "  color: white;"
               "  border: none;"
               "  border-radius: 6px;"
               "  padding: 12px;"
               "  font-size: 14px;"
               "  font-weight: bold;"
               "}"
               "QPushButton:hover {"
               "  background-color: #0b5ed7;"
               "}"
               "QPushButton:disabled {"
               "  background-color: #6c757d;"
               "}"
           );

           cancelButton->setStyleSheet(
               "QPushButton {"
               "  background-color: #6c757d;"
               "  color: white;"
               "  border: none;"
               "  border-radius: 6px;"
               "  padding: 12px;"
               "  font-size: 14px;"
               "}"
               "QPushButton:hover {"
               "  background-color: #5c636a;"
               "}"
           );

           buttonLayout->addStretch();
           buttonLayout->addWidget(saveButton);
           buttonLayout->addWidget(cancelButton);

           // ========== 添加到表单 ==========
           formLayout->addLayout(fieldLayout);
           formLayout->addStretch();
           formLayout->addLayout(buttonLayout);

           // 设置表单组样式
           formGroup->setStyleSheet(
               "#formGroup {"
               "  background-color: white;"
               "  border-radius: 8px;"
               "  border: 1px solid #dee2e6;"
               "}"
               "QLineEdit, QComboBox, QSpinBox, QDoubleSpinBox, QDateEdit {"
               "  border: 1px solid #ced4da;"
               "  border-radius: 4px;"
               "  padding: 10px;"
               "  font-size: 14px;"
               "  min-height: 20px;"
               "}"
               "QLineEdit:focus, QComboBox:focus, QSpinBox:focus, QDoubleSpinBox:focus, QDateEdit:focus {"
               "  border-color: #86b7fe;"
               "  outline: none;"
               "}"
               "QComboBox::drop-down {"
               "  border: none;"
               "}"
           );

           // 添加到主布局
           mainLayout->addWidget(formGroup);

           // 连接信号
           connect(saveButton, SIGNAL(clicked()), this, SLOT(onSaveMember()));
           connect(cancelButton, SIGNAL(clicked()), this, SLOT(onCancelEdit()));
       }
    // ==================== 数据加载函数 ====================


    // ==================== 槽函数实现 ====================

    void MemberWidget::onAddMember()
    {
        isEditing = false;
        editingRow = -1;
        clearForm();

        // 生成会员卡号
        QString cardNo = generateMemberCard();
        cardEdit->setText(cardNo);

        nameEdit->setFocus();

        QMessageBox::information(this, "添加会员",
            "请填写会员信息，然后点击保存。\n会员卡号已自动生成。");
    }

    void MemberWidget::onEditMember()
    {
        QPushButton *senderBtn = qobject_cast<QPushButton*>(sender());
        int row;

        if (senderBtn) {
            // 从按钮获取行号
            row = senderBtn->property("row").toInt();
        } else {
            // 从当前选择获取行号
            row = memberTable->currentRow();
        }

        if (row >= 0 && row < memberTable->rowCount()) {
            isEditing = true;
            editingRow = row;
            populateForm(row);

            QMessageBox::information(this, "编辑会员",
                "请修改会员信息，然后点击保存。");
        } else {
            QMessageBox::warning(this, "警告", "请先选择要编辑的会员！");
        }
    }

    void MemberWidget::onDeleteMember()
    {
        int row = memberTable->currentRow();
        if (row >= 0) {
            QString memberName = memberTable->item(row, 1)->text();
            QString memberCard = memberTable->item(row, 0)->text();

            QMessageBox::StandardButton reply;
            reply = QMessageBox::question(this, "确认删除",
                QString("确定要删除会员【%1】吗？\n会员卡号：%2")
                .arg(memberName).arg(memberCard),
                QMessageBox::Yes | QMessageBox::No);

            if (reply == QMessageBox::Yes) {
                memberTable->removeRow(row);
                clearForm();
                QMessageBox::information(this, "删除成功", "会员已删除！");
            }
        } else {
            QMessageBox::warning(this, "警告", "请先选择要删除的会员！");
        }
    }

    void MemberWidget::onRecharge()
    {
        int row = memberTable->currentRow();
        if (row >= 0) {
            QString memberCard = memberTable->item(row, 0)->text();
            QString memberName = memberTable->item(row, 1)->text();
            QString balanceText = memberTable->item(row, 4)->text();

            // 提取数字部分
            QString cleanBalance = balanceText;
            cleanBalance.remove('¥').remove(',').remove(' ');
            double currentBalance = cleanBalance.toDouble();

            // 创建并显示充值对话框
            if (!rechargeDialog) {
                rechargeDialog = new RechargeDialog(memberName, currentBalance, this);
                connect(rechargeDialog, SIGNAL(rechargeRequested(double, const QString&)),
                        this, SLOT(onRechargeComplete(double, const QString&)));
            } else {
                rechargeDialog->setWindowTitle(QString("会员充值 - %1").arg(memberName));
            }

            rechargeDialog->exec();
        } else {
            QMessageBox::warning(this, "警告", "请先选择要充值的会员！");
        }
    }

    void MemberWidget::onRechargeComplete(double amount, const QString &paymentMethod)
    {
        int row = memberTable->currentRow();
        if (row >= 0) {
            QString memberCard = memberTable->item(row, 0)->text();

            // 更新表格中的余额
            QString oldBalanceText = memberTable->item(row, 4)->text();
            QString cleanBalance = oldBalanceText;
            cleanBalance.remove('¥').remove(',').remove(' ');
            double oldBalance = cleanBalance.toDouble();
            double newBalance = oldBalance + amount;

            // 更新余额显示
            QString newBalanceText = QString("¥%1").arg(newBalance, 0, 'f', 2);
            memberTable->item(row, 4)->setText(newBalanceText);

            // 显示充值成功消息
            QMessageBox::information(this, "充值成功",
                QString("会员充值成功！\n\n"
                       "充值金额：¥%1\n"
                       "支付方式：%2\n"
                       "会员卡号：%3\n"
                       "当前余额：¥%4")
                .arg(amount, 0, 'f', 2)
                .arg(paymentMethod)
                .arg(memberCard)
                .arg(newBalance, 0, 'f', 2));
        }
    }

    void MemberWidget::onSearch()
    {
        QString keyword = searchEdit->text().trimmed().toLower();
        QString level = levelFilter->currentText();

        int visibleCount = 0;

        for (int row = 0; row < memberTable->rowCount(); row++) {
            bool match = true;

            // 等级筛选
            if (level != "全部") {
                QString memberLevel = memberTable->item(row, 3)->text();
                if (memberLevel != level) {
                    match = false;
                }
            }

            // 关键词搜索
            if (!keyword.isEmpty() && match) {
                bool found = false;

                // 搜索姓名、电话、卡号
                for (int col = 0; col < 3; col++) {
                    QString cellText = memberTable->item(row, col)->text().toLower();
                    if (cellText.contains(keyword)) {
                        found = true;
                        break;
                    }
                }

                match = found;
            }

            // 显示/隐藏行
            memberTable->setRowHidden(row, !match);
            if (match) visibleCount++;
        }

        // 显示搜索结果统计
        if (!keyword.isEmpty() || level != "全部") {
            QMessageBox::information(this, "搜索结果",
                QString("找到 %1 位符合条件的会员").arg(visibleCount));
        }
    }

    void MemberWidget::onSendMessage()
    {
        int row = memberTable->currentRow();
        if (row >= 0) {
            QString memberName = memberTable->item(row, 1)->text();
            QString memberPhone = memberTable->item(row, 2)->text();

            // 创建发送消息对话框
            QDialog messageDialog(this);
            messageDialog.setWindowTitle("发送消息");
            messageDialog.resize(500, 400);

            QVBoxLayout *layout = new QVBoxLayout(&messageDialog);
            layout->setContentsMargins(20, 20, 20, 20);
            layout->setSpacing(15);
            // 标题
                    QLabel *titleLabel = new QLabel("📨 发送会员消息");
                    titleLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #333;");
                    titleLabel->setAlignment(Qt::AlignCenter);

                    // 收件人信息
                    QGroupBox *receiverGroup = new QGroupBox("收件人信息");
                    QFormLayout *receiverLayout = new QFormLayout(receiverGroup);

                    QLabel *nameLabel = new QLabel(memberName);
                    QLabel *phoneLabel = new QLabel(memberPhone);
                    nameLabel->setStyleSheet("font-weight: bold; color: #0d6efd;");
                    phoneLabel->setStyleSheet("color: #495057;");

                    receiverLayout->addRow("会员姓名:", nameLabel);
                    receiverLayout->addRow("联系电话:", phoneLabel);

                    // 消息内容
                    QLabel *contentLabel = new QLabel("消息内容:");
                    contentLabel->setStyleSheet("font-weight: bold;");

                    QTextEdit *messageEdit = new QTextEdit;
                    messageEdit->setPlaceholderText("请输入要发送的消息内容...");
                    messageEdit->setMinimumHeight(150);

                    // 消息类型
                    QLabel *typeLabel = new QLabel("消息类型:");
                    typeLabel->setStyleSheet("font-weight: bold;");

                    QComboBox *typeCombo = new QComboBox;
                    typeCombo->addItems(QStringList() << "促销通知" << "生日祝福" << "账户提醒" << "系统通知" << "其他");

                    // 按钮
                    QHBoxLayout *buttonLayout = new QHBoxLayout;
                    QPushButton *sendButton = new QPushButton("📤 发送消息");
                    QPushButton *cancelButton = new QPushButton("取消");

                    sendButton->setStyleSheet(
                        "QPushButton {"
                        "  background-color: #9b59b6;"
                        "  color: white;"
                        "  border: none;"
                        "  border-radius: 6px;"
                        "  padding: 10px 25px;"
                        "  font-size: 14px;"
                        "  font-weight: bold;"
                        "}"
                        "QPushButton:hover {"
                        "  background-color: #8e44ad;"
                        "}"
                    );

                    cancelButton->setStyleSheet(
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

                    buttonLayout->addStretch();
                    buttonLayout->addWidget(cancelButton);
                    buttonLayout->addWidget(sendButton);

                    // 添加到布局
                    layout->addWidget(titleLabel);
                    layout->addWidget(receiverGroup);
                    layout->addWidget(contentLabel);
                    layout->addWidget(messageEdit, 1);
                    layout->addWidget(typeLabel);
                    layout->addWidget(typeCombo);
                    layout->addLayout(buttonLayout);
                    // 连接按钮
                            connect(sendButton, SIGNAL(clicked()), &messageDialog, SLOT(accept()));
                            connect(cancelButton, SIGNAL(clicked()), &messageDialog, SLOT(reject()));

                            if (messageDialog.exec() == QDialog::Accepted) {
                                QString message = messageEdit->toPlainText().trimmed();
                                QString messageType = typeCombo->currentText();

                                if (!message.isEmpty()) {
                                    QMessageBox::information(this, "发送成功",
                                        QString("消息已发送给 %1！\n\n"
                                               "消息类型：%2\n"
                                               "消息内容：%3")
                                        .arg(memberName)
                                        .arg(messageType)
                                        .arg(message.left(50) + (message.length() > 50 ? "..." : "")));
                                }
                            }
                        } else {
                            QMessageBox::warning(this, "警告", "请先选择要发送消息的会员！");
                        }
                    }

    void MemberWidget::onExportMembers()
    {
        QString defaultName = QString("会员数据_%1.xlsx")
                             .arg(QDateTime::currentDateTime().toString("yyyyMMdd"));

        QString fileName = QFileDialog::getSaveFileName(this,
            "导出会员数据", defaultName,
            "Excel文件 (*.xlsx *.xls);;CSV文件 (*.csv);;文本文件 (*.txt)");

        if (!fileName.isEmpty()) {
            int totalMembers = memberTable->rowCount();

            // 创建进度对话框
            QProgressDialog progress("正在导出数据...", "取消", 0, totalMembers, this);
            progress.setWindowTitle("导出进度");
            progress.setWindowModality(Qt::WindowModal);

            // 模拟导出过程（不使用随机延时）
            for (int i = 0; i < totalMembers; i++) {
                progress.setValue(i);
                QCoreApplication::processEvents(); // 处理事件，保持界面响应

                if (progress.wasCanceled()) {
                    break;
                }

                // 模拟处理每条记录的时间（固定延时）
                // 如果需要更真实，可以计算导出到文件的实际操作
                // 这里为了演示，使用固定延时
                QThread::msleep(10); // 10毫秒固定延时
            }

            progress.setValue(totalMembers);

            QMessageBox::information(this, "导出成功",
                QString("已成功导出 %1 位会员数据到：\n%2")
                .arg(totalMembers)
                .arg(fileName));
        }
    }

                    void MemberWidget::onRefresh()
                    {
                        loadMembers();
                        searchEdit->clear();
                        levelFilter->setCurrentIndex(0);

                        // 显示所有行
                        for (int row = 0; row < memberTable->rowCount(); row++) {
                            memberTable->setRowHidden(row, false);
                        }

                        QMessageBox::information(this, "刷新完成", "会员数据已刷新！");
                    }

                    void MemberWidget::onSaveMember()
                    {
                        if (!validateMemberData()) {
                            return;
                        }

                        if (isEditing && editingRow >= 0) {
                            // 更新现有会员
                            updateMemberInTable(editingRow);
                            QMessageBox::information(this, "保存成功", "会员信息已更新！");
                        } else {
                            // 添加新会员
                            addMemberToTable();
                            QMessageBox::information(this, "保存成功", "新会员已添加！");
                        }

                        isEditing = false;
                        editingRow = -1;
                        onTableSelectionChanged();
                    }

                    void MemberWidget::onCancelEdit()
                    {
                        if (isEditing) {
                            QMessageBox::StandardButton reply;
                            reply = QMessageBox::question(this, "取消编辑",
                                "确定要取消编辑吗？所有未保存的修改将丢失。",
                                QMessageBox::Yes | QMessageBox::No);

                            if (reply == QMessageBox::Yes) {
                                clearForm();
                                isEditing = false;
                                editingRow = -1;
                                onTableSelectionChanged();
                                QMessageBox::information(this, "取消编辑", "已取消编辑操作。");
                            }
                        } else {
                            QMessageBox::StandardButton reply;
                            reply = QMessageBox::question(this, "清空表单",
                                "确定要清空当前表单内容吗？",
                                QMessageBox::Yes | QMessageBox::No);

                            if (reply == QMessageBox::Yes) {
                                clearForm();
                                QString newCardNo = generateMemberCard();
                                cardEdit->setText(newCardNo);
                                QMessageBox::information(this, "清空表单", "表单已清空。");
                            }
                        }
                    }
                    void MemberWidget::onTableSelectionChanged()
                    {
                        bool hasSelection = memberTable->currentRow() >= 0;

                        editButton->setEnabled(hasSelection);
                        deleteButton->setEnabled(hasSelection);
                        rechargeButton->setEnabled(hasSelection);
                        messageButton->setEnabled(hasSelection);

                        // 如果没有选择，清空表单
                        if (!hasSelection && !isEditing) {
                            clearForm();
                        }
                    }

                    void MemberWidget::onLevelFilterChanged(int index)
                    {
                        onSearch(); // 直接调用搜索函数
                    }

                    // ==================== 辅助函数 ====================

                    void MemberWidget::clearForm()
                    {
                        nameEdit->clear();
                        phoneEdit->clear();
                        birthdayEdit->setDate(QDate::currentDate().addYears(-25));
                        levelCombo->setCurrentIndex(0);
                        balanceEdit->setValue(0);
                        pointsEdit->setValue(0);
                    }

                    void MemberWidget::populateForm(int row)
                    {
                        if (row < 0 || row >= memberTable->rowCount()) return;

                        // 会员卡号
                        cardEdit->setText(memberTable->item(row, 0)->text());

                        // 姓名
                        nameEdit->setText(memberTable->item(row, 1)->text());

                        // 电话
                        phoneEdit->setText(memberTable->item(row, 2)->text());

                        // 等级
                        QString level = memberTable->item(row, 3)->text();
                        int levelIndex = levelCombo->findText(level);
                        if (levelIndex >= 0) {
                            levelCombo->setCurrentIndex(levelIndex);
                        }

                        // 余额（移除¥符号和逗号）
                        QString balanceStr = memberTable->item(row, 4)->text();
                        balanceStr.remove('¥').remove(',').remove(' ');
                        balanceEdit->setValue(balanceStr.toDouble());

                        // 积分
                        pointsEdit->setValue(memberTable->item(row, 5)->text().toInt());
                    }

                    bool MemberWidget::validateMemberData()
                    {
                        // 获取表单数据
                        QString name = nameEdit->text().trimmed();
                        QString phone = phoneEdit->text().trimmed();

                        // 验证姓名
                        if (name.isEmpty()) {
                            QMessageBox::warning(this, "验证错误", "会员姓名不能为空！");
                            nameEdit->setFocus();
                            return false;
                        }

                        if (name.length() < 2 || name.length() > 20) {
                            QMessageBox::warning(this, "验证错误", "姓名长度应为2-20个字符！");
                            nameEdit->setFocus();
                            return false;
                        }

                        // 验证电话
                        if (phone.isEmpty()) {
                            QMessageBox::warning(this, "验证错误", "联系电话不能为空！");
                            phoneEdit->setFocus();
                            return false;
                        }

                        // 验证电话格式
                        if (!phone.contains("-") || phone.length() != 13) {
                            QMessageBox::warning(this, "验证错误", "电话格式不正确，应为：138-0000-0000");
                            phoneEdit->setFocus();
                            return false;
                        }

                        // 验证生日
                        QDate birthday = birthdayEdit->date();
                        if (birthday > QDate::currentDate()) {
                            QMessageBox::warning(this, "验证错误", "生日不能晚于今天！");
                            birthdayEdit->setFocus();
                            return false;
                        }

                        // 验证余额
                        if (balanceEdit->value() < 0) {
                            QMessageBox::warning(this, "验证错误", "余额不能为负数！");
                            balanceEdit->setFocus();
                            return false;
                        }

                        // 验证积分
                        if (pointsEdit->value() < 0) {
                            QMessageBox::warning(this, "验证错误", "积分不能为负数！");
                            pointsEdit->setFocus();
                            return false;
                        }

                        return true;
                    }

                    void MemberWidget::updateMemberInTable(int row)
                    {
                        // 更新表格数据
                        memberTable->item(row, 0)->setText(cardEdit->text());
                        memberTable->item(row, 1)->setText(nameEdit->text().trimmed());
                        memberTable->item(row, 2)->setText(phoneEdit->text());
                        memberTable->item(row, 3)->setText(levelCombo->currentText());
                        // 更新余额
                            QString balanceText = QString("¥%1").arg(balanceEdit->value(), 0, 'f', 2);
                            memberTable->item(row, 4)->setText(balanceText);
                            memberTable->item(row, 4)->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
                            memberTable->item(row, 4)->setForeground(QBrush(SUCCESS_COLOR));
                            memberTable->item(row, 4)->setFont(QFont("", -1, QFont::Bold));

                            // 更新积分
                            memberTable->item(row, 5)->setText(QString::number(pointsEdit->value()));
                            memberTable->item(row, 5)->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);

                            // 更新生日
                            memberTable->item(row, 6)->setText(birthdayEdit->date().toString("yyyy-MM-dd"));

                            // 更新等级颜色
                            QTableWidgetItem *levelItem = memberTable->item(row, 3);
                            QString level = levelCombo->currentText();
                            if (level == "钻石") {
                                levelItem->setForeground(QBrush(DIAMOND_COLOR));
                                levelItem->setFont(QFont("", -1, QFont::Bold));
                            } else if (level == "黄金") {
                                levelItem->setForeground(QBrush(GOLD_COLOR));
                                levelItem->setFont(QFont("", -1, QFont::Bold));
                            } else if (level == "白银") {
                                levelItem->setForeground(QBrush(SILVER_COLOR));
                            } else {
                                levelItem->setForeground(QBrush(NORMAL_COLOR));
                            }
                        }

                        void MemberWidget::addMemberToTable()
                        {
                            // 获取表单数据
                            QString cardNo = cardEdit->text().trimmed();
                            QString name = nameEdit->text().trimmed();
                            QString phone = phoneEdit->text();
                            QString level = levelCombo->currentText();
                            double balance = balanceEdit->value();
                            int points = pointsEdit->value();
                            QString birthday = birthdayEdit->date().toString("yyyy-MM-dd");
                            QString registerDate = QDate::currentDate().toString("yyyy-MM-dd");

                            // 在表格末尾插入新行
                            int row = memberTable->rowCount();
                            memberTable->insertRow(row);

                            // 填充表格数据
                            QTableWidgetItem *cardItem = new QTableWidgetItem(cardNo);
                            memberTable->setItem(row, 0, cardItem);

                            QTableWidgetItem *nameItem = new QTableWidgetItem(name);
                            memberTable->setItem(row, 1, nameItem);

                            QTableWidgetItem *phoneItem = new QTableWidgetItem(phone);
                            memberTable->setItem(row, 2, phoneItem);

                            QTableWidgetItem *levelItem = new QTableWidgetItem(level);
                            if (level == "钻石") {
                                levelItem->setForeground(QBrush(DIAMOND_COLOR));
                                levelItem->setFont(QFont("", -1, QFont::Bold));
                            } else if (level == "黄金") {
                                levelItem->setForeground(QBrush(GOLD_COLOR));
                                levelItem->setFont(QFont("", -1, QFont::Bold));
                            } else if (level == "白银") {
                                levelItem->setForeground(QBrush(SILVER_COLOR));
                            } else {
                                levelItem->setForeground(QBrush(NORMAL_COLOR));
                            }
                            levelItem->setTextAlignment(Qt::AlignCenter);
                            memberTable->setItem(row, 3, levelItem);

                            // 余额
                            QString balanceText = QString("¥%1").arg(balance, 0, 'f', 2);
                            QTableWidgetItem *balanceItem = new QTableWidgetItem(balanceText);
                            balanceItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
                            balanceItem->setForeground(QBrush(SUCCESS_COLOR));
                            balanceItem->setFont(QFont("", -1, QFont::Bold));
                            memberTable->setItem(row, 4, balanceItem);

                            // 积分
                            QTableWidgetItem *pointsItem = new QTableWidgetItem(QString::number(points));
                            pointsItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
                            memberTable->setItem(row, 5, pointsItem);

                            // 生日
                            QTableWidgetItem *birthdayItem = new QTableWidgetItem(birthday);
                            memberTable->setItem(row, 6, birthdayItem);

                            // 注册时间
                            QTableWidgetItem *registerItem = new QTableWidgetItem(registerDate);
                            memberTable->setItem(row, 7, registerItem);

                            // 操作列 - 创建按钮容器
                            QWidget *actionWidget = new QWidget;
                            actionWidget->setObjectName("actionWidget");

                            QHBoxLayout *actionLayout = new QHBoxLayout(actionWidget);
                            actionLayout->setContentsMargins(8, 4, 8, 4);
                            actionLayout->setSpacing(10);
                            // 编辑按钮
                            QPushButton *editBtn = new QPushButton("编辑");
                            editBtn->setProperty("row", row);
                            editBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
                            editBtn->setMinimumHeight(32);
                            editBtn->setStyleSheet(
                                "QPushButton {"
                                "  background-color: #0d6efd;"
                                "  color: white;"
                                "  border: none;"
                                "  border-radius: 4px;"
                                "  font-size: 12px;"
                                "  font-weight: bold;"
                                "  padding: 6px 12px;"
                                "  min-width: 60px;"
                                "}"
                                "QPushButton:hover {"
                                "  background-color: #0b5ed7;"
                                "}"
                                "QPushButton:pressed {"
                                "  background-color: #0a58ca;"
                                "}"
                            );

                            // 删除按钮
                            QPushButton *deleteBtn = new QPushButton("删除");
                            deleteBtn->setProperty("row", row);
                            deleteBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
                            deleteBtn->setMinimumHeight(32);
                            deleteBtn->setStyleSheet(
                                "QPushButton {"
                                "  background-color: #dc3545;"
                                "  color: white;"
                                "  border: none;"
                                "  border-radius: 4px;"
                                "  font-size: 12px;"
                                "  font-weight: bold;"
                                "  padding: 6px 12px;"
                                "  min-width: 60px;"
                                "}"
                                "QPushButton:hover {"
                                "  background-color: #bb2d3b;"
                                "}"
                                "QPushButton:pressed {"
                                "  background-color: #b02a37;"
                                "}"
                            );

                            actionLayout->addWidget(editBtn);
                            actionLayout->addWidget(deleteBtn);
                            actionLayout->addStretch();

                            connect(editBtn, SIGNAL(clicked()), this, SLOT(onEditMember()));
                            connect(deleteBtn, SIGNAL(clicked()), this, SLOT(onDeleteMember()));

                            memberTable->setCellWidget(row, 8, actionWidget);
                        }

                        int MemberWidget::findMemberByCard(const QString &cardNo)
                        {
                            for (int row = 0; row < memberTable->rowCount(); row++) {
                                QTableWidgetItem *cardItem = memberTable->item(row, 0);
                                if (cardItem && cardItem->text() == cardNo) {
                                    return row;
                                }
                            }
                            return -1;
                        }

                        QString MemberWidget::generateMemberCard()
                        {
                            // 生成格式为：M + 年份 + 月份 + 4位序号
                            QString year = QDate::currentDate().toString("yyyy");
                            QString month = QDate::currentDate().toString("MM");

                            // 查找当天已有的最大序号
                            int maxSeq = 0;
                            QString prefix = QString("M%1%2").arg(year).arg(month);

                            // 查找已有会员的最大序号
                            for (int row = 0; row < memberTable->rowCount(); row++) {
                                QString cardNo = memberTable->item(row, 0)->text();
                                if (cardNo.startsWith(prefix)) {
                                    QString seqStr = cardNo.mid(7); // M202312XXXX
                                    bool ok;
                                    int seq = seqStr.toInt(&ok);
                                    if (ok && seq > maxSeq) {
                                        maxSeq = seq;
                                    }
                                }
                            }

                            // 生成新的序号（递增）
                            int newSeq = maxSeq + 1;
                            return QString("%1%2").arg(prefix).arg(newSeq, 4, 10, QChar('0'));
                        }

                        void MemberWidget::updateMemberBalance(const QString &cardNo, double amount)
                        {
                            int row = findMemberByCard(cardNo);
                            if (row >= 0) {
                                QString oldBalanceText = memberTable->item(row, 4)->text();
                                QString cleanBalance = oldBalanceText;
                                cleanBalance.remove('¥').remove(',').remove(' ');
                                double oldBalance = cleanBalance.toDouble();
                                double newBalance = oldBalance + amount;

                                // 更新表格中的余额
                                QString newBalanceText = QString("¥%1").arg(newBalance, 0, 'f', 2);
                                memberTable->item(row, 4)->setText(newBalanceText);
                                memberTable->item(row, 4)->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
                                memberTable->item(row, 4)->setForeground(QBrush(SUCCESS_COLOR));
                                memberTable->item(row, 4)->setFont(QFont("", -1, QFont::Bold));
                            }
                        }

                        // ==================== RechargeDialog 实现 ====================


                        RechargeDialog::RechargeDialog(const QString &memberName, double currentBalance, QWidget *parent)
                            : QDialog(parent), customAmount(0)
                        {
                            setWindowTitle(QString("会员充值 - %1").arg(memberName));
                            setFixedSize(700, 650);
                            setWindowModality(Qt::ApplicationModal);

                            QVBoxLayout *mainLayout = new QVBoxLayout(this);
                            mainLayout->setContentsMargins(20, 20, 20, 20);
                            mainLayout->setSpacing(15);

                            // 标题
                            QLabel *titleLabel = new QLabel("会员充值");
                            titleLabel->setAlignment(Qt::AlignCenter);
                            titleLabel->setStyleSheet("font-size: 18px; font-weight: bold;");

                            // 会员信息 - 修正：使用传入的会员名
                            QLabel *memberInfoLabel = new QLabel(QString("会员：%1   余额：¥%2")
                                                                .arg(memberName)  // 使用传入的会员名
                                                                .arg(currentBalance, 0, 'f', 2));
                            memberInfoLabel->setStyleSheet("font-size: 14px; padding: 5px;");

                            // 充值金额输入
                            QGroupBox *amountGroup = new QGroupBox("充值金额");
                            QVBoxLayout *amountLayout = new QVBoxLayout(amountGroup);

                            amountEdit = new QLineEdit;
                            amountEdit->setPlaceholderText("输入充值金额");
                            amountEdit->setValidator(new QDoubleValidator(0.01, 999999, 2, this));
                            amountEdit->setStyleSheet("padding: 8px; font-size: 14px; border: 1px solid #ccc;");

                            // 快捷金额按钮
                            QHBoxLayout *quickLayout = new QHBoxLayout;
                            quickLayout->setSpacing(5);

                            QList<int> amounts = {100, 200, 300, 500, 1000};
                            for (int amount : amounts) {
                                QPushButton *btn = new QPushButton(QString("¥%1").arg(amount));
                                btn->setProperty("amount", amount);
                                btn->setStyleSheet("padding: 5px 10px; border: 1px solid #ccc; background-color: white;");
                                quickLayout->addWidget(btn);

                                // 连接快捷按钮
                                connect(btn, &QPushButton::clicked, [=]() {
                                    amountEdit->setText(QString::number(amount));
                                });
                            }

                            quickLayout->addStretch();
                            amountLayout->addWidget(amountEdit);
                            amountLayout->addLayout(quickLayout);

                            // 支付方式选择
                            QGroupBox *paymentGroup = new QGroupBox("支付方式");
                            QVBoxLayout *paymentLayout = new QVBoxLayout(paymentGroup);

                            QComboBox *paymentCombo = new QComboBox;
                            paymentCombo->addItems(QStringList() << "现金" << "微信支付" << "支付宝" << "银行卡" << "会员卡");
                            paymentCombo->setStyleSheet("padding: 8px; font-size: 14px;");
                            paymentLayout->addWidget(paymentCombo);

                            // 操作员和备注
                            QGroupBox *infoGroup = new QGroupBox("其他信息");
                            QGridLayout *infoLayout = new QGridLayout(infoGroup);

                            QLabel *operatorLabel = new QLabel("操作员:");
                            QLineEdit *operatorEdit = new QLineEdit("001");

                            QLabel *remarkLabel = new QLabel("备注:");
                            QLineEdit *remarkEdit = new QLineEdit;
                            remarkEdit->setPlaceholderText("选填");

                            infoLayout->addWidget(operatorLabel, 0, 0);
                            infoLayout->addWidget(operatorEdit, 0, 1);
                            infoLayout->addWidget(remarkLabel, 1, 0);
                            infoLayout->addWidget(remarkEdit, 1, 1);

                            // 充值信息显示
                            QLabel *summaryLabel = new QLabel("请输入充值金额");
                            summaryLabel->setStyleSheet("color: #666; padding: 10px; border: 1px solid #eee;");

                            // 实时更新充值信息
                            connect(amountEdit, &QLineEdit::textChanged, [=](const QString &text) {
                                if (!text.isEmpty()) {
                                    bool ok;
                                    double amount = text.toDouble(&ok);
                                    if (ok && amount > 0) {
                                        // 计算赠送金额
                                        double bonus = 0;
                                        if (amount >= 1000) bonus = 80;
                                        else if (amount >= 500) bonus = 30;
                                        else if (amount >= 300) bonus = 15;
                                        else if (amount >= 200) bonus = 10;
                                        else if (amount >= 100) bonus = 5;

                                        double total = amount + bonus;
                                        double newBalance = currentBalance + total;

                                        summaryLabel->setText(
                                            QString("充值金额：¥%1\n"
                                                   "赠送金额：¥%2\n"
                                                   "合计到账：¥%3\n"
                                                   "充值后余额：¥%4")
                                            .arg(amount, 0, 'f', 2)
                                            .arg(bonus, 0, 'f', 2)
                                            .arg(total, 0, 'f', 2)
                                            .arg(newBalance, 0, 'f', 2)
                                        );
                                    }
                                } else {
                                    summaryLabel->setText("请输入充值金额");
                                }
                            });

                            // 按钮区域
                            QHBoxLayout *buttonLayout = new QHBoxLayout;
                            buttonLayout->setSpacing(20);

                            QPushButton *cancelButton = new QPushButton("取消");
                            QPushButton *confirmButton = new QPushButton("确认充值");

                            cancelButton->setFixedSize(100, 35);
                            confirmButton->setFixedSize(100, 35);

                            cancelButton->setStyleSheet("background-color: #ccc; padding: 8px;");
                            confirmButton->setStyleSheet("background-color: #28a745; color: white; padding: 8px; font-weight: bold;");

                            buttonLayout->addStretch();
                            buttonLayout->addWidget(cancelButton);
                            buttonLayout->addWidget(confirmButton);

                            // 连接按钮
                            connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
                            connect(confirmButton, &QPushButton::clicked, [=]() {
                                if (amountEdit->text().isEmpty()) {
                                    QMessageBox::warning(this, "提示", "请输入充值金额");
                                    return;
                                }

                                bool ok;
                                double amount = amountEdit->text().toDouble(&ok);
                                if (!ok || amount <= 0) {
                                    QMessageBox::warning(this, "提示", "充值金额无效");
                                    return;
                                }

                                QString paymentMethod = paymentCombo->currentText();
                                emit rechargeRequested(amount, paymentMethod);
                                accept();
                            });

                            // 添加到主布局
                            mainLayout->addWidget(titleLabel);
                            mainLayout->addWidget(memberInfoLabel);
                            mainLayout->addWidget(amountGroup);
                            mainLayout->addWidget(paymentGroup);
                            mainLayout->addWidget(infoGroup);
                            mainLayout->addWidget(summaryLabel);
                            mainLayout->addLayout(buttonLayout);
                            mainLayout->addStretch();
                        }

                        void RechargeDialog::onRechargeClicked()
                        {
                            double amount = customAmount;

                            if (amount <= 0) {
                                QMessageBox::warning(this, "输入错误", "充值金额必须大于0！");
                                return;
                            }

                            QString paymentMethod = paymentCombo->currentText();
                            // 移除图标和空格
                            paymentMethod = paymentMethod.mid(paymentMethod.indexOf(" ") + 1);

                            emit rechargeRequested(amount, paymentMethod);
                            accept();
                        }

                        void RechargeDialog::onAmountButtonClicked()
                        {
                            QPushButton *clickedButton = qobject_cast<QPushButton*>(sender());
                            if (clickedButton) {
                                // 取消其他按钮的选中状态
                                foreach (QPushButton *button, amountButtons) {
                                    if (button != clickedButton) {
                                        button->setChecked(false);
                                    }
                                }

                                // 获取选择的金额
                                double amount = clickedButton->property("amount").toDouble();
                                customAmount = amount;
                                amountEdit->setText(QString::number(amount));

                                updateTotalAmount();
                            }
                        }

                        void RechargeDialog::updateTotalAmount()
                        {
                            QString text = amountEdit->text();
                            if (!text.isEmpty()) {
                                bool ok;
                                double amount = text.toDouble(&ok);
                                if (ok) {
                                    customAmount = amount;
                                }
                            }

                            // 如果金额大于0，启用确认按钮
                            confirmButton->setEnabled(customAmount > 0);

                            // 更新汇总信息
                            QLabel *summaryLabel = findChild<QLabel*>("summaryLabel");
                            if (summaryLabel) {
                                if (customAmount > 0) {
                                    // 固定赠送规则
                                    double bonus = 0;
                                    if (customAmount >= 10000) bonus = 1000;
                                    else if (customAmount >= 5000) bonus = 500;
                                    else if (customAmount >= 2000) bonus = 200;
                                    else if (customAmount >= 1000) bonus = 80;
                                    else if (customAmount >= 800) bonus = 50;
                                    else if (customAmount >= 500) bonus = 30;
                                    else if (customAmount >= 300) bonus = 15;
                                    else if (customAmount >= 200) bonus = 10;
                                    else if (customAmount >= 100) bonus = 5;

                                    double totalAmount = customAmount + bonus;

                                    // 根据当前余额计算充值后余额
                                    QString balanceText = currentBalanceLabel->text();
                                    QString cleanBalance = balanceText;
                                    cleanBalance.remove('¥').remove(',').remove(' ');
                                    double currentBalance = cleanBalance.toDouble();
                                    double newBalance = currentBalance + totalAmount;

                                    summaryLabel->setText(
                                        QString("充值金额：<span style='font-size: 18px; font-weight: bold; color: #0d6efd;'>¥%1</span>\n"
                                               "赠送金额：<span style='font-size: 16px; color: #e67e22;'>¥%2</span>\n"
                                               "----------------------\n"
                                               "到账总额：<span style='font-size: 20px; font-weight: bold; color: #198754;'>¥%3</span>\n"
                                               "充值后余额：<span style='font-size: 18px; font-weight: bold; color: #198754;'>¥%4</span>")
                                        .arg(customAmount, 0, 'f', 2)
                                        .arg(bonus, 0, 'f', 2)
                                        .arg(totalAmount, 0, 'f', 2)
                                        .arg(newBalance, 0, 'f', 2)
                                    );
                                } else {
                                    summaryLabel->setText("<span style='color: #666;'>请选择或输入充值金额</span>");
                                }
                            }
                        }
