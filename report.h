#ifndef REPORT_H
#define REPORT_H

#include <QObject>
#include <QDate>
#include <QJsonObject>
#include <QVector>
#include <QJsonArray>

class Report : public QObject {
    Q_OBJECT
    Q_PROPERTY(QDate startDate READ startDate WRITE setStartDate NOTIFY startDateChanged)
    Q_PROPERTY(QDate endDate READ endDate WRITE setEndDate NOTIFY endDateChanged)
    Q_PROPERTY(double totalIncome READ totalIncome NOTIFY totalIncomeChanged)
    Q_PROPERTY(int totalRentals READ totalRentals NOTIFY totalRentalsChanged)
    Q_PROPERTY(double averageRentalPrice READ averageRentalPrice NOTIFY averageRentalPriceChanged)
    Q_PROPERTY(QString mostPopularCar READ mostPopularCar NOTIFY mostPopularCarChanged)

public:
    explicit Report(QObject *parent = nullptr);
    Report(const QDate &startDate, const QDate &endDate, QObject *parent = nullptr);

    QDate startDate() const { return m_startDate; }
    QDate endDate() const { return m_endDate; }
    double totalIncome() const { return m_totalIncome; }
    int totalRentals() const { return m_totalRentals; }
    double averageRentalPrice() const;
    QString mostPopularCar() const { return m_mostPopularCar; }

    void setStartDate(const QDate &date);
    void setEndDate(const QDate &date);

    void addRental(double price);
    void setMostPopularCar(const QString &car);
    void addFine(double amount);
    void calculateStatistics();

    QJsonObject toJson() const;

signals:
    void startDateChanged();
    void endDateChanged();
    void totalIncomeChanged();
    void totalRentalsChanged();
    void averageRentalPriceChanged();
    void mostPopularCarChanged();

private:
    QDate m_startDate;
    QDate m_endDate;
    double m_totalIncome;
    int m_totalRentals;
    QString m_mostPopularCar;
    QVector<double> m_rentalPrices;
};

#endif // REPORT_H
