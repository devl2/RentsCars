#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H

#include <QDialog>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QStackedWidget>

class LoginWindow : public QDialog
{
    Q_OBJECT

public:
    explicit LoginWindow(QWidget *parent = nullptr);

private slots:
    void handleLogin();
    void handleRegister();
    void onLoginSuccess();
    void onLoginFailed(const QString &error);
    void onRegistrationSuccess();
    void onRegistrationFailed(const QString &error);
    void showLoginForm();
    void showRegisterForm();

signals:
    void loginSuccessful();
    void loginCancelled();

private:
    void setupUI();
    void createLoginWidget();
    void createRegisterWidget();

    QLineEdit *loginEmailEdit;
    QLineEdit *loginPasswordEdit;
    QPushButton *loginButton;

    QLineEdit *regNameEdit;
    QLineEdit *regEmailEdit;
    QLineEdit *regPasswordEdit;
    QPushButton *registerButton;

    QLabel *statusLabel;
    QStackedWidget *stackedWidget;
    QWidget *loginWidget;
    QWidget *registerWidget;
};

#endif // LOGINWINDOW_H
