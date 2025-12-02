#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QHBoxLayout>
#include "Purchaser.h"

class LoginDialog : public QDialog
{
    Q_OBJECT
public:
    LoginDialog(Purchaser *purchaser, QWidget *parent = nullptr);

private slots:
    void onLogin();
    void onRegister();

private:
    Purchaser *m_purchaser;
    QLineEdit *m_usernameEdit;
    QLineEdit *m_passwordEdit;
};

#endif // LOGINDIALOG_H