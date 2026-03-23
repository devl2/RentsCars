#ifndef AUTHMANAGER_H
#define AUTHMANAGER_H

#include <QObject>
#include <QString>
#include <QList>
#include "user.h"
#include "databasemanager.h"

class AuthManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(User* currentUser READ currentUser NOTIFY currentUserChanged)
    Q_PROPERTY(bool isLoggedIn READ isLoggedIn NOTIFY loginStatusChanged)
    Q_PROPERTY(bool isAdmin READ isAdmin NOTIFY currentUserChanged)

public:
    static AuthManager* instance();

    User* currentUser() const { return m_currentUser; }
    bool isLoggedIn() const { return m_currentUser != nullptr; }
    bool isAdmin() const { return m_currentUser && m_currentUser->isAdmin(); }

    Q_INVOKABLE bool login(const QString &email, const QString &password);
    Q_INVOKABLE bool registerUser(const QString &name, const QString &email,
                                 const QString &password, const QString &phone = "",
                                 const QString &licenseNumber = "");
    Q_INVOKABLE void logout();
    Q_INVOKABLE bool changePassword(const QString &oldPassword, const QString &newPassword);
    Q_INVOKABLE bool updateProfile(const QString &name, const QString &phone,
                                  const QString &licenseNumber);

    Q_INVOKABLE bool validateEmail(const QString &email);
    Q_INVOKABLE bool validatePassword(const QString &password);

    bool saveUsersToFile(const QString &filePath = "");
    bool loadUsersFromFile(const QString &filePath = "");

    void createTestUsers();

signals:
    void currentUserChanged();
    void loginStatusChanged();
    void loginSuccess();
    void loginFailed(const QString &error);
    void registrationSuccess();
    void registrationFailed(const QString &error);
    void logoutSuccess();
    void profileUpdated();

private:
    AuthManager(QObject *parent = nullptr);
    ~AuthManager();

    QString hashPassword(const QString &password);
    bool verifyPassword(const QString &password, const QString &hash);

    bool addUser(User *user);
    User* findUserByEmail(const QString &email);
    int getNextUserId();

    QList<User*> m_users;
    User* m_currentUser;
    DatabaseManager* m_dbManager;
    QString m_dataFilePath;

    static AuthManager* m_instance;
};

#endif // AUTHMANAGER_H
