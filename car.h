#ifndef CAR_H
#define CAR_H

#include <QObject>
#include <QString>
#include <QDate>
#include <QJsonObject>

class Car : public QObject {
    Q_OBJECT
    Q_PROPERTY(int id READ id NOTIFY idChanged)
    Q_PROPERTY(QString brand READ brand WRITE setBrand NOTIFY brandChanged)
    Q_PROPERTY(QString model READ model WRITE setModel NOTIFY modelChanged)
    Q_PROPERTY(int year READ year WRITE setYear NOTIFY yearChanged)
    Q_PROPERTY(QString color READ color WRITE setColor NOTIFY colorChanged)
    Q_PROPERTY(double pricePerDay READ pricePerDay WRITE setPricePerDay NOTIFY pricePerDayChanged)
    Q_PROPERTY(int status READ status WRITE setStatus NOTIFY statusChanged)
    Q_PROPERTY(int category READ category WRITE setCategory NOTIFY categoryChanged)
    Q_PROPERTY(QString imageUrl READ imageUrl WRITE setImageUrl NOTIFY imageUrlChanged)

public:
    enum Status { AVAILABLE = 0, RENTED = 1, MAINTENANCE = 2 };
    Q_ENUM(Status)

    enum Category { ECONOMY = 0, STANDARD = 1, LUXURY = 2, SUV = 3 };
    Q_ENUM(Category)

    explicit Car(QObject *parent = nullptr);
    Car(int id, const QString &brand, const QString &model,
        double pricePerDay, Category category = STANDARD, QObject *parent = nullptr);

    // Геттеры
    int id() const { return m_id; }
    QString brand() const { return m_brand; }
    QString model() const { return m_model; }
    int year() const { return m_year; }
    QString color() const { return m_color; }
    double pricePerDay() const { return m_pricePerDay; }
    int status() const { return static_cast<int>(m_status); }
    int category() const { return static_cast<int>(m_category); }
    QString imageUrl() const { return m_imageUrl; }

    Status statusEnum() const { return m_status; }
    Category categoryEnum() const { return m_category; }

    void setBrand(const QString &brand);
    void setModel(const QString &model);
    void setYear(int year);
    void setColor(const QString &color);
    void setPricePerDay(double pricePerDay);
    void setImageUrl(const QString &imageUrl);

    void setStatus(int status);
    void setCategory(int category);

    void setStatus(Status status);
    void setCategory(Category category);

    QString fullName() const { return m_brand + " " + m_model; }
    QString statusString() const;
    QString categoryString() const;

    QJsonObject toJson() const;
    static Car* fromJson(const QJsonObject &json, QObject *parent = nullptr);

signals:
    void idChanged();
    void brandChanged();
    void modelChanged();
    void yearChanged();
    void colorChanged();
    void pricePerDayChanged();
    void statusChanged();
    void categoryChanged();
    void imageUrlChanged();

private:
    int m_id;
    QString m_brand;
    QString m_model;
    int m_year;
    QString m_color;
    double m_pricePerDay;
    Status m_status;
    Category m_category;
    QString m_imageUrl;
};
#endif // CAR_H
