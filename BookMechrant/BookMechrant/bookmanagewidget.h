#ifndef BOOKMANAGEWIDGET_H
#define BOOKMANAGEWIDGET_H

#include <QWidget>
#include <QTableWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QHeaderView>

class BookManageWidget : public QWidget
{
    Q_OBJECT

public:
    explicit BookManageWidget(QWidget *parent = 0);

private slots:
    void onAddBook();
    void onEditBook();
    void onDeleteBook();
    void onSearch();
    void onImport();
    void onExport();
    void onTableSelectionChanged();
    void onSaveBook();
    void onCancelEdit();
    void onRefresh();
    void onCategoryFilterChanged(int index);

private:
    void createToolbar();
    void createTable();
    void createForm();
    void loadBooks();
    void clearForm();
    void populateForm(int row);
    bool validateBookData();
    void updateBookInTable(int row);
    void addBookToTable();
    int findBookByISBN(const QString &isbn);

    // 工具栏
    QWidget *toolbar;
    QLineEdit *searchEdit;
    QComboBox *categoryFilter;
    QPushButton *addButton;
    QPushButton *editButton;
    QPushButton *deleteButton;
    QPushButton *importButton;
    QPushButton *exportButton;
    QPushButton *refreshButton;

    // 图书表格
    QTableWidget *bookTable;

    // 编辑表单
    QGroupBox *formGroup;
    QLineEdit *isbnEdit;
    QLineEdit *titleEdit;
    QLineEdit *authorEdit;
    QComboBox *categoryCombo;
    QDoubleSpinBox *priceEdit;
    QDoubleSpinBox *costEdit;
    QSpinBox *stockEdit;
    QSpinBox *warningStockEdit;
    QPushButton *saveButton;
    QPushButton *cancelButton;

    // 布局
    QHBoxLayout *mainLayout;

    // 状态
    bool isEditing;
    int editingRow;
};

#endif // BOOKMANAGEWIDGET_H
