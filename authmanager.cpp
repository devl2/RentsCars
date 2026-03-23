#include "authmanager.h"
#include <QDebug>
#include <QStandardPaths>
#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QFile>
#include <QCryptographicHash>
#include <QJsonObject>
#include <QByteArray>

AuthManager* AuthManager::m_instance = nullptr;

AuthManager* AuthManager::instance() {
    if (!m_instance) {
        m_instance = new AuthManager();
    }
    return m_instance;
}

AuthManager::AuthManager(QObject *parent)
    : QObject(parent), m_currentUser(nullptr) {

    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(dataDir);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    m_dataFilePath = dataDir + "/users.json";

    loadUsersFromFile();

    if (m_users.isEmpty()) {
        createTestUsers();
        saveUsersToFile();
    }

    qDebug() << "AuthManager initialized. Users:" << m_users.size();
}

AuthManager::~AuthManager() {
    saveUsersToFile();
    qDeleteAll(m_users);
}

bool AuthManager::login(const QString &email, const QString &password) {
    qDebug() << "Attempting login for email:" << email;

    User* user = findUserByEmail(email);
    if (!user) {
        qDebug() << "User not found";
        emit loginFailed("Пользователь с таким email не найден");
        return false;
    }

    QString storedPassword = user->property("passwordHash").toString();
    if (storedPassword.isEmpty()) {
        if (password == "admin123" && email == "admin@mail.com") {
            m_currentUser = user;
            emit loginSuccess();
            emit currentUserChanged();
            emit loginStatusChanged();
            qDebug() << "Login successful (admin)";
            return true;
        }
        if (password == "user123" && email == "user@mail.com") {
            m_currentUser = user;
            emit loginSuccess();
            emit currentUserChanged();
            emit loginStatusChanged();
            qDebug() << "Login successful (user)";
            return true;
        }
    } else {
        if (verifyPassword(password, storedPassword)) {
            m_currentUser = user;
            emit loginSuccess();
            emit currentUserChanged();
            emit loginStatusChanged();
            qDebug() << "Login successful";
            return true;
        }
    }

    qDebug() << "Invalid password";
    emit loginFailed("Неверный пароль");
    return false;
}

bool AuthManager::registerUser(const QString &name, const QString &email,
                              const QString &password, const QString &phone,
                              const QString &licenseNumber) {

    qDebug() << "Attempting registration for email:" << email;

    if (!validateEmail(email)) {
        emit registrationFailed("Некорректный email");
        return false;
    }

    if (!validatePassword(password)) {
        emit registrationFailed("Пароль должен содержать минимум 6 символов");
        return false;
    }

    if (findUserByEmail(email)) {
        emit registrationFailed("Пользователь с таким email уже существует");
        return false;
    }

    int id = getNextUserId();
    User* user = new User(id, name, email, User::CLIENT, this);
    user->setPhone(phone);
    user->setLicenseNumber(licenseNumber);

    QString passwordHash = hashPassword(password);
    user->setProperty("passwordHash", passwordHash);

    m_users.append(user);

    m_currentUser = user;

    saveUsersToFile();

    emit registrationSuccess();
    emit currentUserChanged();
    emit loginStatusChanged();

    qDebug() << "Registration successful. New user ID:" << id;
    return true;
}

void AuthManager::logout() {
    if (m_currentUser) {
        qDebug() << "Logging out user:" << m_currentUser->email();
        m_currentUser = nullptr;
        emit logoutSuccess();
        emit currentUserChanged();
        emit loginStatusChanged();
    }
}

bool AuthManager::changePassword(const QString &oldPassword, const QString &newPassword) {
    if (!m_currentUser) {
        return false;
    }

    QString storedHash = m_currentUser->property("passwordHash").toString();
    if (!verifyPassword(oldPassword, storedHash)) {
        return false;
    }

    if (!validatePassword(newPassword)) {
        return false;
    }

    QString newHash = hashPassword(newPassword);
    m_currentUser->setProperty("passwordHash", newHash);

    saveUsersToFile();

    return true;
}

bool AuthManager::updateProfile(const QString &name, const QString &phone,
                               const QString &licenseNumber) {
    if (!m_currentUser) {
        return false;
    }

    m_currentUser->setName(name);
    m_currentUser->setPhone(phone);
    m_currentUser->setLicenseNumber(licenseNumber);

    saveUsersToFile();

    emit profileUpdated();
    emit currentUserChanged();

    return true;
}

