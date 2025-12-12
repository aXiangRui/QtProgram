#include "BookManageWidget.h"
#include <QHeaderView>
#include <QGroupBox>
#include <QFormLayout>
#include <QMessageBox>
#include <QFileDialog>
#include <QLabel>
#include <QDateTime>
#include <QFile>
#include <QTextStream>
#include <QScrollBar>

BookManageWidget::BookManageWidget(QWidget *parent)
    : QWidget(parent), isEditing(false), editingRow(-1)
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
    loadBooks();

    // 初始状态
    onTableSelectionChanged();
}

void BookManageWidget::createToolbar()
{
    // 工具栏容器
    toolbar = new QWidget;
    toolbar->setFixedWidth(220);
    toolbar->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);

    QVBoxLayout *toolLayout = new QVBoxLayout(toolbar);
    toolLayout->setContentsMargins(0, 10, 0, 10);
    toolLayout->setSpacing(15);

    // ========== 搜索区域 ==========
    QGroupBox *searchGroup = new QGroupBox("搜索");
    searchGroup->setStyleSheet("QGroupBox { font-weight: bold; color: #555; }");
    QVBoxLayout *searchLayout = new QVBoxLayout(searchGroup);
    searchLayout->setContentsMargins(10, 20, 10, 15);
    searchLayout->setSpacing(8);

    QLabel *searchLabel = new QLabel("快速搜索:");
    searchEdit = new QLineEdit;
    searchEdit->setPlaceholderText("书名、作者、ISBN...");

    QPushButton *searchButton = new QPushButton("搜索");
    searchButton->setFixedHeight(32);

    searchLayout->addWidget(searchLabel);
    searchLayout->addWidget(searchEdit);
    searchLayout->addWidget(searchButton);

    // ========== 分类筛选 ==========
    QGroupBox *filterGroup = new QGroupBox("分类筛选");
    filterGroup->setStyleSheet("QGroupBox { font-weight: bold; color: #555; }");
    QVBoxLayout *filterLayout = new QVBoxLayout(filterGroup);
    filterLayout->setContentsMargins(10, 20, 10, 15);
    filterLayout->setSpacing(8);

    QLabel *filterLabel = new QLabel("按分类筛选:");
    categoryFilter = new QComboBox;
    categoryFilter->addItems(QStringList() << "全部" << "文学" << "科技"
                                           << "教育" << "艺术" << "少儿" << "其他");

    filterLayout->addWidget(filterLabel);
    filterLayout->addWidget(categoryFilter);

    // ========== 操作按钮 ==========
    QGroupBox *actionGroup = new QGroupBox("图书操作");
    actionGroup->setStyleSheet("QGroupBox { font-weight: bold; color: #555; }");
    QVBoxLayout *actionLayout = new QVBoxLayout(actionGroup);
    actionLayout->setContentsMargins(10, 20, 10, 15);
    actionLayout->setSpacing(8);

    // 创建操作按钮
    addButton = new QPushButton("➕ 添加图书");
    editButton = new QPushButton("✏️ 编辑图书");
    deleteButton = new QPushButton("🗑️ 删除图书");
    importButton = new QPushButton("📥 批量导入");
    exportButton = new QPushButton("📤 导出数据");
    refreshButton = new QPushButton("🔄 刷新数据");

    // 设置按钮样式
    QString baseButtonStyle =
        "QPushButton {"
        "  padding: 10px;"
        "  border-radius: 6px;"
        "  font-size: 14px;"
        "  text-align: left;"
        "  border: 1px solid #ddd;"
        "  background-color: white;"
        "}"
        "QPushButton:hover {"
        "  background-color: #f8f9fa;"
        "  border-color: #adb5bd;"
        "}"
        "QPushButton:pressed {"
        "  background-color: #e9ecef;"
        "}";

    addButton->setStyleSheet(baseButtonStyle + "color: #0d6efd; font-weight: bold;");
    editButton->setStyleSheet(baseButtonStyle + "color: #6c757d;");
    deleteButton->setStyleSheet(baseButtonStyle + "color: #dc3545;");
    importButton->setStyleSheet(baseButtonStyle);
    exportButton->setStyleSheet(baseButtonStyle);
    refreshButton->setStyleSheet(baseButtonStyle);

    // 添加到布局
    actionLayout->addWidget(addButton);
    actionLayout->addWidget(editButton);
    actionLayout->addWidget(deleteButton);
    actionLayout->addWidget(importButton);
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
    connect(addButton, SIGNAL(clicked()), this, SLOT(onAddBook()));
    connect(editButton, SIGNAL(clicked()), this, SLOT(onEditBook()));
    connect(deleteButton, SIGNAL(clicked()), this, SLOT(onDeleteBook()));
    connect(importButton, SIGNAL(clicked()), this, SLOT(onImport()));
    connect(exportButton, SIGNAL(clicked()), this, SLOT(onExport()));
    connect(refreshButton, SIGNAL(clicked()), this, SLOT(onRefresh()));
    connect(searchButton, SIGNAL(clicked()), this, SLOT(onSearch()));
    connect(searchEdit, SIGNAL(returnPressed()), this, SLOT(onSearch()));
    connect(categoryFilter, SIGNAL(currentIndexChanged(int)),
            this, SLOT(onCategoryFilterChanged(int)));
}

