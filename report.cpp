#include "report.h"
#include <QDebug>

Report::Report(QObject *parent)
    : QObject(parent),
      m_totalIncome(0.0),
      m_totalRentals(0) {
    m_startDate = QDate::currentDate().addDays(-30);
    m_endDate = QDate::currentDate();
}

Report::Report(const QDate &startDate, const QDate &endDate, QObject *parent)
    : QObject(parent),
      m_startDate(startDate),
      m_endDate(endDate),
      m_totalIncome(0.0),
      m_totalRentals(0) {
}

void Report::setStartDate(const QDate &date) {
    if (m_startDate != date) {
        m_startDate = date;
        emit startDateChanged();
    }
}

void Report::setEndDate(const QDate &date) {
    if (m_endDate != date) {
        m_endDate = date;
        emit endDateChanged();
    }
}

void Report::addRental(double price) {
    m_rentalPrices.append(price);
    m_totalIncome += price;
    m_totalRentals++;

    emit totalIncomeChanged();
    emit totalRentalsChanged();
}

void Report::setMostPopularCar(const QString &car) {
    if (m_mostPopularCar != car) {
        m_mostPopularCar = car;
        emit mostPopularCarChanged();
    }
}

void Report::addFine(double amount) {
    m_totalIncome += amount;
    emit totalIncomeChanged();
}

double Report::averageRentalPrice() const {
    if (m_totalRentals == 0) {
        return 0.0;
    }
    return m_totalIncome / m_totalRentals;
}

void Report::calculateStatistics() {

    if (m_rentalPrices.isEmpty()) {
        m_mostPopularCar = "Нет данных";
    } else {
        m_mostPopularCar = "BMW M5";
    }

    emit averageRentalPriceChanged();
    emit mostPopularCarChanged();
}

QJsonObject Report::toJson() const {
    QJsonObject obj;
    obj["startDate"] = m_startDate.toString(Qt::ISODate);
    obj["endDate"] = m_endDate.toString(Qt::ISODate);
    obj["totalIncome"] = m_totalIncome;
    obj["totalRentals"] = m_totalRentals;
    obj["mostPopularCar"] = m_mostPopularCar;

    QJsonArray pricesArray;
    for (double price : m_rentalPrices) {
        pricesArray.append(price);
    }
    obj["rentalPrices"] = pricesArray;

    return obj;
}
