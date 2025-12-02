#include "../include/LoginDialog.h"
#include <QMessageBox>

LoginDialog::LoginDialog(Purchaser *purchaser, QWidget *parent)
    : QDialog(parent), m_purchaser(purchaser)
{
    setWindowTitle("登录");
    setFixedSize(300, 200);
    
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    QFormLayout *formLayout = new QFormLayout();
    
    m_usernameEdit = new QLineEdit();
    m_passwordEdit = new QLineEdit();
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    
    formLayout->addRow("用户名:", m_usernameEdit);
    formLayout->addRow("密码:", m_passwordEdit);
    
    mainLayout->addLayout(formLayout);
    
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    
    QPushButton *loginButton = new QPushButton("登录");
    QPushButton *registerButton = new QPushButton("注册");
    QPushButton *cancelButton = new QPushButton("取消");
    
    buttonLayout->addWidget(loginButton);
    buttonLayout->addWidget(registerButton);
    buttonLayout->addWidget(cancelButton);
    
    mainLayout->addLayout(buttonLayout);
    
    connect(loginButton, &QPushButton::clicked, this, &LoginDialog::onLogin);
    connect(registerButton, &QPushButton::clicked, this, &LoginDialog::onRegister);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
}

void LoginDialog::onLogin()
{
    QString username = m_usernameEdit->text();
    QString password = m_passwordEdit->text();
    
    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "警告", "用户名和密码不能为空");
        return;
    }
    
    if (m_purchaser->Login(username, password)) {
        accept();
    } else {
        QMessageBox::warning(this, "登录失败", "用户名或密码错误");
    }
}

void LoginDialog::onRegister()
{
    QString username = m_usernameEdit->text();
    QString password = m_passwordEdit->text();
    
    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "警告", "用户名和密码不能为空");
        return;
    }
    
    if (m_purchaser->Regis(username, password)) {
        QMessageBox::information(this, "注册成功", "注册成功，请登录");
    } else {
        QMessageBox::warning(this, "注册失败", "用户名已存在");
    }
}