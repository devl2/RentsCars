#ifndef FINE_H
#define FINE_H

#include <QObject>
#include <QDate>
#include <QJsonObject>

class Fine : public QObject {
    Q_OBJECT
    Q_PROPERTY(int id READ id NOTIFY idChanged)
    Q_PROPERTY(int rentalId READ rentalId WRITE setRentalId NOTIFY rentalIdChanged)
    Q_PROPERTY(double amount READ amount WRITE setAmount NOTIFY amountChanged)
    Q_PROPERTY(QString reason READ reason WRITE setReason NOTIFY reasonChanged)
    Q_PROPERTY(QDate issueDate READ issueDate WRITE setIssueDate NOTIFY issueDateChanged)
    Q_PROPERTY(bool paid READ paid WRITE setPaid NOTIFY paidChanged)
    Q_PROPERTY(QDate paymentDate READ paymentDate WRITE setPaymentDate NOTIFY paymentDateChanged)

public:
    explicit Fine(QObject *parent = nullptr);
    Fine(int id, int rentalId, double amount, const QString &reason,
         const QDate &issueDate, bool paid = false, QObject *parent = nullptr);

    int id() const { return m_id; }
    int rentalId() const { return m_rentalId; }
    double amount() const { return m_amount; }
    QString reason() const { return m_reason; }
    QDate issueDate() const { return m_issueDate; }
    bool paid() const { return m_paid; }
    QDate paymentDate() const { return m_paymentDate; }

    void setRentalId(int rentalId);
    void setAmount(double amount);
    void setReason(const QString &reason);
    void setIssueDate(const QDate &date);
    void setPaid(bool paid);
     void setPaymentDate(const QDate &date);

    QJsonObject toJson() const;
    static Fine* fromJson(const QJsonObject &json, QObject *parent = nullptr);

signals:
    void idChanged();
    void rentalIdChanged();
    void amountChanged();
    void reasonChanged();
    void issueDateChanged();
    void paidChanged();
    void paymentDateChanged();

private:
    int m_id;
    int m_rentalId;
    double m_amount;
    QString m_reason;
    QDate m_issueDate;
    bool m_paid;
    QDate m_paymentDate;
};

#endif // FINE_H
