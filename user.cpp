#include "user.h"
#include <QJsonObject>
#include <QDate>

User::User(QObject *parent)
    : QObject(parent),
      m_id(0),
      m_role(CLIENT),
      m_registrationDate(QDate::currentDate()) {
}

User::User(int id, const QString &name, const QString &email,
           Role role, QObject *parent)
    : QObject(parent),
      m_id(id),
      m_name(name),
      m_email(email),
      m_role(role),
      m_registrationDate(QDate::currentDate()) {
}

void User::setRole(int role) {
    setRole(static_cast<Role>(role));
}

void User::setRole(Role role) {
    if (m_role != role) {
        m_role = role;
        emit roleChanged();
    }
}

void User::setName(const QString &name) {
    if (m_name != name) {
        m_name = name;
        emit nameChanged();
    }
}

void User::setEmail(const QString &email) {
    if (m_email != email) {
        m_email = email;
        emit emailChanged();
    }
}

void User::setPhone(const QString &phone) {
    if (m_phone != phone) {
        m_phone = phone;
        emit phoneChanged();
    }
}

void User::setLicenseNumber(const QString &licenseNumber) {
    if (m_licenseNumber != licenseNumber) {
        m_licenseNumber = licenseNumber;
        emit licenseNumberChanged();
    }
}

QJsonObject User::toJson() const {
    QJsonObject obj;
    obj["id"] = m_id;
    obj["name"] = m_name;
    obj["email"] = m_email;
    obj["phone"] = m_phone;
    obj["licenseNumber"] = m_licenseNumber;
    obj["role"] = static_cast<int>(m_role);
    obj["registrationDate"] = m_registrationDate.toString(Qt::ISODate);
    return obj;
}

User* User::fromJson(const QJsonObject &json, QObject *parent) {
    User* user = new User(parent);

    if (json.contains("id")) {
        user->m_id = json["id"].toInt();
    }

    if (json.contains("name")) {
        user->m_name = json["name"].toString();
    }

    if (json.contains("email")) {
        user->m_email = json["email"].toString();
    }

    if (json.contains("phone")) {
        user->m_phone = json["phone"].toString();
    }

    if (json.contains("licenseNumber")) {
        user->m_licenseNumber = json["licenseNumber"].toString();
    }

    if (json.contains("role")) {
        user->m_role = static_cast<Role>(json["role"].toInt());
    }

    if (json.contains("registrationDate")) {
        user->m_registrationDate = QDate::fromString(json["registrationDate"].toString(), Qt::ISODate);
    }

    return user;
}