bool AuthManager::validateEmail(const QString &email) {
    return email.contains('@') && email.contains('.') && email.length() > 5;
}

bool AuthManager::validatePassword(const QString &password) {
    return password.length() >= 6;
}

bool AuthManager::saveUsersToFile(const QString &filePath) {
    QJsonArray usersArray;

    for (User* user : m_users) {
        QJsonObject userObj;
        userObj["id"] = user->id();
        userObj["name"] = user->name();
        userObj["email"] = user->email();
        userObj["phone"] = user->phone();
        userObj["licenseNumber"] = user->licenseNumber();
        userObj["role"] = user->role();
        userObj["registrationDate"] = user->registrationDate().toString(Qt::ISODate);

        QString passwordHash = user->property("passwordHash").toString();
        if (!passwordHash.isEmpty()) {
            userObj["passwordHash"] = passwordHash;
        }

        usersArray.append(userObj);
    }

    QJsonObject root;
    root["users"] = usersArray;

    QJsonDocument doc(root);
    QFile file(filePath.isEmpty() ? m_dataFilePath : filePath);

    if (!file.open(QIODevice::WriteOnly)) {
        qDebug() << "Failed to save users to file:" << file.errorString();
        return false;
    }

    file.write(doc.toJson());
    file.close();

    qDebug() << "Users saved to file. Count:" << m_users.size();
    return true;
}

bool AuthManager::loadUsersFromFile(const QString &filePath) {
    QString actualPath = filePath.isEmpty() ? m_dataFilePath : filePath;
    QFile file(actualPath);

    if (!file.exists()) {
        qDebug() << "Users file does not exist:" << actualPath;
        return false;
    }

    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Failed to open users file:" << file.errorString();
        return false;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull()) {
        qDebug() << "Invalid JSON in users file";
        return false;
    }

    QJsonObject root = doc.object();
    QJsonArray usersArray = root["users"].toArray();

    qDeleteAll(m_users);
    m_users.clear();

    for (const QJsonValue &value : usersArray) {
        QJsonObject userObj = value.toObject();

        int id = userObj["id"].toInt();
        QString name = userObj["name"].toString();
        QString email = userObj["email"].toString();
        int role = userObj["role"].toInt();

        User* user = new User(id, name, email, static_cast<User::Role>(role), this);
        user->setPhone(userObj["phone"].toString());
        user->setLicenseNumber(userObj["licenseNumber"].toString());

        if (userObj.contains("passwordHash")) {
            user->setProperty("passwordHash", userObj["passwordHash"].toString());
        }

        m_users.append(user);
    }

    qDebug() << "Users loaded from file. Count:" << m_users.size();
    return true;
}

void AuthManager::createTestUsers() {
    User* admin = new User(1, "Администратор", "admin@mail.com", User::ADMIN, this);
    admin->setPhone("+79991234567");
    admin->setProperty("passwordHash", hashPassword("admin123"));
    m_users.append(admin);

    User* client = new User(2, "Иван Иванов", "user@mail.com", User::CLIENT, this);
    client->setPhone("+79998765432");
    client->setLicenseNumber("AB123456");
    client->setProperty("passwordHash", hashPassword("user123"));
    m_users.append(client);

    User* client2 = new User(3, "Мария Петрова", "maria@mail.com", User::CLIENT, this);
    client2->setPhone("+79997654321");
    client2->setLicenseNumber("CD789012");
    client2->setProperty("passwordHash", hashPassword("maria123"));
    m_users.append(client2);

    qDebug() << "Test users created. Total:" << m_users.size();
}

QString AuthManager::hashPassword(const QString &password) {
    QByteArray hash = QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256);
    return QString(hash.toHex());
}

bool AuthManager::verifyPassword(const QString &password, const QString &hash) {
    QString inputHash = hashPassword(password);
    return inputHash == hash;
}

bool AuthManager::addUser(User *user) {
    if (findUserByEmail(user->email())) {
        return false;
    }
    m_users.append(user);
    return true;
}

User* AuthManager::findUserByEmail(const QString &email) {
    for (User* user : m_users) {
        if (user->email().toLower() == email.toLower()) {
            return user;
        }
    }
    return nullptr;
}

int AuthManager::getNextUserId() {
    int maxId = 0;
    for (User* user : m_users) {
        if (user->id() > maxId) {
            maxId = user->id();
        }
    }
    return maxId + 1;
}
