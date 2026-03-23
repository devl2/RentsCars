#ifndef USER_H
#define USER_H

#include <QObject>
#include <QString>
#include <QDate>
#include <QJsonObject>

class User : public QObject {
    Q_OBJECT
    Q_PROPERTY(int id READ id NOTIFY idChanged)
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(QString email READ email WRITE setEmail NOTIFY emailChanged)
    Q_PROPERTY(QString phone READ phone WRITE setPhone NOTIFY phoneChanged)
    Q_PROPERTY(QString licenseNumber READ licenseNumber WRITE setLicenseNumber NOTIFY licenseNumberChanged)
    Q_PROPERTY(int role READ role WRITE setRole NOTIFY roleChanged)
    Q_PROPERTY(QDate registrationDate READ registrationDate NOTIFY registrationDateChanged)

public:
    enum Role { CLIENT = 0, ADMIN = 1 };
    Q_ENUM(Role)

    explicit User(QObject *parent = nullptr);
    User(int id, const QString &name, const QString &email,
         Role role = CLIENT, QObject *parent = nullptr);

    int id() const { return m_id; }
    QString name() const { return m_name; }
    QString email() const { return m_email; }
    QString phone() const { return m_phone; }
    QString licenseNumber() const { return m_licenseNumber; }
    int role() const { return static_cast<int>(m_role); }
    Role roleEnum() const { return m_role; }
    QDate registrationDate() const { return m_registrationDate; }
    bool isAdmin() const { return m_role == ADMIN; }

    void setName(const QString &name);
    void setEmail(const QString &email);
    void setPhone(const QString &phone);
    void setLicenseNumber(const QString &licenseNumber);
    void setRole(int role);
    void setRole(Role role);

    QJsonObject toJson() const;
    static User* fromJson(const QJsonObject &json, QObject *parent = nullptr);

signals:
    void idChanged();
    void nameChanged();
    void emailChanged();
    void phoneChanged();
    void licenseNumberChanged();
    void roleChanged();
    void registrationDateChanged();

private:
    int m_id;
    QString m_name;
    QString m_email;
    QString m_phone;
    QString m_licenseNumber;
    Role m_role;
    QDate m_registrationDate;
};

#endif // USER_H
