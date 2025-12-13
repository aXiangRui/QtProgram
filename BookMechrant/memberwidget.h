#ifndef MEMBERWIDGET_H
#define MEMBERWIDGET_H

#include <QWidget>
#include <QTableWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QDateEdit>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QGroupBox>
#include <QDialog>

class RechargeDialog;  // 前向声明

class MemberWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MemberWidget(QWidget *parent = nullptr);
    ~MemberWidget();

private slots:
    void onAddMember();
    void onEditMember();
    void onDeleteMember();
    void onRecharge();
    void onSearch();
    void onSendMessage();
    void onExportMembers();
    void onRefresh();
    void onSaveMember();
    void onCancelEdit();
    void onTableSelectionChanged();
    void onLevelFilterChanged(int index);
    void onRechargeComplete(double amount, const QString &paymentMethod);

private:
    void createToolbar();
    void createTable();
    void createForm();
    void loadMembers();
    void clearForm();
    void populateForm(int row);
    bool validateMemberData();
    void updateMemberInTable(int row);
    void addMemberToTable();
    int findMemberByCard(const QString &cardNo);
    QString generateMemberCard();
    void updateMemberBalance(const QString &cardNo, double amount);

    // 工具栏
    QWidget *toolbar;
    QLineEdit *searchEdit;
    QComboBox *levelFilter;
    QPushButton *addButton;
    QPushButton *editButton;
    QPushButton *deleteButton;
    QPushButton *rechargeButton;
    QPushButton *messageButton;
    QPushButton *exportButton;
    QPushButton *refreshButton;

    // 会员表格
    QTableWidget *memberTable;

    // 编辑表单
    QGroupBox *formGroup;
    QLineEdit *nameEdit;
    QLineEdit *phoneEdit;
    QLineEdit *cardEdit;
    QDateEdit *birthdayEdit;
    QComboBox *levelCombo;
    QDoubleSpinBox *balanceEdit;
    QSpinBox *pointsEdit;
    QPushButton *saveButton;
    QPushButton *cancelButton;

    // 布局
    QHBoxLayout *mainLayout;

    // 充值对话框
    RechargeDialog *rechargeDialog;

    // 状态
    bool isEditing;
    int editingRow;
};

// 充值对话框类
class RechargeDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RechargeDialog(const QString &memberName, double currentBalance, QWidget *parent = nullptr);

signals:
    void rechargeRequested(double amount, const QString &paymentMethod);

private slots:
    void onRechargeClicked();
    void onAmountButtonClicked();
    void updateTotalAmount();

private:
    QLabel *memberNameLabel;
    QLabel *currentBalanceLabel;
    QLineEdit *amountEdit;
    QComboBox *paymentCombo;
    QPushButton *confirmButton;
    QPushButton *cancelButton;
    QList<QPushButton*> amountButtons;

    double customAmount;
};

#endif // MEMBERWIDGET_H