void BookManageWidget::createTable()
{
    // 表格容器
    QWidget *tableContainer = new QWidget;
    tableContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QVBoxLayout *tableLayout = new QVBoxLayout(tableContainer);
    tableLayout->setContentsMargins(0, 0, 0, 0);

    // 创建表格
    bookTable = new QTableWidget;
    bookTable->setObjectName("bookTable");
    bookTable->setColumnCount(10);

    // 设置表头
    QStringList headers;
    headers << "ISBN号" << "图书名称" << "作者" << "分类" << "售价"
            << "成本" << "库存" << "预警库存" << "状态" << "操作";
    bookTable->setHorizontalHeaderLabels(headers);

    // 获取表头
    QHeaderView *header = bookTable->horizontalHeader();

    // 关键设置：操作列使用拉伸模式
    header->setStretchLastSection(true);  // 最后一列拉伸

    // 设置表格属性
    bookTable->setAlternatingRowColors(true);
    bookTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    bookTable->setSelectionMode(QAbstractItemView::SingleSelection);
    bookTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    bookTable->verticalHeader()->setVisible(false);
    bookTable->setSortingEnabled(true);

    // 设置列宽 - 确保表头文字完整显示
    // 操作列会自动拉伸，所以不需要设置固定宽度
    bookTable->setColumnWidth(0, 140);  // ISBN号
    bookTable->setColumnWidth(1, 200);  // 图书名称
    bookTable->setColumnWidth(2, 110);  // 作者
    bookTable->setColumnWidth(3, 70);   // 分类
    bookTable->setColumnWidth(4, 70);   // 售价
    bookTable->setColumnWidth(5, 70);   // 成本
    bookTable->setColumnWidth(6, 60);   // 库存
    bookTable->setColumnWidth(7, 85);   // 预警库存
    bookTable->setColumnWidth(8, 65);   // 状态

    // 设置表头样式
    header->setMinimumHeight(40);
    header->setDefaultAlignment(Qt::AlignCenter);

    // 设置表格样式
    bookTable->setStyleSheet(
        "#bookTable {"
        "  border: 1px solid #dee2e6;"
        "  border-radius: 8px;"
        "  background-color: white;"
        "  font-size: 13px;"
        "  selection-background-color: #0d6efd;"
        "  selection-color: white;"
        "}"
        "#bookTable::item {"
        "  padding: 10px 6px;"
        "  border-bottom: 1px solid #f0f0f0;"
        "}"
        "QHeaderView::section {"
        "  background-color: #f8f9fa;"
        "  padding: 12px 5px;"
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

    tableLayout->addWidget(bookTable);
    mainLayout->addWidget(tableContainer, 1);

    // 连接选择变化信号
    connect(bookTable, SIGNAL(itemSelectionChanged()),
            this, SLOT(onTableSelectionChanged()));
}



void BookManageWidget::createForm()
{
    // 编辑表单组 - 加宽到350px
    formGroup = new QGroupBox("图书信息编辑");
    formGroup->setFixedWidth(350);  // 加宽30px
    formGroup->setObjectName("formGroup");

    QVBoxLayout *formLayout = new QVBoxLayout(formGroup);
    formLayout->setContentsMargins(20, 20, 20, 20);
    formLayout->setSpacing(15);

    // ========== 表单字段 ==========
    QFormLayout *fieldLayout = new QFormLayout;
    fieldLayout->setSpacing(12);
    fieldLayout->setContentsMargins(0, 0, 0, 0);

    // 创建表单控件
    isbnEdit = new QLineEdit;
    isbnEdit->setPlaceholderText("例如：978-7-02-000220-7");
    isbnEdit->setMaxLength(17);

    titleEdit = new QLineEdit;
    titleEdit->setPlaceholderText("输入图书名称");
    titleEdit->setMaxLength(100);

    authorEdit = new QLineEdit;
    authorEdit->setPlaceholderText("输入作者姓名");
    authorEdit->setMaxLength(50);

    categoryCombo = new QComboBox;
    categoryCombo->addItems(QStringList() << "文学" << "科技" << "教育"
                                          << "艺术" << "少儿" << "其他");

    priceEdit = new QDoubleSpinBox;
    priceEdit->setRange(0, 9999.99);
    priceEdit->setDecimals(2);
    priceEdit->setPrefix("¥ ");
    priceEdit->setValue(0);

    costEdit = new QDoubleSpinBox;
    costEdit->setRange(0, 9999.99);
    costEdit->setDecimals(2);
    costEdit->setPrefix("¥ ");
    costEdit->setValue(0);

    stockEdit = new QSpinBox;
    stockEdit->setRange(0, 9999);
    stockEdit->setValue(0);

    warningStockEdit = new QSpinBox;
    warningStockEdit->setRange(0, 999);
    warningStockEdit->setValue(10);

    // 添加到表单布局
    fieldLayout->addRow("ISBN:", isbnEdit);
    fieldLayout->addRow("书名:", titleEdit);
    fieldLayout->addRow("作者:", authorEdit);
    fieldLayout->addRow("分类:", categoryCombo);
    fieldLayout->addRow("售价:", priceEdit);
    fieldLayout->addRow("成本:", costEdit);
    fieldLayout->addRow("库存:", stockEdit);
    fieldLayout->addRow("预警库存:", warningStockEdit);

    // ========== 按钮区域 ==========
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->setSpacing(12);

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
        "  padding: 10px;"
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
        "  padding: 10px;"
        "  font-size: 14px;"
        "}"
        "QPushButton:hover {"
        "  background-color: #5c636a;"
        "}"
    );

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
        "QLabel {"
        "  color: #495057;"
        "  font-weight: bold;"
        "}"
        "QLineEdit, QComboBox, QSpinBox, QDoubleSpinBox {"
        "  border: 1px solid #ced4da;"
        "  border-radius: 4px;"
        "  padding: 8px;"
        "  font-size: 13px;"
        "}"
        "QLineEdit:focus, QComboBox:focus, QSpinBox:focus, QDoubleSpinBox:focus {"
        "  border-color: #86b7fe;"
        "  outline: none;"
        "}"
    );

    // 添加到主布局
    mainLayout->addWidget(formGroup);

    // 连接信号
    connect(saveButton, SIGNAL(clicked()), this, SLOT(onSaveBook()));
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(onCancelEdit()));
}

