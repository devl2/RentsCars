#include "loginwindow.h"
#include "authmanager.h"
#include "mainwindow.h"
#include "clientwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QMessageBox>
#include <QIcon>
#include <QFont>
#include <QPalette>
#include <QSpacerItem>
#include <QStackedWidget>

LoginWindow::LoginWindow(QWidget *parent)
    : QDialog(parent)
    , loginEmailEdit(nullptr)
    , loginPasswordEdit(nullptr)
    , loginButton(nullptr)
    , regNameEdit(nullptr)
    , regEmailEdit(nullptr)
    , regPasswordEdit(nullptr)
    , registerButton(nullptr)
    , statusLabel(nullptr)
    , stackedWidget(nullptr)
    , loginWidget(nullptr)
    , registerWidget(nullptr)
{
    setupUI();
    setWindowTitle("Система аренды автомобилей");
    setFixedSize(450, 550);

    connect(AuthManager::instance(), &AuthManager::loginSuccess,
            this, &LoginWindow::onLoginSuccess);
    connect(AuthManager::instance(), &AuthManager::loginFailed,
            this, &LoginWindow::onLoginFailed);
    connect(AuthManager::instance(), &AuthManager::registrationSuccess,
            this, &LoginWindow::onRegistrationSuccess);
    connect(AuthManager::instance(), &AuthManager::registrationFailed,
            this, &LoginWindow::onRegistrationFailed);
}

void LoginWindow::setupUI() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    QFont appTitleFont;
    appTitleFont.setPointSize(18);
    appTitleFont.setBold(true);
    appTitleFont.setFamily("Arial");

    stackedWidget = new QStackedWidget(this);
    stackedWidget->setStyleSheet("QStackedWidget { background-color: white; }");

    createLoginWidget();
    createRegisterWidget();

    stackedWidget->addWidget(loginWidget);
    stackedWidget->addWidget(registerWidget);

    mainLayout->addWidget(stackedWidget, 1);

    statusLabel = new QLabel("");
    statusLabel->setAlignment(Qt::AlignCenter);
    statusLabel->setStyleSheet("color: red; font-size: 12px; padding: 10px; background-color: #f8f9fa; border-top: 1px solid #dee2e6;");
    statusLabel->setWordWrap(true);
    mainLayout->addWidget(statusLabel);
}

