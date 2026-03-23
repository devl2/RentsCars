#include "rental.h"
#include <QDebug>
#include <QDate>

Rental::Rental(QObject *parent)
    : QObject(parent),
      m_id(0),
      m_userId(0),
      m_carId(0),
      m_totalPrice(0.0),
      m_status(ACTIVE) {
    m_startDate = QDate::currentDate();
    m_endDate = QDate::currentDate().addDays(1);
}

Rental::Rental(int id, int userId, int carId, const QDate &startDate,
               const QDate &endDate, double totalPrice, Status status, QObject *parent)
    : QObject(parent),
      m_id(id),
      m_userId(userId),
      m_carId(carId),
      m_startDate(startDate),
      m_endDate(endDate),
      m_totalPrice(totalPrice),
      m_status(status) {
}

void Rental::setUserId(int userId) {
    if (m_userId != userId) {
        m_userId = userId;
        emit userIdChanged();
    }
}

void Rental::setCarId(int carId) {
    if (m_carId != carId) {
        m_carId = carId;
        emit carIdChanged();
    }
}

void Rental::setStartDate(const QDate &startDate) {
    if (m_startDate != startDate) {
        m_startDate = startDate;
        emit startDateChanged();
    }
}

void Rental::setEndDate(const QDate &endDate) {
    if (m_endDate != endDate) {
        m_endDate = endDate;
        emit endDateChanged();
    }
}

void Rental::setTotalPrice(double totalPrice) {
    if (!qFuzzyCompare(m_totalPrice, totalPrice)) {
        m_totalPrice = totalPrice;
        emit totalPriceChanged();
    }
}


void Rental::setStatus(Status status) {
    if (m_status != status) {
        m_status = status;
        emit statusChanged();
    }
}

void Rental::setStatus(int status) {
    setStatus(static_cast<Status>(status));
}

void Rental::setActualReturnDate(const QDate &date) {
    if (m_actualReturnDate != date) {
        m_actualReturnDate = date;
        emit actualReturnDateChanged();
    }
}

int Rental::rentalDays() const {
    if (!m_startDate.isValid() || !m_endDate.isValid()) {
        return 0;
    }

    int days = m_startDate.daysTo(m_endDate);
    return days > 0 ? days : 1;
}

bool Rental::isOverdue() const {
    if (m_status == COMPLETED || m_status == CANCELLED) {
        return false;
    }

    QDate today = QDate::currentDate();
    return today > m_endDate;
}

double Rental::calculatePenalty(double dailyPenaltyRate) const {
    if (!isOverdue() || !m_actualReturnDate.isValid()) {
        return 0.0;
    }

    int overdueDays = m_endDate.daysTo(m_actualReturnDate);
    if (overdueDays <= 0) {
        return 0.0;
    }

    double dailyCost = m_totalPrice / rentalDays();

    return dailyCost * dailyPenaltyRate * overdueDays;
}

bool Rental::canBeCancelled() const {
    QDate today = QDate::currentDate();

    if (m_status != ACTIVE && m_status != OVERDUE) {
        return false;
    }

    int daysUntilStart = today.daysTo(m_startDate);

    return true;
}

bool Rental::cancel() {
    if (!canBeCancelled()) {
        qDebug() << "Невозможно отменить аренду в текущем статусе:" << m_status;
        return false;
    }

    QDate today = QDate::currentDate();
    int hoursUntilStart = today.daysTo(m_startDate) * 24;

    if (hoursUntilStart < 24) {
        qDebug() << "Поздняя отмена аренды #" << m_id << "за" << hoursUntilStart << "часов до начала";
    }

    setStatus(CANCELLED);

    qDebug() << "Аренда #" << m_id << "отменена пользователем" << m_userId;

    emit cancelled();

    return true;
}

bool Rental::cancelWithPenalty(double penaltyRate) {
    if (!canBeCancelled()) {
        return false;
    }

    QDate today = QDate::currentDate();
    int daysUntilStart = today.daysTo(m_startDate);

    if (daysUntilStart <= 1) {
        double penaltyAmount = m_totalPrice * penaltyRate;
        qDebug() << "Применен штраф за позднюю отмену:" << penaltyAmount
                 << "руб. (ставка:" << penaltyRate * 100 << "%)";

    }

    return cancel();
}

QString Rental::statusString() const {
    switch (m_status) {
        case ACTIVE: return "Активная";
        case COMPLETED: return "Завершена";
        case CANCELLED: return "Отменена";
        case OVERDUE: return "Просрочена";
        default: return "Неизвестно";
    }
}

QJsonObject Rental::toJson() const {
    QJsonObject obj;
    obj["id"] = m_id;
    obj["userId"] = m_userId;
    obj["carId"] = m_carId;
    obj["startDate"] = m_startDate.toString(Qt::ISODate);
    obj["endDate"] = m_endDate.toString(Qt::ISODate);
    obj["totalPrice"] = m_totalPrice;
    obj["status"] = m_status;

    if (m_actualReturnDate.isValid()) {
        obj["actualReturnDate"] = m_actualReturnDate.toString(Qt::ISODate);
    }

    return obj;
}

Rental* Rental::fromJson(const QJsonObject &json, QObject *parent) {
    Rental* rental = new Rental(parent);

    if (json.contains("id")) {
        rental->m_id = json["id"].toInt();
    }

    if (json.contains("userId")) {
        rental->m_userId = json["userId"].toInt();
    }

    if (json.contains("carId")) {
        rental->m_carId = json["carId"].toInt();
    }

    if (json.contains("startDate")) {
        rental->m_startDate = QDate::fromString(json["startDate"].toString(), Qt::ISODate);
    }

    if (json.contains("endDate")) {
        rental->m_endDate = QDate::fromString(json["endDate"].toString(), Qt::ISODate);
    }

    if (json.contains("totalPrice")) {
        rental->m_totalPrice = json["totalPrice"].toDouble();
    }

    if (json.contains("status")) {
        rental->m_status = static_cast<Status>(json["status"].toInt());
    }

    if (json.contains("actualReturnDate")) {
        QString dateStr = json["actualReturnDate"].toString();
        if (!dateStr.isEmpty()) {
            rental->m_actualReturnDate = QDate::fromString(dateStr, Qt::ISODate);
        }
    }

    return rental;
}
