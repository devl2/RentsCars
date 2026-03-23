#ifndef RENTAL_H
#define RENTAL_H

#include <QObject>
#include <QDate>
#include <QJsonObject>
#include "user.h"
#include "car.h"

class Rental : public QObject {
    Q_OBJECT
    Q_PROPERTY(int id READ id NOTIFY idChanged)
    Q_PROPERTY(int userId READ userId WRITE setUserId NOTIFY userIdChanged)
    Q_PROPERTY(int carId READ carId WRITE setCarId NOTIFY carIdChanged)
    Q_PROPERTY(QDate startDate READ startDate WRITE setStartDate NOTIFY startDateChanged)
    Q_PROPERTY(QDate endDate READ endDate WRITE setEndDate NOTIFY endDateChanged)
    Q_PROPERTY(double totalPrice READ totalPrice WRITE setTotalPrice NOTIFY totalPriceChanged)
    Q_PROPERTY(int status READ status WRITE setStatus NOTIFY statusChanged)
    Q_PROPERTY(QDate actualReturnDate READ actualReturnDate WRITE setActualReturnDate NOTIFY actualReturnDateChanged)

public:
    enum Status { ACTIVE = 0, COMPLETED = 1, CANCELLED = 2, OVERDUE = 3 };
    Q_ENUM(Status)

    explicit Rental(QObject *parent = nullptr);
    Rental(int id, int userId, int carId, const QDate &startDate,
           const QDate &endDate, double totalPrice, Status status = ACTIVE,
           QObject *parent = nullptr);

    int id() const { return m_id; }
    int userId() const { return m_userId; }
    int carId() const { return m_carId; }
    QDate startDate() const { return m_startDate; }
    QDate endDate() const { return m_endDate; }
    double totalPrice() const { return m_totalPrice; }
    Status status() const { return m_status; }
    QDate actualReturnDate() const { return m_actualReturnDate; }

    void setUserId(int userId);
    void setCarId(int carId);
    void setStartDate(const QDate &startDate);
    void setEndDate(const QDate &endDate);
    void setTotalPrice(double totalPrice);
    void setStatus(int status);
    void setStatus(Status status);
    void setActualReturnDate(const QDate &date);

    int rentalDays() const;
    bool isOverdue() const;
    double calculatePenalty(double dailyPenaltyRate) const;
    bool cancel();
    bool cancelWithPenalty(double penaltyRate = 0.1);
    bool canBeCancelled() const;
    QString statusString() const;

    QJsonObject toJson() const;
    static Rental* fromJson(const QJsonObject &json, QObject *parent = nullptr);

signals:
    void idChanged();
    void userIdChanged();
    void carIdChanged();
    void startDateChanged();
    void endDateChanged();
    void totalPriceChanged();
    void statusChanged();
    void actualReturnDateChanged();
    void cancelled();

private:
    int m_id;
    int m_userId;
    int m_carId;
    QDate m_startDate;
    QDate m_endDate;
    double m_totalPrice;
    Status m_status;
    QDate m_actualReturnDate;
};

#endif // RENTAL_H
