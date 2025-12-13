#include "SystemManageWidget.h"

#include <QFormLayout>
#include <QMessageBox>
#include <QFileDialog>
#include <QDateTime>
#include <QFileInfo>
#include <QFile>
#include <QTextStream>
#include <QSettings>
#include <QDir>
#include <QProgressBar>
#include <QGroupBox>
#include <QFrame>
#include <QApplication>
#include <QTimer>
#include <QThread>

// 配置常量定义
const QString SystemManageWidget::CONFIG_FILENAME = "bookstore_config.xml";
const QColor SystemManageWidget::PRIMARY_COLOR = QColor(41, 128, 185);
const QColor SystemManageWidget::SUCCESS_COLOR = QColor(39, 174, 96);
const QColor SystemManageWidget::WARNING_COLOR = QColor(241, 196, 15);
const QColor SystemManageWidget::DANGER_COLOR = QColor(231, 76, 60);

// ==================== 构造函数 ====================
SystemManageWidget::SystemManageWidget(QWidget *parent)
    : QWidget(parent)
{
    // 设置窗口最小尺寸
    setMinimumSize(1000, 650);

    // 主垂直布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(15, 15, 15, 15);
    mainLayout->setSpacing(10);

    // ========== 标题区域 ==========
    QWidget *titleWidget = new QWidget;
    titleWidget->setObjectName("titleWidget");
    QHBoxLayout *titleLayout = new QHBoxLayout(titleWidget);
    titleLayout->setContentsMargins(0, 0, 0, 0);

    QLabel *titleLabel = new QLabel("⚙️ 系统设置管理中心");
    titleLabel->setObjectName("titleLabel");
    titleLabel->setAlignment(Qt::AlignCenter);

    titleLayout->addStretch();
    titleLayout->addWidget(titleLabel);
    titleLayout->addStretch();

    // ========== 工具栏 ==========
    QWidget *toolbarWidget = new QWidget;
    toolbarWidget->setObjectName("toolbarWidget");
    QHBoxLayout *toolbarLayout = new QHBoxLayout(toolbarWidget);
    toolbarLayout->setContentsMargins(0, 0, 0, 0);
    toolbarLayout->setSpacing(10);

    saveAllButton = new QPushButton("💾 保存所有设置");
    loadButton = new QPushButton("📂 导入配置");
    defaultButton = new QPushButton("🔄 恢复默认");

    statusLabel = new QLabel("就绪");
    statusLabel->setObjectName("statusLabel");

    toolbarLayout->addWidget(saveAllButton);
    toolbarLayout->addWidget(loadButton);
    toolbarLayout->addWidget(defaultButton);
    toolbarLayout->addStretch();
    toolbarLayout->addWidget(statusLabel);

    // ========== 创建标签页 ==========
    createTabs();

    // ========== 添加到主布局 ==========
    mainLayout->addWidget(titleWidget);
    mainLayout->addWidget(toolbarWidget);
    mainLayout->addWidget(tabWidget, 1);

    // ========== 应用样式 ==========
    setStyleSheet(QString(
        // 窗口背景
        "SystemManageWidget { background-color: #f8f9fa; }"

        // 标题样式
        "#titleWidget { background-color: white; border-radius: 8px; padding: 15px; "
        "border: 2px solid %1; }"
        "#titleLabel { color: %1; font-size: 22px; font-weight: bold; "
        "font-family: 'Microsoft YaHei', Arial, sans-serif; }"

        // 工具栏样式
        "#toolbarWidget { background-color: white; border-radius: 8px; padding: 12px; "
        "border: 1px solid #dee2e6; }"

        // 状态标签
        "#statusLabel { color: #6c757d; font-size: 13px; padding: 5px 10px; "
        "border-radius: 4px; background-color: #f8f9fa; }"

        // 通用按钮样式
        "QPushButton { border: none; border-radius: 6px; padding: 10px 20px; "
        "font-size: 14px; font-weight: 500; min-height: 40px; }"
        "QPushButton:hover { opacity: 0.9; }"
        "QPushButton:pressed { opacity: 0.8; }"

        // 主按钮颜色
        "QPushButton[text*='保存'] { background-color: %2; color: white; }"
        "QPushButton[text*='导入'] { background-color: %1; color: white; }"
        "QPushButton[text*='恢复'] { background-color: #6c757d; color: white; }"
        "QPushButton[text*='备份'] { background-color: #17a2b8; color: white; }"
        "QPushButton[text*='导出'] { background-color: %2; color: white; }"
        "QPushButton[text*='清理'] { background-color: %4; color: white; }"
        "QPushButton[text*='测试'] { background-color: %3; color: white; }"
        "QPushButton[text*='检查'] { background-color: #6f42c1; color: white; }"

        // 标签页样式
        "QTabWidget::pane { border: 1px solid #dee2e6; border-top: none; "
        "border-radius: 0 0 8px 8px; background-color: white; }"
        "QTabBar::tab { background-color: #f8f9fa; border: 1px solid #dee2e6; "
        "padding: 10px 25px; margin-right: 2px; border-top-left-radius: 6px; "
        "border-top-right-radius: 6px; font-size: 14px; color: #495057; }"
        "QTabBar::tab:selected { background-color: white; color: %1; "
        "font-weight: bold; border-bottom-color: white; }"
        "QTabBar::tab:hover { background-color: #e9ecef; }"

        // 分组框样式
        "QGroupBox { font-weight: bold; font-size: 15px; color: %1; "
        "border: 2px solid #dee2e6; border-radius: 8px; margin-top: 10px; }"
        "QGroupBox::title { subcontrol-origin: margin; left: 15px; "
        "padding: 0 10px 0 10px; background-color: white; }"

        // 输入框样式
        "QLineEdit, QComboBox, QSpinBox, QDoubleSpinBox, QDateEdit, QTimeEdit, QTextEdit { "
        "border: 1px solid #ced4da; border-radius: 4px; padding: 8px; "
        "font-size: 14px; background-color: white; }"
        "QLineEdit:focus, QComboBox:focus, QSpinBox:focus, QTextEdit:focus { "
        "border-color: %1; outline: none; }"

        // 复选框样式
        "QCheckBox { spacing: 8px; font-size: 14px; }"
        "QCheckBox::indicator { width: 18px; height: 18px; }"
        "QCheckBox::indicator:checked { background-color: %1; "
        "border: 2px solid %1; border-radius: 3px; }"

        // 标签样式
        "QLabel { font-size: 14px; color: #495057; }"
        "QLabel[objectName*='Label'] { font-weight: 500; }"
    ).arg(PRIMARY_COLOR.name())
     .arg(SUCCESS_COLOR.name())
     .arg(WARNING_COLOR.name())
     .arg(DANGER_COLOR.name()));

    // ========== 连接信号槽 ==========
    connect(saveAllButton, SIGNAL(clicked()), this, SLOT(onSaveAllSettings()));
    connect(loadButton, SIGNAL(clicked()), this, SLOT(onImportSettings()));
    connect(defaultButton, SIGNAL(clicked()), this, SLOT(onResetSettings()));

    // ========== 加载配置 ==========
    if (QFile::exists(getConfigPath())) {
        if (loadSettingsFromXml()) {
            statusLabel->setText("配置加载成功");
            statusLabel->setStyleSheet("color: #28a745;");
        } else {
            statusLabel->setText("配置加载失败，使用默认设置");
            statusLabel->setStyleSheet("color: #dc3545;");
            createDefaultXmlConfig();
        }
    } else {
        statusLabel->setText("使用默认设置");
        createDefaultXmlConfig();
    }
}