void BookManageWidget::loadBooks()
{
    // 清空表格
    bookTable->setRowCount(0);

    // 模拟图书数据
    QStringList booksData = QStringList()
        << "9787020002207|红楼梦|曹雪芹|文学|59.80|35.00|120|20|正常"
        << "9787020008728|三国演义|罗贯中|文学|49.90|28.00|85|15|正常"
        << "9787500601593|西游记|吴承恩|文学|45.00|25.00|92|20|正常"
        << "9787101003048|史记|司马迁|文学|68.00|40.00|45|10|预警"
        << "9787532744306|时间简史|霍金|科技|38.00|22.00|23|15|紧急"
        << "9787505738968|人类简史|尤瓦尔|科技|68.00|40.00|56|20|正常"
        << "9787561772045|教育学原理|王道俊|教育|39.80|24.00|67|15|正常"
        << "9787301127606|艺术的故事|贡布里希|艺术|280.00|180.00|12|5|紧急"
        << "9787208061644|追风筝的人|卡勒德·胡赛尼|文学|36.00|22.00|45|10|正常"
        << "9787020043270|百年孤独|加西亚·马尔克斯|文学|39.50|25.00|38|10|预警";

    for (int i = 0; i < booksData.size(); i++) {
        QStringList fields = booksData[i].split('|');
        if (fields.size() >= 9) {
            int row = bookTable->rowCount();
            bookTable->insertRow(row);

            // 填充前9列数据
            for (int col = 0; col < 9; col++) {
                QTableWidgetItem *item = new QTableWidgetItem(fields[col]);
                bookTable->setItem(row, col, item);

                // 设置状态颜色
                if (col == 8) {  // 状态列
                    QString status = fields[col];
                    if (status == "紧急") {
                        item->setForeground(QColor("#dc3545"));
                        item->setFont(QFont("", -1, QFont::Bold));
                    } else if (status == "预警") {
                        item->setForeground(QColor("#fd7e14"));
                        item->setFont(QFont("", -1, QFont::Bold));
                    } else {
                        item->setForeground(QColor("#198754"));
                    }
                    item->setTextAlignment(Qt::AlignCenter);
                }

                // 数字列居右对齐
                if (col == 4 || col == 5 || col == 6 || col == 7) {
                    item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
                }
            }

            // 操作列 - 创建按钮容器
            QWidget *actionWidget = new QWidget;
            actionWidget->setObjectName("actionWidget");

            QHBoxLayout *actionLayout = new QHBoxLayout(actionWidget);
            actionLayout->setContentsMargins(4, 2, 4, 2);
            actionLayout->setSpacing(8);

            // 编辑按钮
            QPushButton *editBtn = new QPushButton("编辑");
            editBtn->setProperty("row", row);
            editBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
            editBtn->setMinimumHeight(30);
            editBtn->setStyleSheet(
                "QPushButton {"
                "  background-color: #0d6efd;"
                "  color: white;"
                "  border: none;"
                "  border-radius: 4px;"
                "  font-size: 12px;"
                "  font-weight: bold;"
                "  padding: 5px;"
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
            deleteBtn->setMinimumHeight(30);
            deleteBtn->setStyleSheet(
                "QPushButton {"
                "  background-color: #dc3545;"
                "  color: white;"
                "  border: none;"
                "  border-radius: 4px;"
                "  font-size: 12px;"
                "  font-weight: bold;"
                "  padding: 5px;"
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

            // 添加按钮到布局
            actionLayout->addWidget(editBtn);
            actionLayout->addWidget(deleteBtn);

            // 连接信号
            connect(editBtn, SIGNAL(clicked()), this, SLOT(onEditBook()));
            connect(deleteBtn, SIGNAL(clicked()), this, SLOT(onDeleteBook()));

            // 设置单元格部件
            bookTable->setCellWidget(row, 9, actionWidget);
        }
    }

    // 更新按钮状态
    onTableSelectionChanged();
}

              void BookManageWidget::onAddBook()
              {
                  isEditing = false;
                  editingRow = -1;
                  clearForm();

                  // 生成建议的ISBN
                  QString isbn = QString("978-%1-%2-%3")
                      .arg(qrand() % 9 + 1, 1, 10)
                      .arg(qrand() % 90 + 10, 2, 10, QChar('0'))
                      .arg(qrand() % 9000 + 1000, 4, 10, QChar('0'));

                  isbnEdit->setText(isbn);
                  isbnEdit->setFocus();

                  QMessageBox::information(this, "添加图书",
                      "请填写图书信息，然后点击保存。\nISBN已自动生成。");
              }

              void BookManageWidget::onEditBook()
              {
                  QPushButton *senderBtn = qobject_cast<QPushButton*>(sender());
                  int row;

                  if (senderBtn) {
                      // 从按钮获取行号
                      row = senderBtn->property("row").toInt();
                  } else {
                      // 从当前选择获取行号
                      row = bookTable->currentRow();
                  }

                  if (row >= 0 && row < bookTable->rowCount()) {
                      isEditing = true;
                      editingRow = row;
                      populateForm(row);
                      QMessageBox::information(this, "编辑图书",
                          "请修改图书信息，然后点击保存。");
                  } else {
                      QMessageBox::warning(this, "警告", "请先选择要编辑的图书！");
                  }
              }

              void BookManageWidget::onDeleteBook()
              {
                  int row = bookTable->currentRow();
                  if (row >= 0) {
                      QString bookName = bookTable->item(row, 1)->text();

                      QMessageBox::StandardButton reply;
                      reply = QMessageBox::question(this, "确认删除",
                          QString("确定要删除图书《%1》吗？").arg(bookName),
                          QMessageBox::Yes | QMessageBox::No);

                      if (reply == QMessageBox::Yes) {
                          bookTable->removeRow(row);
                          clearForm();
                          QMessageBox::information(this, "删除成功", "图书已删除！");
                      }
                  } else {
                      QMessageBox::warning(this, "警告", "请先选择要删除的图书！");
                  }
              }

              void BookManageWidget::onSearch()
              {
                  QString keyword = searchEdit->text().trimmed().toLower();
                  QString category = categoryFilter->currentText();

                  int visibleCount = 0;

                  for (int row = 0; row < bookTable->rowCount(); row++) {
                      bool match = true;

                      // 分类筛选
                      if (category != "全部") {
                          QString bookCategory = bookTable->item(row, 3)->text();
                          if (bookCategory != category) {
                              match = false;
                          }
                      }

                      // 关键词搜索
                      if (!keyword.isEmpty() && match) {
                          bool found = false;
                          // 搜索ISBN、书名、作者
                          for (int col = 0; col < 3; col++) {
                              QString cellText = bookTable->item(row, col)->text().toLower();
                              if (cellText.contains(keyword)) {
                                  found = true;
                                  break;
                              }
                          }
                          match = found;
                      }

                      // 显示/隐藏行
                      bookTable->setRowHidden(row, !match);
                      if (match) visibleCount++;
                  }

                  // 显示搜索结果统计
                  if (!keyword.isEmpty() || category != "全部") {
                      QMessageBox::information(this, "搜索结果",
                          QString("找到 %1 本符合条件的图书").arg(visibleCount));
                  }
              }

              void BookManageWidget::onImport()
              {
                  QString fileName = QFileDialog::getOpenFileName(this,
                      "选择导入文件", "", "Excel文件 (*.xls *.xlsx);;CSV文件 (*.csv);;所有文件 (*.*)");

                  if (!fileName.isEmpty()) {
                      // 模拟导入过程
                      QMessageBox::information(this, "导入成功",
                          QString("已从文件导入数据：\n%1\n\n模拟导入了15本图书。").arg(fileName));

                      // 模拟数据更新
                      loadBooks();
                  }
              }

              void BookManageWidget::onExport()
              {
                  QString defaultName = QString("图书数据_%1.xls")
                      .arg(QDateTime::currentDateTime().toString("yyyyMMdd"));

                  QString fileName = QFileDialog::getSaveFileName(this,
                      "导出数据", defaultName, "Excel文件 (*.xls *.xlsx);;CSV文件 (*.csv)");

                  if (!fileName.isEmpty()) {
                      // 模拟导出过程
                      int totalBooks = bookTable->rowCount();
                      QMessageBox::information(this, "导出成功",
                          QString("已导出 %1 本图书数据到：\n%2").arg(totalBooks).arg(fileName));
                  }
              }

              void BookManageWidget::onTableSelectionChanged()
              {
                  bool hasSelection = bookTable->currentRow() >= 0;
                  editButton->setEnabled(hasSelection);
                  deleteButton->setEnabled(hasSelection);

                  // 如果没有选择，清空表单
                  if (!hasSelection && !isEditing) {
                      clearForm();
                  }
              }

              void BookManageWidget::onSaveBook()
              {
                  if (!validateBookData()) {
                      return;
                  }

                  if (isEditing && editingRow >= 0) {
                      // 更新现有图书
                      updateBookInTable(editingRow);
                      QMessageBox::information(this, "保存成功", "图书信息已更新！");
                  } else {
                      // 添加新图书
                      addBookToTable();
                      QMessageBox::information(this, "保存成功", "新图书已添加！");
                  }

                  isEditing = false;
                  editingRow = -1;
                  onTableSelectionChanged();
              }
              void BookManageWidget::onCancelEdit()
              {
                  if (isEditing) {
                      // 如果是编辑模式，询问是否取消编辑
                      QMessageBox::StandardButton reply;
                      reply = QMessageBox::question(this, "取消编辑",
                          "确定要取消编辑吗？所有未保存的修改将丢失。",
                          QMessageBox::Yes | QMessageBox::No);

                      if (reply == QMessageBox::Yes) {
                          // 取消编辑，清空表单
                          clearForm();
                          isEditing = false;
                          editingRow = -1;

                          // 更新按钮状态
                          onTableSelectionChanged();

                          QMessageBox::information(this, "取消编辑", "已取消编辑操作。");
                      }
                  } else {
                      // 如果是添加模式，询问是否清空表单
                      QMessageBox::StandardButton reply;
                      reply = QMessageBox::question(this, "清空表单",
                          "确定要清空当前表单内容吗？",
                          QMessageBox::Yes | QMessageBox::No);

                      if (reply == QMessageBox::Yes) {
                          clearForm();

                          // 重新生成一个ISBN
                          QString newISBN = QString("978-%1-%2-%3")
                              .arg(qrand() % 9 + 1, 1, 10)
                              .arg(qrand() % 90 + 10, 2, 10, QChar('0'))
                              .arg(qrand() % 9000 + 1000, 4, 10, QChar('0'));
                          isbnEdit->setText(newISBN);

                          QMessageBox::information(this, "清空表单", "表单已清空。");
                      }
                  }
              }

              void BookManageWidget::onRefresh()
              {
                  loadBooks();
                  searchEdit->clear();
                  categoryFilter->setCurrentIndex(0);

                  // 显示所有行
                  for (int row = 0; row < bookTable->rowCount(); row++) {
                      bookTable->setRowHidden(row, false);
                  }

                  QMessageBox::information(this, "刷新完成", "图书数据已刷新！");
              }

              void BookManageWidget::onCategoryFilterChanged(int index)
              {
                  onSearch();  // 直接调用搜索函数
              }

              void BookManageWidget::clearForm()
              {
                  isbnEdit->clear();
                  titleEdit->clear();
                  authorEdit->clear();
                  categoryCombo->setCurrentIndex(0);
                  priceEdit->setValue(0);
                  costEdit->setValue(0);
                  stockEdit->setValue(0);
                  warningStockEdit->setValue(10);
              }

              void BookManageWidget::populateForm(int row)
              {
                  if (row < 0 || row >= bookTable->rowCount()) return;

                  isbnEdit->setText(bookTable->item(row, 0)->text());
                  titleEdit->setText(bookTable->item(row, 1)->text());
                  authorEdit->setText(bookTable->item(row, 2)->text());

                  // 分类
                  QString category = bookTable->item(row, 3)->text();
                  int categoryIndex = categoryCombo->findText(category);
                  if (categoryIndex >= 0) {
                      categoryCombo->setCurrentIndex(categoryIndex);
                  }

                  // 售价（移除¥符号）
                  QString priceStr = bookTable->item(row, 4)->text();
                  priceStr.remove('¥').remove(' ').remove(',');
                  priceEdit->setValue(priceStr.toDouble());

                  // 成本
                  QString costStr = bookTable->item(row, 5)->text();
                  costStr.remove('¥').remove(' ').remove(',');
                  costEdit->setValue(costStr.toDouble());

                  // 库存
                  stockEdit->setValue(bookTable->item(row, 6)->text().toInt());

                  // 预警库存
                  warningStockEdit->setValue(bookTable->item(row, 7)->text().toInt());
              }

              bool BookManageWidget::validateBookData()
              {
                  // 获取表单数据
                  QString isbn = isbnEdit->text().trimmed();
                  QString title = titleEdit->text().trimmed();
                  QString author = authorEdit->text().trimmed();

                  // 验证ISBN
                  if (isbn.isEmpty()) {
                      QMessageBox::warning(this, "验证错误", "ISBN不能为空！");
                      isbnEdit->setFocus();
                      return false;
                  }

                  // 验证ISBN格式（简单验证）
                  if (isbn.length() < 10) {
                      QMessageBox::warning(this, "验证错误", "ISBN格式不正确，应为10或13位！");
                      isbnEdit->setFocus();
                      return false;
                  }

                  // 验证书名
                  if (title.isEmpty()) {
                      QMessageBox::warning(this, "验证错误", "书名不能为空！");
                      titleEdit->setFocus();
                      return false;
                  }

                  // 验证作者
                  if (author.isEmpty()) {
                      QMessageBox::warning(this, "验证错误", "作者不能为空！");
                      authorEdit->setFocus();
                      return false;
                  }

                  // 验证售价
                  if (priceEdit->value() <= 0) {
                      QMessageBox::warning(this, "验证错误", "售价必须大于0！");
                      priceEdit->setFocus();
                      return false;
                  }

                  // 验证成本
                  if (costEdit->value() <= 0) {
                      QMessageBox::warning(this, "验证错误", "成本必须大于0！");
                      costEdit->setFocus();
                      return false;
                  }

                  // 验证库存
                  if (stockEdit->value() < 0) {
                      QMessageBox::warning(this, "验证错误", "库存不能为负数！");
                      stockEdit->setFocus();
                      return false;
                  }

                  // 验证预警库存
                  if (warningStockEdit->value() < 0) {
                      QMessageBox::warning(this, "验证错误", "预警库存不能为负数！");
                      warningStockEdit->setFocus();
                      return false;
                  }

                  // 检查ISBN是否重复（只在添加时检查）
                  if (!isEditing) {
                      int existingRow = findBookByISBN(isbn);
                      if (existingRow >= 0) {
                          QString existingTitle = bookTable->item(existingRow, 1)->text();
                          QMessageBox::warning(this, "验证错误",
                              QString("ISBN %1 已存在！\n对应图书：%2").arg(isbn).arg(existingTitle));
                          isbnEdit->setFocus();
                          return false;
                      }
                  }

                  return true;
              }

              int BookManageWidget::findBookByISBN(const QString &isbn)
              {
                  // 遍历表格查找指定ISBN的图书
                  for (int row = 0; row < bookTable->rowCount(); row++) {
                      QTableWidgetItem *isbnItem = bookTable->item(row, 0); // 第0列是ISBN
                      if (isbnItem && isbnItem->text() == isbn) {
                          return row; // 找到，返回行号
                      }
                  }
                  return -1; // 没找到
              }

              void BookManageWidget::updateBookInTable(int row)
              {
                  // 更新前9列
                  bookTable->item(row, 0)->setText(isbnEdit->text().trimmed());
                  bookTable->item(row, 1)->setText(titleEdit->text().trimmed());
                  bookTable->item(row, 2)->setText(authorEdit->text().trimmed());
                  bookTable->item(row, 3)->setText(categoryCombo->currentText());

                  // 售价 - 去掉"¥"符号，只保留数字
                  bookTable->item(row, 4)->setText(QString("%1").arg(priceEdit->value(), 0, 'f', 2));
                  bookTable->item(row, 4)->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);

                  // 成本 - 去掉"¥"符号，只保留数字
                  bookTable->item(row, 5)->setText(QString("%1").arg(costEdit->value(), 0, 'f', 2));
                  bookTable->item(row, 5)->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);

                  // 库存 - 右对齐
                  bookTable->item(row, 6)->setText(QString::number(stockEdit->value()));
                  bookTable->item(row, 6)->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);

                  // 预警库存 - 右对齐
                  bookTable->item(row, 7)->setText(QString::number(warningStockEdit->value()));
                  bookTable->item(row, 7)->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);

                  // 更新状态
                  QString status;
                  int stock = stockEdit->value();
                  int warningStock = warningStockEdit->value();

                  if (stock == 0) {
                      status = "缺货";
                  } else if (stock < 5) {
                      status = "紧急";
                  } else if (stock < warningStock) {
                      status = "预警";
                  } else {
                      status = "正常";
                  }

                  QTableWidgetItem *statusItem = bookTable->item(row, 8);
                  statusItem->setText(status);

                  // 设置状态颜色
                  if (status == "紧急" || status == "缺货") {
                      statusItem->setForeground(QColor("#dc3545"));
                      statusItem->setFont(QFont("", -1, QFont::Bold));
                  } else if (status == "预警") {
                      statusItem->setForeground(QColor("#fd7e14"));
                      statusItem->setFont(QFont("", -1, QFont::Bold));
                  } else {
                      statusItem->setForeground(QColor("#198754"));
                  }
                  statusItem->setTextAlignment(Qt::AlignCenter);
              }


              void BookManageWidget::addBookToTable()
              {
                  // 获取表单数据
                  QString isbn = isbnEdit->text().trimmed();
                  QString title = titleEdit->text().trimmed();
                  QString author = authorEdit->text().trimmed();
                  QString category = categoryCombo->currentText();
                  double price = priceEdit->value();
                  double cost = costEdit->value();
                  int stock = stockEdit->value();
                  int warningStock = warningStockEdit->value();

                  // 计算状态
                  QString status;
                  if (stock == 0) {
                      status = "缺货";
                  } else if (stock < 5) {
                      status = "紧急";
                  } else if (stock < warningStock) {
                      status = "预警";
                  } else {
                      status = "正常";
                  }

                  // 在表格末尾插入新行
                  int row = bookTable->rowCount();
                  bookTable->insertRow(row);

                  // 填充前9列数据 - 修复格式
                  // ISBN
                  QTableWidgetItem *isbnItem = new QTableWidgetItem(isbn);
                  bookTable->setItem(row, 0, isbnItem);

                  // 书名
                  QTableWidgetItem *titleItem = new QTableWidgetItem(title);
                  bookTable->setItem(row, 1, titleItem);

                  // 作者
                  QTableWidgetItem *authorItem = new QTableWidgetItem(author);
                  bookTable->setItem(row, 2, authorItem);

                  // 分类
                  QTableWidgetItem *categoryItem = new QTableWidgetItem(category);
                  bookTable->setItem(row, 3, categoryItem);

                  // 售价 - 去掉"¥"符号，右对齐
                  QTableWidgetItem *priceItem = new QTableWidgetItem(QString("%1").arg(price, 0, 'f', 2));
                  priceItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
                  bookTable->setItem(row, 4, priceItem);

                  // 成本 - 去掉"¥"符号，右对齐
                  QTableWidgetItem *costItem = new QTableWidgetItem(QString("%1").arg(cost, 0, 'f', 2));
                  costItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
                  bookTable->setItem(row, 5, costItem);

                  // 库存 - 右对齐
                  QTableWidgetItem *stockItem = new QTableWidgetItem(QString::number(stock));
                  stockItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
                  bookTable->setItem(row, 6, stockItem);

                  // 预警库存 - 右对齐
                  QTableWidgetItem *warningItem = new QTableWidgetItem(QString::number(warningStock));
                  warningItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
                  bookTable->setItem(row, 7, warningItem);

                  // 状态列 - 居中对齐
                  QTableWidgetItem *statusItem = new QTableWidgetItem(status);
                  if (status == "紧急" || status == "缺货") {
                      statusItem->setForeground(QColor("#dc3545"));
                      statusItem->setFont(QFont("", -1, QFont::Bold));
                  } else if (status == "预警") {
                      statusItem->setForeground(QColor("#fd7e14"));
                      statusItem->setFont(QFont("", -1, QFont::Bold));
                  } else {
                      statusItem->setForeground(QColor("#198754"));
                  }
                  statusItem->setTextAlignment(Qt::AlignCenter);
                  bookTable->setItem(row, 8, statusItem);

                  // 操作列 - 创建按钮容器
                  QWidget *actionWidget = new QWidget;
                  actionWidget->setObjectName("actionWidget");

                  QHBoxLayout *actionLayout = new QHBoxLayout(actionWidget);
                  actionLayout->setContentsMargins(4, 2, 4, 2);
                  actionLayout->setSpacing(8);

                  // 编辑按钮
                  QPushButton *editBtn = new QPushButton("编辑");
                  editBtn->setProperty("row", row);
                  editBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
                  editBtn->setMinimumHeight(28);
                  editBtn->setStyleSheet(
                      "QPushButton {"
                      "  background-color: #0d6efd;"
                      "  color: white;"
                      "  border: none;"
                      "  border-radius: 4px;"
                      "  font-size: 12px;"
                      "  font-weight: bold;"
                      "  padding: 5px;"
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
                  deleteBtn->setMinimumHeight(28);
                  deleteBtn->setStyleSheet(
                      "QPushButton {"
                      "  background-color: #dc3545;"
                      "  color: white;"
                      "  border: none;"
                      "  border-radius: 4px;"
                      "  font-size: 12px;"
                      "  font-weight: bold;"
                      "  padding: 5px;"
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

                  connect(editBtn, SIGNAL(clicked()), this, SLOT(onEditBook()));
                  connect(deleteBtn, SIGNAL(clicked()), this, SLOT(onDeleteBook()));

                  bookTable->setCellWidget(row, 9, actionWidget);
              }
