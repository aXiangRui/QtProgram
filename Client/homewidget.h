#ifndef HOMEWIDGET_H
#define HOMEWIDGET_H

#include <QWidget>
#include <QApplication>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QComboBox>
#include <QPushButton>
#include <QSpinBox>
#include <QSlider>
#include <QTextEdit>
#include <QMessageBox>

namespace Ui {
class HomeWidget;
}

class HomeWidget : public QWidget
{
    Q_OBJECT

public:
    explicit HomeWidget(QWidget *parent = 0);
    ~HomeWidget();

private:
    Ui::HomeWidget *ui;
};

#endif // HOMEWIDGET_H