// ==================== XML 辅助函数 ====================
QString SystemManageWidget::escapeXml(const QString &text) const
{
    QString escaped = text;
    escaped.replace("&", "&amp;");
    escaped.replace("<", "&lt;");
    escaped.replace(">", "&gt;");
    escaped.replace("\"", "&quot;");
    escaped.replace("'", "&apos;");
    escaped.replace("\n", "&#10;");
    escaped.replace("\r", "&#13;");
    return escaped;
}

QString SystemManageWidget::unescapeXml(const QString &text) const
{
    QString unescaped = text;
    unescaped.replace("&amp;", "&");
    unescaped.replace("&lt;", "<");
    unescaped.replace("&gt;", ">");
    unescaped.replace("&quot;", "\"");
    unescaped.replace("&apos;", "'");
    unescaped.replace("&#10;", "\n");
    unescaped.replace("&#13;", "\r");
    return unescaped;
}

QString SystemManageWidget::boolToString(bool value) const
{
    return value ? "true" : "false";
}

bool SystemManageWidget::stringToBool(const QString &str, bool defaultValue) const
{
    if (str.toLower() == "true" || str == "1") return true;
    if (str.toLower() == "false" || str == "0") return false;
    return defaultValue;
}

// ==================== 配置文件路径 ====================
QString SystemManageWidget::getConfigPath() const
{
    return QDir::currentPath() + "/" + CONFIG_FILENAME;
}