void LoginWindow::createLoginWidget() {
    loginWidget = new QWidget();
    QVBoxLayout *loginLayout = new QVBoxLayout(loginWidget);
    loginLayout->setContentsMargins(40, 40, 40, 20);
    loginLayout->setSpacing(25);

    QLabel *titleLabel = new QLabel("ВХОД");
    titleLabel->setAlignment(Qt::AlignCenter);

    QFont titleFont;
    titleFont.setPointSize(28);
    titleFont.setFamily("Arial");
    titleLabel->setFont(titleFont);

    loginLayout->addWidget(titleLabel);
    loginLayout->addSpacing(40);

    QFormLayout *formLayout = new QFormLayout();
    formLayout->setSpacing(15);
    formLayout->setLabelAlignment(Qt::AlignRight);

    QLabel *emailLabel = new QLabel("Почта");
    emailLabel->setFixedWidth(80);

    loginEmailEdit = new QLineEdit();
    loginEmailEdit->setPlaceholderText("example@mail.com");
    loginEmailEdit->setMinimumHeight(35);
    loginEmailEdit->setStyleSheet("QLineEdit { padding: 8px; border: 1px solid #ccc; border-radius: 3px; }");

    formLayout->addRow(emailLabel, loginEmailEdit);

    QLabel *passwordLabel = new QLabel("Пароль");
    passwordLabel->setFixedWidth(80);

    loginPasswordEdit = new QLineEdit();
    loginPasswordEdit->setPlaceholderText("Введите пароль");
    loginPasswordEdit->setEchoMode(QLineEdit::Password);
    loginPasswordEdit->setMinimumHeight(35);
    loginPasswordEdit->setStyleSheet("QLineEdit { padding: 8px; border: 1px solid #ccc; border-radius: 3px; }");

    formLayout->addRow(passwordLabel, loginPasswordEdit);

    loginLayout->addLayout(formLayout);
    loginLayout->addSpacing(10);

    QLabel *forgotPasswordLink = new QLabel("<a href=\"#\" style='color: #0066cc; text-decoration: none;'>Забыли пароль?</a>");
    forgotPasswordLink->setAlignment(Qt::AlignRight);

    QHBoxLayout *forgotLayout = new QHBoxLayout();
    forgotLayout->addStretch();
    forgotLayout->addWidget(forgotPasswordLink);

    loginLayout->addLayout(forgotLayout);
    loginLayout->addSpacing(30);

    loginButton = new QPushButton("Войти");
    loginButton->setMinimumHeight(45);

    loginButton->setStyleSheet(
        "QPushButton {"
        "   background: qlineargradient(x1:0, y1:0, x2:1, y2:1,"
        "                               stop:0 #4169E1, stop:1 #8A2BE2);"
        "   color: white;"
        "   font-weight: bold;"
        "   font-size: 16px;"
        "   border: none;"
        "   border-radius: 5px;"
        "}"
        "QPushButton:hover {"
        "   background: qlineargradient(x1:0, y1:0, x2:1, y2:1,"
        "                               stop:0 #3159D1, stop:1 #7A1BD2);"
        "}"
        "QPushButton:pressed {"
        "   background: qlineargradient(x1:0, y1:0, x2:1, y2:1,"
        "                               stop:0 #2159C1, stop:1 #6A0BC2);"
        "}"
    );

    connect(loginButton, &QPushButton::clicked, this, &LoginWindow::handleLogin);
    loginLayout->addWidget(loginButton);

    loginLayout->addStretch();

    QLabel *registerLink = new QLabel("Нет аккаунта? <a href=\"#\" style='color: #0066cc;'>Зарегистрируйтесь</a>");
    registerLink->setAlignment(Qt::AlignCenter);
    connect(registerLink, &QLabel::linkActivated, this, &LoginWindow::showRegisterForm);

    loginLayout->addWidget(registerLink);
}

void LoginWindow::createRegisterWidget() {
    registerWidget = new QWidget();
    QVBoxLayout *registerLayout = new QVBoxLayout(registerWidget);
    registerLayout->setContentsMargins(40, 40, 40, 20);
    registerLayout->setSpacing(25);

    QLabel *titleLabel = new QLabel("РЕГИСТРАЦИЯ");
    titleLabel->setAlignment(Qt::AlignCenter);

    QFont titleFont;
    titleFont.setPointSize(28);
    titleFont.setFamily("Arial");
    titleLabel->setFont(titleFont);

    registerLayout->addWidget(titleLabel);
    registerLayout->addSpacing(30);

    QFormLayout *formLayout = new QFormLayout();
    formLayout->setSpacing(15);
    formLayout->setLabelAlignment(Qt::AlignRight);

    QLabel *nameLabel = new QLabel("Имя");
    nameLabel->setFixedWidth(80);

    regNameEdit = new QLineEdit();
    regNameEdit->setPlaceholderText("Иван Иванов");
    regNameEdit->setMinimumHeight(35);
    regNameEdit->setStyleSheet("QLineEdit { padding: 8px; border: 1px solid #ccc; border-radius: 3px; }");

    formLayout->addRow(nameLabel, regNameEdit);

    QLabel *emailLabel = new QLabel("Почта");
    emailLabel->setFixedWidth(80);

    regEmailEdit = new QLineEdit();
    regEmailEdit->setPlaceholderText("example@mail.com");
    regEmailEdit->setMinimumHeight(35);
    regEmailEdit->setStyleSheet("QLineEdit { padding: 8px; border: 1px solid #ccc; border-radius: 3px; }");

    formLayout->addRow(emailLabel, regEmailEdit);

    QLabel *passwordLabel = new QLabel("Пароль");
    passwordLabel->setFixedWidth(80);

    regPasswordEdit = new QLineEdit();
    regPasswordEdit->setPlaceholderText("Минимум 6 символов");
    regPasswordEdit->setEchoMode(QLineEdit::Password);
    regPasswordEdit->setMinimumHeight(35);
    regPasswordEdit->setStyleSheet("QLineEdit { padding: 8px; border: 1px solid #ccc; border-radius: 3px; }");

    formLayout->addRow(passwordLabel, regPasswordEdit);

    registerLayout->addLayout(formLayout);
    registerLayout->addSpacing(40);

    registerButton = new QPushButton("Зарегистрироваться");
    registerButton->setMinimumHeight(45);

    registerButton->setStyleSheet(
        "QPushButton {"
        "   background: qlineargradient(x1:0, y1:0, x2:1, y2:1,"
        "                               stop:0 #4169E1, stop:1 #8A2BE2);"
        "   color: white;"
        "   font-weight: bold;"
        "   font-size: 16px;"
        "   border: none;"
        "   border-radius: 5px;"
        "}"
        "QPushButton:hover {"
        "   background: qlineargradient(x1:0, y1:0, x2:1, y2:1,"
        "                               stop:0 #3159D1, stop:1 #7A1BD2);"
        "}"
        "QPushButton:pressed {"
        "   background: qlineargradient(x1:0, y1:0, x2:1, y2:1,"
        "                               stop:0 #2159C1, stop:1 #6A0BC2);"
        "}"
    );

    connect(registerButton, &QPushButton::clicked, this, &LoginWindow::handleRegister);
    registerLayout->addWidget(registerButton);

    registerLayout->addStretch();

    QLabel *loginLink = new QLabel("Уже есть аккаунт? <a href=\"#\" style='color: #0066cc;'>Войдите</a>");
    loginLink->setAlignment(Qt::AlignCenter);
    connect(loginLink, &QLabel::linkActivated, this, &LoginWindow::showLoginForm);

    registerLayout->addWidget(loginLink);
}

