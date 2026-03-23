#include "fine.h"
#include <QDebug>

Fine::Fine(QObject *parent)
    : QObject(parent),
      m_id(0),
      m_rentalId(0),
      m_amount(0.0),
      m_paid(false) {
    m_issueDate = QDate::currentDate();
}

Fine::Fine(int id, int rentalId, double amount, const QString &reason,
           const QDate &issueDate, bool paid, QObject *parent)
    : QObject(parent),
      m_id(id),
      m_rentalId(rentalId),
      m_amount(amount),
      m_reason(reason),
      m_issueDate(issueDate),
      m_paid(paid) {
}

void Fine::setRentalId(int rentalId) {
    if (m_rentalId != rentalId) {
        m_rentalId = rentalId;
        emit rentalIdChanged();
    }
}

void Fine::setAmount(double amount) {
    if (!qFuzzyCompare(m_amount, amount)) {
        m_amount = amount;
        emit amountChanged();
    }
}

void Fine::setReason(const QString &reason) {
    if (m_reason != reason) {
        m_reason = reason;
        emit reasonChanged();
    }
}

void Fine::setIssueDate(const QDate &date) {
    if (m_issueDate != date) {
        m_issueDate = date;
        emit issueDateChanged();
    }
}

void Fine::setPaymentDate(const QDate &date) {
    if (m_paymentDate != date) {
        m_paymentDate = date;
        emit paymentDateChanged();
    }
}

void Fine::setPaid(bool paid) {
    if (m_paid != paid) {
        m_paid = paid;
        emit paidChanged();
    }
}

QJsonObject Fine::toJson() const {
    QJsonObject obj;
    obj["id"] = m_id;
    obj["rentalId"] = m_rentalId;
    obj["amount"] = m_amount;
    obj["reason"] = m_reason;
    obj["issueDate"] = m_issueDate.toString(Qt::ISODate);
    obj["paid"] = m_paid;
    if (m_paymentDate.isValid()) {
        obj["paymentDate"] = m_paymentDate.toString(Qt::ISODate);
    }
    return obj;
}

Fine* Fine::fromJson(const QJsonObject &json, QObject *parent) {
    Fine* fine = new Fine(parent);

    if (json.contains("id")) {
        fine->m_id = json["id"].toInt();
    }

    if (json.contains("rentalId")) {
        fine->m_rentalId = json["rentalId"].toInt();
    }

    if (json.contains("amount")) {
        fine->m_amount = json["amount"].toDouble();
    }

    if (json.contains("reason")) {
        fine->m_reason = json["reason"].toString();
    }

    if (json.contains("issueDate")) {
        fine->m_issueDate = QDate::fromString(json["issueDate"].toString(), Qt::ISODate);
    }

    if (json.contains("paid")) {
        fine->m_paid = json["paid"].toBool();
    }

    if (json.contains("paymentDate")) {
        fine->m_paymentDate = QDate::fromString(json["paymentDate"].toString(), Qt::ISODate);
    }

    return fine;
}