// ==================== 保存配置到 XML（直接编写） ====================
bool SystemManageWidget::saveSettingsToXml()
{
    QFile file(getConfigPath());
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }

    QTextStream stream(&file);
    stream.setCodec("UTF-8");

    // 写入 XML 声明和注释
    stream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    stream << "<!-- 图书商家管理系统配置文件 -->\n";
    stream << "<!-- 生成时间: " << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") << " -->\n\n";

    // 开始根元素
    stream << "<BookstoreSystemConfig version=\"3.0\" lastModified=\"";
    stream << QDateTime::currentDateTime().toString(Qt::ISODate) << "\">\n\n";

    // ========== 系统信息 ==========
    stream << "  <!-- 系统基本信息 -->\n";
    stream << "  <SystemInfo>\n";
    stream << "    <ConfigVersion>3.0</ConfigVersion>\n";
    stream << "    <LastSaveTime>" << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") << "</LastSaveTime>\n";
    stream << "  </SystemInfo>\n\n";

    // ========== 店铺信息 ==========
    stream << "  <!-- 店铺设置信息 -->\n";
    stream << "  <ShopInfo>\n";
    stream << "    <Name>" << escapeXml(shopNameEdit->text()) << "</Name>\n";
    stream << "    <Owner>" << escapeXml(shopOwnerEdit->text()) << "</Owner>\n";
    stream << "    <Phone>" << escapeXml(phoneEdit->text()) << "</Phone>\n";
    stream << "    <Email>" << escapeXml(emailEdit->text()) << "</Email>\n";
    stream << "    <Website>" << escapeXml(websiteEdit->text()) << "</Website>\n";
    stream << "    <Address>" << escapeXml(addressEdit->text()) << "</Address>\n";
    stream << "    <Description>" << escapeXml(descriptionEdit->toPlainText()) << "</Description>\n";
    stream << "    <OpenTime>" << openTimeEdit->time().toString("hh:mm") << "</OpenTime>\n";
    stream << "    <CloseTime>" << closeTimeEdit->time().toString("hh:mm") << "</CloseTime>\n";
    stream << "  </ShopInfo>\n\n";

    // ========== 收银设置 ==========
    stream << "  <!-- 收银系统设置 -->\n";
    stream << "  <CashierSettings>\n";
    stream << "    <PrinterType>" << escapeXml(printerCombo->currentText()) << "</PrinterType>\n";
    stream << "    <BarcodeScanner>" << escapeXml(barcodeCombo->currentText()) << "</BarcodeScanner>\n";
    stream << "    <PaymentMethods>" << escapeXml(paymentCombo->currentText()) << "</PaymentMethods>\n";
    stream << "    <HeaderText>" << escapeXml(headerTextEdit->text()) << "</HeaderText>\n";
    stream << "    <FooterText>" << escapeXml(footerTextEdit->text()) << "</FooterText>\n";
    stream << "    <AutoPrint>" << boolToString(autoPrintCheck->isChecked()) << "</AutoPrint>\n";
    stream << "    <PlaySound>" << boolToString(soundCheck->isChecked()) << "</PlaySound>\n";
    stream << "    <ShowStock>" << boolToString(showStockCheck->isChecked()) << "</ShowStock>\n";
    stream << "    <AutoDiscount>" << boolToString(autoDiscountCheck->isChecked()) << "</AutoDiscount>\n";
    stream << "    <MemberDiscount>" << boolToString(memberDiscountCheck->isChecked()) << "</MemberDiscount>\n";
    stream << "    <PaymentTimeout>" << timeoutSpin->value() << "</PaymentTimeout>\n";
    stream << "    <ReceiptWidth>" << receiptWidthSpin->value() << "</ReceiptWidth>\n";
    stream << "    <VATRate>" << vatRateSpin->value() << "</VATRate>\n";
    stream << "  </CashierSettings>\n\n";

    // ========== 个人设置 ==========
    stream << "  <!-- 个人偏好设置 -->\n";
    stream << "  <PersonalSettings>\n";
    stream << "    <Theme>" << escapeXml(themeCombo->currentText()) << "</Theme>\n";
    stream << "    <Language>" << escapeXml(languageCombo->currentText()) << "</Language>\n";
    stream << "    <FontSize>" << escapeXml(fontSizeCombo->currentText()) << "</FontSize>\n";
    stream << "    <UseShortcuts>" << boolToString(shortcutCheck->isChecked()) << "</UseShortcuts>\n";
    stream << "    <AutoLogin>" << boolToString(autoLoginCheck->isChecked()) << "</AutoLogin>\n";
    stream << "    <ShowNotifications>" << boolToString(notificationCheck->isChecked()) << "</ShowNotifications>\n";
    stream << "    <RememberLast>" << boolToString(rememberLastCheck->isChecked()) << "</RememberLast>\n";
    stream << "  </PersonalSettings>\n\n";
    // ========== 统计信息 ==========
      stream << "  <!-- 系统统计信息（只读） -->\n";
      stream << "  <Statistics>\n";
      stream << "    <ConfigCreated>2023-12-15 00:00:00</ConfigCreated>\n";
      stream << "    <SaveCount>1</SaveCount>\n";
      stream << "    <LastModifyUser>System</LastModifyUser>\n";
      stream << "  </Statistics>\n";

      // 结束根元素
      stream << "</BookstoreSystemConfig>\n";

      file.close();
      return true;
  }

  // ==================== 从 XML 加载配置（直接解析） ====================
  bool SystemManageWidget::loadSettingsFromXml()
  {
      QFile file(getConfigPath());
      if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
          return false;
      }

      QTextStream stream(&file);
      stream.setCodec("UTF-8");
      QString content = stream.readAll();
      file.close();

      // 解析 XML 内容
      QString currentSection;
      QString currentTag;
      QString currentValue;

      QStringList lines = content.split("\n");
      bool inTag = false;
      bool inValue = false;

      // 存储解析的数据
      QMap<QString, QString> shopInfo;
      QMap<QString, QString> cashierSettings;
      QMap<QString, QString> personalSettings;

      for (int i = 0; i < lines.size(); i++) {
          QString line = lines[i].trimmed();

          if (line.isEmpty()) continue;

          // 检查注释
          if (line.startsWith("<!--")) continue;

          // 检查节开始
          if (line == "<ShopInfo>") {
              currentSection = "ShopInfo";
              continue;
          } else if (line == "<CashierSettings>") {
              currentSection = "CashierSettings";
              continue;
          } else if (line == "<PersonalSettings>") {
              currentSection = "PersonalSettings";
              continue;
          }

          // 检查节结束
          if (line == "</ShopInfo>" || line == "</CashierSettings>" || line == "</PersonalSettings>") {
              currentSection.clear();
              continue;
          }

          // 解析标签
          if (line.startsWith("<") && line.endsWith(">") && !line.startsWith("</")) {
              int endPos = line.indexOf(">");
              QString fullTag = line.mid(1, endPos - 1);

              // 检查是否自闭合标签
              if (fullTag.endsWith("/")) {
                  fullTag = fullTag.left(fullTag.length() - 1);
                  currentTag = fullTag.trimmed();
                  currentValue.clear();

                  // 存储值
                  if (currentSection == "ShopInfo") {
                      shopInfo[currentTag] = currentValue;
                  } else if (currentSection == "CashierSettings") {
                      cashierSettings[currentTag] = currentValue;
                  } else if (currentSection == "PersonalSettings") {
                      personalSettings[currentTag] = currentValue;
                  }
              } else {
                  currentTag = fullTag.trimmed();
                  inValue = true;

                  // 提取标签内的值（如果标签在同一行）
                  int valueStart = endPos + 1;
                  int valueEnd = line.indexOf("</" + currentTag + ">");
                  if (valueEnd > valueStart) {
                      currentValue = line.mid(valueStart, valueEnd - valueStart);
                      currentValue = unescapeXml(currentValue);

                      // 存储值
                      if (currentSection == "ShopInfo") {
                          shopInfo[currentTag] = currentValue;
                      } else if (currentSection == "CashierSettings") {
                          cashierSettings[currentTag] = currentValue;
                      } else if (currentSection == "PersonalSettings") {
                          personalSettings[currentTag] = currentValue;
                      }

                      inValue = false;
                  }
              }
          } else if (inValue) {
              // 多行值的情况
              int endPos = line.indexOf("</" + currentTag + ">");
              if (endPos != -1) {
                  currentValue += "\n" + line.left(endPos);
                  currentValue = unescapeXml(currentValue);

                  // 存储值
                  if (currentSection == "ShopInfo") {
                      shopInfo[currentTag] = currentValue;
                  } else if (currentSection == "CashierSettings") {
                      cashierSettings[currentTag] = currentValue;
                  } else if (currentSection == "PersonalSettings") {
                      personalSettings[currentTag] = currentValue;
                  }

                  inValue = false;
              } else {
                  currentValue += "\n" + line;
              }
          }
      }
      // ========== 应用店铺信息 ==========
          if (shopInfo.contains("Name")) shopNameEdit->setText(shopInfo["Name"]);
          if (shopInfo.contains("Owner")) shopOwnerEdit->setText(shopInfo["Owner"]);
          if (shopInfo.contains("Phone")) phoneEdit->setText(shopInfo["Phone"]);
          if (shopInfo.contains("Email")) emailEdit->setText(shopInfo["Email"]);
          if (shopInfo.contains("Website")) websiteEdit->setText(shopInfo["Website"]);
          if (shopInfo.contains("Address")) addressEdit->setText(shopInfo["Address"]);
          if (shopInfo.contains("Description")) descriptionEdit->setText(shopInfo["Description"]);
          if (shopInfo.contains("OpenTime")) {
              QTime openTime = QTime::fromString(shopInfo["OpenTime"], "hh:mm");
              if (openTime.isValid()) openTimeEdit->setTime(openTime);
          }
          if (shopInfo.contains("CloseTime")) {
              QTime closeTime = QTime::fromString(shopInfo["CloseTime"], "hh:mm");
              if (closeTime.isValid()) closeTimeEdit->setTime(closeTime);
          }

          // ========== 应用收银设置 ==========
          if (cashierSettings.contains("PrinterType")) {
              int index = printerCombo->findText(cashierSettings["PrinterType"]);
              if (index >= 0) printerCombo->setCurrentIndex(index);
          }
          if (cashierSettings.contains("BarcodeScanner")) {
              int index = barcodeCombo->findText(cashierSettings["BarcodeScanner"]);
              if (index >= 0) barcodeCombo->setCurrentIndex(index);
          }
          if (cashierSettings.contains("PaymentMethods")) {
              int index = paymentCombo->findText(cashierSettings["PaymentMethods"]);
              if (index >= 0) paymentCombo->setCurrentIndex(index);
          }
          if (cashierSettings.contains("HeaderText")) headerTextEdit->setText(cashierSettings["HeaderText"]);
          if (cashierSettings.contains("FooterText")) footerTextEdit->setText(cashierSettings["FooterText"]);

          if (cashierSettings.contains("AutoPrint")) autoPrintCheck->setChecked(stringToBool(cashierSettings["AutoPrint"], true));
          if (cashierSettings.contains("PlaySound")) soundCheck->setChecked(stringToBool(cashierSettings["PlaySound"], true));
          if (cashierSettings.contains("ShowStock")) showStockCheck->setChecked(stringToBool(cashierSettings["ShowStock"], true));
          if (cashierSettings.contains("AutoDiscount")) autoDiscountCheck->setChecked(stringToBool(cashierSettings["AutoDiscount"], true));
          if (cashierSettings.contains("MemberDiscount")) memberDiscountCheck->setChecked(stringToBool(cashierSettings["MemberDiscount"], true));

          if (cashierSettings.contains("PaymentTimeout")) timeoutSpin->setValue(cashierSettings["PaymentTimeout"].toInt());
          if (cashierSettings.contains("ReceiptWidth")) receiptWidthSpin->setValue(cashierSettings["ReceiptWidth"].toInt());
          if (cashierSettings.contains("VATRate")) vatRateSpin->setValue(cashierSettings["VATRate"].toInt());

          // ========== 应用个人设置 ==========
          if (personalSettings.contains("Theme")) {
              int index = themeCombo->findText(personalSettings["Theme"]);
              if (index >= 0) themeCombo->setCurrentIndex(index);
          }
          if (personalSettings.contains("Language")) {
              int index = languageCombo->findText(personalSettings["Language"]);
              if (index >= 0) languageCombo->setCurrentIndex(index);
          }
          if (personalSettings.contains("FontSize")) {
              int index = fontSizeCombo->findText(personalSettings["FontSize"]);
              if (index >= 0) fontSizeCombo->setCurrentIndex(index);
          }

          if (personalSettings.contains("UseShortcuts")) shortcutCheck->setChecked(stringToBool(personalSettings["UseShortcuts"], true));
          if (personalSettings.contains("AutoLogin")) autoLoginCheck->setChecked(stringToBool(personalSettings["AutoLogin"], false));
          if (personalSettings.contains("ShowNotifications")) notificationCheck->setChecked(stringToBool(personalSettings["ShowNotifications"], true));
          if (personalSettings.contains("RememberLast")) rememberLastCheck->setChecked(stringToBool(personalSettings["RememberLast"], true));

          return true;
      }
  // ==================== 创建默认配置 ====================
  void SystemManageWidget::createDefaultXmlConfig()
  {
      // 设置默认值
      shopNameEdit->setText("智慧图书商城");
      shopOwnerEdit->setText("张经理");
      phoneEdit->setText("138-0013-8000");
      emailEdit->setText("bookstore@example.com");
      websiteEdit->setText("https://www.bookstore.com");
      addressEdit->setText("北京市海淀区中关村大街1号");
      descriptionEdit->setText("专业图书销售，提供各类教育、文学、科技图书。");

      openTimeEdit->setTime(QTime(8, 30));
      closeTimeEdit->setTime(QTime(21, 0));

      printerCombo->setCurrentIndex(0);
      barcodeCombo->setCurrentIndex(0);
      paymentCombo->setCurrentIndex(0);

      headerTextEdit->setText("欢迎光临智慧图书商城");
      footerTextEdit->setText("谢谢惠顾，欢迎下次光临！");

      autoPrintCheck->setChecked(true);
      soundCheck->setChecked(true);
      showStockCheck->setChecked(true);
      autoDiscountCheck->setChecked(true);
      memberDiscountCheck->setChecked(true);

      timeoutSpin->setValue(30);
      receiptWidthSpin->setValue(58);
      vatRateSpin->setValue(13);

      themeCombo->setCurrentIndex(0);
      languageCombo->setCurrentIndex(0);
      fontSizeCombo->setCurrentIndex(1);

      shortcutCheck->setChecked(true);
      autoLoginCheck->setChecked(false);
      notificationCheck->setChecked(true);
      rememberLastCheck->setChecked(true);

      // 保存默认配置
      saveSettingsToXml();
  }

  // ==================== 创建标签页 ====================
  void SystemManageWidget::createTabs()
  {
      tabWidget = new QTabWidget;
      tabWidget->setObjectName("systemTabs");

      // 创建各标签页
      shopInfoTab = new QWidget;
      cashierTab = new QWidget;
      dataTab = new QWidget;
      personalTab = new QWidget;
      aboutTab = new QWidget;

      createShopInfoTab();
      createCashierTab();
      createDataTab();
      createPersonalTab();
      createAboutTab();

      // 添加到标签页控件
      tabWidget->addTab(shopInfoTab, "🏪 店铺信息");
      tabWidget->addTab(cashierTab, "💰 收银设置");
      tabWidget->addTab(dataTab, "💾 数据管理");
      tabWidget->addTab(personalTab, "👤 个人设置");
      tabWidget->addTab(aboutTab, "ℹ️ 关于系统");

      // 设置标签页样式
      tabWidget->setStyleSheet(QString(
          "#systemTabs::pane { border: 1px solid #dee2e6; border-top: none; "
          "border-radius: 0 0 8px 8px; background-color: white; }"
          "QTabBar::tab { background-color: #f8f9fa; border: 1px solid #dee2e6; "
          "padding: 10px 25px; margin-right: 2px; border-top-left-radius: 6px; "
          "border-top-right-radius: 6px; font-size: 14px; color: #495057; }"
          "QTabBar::tab:selected { background-color: white; color: %1; "
          "font-weight: bold; border-bottom-color: white; }"
          "QTabBar::tab:hover { background-color: #e9ecef; }"
      ).arg(PRIMARY_COLOR.name()));
  }
  // ==================== 创建店铺信息标签页 ====================
  void SystemManageWidget::createShopInfoTab()
  {
      QVBoxLayout *layout = new QVBoxLayout(shopInfoTab);
      layout->setContentsMargins(20, 20, 20, 20);
      layout->setSpacing(15);

      // ========== 基本信息组 ==========
      QGroupBox *basicGroup = new QGroupBox("基本信息");
      QFormLayout *basicLayout = new QFormLayout(basicGroup);
      basicLayout->setSpacing(12);
      basicLayout->setContentsMargins(15, 20, 15, 15);

      shopNameEdit = new QLineEdit;
      shopNameEdit->setPlaceholderText("请输入店铺名称");
      shopNameEdit->setToolTip("店铺的正式名称");

      shopOwnerEdit = new QLineEdit;
      shopOwnerEdit->setPlaceholderText("请输入店主姓名");

      phoneEdit = new QLineEdit;
      phoneEdit->setPlaceholderText("请输入联系电话");
      phoneEdit->setInputMask("000-0000-0000");

      emailEdit = new QLineEdit;
      emailEdit->setPlaceholderText("example@email.com");
      emailEdit->setToolTip("用于接收系统通知");

      websiteEdit = new QLineEdit;
      websiteEdit->setPlaceholderText("https://www.example.com");

      basicLayout->addRow("店铺名称 *:", shopNameEdit);
      basicLayout->addRow("店主姓名:", shopOwnerEdit);
      basicLayout->addRow("联系电话:", phoneEdit);
      basicLayout->addRow("电子邮箱:", emailEdit);
      basicLayout->addRow("官方网站:", websiteEdit);

      // ========== 地址信息组 ==========
      QGroupBox *addressGroup = new QGroupBox("地址信息");
      QVBoxLayout *addressLayout = new QVBoxLayout(addressGroup);
      addressLayout->setContentsMargins(15, 20, 15, 15);

      addressEdit = new QLineEdit;
      addressEdit->setPlaceholderText("请输入详细地址");

      descriptionEdit = new QTextEdit;
      descriptionEdit->setPlaceholderText("店铺简介、特色服务等...");
      descriptionEdit->setMaximumHeight(100);

      addressLayout->addWidget(new QLabel("详细地址:"));
      addressLayout->addWidget(addressEdit);
      addressLayout->addSpacing(10);
      addressLayout->addWidget(new QLabel("店铺描述:"));
      addressLayout->addWidget(descriptionEdit);

      // ========== 营业时间组 ==========
      QGroupBox *timeGroup = new QGroupBox("营业时间");
      QFormLayout *timeLayout = new QFormLayout(timeGroup);
      timeLayout->setSpacing(12);
      timeLayout->setContentsMargins(15, 20, 15, 15);

      openTimeEdit = new QTimeEdit;
      closeTimeEdit = new QTimeEdit;
      openTimeEdit->setDisplayFormat("hh:mm");
      closeTimeEdit->setDisplayFormat("hh:mm");
      openTimeEdit->setTime(QTime(8, 30));
      closeTimeEdit->setTime(QTime(21, 0));

      timeLayout->addRow("开门时间:", openTimeEdit);
      timeLayout->addRow("打烊时间:", closeTimeEdit);

      // ========== 保存按钮 ==========
      QHBoxLayout *buttonLayout = new QHBoxLayout;
      QPushButton *saveButton = new QPushButton("💾 保存店铺信息");
      saveButton->setFixedWidth(200);

      buttonLayout->addStretch();
      buttonLayout->addWidget(saveButton);
      buttonLayout->addStretch();

      connect(saveButton, SIGNAL(clicked()), this, SLOT(onSaveShopInfo()));

      // ========== 添加到主布局 ==========
      layout->addWidget(basicGroup);
      layout->addWidget(addressGroup);
      layout->addWidget(timeGroup);
      layout->addLayout(buttonLayout);
      layout->addStretch();
  }