void LoginWindow::handleLogin() {
    QString email = loginEmailEdit->text().trimmed();
    QString password = loginPasswordEdit->text();

    if (email.isEmpty() || password.isEmpty()) {
        statusLabel->setText("Заполните все поля");
        return;
    }

    AuthManager::instance()->login(email, password);
}

void LoginWindow::handleRegister() {
    QString name = regNameEdit->text().trimmed();
    QString email = regEmailEdit->text().trimmed();
    QString password = regPasswordEdit->text();

    if (name.isEmpty() || email.isEmpty() || password.isEmpty()) {
        statusLabel->setText("Заполните все поля");
        return;
    }

    if (password.length() < 6) {
        statusLabel->setText("Пароль должен содержать минимум 6 символов");
        return;
    }

    AuthManager::instance()->registerUser(name, email, password, "", "");
}

void LoginWindow::onLoginSuccess() {
    User* currentUser = AuthManager::instance()->currentUser();

    if (!currentUser) {
        statusLabel->setText("Ошибка: пользователь не найден");
        return;
    }

    emit loginSuccessful();  // Этот сигнал обрабатывается в main.cpp
    accept();
}

void LoginWindow::onLoginFailed(const QString &error) {
    statusLabel->setText("Ошибка входа: " + error);
    loginPasswordEdit->clear();
}

void LoginWindow::onRegistrationSuccess() {
    statusLabel->setText("Регистрация успешна! Вы вошли в систему.");
    statusLabel->setStyleSheet("color: green; font-size: 12px; padding: 10px; background-color: #f8f9fa; border-top: 1px solid #dee2e6;");

    regNameEdit->clear();
    regEmailEdit->clear();
    regPasswordEdit->clear();
}

void LoginWindow::onRegistrationFailed(const QString &error) {
    statusLabel->setText("Ошибка регистрации: " + error);
    regPasswordEdit->clear();
}

void LoginWindow::showLoginForm() {
    stackedWidget->setCurrentWidget(loginWidget);
    statusLabel->clear();
    statusLabel->setStyleSheet("color: red; font-size: 12px; padding: 10px; background-color: #f8f9fa; border-top: 1px solid #dee2e6;");
}

void LoginWindow::showRegisterForm() {
    stackedWidget->setCurrentWidget(registerWidget);
    statusLabel->clear();
    statusLabel->setStyleSheet("color: red; font-size: 12px; padding: 10px; background-color: #f8f9fa; border-top: 1px solid #dee2e6;");
}