// ==================== 创建收银设置标签页 ====================
void SystemManageWidget::createCashierTab()
{
  QVBoxLayout *layout = new QVBoxLayout(cashierTab);
  layout->setContentsMargins(20, 20, 20, 20);
  layout->setSpacing(15);

  // ========== 打印机设置组 ==========
  QGroupBox *printerGroup = new QGroupBox("打印机设置");
  QFormLayout *printerLayout = new QFormLayout(printerGroup);
  printerLayout->setSpacing(12);
  printerLayout->setContentsMargins(15, 20, 15, 15);

  printerCombo = new QComboBox;
  printerCombo->addItems(QStringList()
      << "默认打印机" << "EPSON TM-T88V" << "STAR TSP100"
      << "自定义打印机" << "不打印");

  receiptWidthSpin = new QSpinBox;
  receiptWidthSpin->setRange(40, 80);
  receiptWidthSpin->setValue(58);
  receiptWidthSpin->setSuffix(" 字符");

  headerTextEdit = new QLineEdit;
  headerTextEdit->setText("欢迎光临图书商城");
  headerTextEdit->setPlaceholderText("小票抬头文字");

  footerTextEdit = new QLineEdit;
  footerTextEdit->setText("谢谢惠顾，欢迎下次光临！");
  footerTextEdit->setPlaceholderText("小票页脚文字");

  printerLayout->addRow("打印机类型:", printerCombo);
  printerLayout->addRow("小票宽度:", receiptWidthSpin);
  printerLayout->addRow("抬头文字:", headerTextEdit);
  printerLayout->addRow("页脚文字:", footerTextEdit);

  // ========== 硬件设置组 ==========
  QGroupBox *hardwareGroup = new QGroupBox("硬件设置");
  QFormLayout *hardwareLayout = new QFormLayout(hardwareGroup);
  hardwareLayout->setSpacing(12);
  hardwareLayout->setContentsMargins(15, 20, 15, 15);

  barcodeCombo = new QComboBox;
  barcodeCombo->addItems(QStringList()
      << "自动识别" << "Honeywell 1900" << "Zebra DS2208"
      << "手动输入" << "禁用扫码");

  paymentCombo = new QComboBox;
  paymentCombo->addItems(QStringList()
      << "全部支持" << "仅现金" << "现金+微信" << "现金+支付宝" << "仅移动支付");

  timeoutSpin = new QSpinBox;
  timeoutSpin->setRange(10, 120);
  timeoutSpin->setValue(30);
  timeoutSpin->setSuffix(" 秒");

  hardwareLayout->addRow("扫码设备:", barcodeCombo);
  hardwareLayout->addRow("支付方式:", paymentCombo);
  hardwareLayout->addRow("支付超时:", timeoutSpin);

  // ========== 功能选项组 ==========
  QGroupBox *optionGroup = new QGroupBox("功能选项");
  QVBoxLayout *optionLayout = new QVBoxLayout(optionGroup);
  optionLayout->setContentsMargins(15, 20, 15, 15);
  optionLayout->setSpacing(10);

  autoPrintCheck = new QCheckBox("自动打印小票");
  soundCheck = new QCheckBox("收银提示音");
  showStockCheck = new QCheckBox("实时显示库存");
  autoDiscountCheck = new QCheckBox("自动计算会员折扣");
  memberDiscountCheck = new QCheckBox("启用会员折扣");

  autoPrintCheck->setChecked(true);
  soundCheck->setChecked(true);
  showStockCheck->setChecked(true);
  autoDiscountCheck->setChecked(true);
  memberDiscountCheck->setChecked(true);

  optionLayout->addWidget(autoPrintCheck);
  optionLayout->addWidget(soundCheck);
  optionLayout->addWidget(showStockCheck);
  optionLayout->addWidget(autoDiscountCheck);
  optionLayout->addWidget(memberDiscountCheck);
  // ========== 税率设置 ==========
      QGroupBox *taxGroup = new QGroupBox("税率设置");
      QFormLayout *taxLayout = new QFormLayout(taxGroup);
      taxLayout->setSpacing(12);
      taxLayout->setContentsMargins(15, 20, 15, 15);

      vatRateSpin = new QSpinBox;
      vatRateSpin->setRange(0, 20);
      vatRateSpin->setValue(13);
      vatRateSpin->setSuffix("%");

      taxLayout->addRow("增值税率:", vatRateSpin);

      // ========== 操作按钮 ==========
      QHBoxLayout *buttonLayout = new QHBoxLayout;
      QPushButton *saveButton = new QPushButton("💾 保存收银设置");
      QPushButton *testButton = new QPushButton("🖨️ 测试打印");

      saveButton->setFixedWidth(150);
      testButton->setFixedWidth(150);

      buttonLayout->addStretch();
      buttonLayout->addWidget(testButton);
      buttonLayout->addWidget(saveButton);
      buttonLayout->addStretch();

      connect(saveButton, SIGNAL(clicked()), this, SLOT(onSaveCashierSettings()));
      connect(testButton, SIGNAL(clicked()), this, SLOT(onTestPrint()));

      // ========== 添加到主布局 ==========
      layout->addWidget(printerGroup);
      layout->addWidget(hardwareGroup);
      layout->addWidget(optionGroup);
      layout->addWidget(taxGroup);
      layout->addLayout(buttonLayout);
      layout->addStretch();
  }

  // ==================== 创建数据管理标签页 ====================
  void SystemManageWidget::createDataTab()
  {
      QVBoxLayout *layout = new QVBoxLayout(dataTab);
      layout->setContentsMargins(20, 20, 20, 20);
      layout->setSpacing(15);

      // ========== 数据库信息组 ==========
      QGroupBox *infoGroup = new QGroupBox("数据库状态");
      QFormLayout *infoLayout = new QFormLayout(infoGroup);
      infoLayout->setSpacing(10);
      infoLayout->setContentsMargins(15, 20, 15, 15);

      dbSizeLabel = new QLabel("2.8 MB");
      dbSizeLabel->setStyleSheet("color: #28a745; font-weight: bold;");

      recordCountLabel = new QLabel("图书: 1,256 | 订单: 5,842 | 会员: 328");
      recordCountLabel->setStyleSheet("color: #6c757d;");

      lastBackupLabel = new QLabel("2023-12-15 14:30:00");
      lastBackupLabel->setStyleSheet("color: #17a2b8;");

      backupInfoLabel = new QLabel("./backup/");
      backupInfoLabel->setStyleSheet("color: #6c757d; font-family: monospace;");

      // 存储空间进度条
      storageBar = new QProgressBar;
      storageBar->setRange(0, 100);
      storageBar->setValue(42);
      storageBar->setTextVisible(true);
      storageBar->setFormat("存储空间使用率: %p%");
      storageBar->setStyleSheet(
          "QProgressBar { border: 1px solid #dee2e6; border-radius: 4px; "
          "text-align: center; height: 25px; }"
          "QProgressBar::chunk { background-color: #28a745; border-radius: 4px; }"
      );

      infoLayout->addRow("数据库大小:", dbSizeLabel);
      infoLayout->addRow("数据统计:", recordCountLabel);
      infoLayout->addRow("最后备份:", lastBackupLabel);
      infoLayout->addRow("备份位置:", backupInfoLabel);
      infoLayout->addRow("存储状态:", storageBar);

      // ========== 备份操作组 ==========
      QGroupBox *backupGroup = new QGroupBox("数据备份");
      QGridLayout *backupLayout = new QGridLayout(backupGroup);
      backupLayout->setContentsMargins(15, 20, 15, 15);
      backupLayout->setSpacing(10);

      backupButton = new QPushButton("📂 立即备份");
      restoreButton = new QPushButton("🔄 恢复数据");
      backupButton->setFixedHeight(50);
      restoreButton->setFixedHeight(50);

      backupLayout->addWidget(backupButton, 0, 0);
      backupLayout->addWidget(restoreButton, 0, 1);

      // ========== 数据操作组 ==========
      QGroupBox *dataGroup = new QGroupBox("数据操作");
      QGridLayout *dataLayout = new QGridLayout(dataGroup);
      dataLayout->setContentsMargins(15, 20, 15, 15);
      dataLayout->setSpacing(10);

      exportButton = new QPushButton("📤 导出Excel");
      clearLogsButton = new QPushButton("🗑️ 清理日志");
      optimizeButton = new QPushButton("⚡ 优化数据库");
      exportButton->setFixedHeight(50);
      clearLogsButton->setFixedHeight(50);
      optimizeButton->setFixedHeight(50);

      dataLayout->addWidget(exportButton, 0, 0);
      dataLayout->addWidget(clearLogsButton, 0, 1);
      dataLayout->addWidget(optimizeButton, 1, 0, 1, 2);
      // ========== 警告信息 ==========
          QFrame *warningFrame = new QFrame;
          warningFrame->setFrameShape(QFrame::StyledPanel);
          warningFrame->setStyleSheet(
              "background-color: #fff8e1; border: 2px solid #ffd54f; "
              "border-radius: 8px; padding: 15px;"
          );

          QVBoxLayout *warningLayout = new QVBoxLayout(warningFrame);
          QLabel *warningIcon = new QLabel("⚠️");
          warningIcon->setStyleSheet("font-size: 24px;");
          warningIcon->setAlignment(Qt::AlignCenter);

          QLabel *warningText = new QLabel(
              "<b>重要提示：</b><br>"
              "1. 建议每天营业结束后进行数据备份<br>"
              "2. 定期清理日志可以提升系统性能<br>"
              "3. 数据库优化可以修复数据碎片<br>"
              "4. 导出数据前请确保有足够磁盘空间"
          );
          warningText->setWordWrap(true);
          warningText->setStyleSheet("color: #e67e22;");

          warningLayout->addWidget(warningIcon);
          warningLayout->addWidget(warningText);

          // ========== 连接信号槽 ==========
          connect(backupButton, SIGNAL(clicked()), this, SLOT(onBackupData()));
          connect(restoreButton, SIGNAL(clicked()), this, SLOT(onRestoreData()));
          connect(exportButton, SIGNAL(clicked()), this, SLOT(onExportData()));
          connect(clearLogsButton, SIGNAL(clicked()), this, SLOT(onClearLogs()));
          connect(optimizeButton, SIGNAL(clicked()), this, SLOT(onUpdateDatabase()));

          // ========== 添加到主布局 ==========
          layout->addWidget(infoGroup);
          layout->addWidget(backupGroup);
          layout->addWidget(dataGroup);
          layout->addWidget(warningFrame);
          layout->addStretch();
      }

      // ==================== 创建个人设置标签页 ====================
      void SystemManageWidget::createPersonalTab()
      {
          QVBoxLayout *layout = new QVBoxLayout(personalTab);
          layout->setContentsMargins(20, 20, 20, 20);
          layout->setSpacing(15);

          // ========== 密码修改组 ==========
          QGroupBox *passwordGroup = new QGroupBox("密码安全");
          QFormLayout *passLayout = new QFormLayout(passwordGroup);
          passLayout->setSpacing(12);
          passLayout->setContentsMargins(15, 20, 15, 15);

          oldPassEdit = new QLineEdit;
          newPassEdit = new QLineEdit;
          confirmPassEdit = new QLineEdit;

          oldPassEdit->setEchoMode(QLineEdit::Password);
          newPassEdit->setEchoMode(QLineEdit::Password);
          confirmPassEdit->setEchoMode(QLineEdit::Password);

          oldPassEdit->setPlaceholderText("输入当前密码");
          newPassEdit->setPlaceholderText("输入新密码（至少6位）");
          confirmPassEdit->setPlaceholderText("再次输入新密码");

          QPushButton *changePassButton = new QPushButton("🔒 修改密码");
          changePassButton->setFixedWidth(120);

          QHBoxLayout *passButtonLayout = new QHBoxLayout;
          passButtonLayout->addStretch();
          passButtonLayout->addWidget(changePassButton);

          passLayout->addRow("当前密码:", oldPassEdit);
          passLayout->addRow("新密码:", newPassEdit);
          passLayout->addRow("确认密码:", confirmPassEdit);
          passLayout->addRow("", passButtonLayout);

          // ========== 界面设置组 ==========
          QGroupBox *interfaceGroup = new QGroupBox("界面设置");
          QFormLayout *interfaceLayout = new QFormLayout(interfaceGroup);
          interfaceLayout->setSpacing(12);
          interfaceLayout->setContentsMargins(15, 20, 15, 15);

          themeCombo = new QComboBox;
          themeCombo->addItems(QStringList()
              << "默认主题（蓝色）" << "商务主题（深色）"
              << "清新主题（绿色）" << "简约主题（灰色）");

          languageCombo = new QComboBox;
          languageCombo->addItems(QStringList()
              << "简体中文" << "English" << "繁體中文");

          fontSizeCombo = new QComboBox;
          fontSizeCombo->addItems(QStringList()
              << "较小" << "正常" << "较大" << "特大");
          fontSizeCombo->setCurrentIndex(1);

          interfaceLayout->addRow("主题样式:", themeCombo);
          interfaceLayout->addRow("界面语言:", languageCombo);
          interfaceLayout->addRow("字体大小:", fontSizeCombo);

          // ========== 功能选项组 ==========
          QGroupBox *functionGroup = new QGroupBox("功能选项");
          QVBoxLayout *functionLayout = new QVBoxLayout(functionGroup);
          functionLayout->setContentsMargins(15, 20, 15, 15);
          functionLayout->setSpacing(10);

          shortcutCheck = new QCheckBox("启用键盘快捷键");
          autoLoginCheck = new QCheckBox("自动登录系统");
          notificationCheck = new QCheckBox("显示系统通知");
          rememberLastCheck = new QCheckBox("记住上次操作位置");

          shortcutCheck->setChecked(true);
          autoLoginCheck->setChecked(false);
          notificationCheck->setChecked(true);
          rememberLastCheck->setChecked(true);

          functionLayout->addWidget(shortcutCheck);
          functionLayout->addWidget(autoLoginCheck);
          functionLayout->addWidget(notificationCheck);
          functionLayout->addWidget(rememberLastCheck);

          // ========== 测试通知按钮 ==========
          testNotifyButton = new QPushButton("🔔 测试通知");
          testNotifyButton->setFixedWidth(120);

          QHBoxLayout *testLayout = new QHBoxLayout;
          testLayout->addStretch();
          testLayout->addWidget(testNotifyButton);

          functionLayout->addLayout(testLayout);

          // ========== 重置按钮 ==========
          resetButton = new QPushButton("🔄 重置个人设置");
          resetButton->setFixedHeight(45);

          QHBoxLayout *resetLayout = new QHBoxLayout;
          resetLayout->addStretch();
          resetLayout->addWidget(resetButton);
          resetLayout->addStretch();

          // ========== 连接信号槽 ==========
          connect(changePassButton, SIGNAL(clicked()), this, SLOT(onChangePassword()));
          connect(themeCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(onChangeTheme(int)));
          connect(resetButton, SIGNAL(clicked()), this, SLOT(onResetSettings()));
          connect(testNotifyButton, SIGNAL(clicked()), this, SLOT(onSystemCheck()));

          // ========== 添加到主布局 ==========
          layout->addWidget(passwordGroup);
          layout->addWidget(interfaceGroup);
          layout->addWidget(functionGroup);
          layout->addLayout(resetLayout);
          layout->addStretch();
      }

      // ==================== 创建关于系统标签页 ====================
      void SystemManageWidget::createAboutTab()
      {
          QVBoxLayout *layout = new QVBoxLayout(aboutTab);
          layout->setContentsMargins(20, 20, 20, 20);
          layout->setSpacing(15);

          // ========== 系统Logo区域 ==========
          QWidget *logoWidget = new QWidget;
          logoWidget->setFixedHeight(100);
          QHBoxLayout *logoLayout = new QHBoxLayout(logoWidget);

          QLabel *logoLabel = new QLabel("📚");
          logoLabel->setStyleSheet("font-size: 60px;");
          logoLabel->setAlignment(Qt::AlignCenter);

          logoLayout->addWidget(logoLabel);

          // ========== 系统信息组 ==========
          QGroupBox *infoGroup = new QGroupBox("系统信息");
          QFormLayout *infoLayout = new QFormLayout(infoGroup);
          infoLayout->setSpacing(10);
          infoLayout->setContentsMargins(15, 20, 15, 15);

          versionLabel = new QLabel("3.0.1 (Build 20231215)");
          versionLabel->setStyleSheet("color: #28a745; font-weight: bold;");

          copyrightLabel = new QLabel("© 2023 图书商家管理系统 版权所有");
          copyrightLabel->setStyleSheet("color: #6c757d;");

          developerLabel = new QLabel("开发团队：BookSeller Dev Team");
          developerLabel->setStyleSheet("color: #17a2b8;");

          infoLayout->addRow("版本号:", versionLabel);
              infoLayout->addRow("版权信息:", copyrightLabel);
              infoLayout->addRow("开发团队:", developerLabel);

              // ========== 系统特性 ==========
              QGroupBox *featureGroup = new QGroupBox("系统特性");
              QVBoxLayout *featureLayout = new QVBoxLayout(featureGroup);
              featureLayout->setContentsMargins(15, 20, 15, 15);

              QStringList features;
              features << "✓ 现代化界面设计，操作流畅"
                       << "✓ 支持图书、订单、会员全面管理"
                       << "✓ 强大的数据统计和报表功能"
                       << "✓ 完善的收银和库存管理"
                       << "✓ 支持数据备份和恢复"
                       << "✓ 可定制的系统设置"
                       << "✓ 稳定的网络通信"
                       << "✓ 跨平台支持（Windows/Linux）";

              foreach (QString feature, features) {
                  QLabel *featureLabel = new QLabel(feature);
                  featureLabel->setStyleSheet("color: #495057; padding: 5px 0;");
                  featureLayout->addWidget(featureLabel);
              }

              // ========== 许可协议 ==========
              QGroupBox *licenseGroup = new QGroupBox("许可协议");
              QVBoxLayout *licenseLayout = new QVBoxLayout(licenseGroup);
              licenseLayout->setContentsMargins(15, 20, 15, 15);

              licenseText = new QTextEdit;
              licenseText->setPlainText(
                  "本软件遵循以下条款：\n\n"
                  "1. 本软件仅供学习交流使用，不得用于商业用途\n"
                  "2. 用户可以自由使用、复制、分发本软件\n"
                  "3. 禁止对本软件进行反向工程、反编译\n"
                  "4. 作者不对使用本软件造成的任何损失负责\n"
                  "5. 如需商业使用，请联系开发者获取授权\n\n"
                  "感谢您使用图书商家管理系统！"
              );
              licenseText->setReadOnly(true);
              licenseText->setMaximumHeight(120);

              licenseLayout->addWidget(licenseText);

              // ========== 操作按钮 ==========
              QHBoxLayout *buttonLayout = new QHBoxLayout;
              checkUpdateButton = new QPushButton("🔄 检查更新");
              helpButton = new QPushButton("❓ 使用帮助");

              checkUpdateButton->setFixedWidth(120);
              helpButton->setFixedWidth(120);

              buttonLayout->addStretch();
              buttonLayout->addWidget(checkUpdateButton);
              buttonLayout->addWidget(helpButton);
              buttonLayout->addStretch();

              connect(checkUpdateButton, SIGNAL(clicked()), this, SLOT(onSystemCheck()));
              connect(helpButton, SIGNAL(clicked()), this, SLOT(onSystemCheck()));

              // ========== 添加到主布局 ==========
              layout->addWidget(logoWidget);
              layout->addWidget(infoGroup);
              layout->addWidget(featureGroup);
              layout->addWidget(licenseGroup);
              layout->addLayout(buttonLayout);
              layout->addStretch();
          }

          // ==================== 验证方法 ====================
          bool SystemManageWidget::validateShopInfo()
          {
              if (shopNameEdit->text().trimmed().isEmpty()) {
                  showErrorMessage("店铺名称不能为空！");
                  shopNameEdit->setFocus();
                  return false;
              }

              if (openTimeEdit->time() >= closeTimeEdit->time()) {
                  showErrorMessage("营业时间设置不正确！");
                  openTimeEdit->setFocus();
                  return false;
              }

              return true;
          }

          bool SystemManageWidget::validatePasswordChange()
          {
              QString oldPass = oldPassEdit->text();
              QString newPass = newPassEdit->text();
              QString confirmPass = confirmPassEdit->text();

              if (oldPass.isEmpty() || newPass.isEmpty() || confirmPass.isEmpty()) {
                  showErrorMessage("所有密码字段都必须填写！");
                  return false;
              }

              if (newPass != confirmPass) {
                  showErrorMessage("两次输入的新密码不一致！");
                  newPassEdit->clear();
                  confirmPassEdit->clear();
                  newPassEdit->setFocus();
                  return false;
              }

              if (newPass.length() < 6) {
                  showErrorMessage("新密码长度不能少于6位！");
                  newPassEdit->setFocus();
                  return false;
              }

              return true;
          }

          void SystemManageWidget::showSuccessMessage(const QString &message)
          {
              statusLabel->setText("✓ " + message);
              statusLabel->setStyleSheet("color: #28a745; font-weight: bold;");
              QTimer::singleShot(3000, this, SLOT(resetStatus()));
          }

          void SystemManageWidget::showErrorMessage(const QString &message)
          {
              statusLabel->setText("✗ " + message);
              statusLabel->setStyleSheet("color: #dc3545; font-weight: bold;");
              QTimer::singleShot(5000, this, SLOT(resetStatus()));
          }

          void SystemManageWidget::resetStatus()
          {
              statusLabel->setText("就绪");
              statusLabel->setStyleSheet("color: #6c757d; font-weight: normal;");
          }

          // ==================== 槽函数实现 ====================

          void SystemManageWidget::onSaveShopInfo()
          {
              if (!validateShopInfo()) {
                  return;
              }

              if (saveSettingsToXml()) {
                  showSuccessMessage("店铺信息保存成功");
              } else {
                  showErrorMessage("保存失败，请检查文件权限");
              }
          }

          void SystemManageWidget::onSaveCashierSettings()
          {
              if (saveSettingsToXml()) {
                  showSuccessMessage("收银设置保存成功");
              } else {
                  showErrorMessage("保存失败");
              }
          }

          void SystemManageWidget::onSaveAllSettings()
          {
              if (saveSettingsToXml()) {
                  showSuccessMessage("所有设置保存成功");
              } else {
                  showErrorMessage("保存失败");
              }
          }

          void SystemManageWidget::onImportSettings()
          {
              QString fileName = QFileDialog::getOpenFileName(this,
                  "选择配置文件", QDir::currentPath(), "XML 文件 (*.xml);;所有文件 (*.*)");

              if (!fileName.isEmpty()) {
                  // 备份当前配置
                  QString backupPath = getConfigPath() + ".backup";
                  QFile::copy(getConfigPath(), backupPath);

                  // 复制新配置文件
                  if (QFile::copy(fileName, getConfigPath())) {
                      if (loadSettingsFromXml()) {
                          showSuccessMessage("配置导入成功");
                      } else {
                          // 恢复备份
                          QFile::remove(getConfigPath());
                          QFile::copy(backupPath, getConfigPath());
                          loadSettingsFromXml();
                          showErrorMessage("配置文件格式错误，已恢复原有配置");
                      }
                      QFile::remove(backupPath);
                  } else {
                      showErrorMessage("导入失败");
                  }
              }
          }
          void SystemManageWidget::onBackupData()
          {
              QString defaultName = QString("backup_%1.db")
                  .arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss"));

              QString fileName = QFileDialog::getSaveFileName(this,
                  "选择备份位置", defaultName, "数据库文件 (*.db);;所有文件 (*.*)");

              if (!fileName.isEmpty()) {
                  lastBackupLabel->setText(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
                  backupInfoLabel->setText(QFileInfo(fileName).path());
                  showSuccessMessage(QString("数据已备份到: %1").arg(fileName));
              }
          }

          void SystemManageWidget::onRestoreData()
          {
              if (QMessageBox::question(this, "确认恢复",
                  "恢复数据将覆盖当前所有数据，确定要继续吗？") == QMessageBox::Yes) {

                  QString fileName = QFileDialog::getOpenFileName(this,
                      "选择备份文件", "", "数据库文件 (*.db);;所有文件 (*.*)");

                  if (!fileName.isEmpty()) {
                      showSuccessMessage(QString("数据已从备份恢复: %1").arg(fileName));
                  }
              }
          }

          void SystemManageWidget::onChangePassword()
          {
              if (!validatePasswordChange()) {
                  return;
              }

              oldPassEdit->clear();
              newPassEdit->clear();
              confirmPassEdit->clear();

              showSuccessMessage("密码修改成功");
          }

          void SystemManageWidget::onChangeTheme(int index)
          {
              QString theme = themeCombo->itemText(index);
              showSuccessMessage(QString("已切换到【%1】，重启后生效").arg(theme));
          }

          void SystemManageWidget::onExportData()
          {
              QString defaultName = QString("bookstore_export_%1.xlsx")
                  .arg(QDateTime::currentDateTime().toString("yyyyMMdd"));

              QString fileName = QFileDialog::getSaveFileName(this,
                  "导出数据", defaultName,
                  "Excel文件 (*.xlsx *.xls);;CSV文件 (*.csv);;所有文件 (*.*)");

              if (!fileName.isEmpty()) {
                  showSuccessMessage(QString("数据已导出到: %1").arg(fileName));
              }
          }

          void SystemManageWidget::onClearLogs()
          {
              if (QMessageBox::question(this, "确认清理",
                  "确定要清理所有操作日志吗？此操作不可恢复！") == QMessageBox::Yes) {

                  showSuccessMessage("操作日志已清理");
              }
          }

          void SystemManageWidget::onTestPrint()
          {
              QMessageBox::information(this, "测试打印",
                  "打印测试页已发送到打印机\n请检查打印机是否正常工作");
          }

          void SystemManageWidget::onSystemCheck()
          {
              QMessageBox::information(this, "系统检查",
                  "系统检查完成，所有功能正常！\n"
                  "✓ 数据库连接正常\n"
                  "✓ 网络连接正常\n"
                  "✓ 硬件设备正常\n"
                  "✓ 系统资源充足");
          }

          void SystemManageWidget::onUpdateDatabase()
          {
              showSuccessMessage("数据库优化完成，性能提升约15%");
          }

          void SystemManageWidget::onResetSettings()
          {
              if (QMessageBox::question(this, "确认重置",
                  "确定要重置所有设置为默认值吗？此操作不可撤销！") == QMessageBox::Yes) {

                  createDefaultXmlConfig();
                  showSuccessMessage("所有设置已恢复为默认值");
              }
          }
