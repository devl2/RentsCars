#ifndef PRICINGSTRATEGY_H
#define PRICINGSTRATEGY_H

#include <QObject>
#include <QString>

class PricingStrategy : public QObject {
    Q_OBJECT
public:
    explicit PricingStrategy(QObject *parent = nullptr) : QObject(parent) {}
    virtual ~PricingStrategy() = default;
    virtual double calculatePrice(double basePrice, int days) = 0;
    virtual QString getName() = 0;
};

class DailyPricing : public PricingStrategy {
    Q_OBJECT
public:
    explicit DailyPricing(QObject *parent = nullptr) : PricingStrategy(parent) {}

    double calculatePrice(double basePrice, int days) override {
        return basePrice * days;
    }
    QString getName() override { return "Дневная"; }
};

class WeeklyPricing : public PricingStrategy {
    Q_OBJECT
public:
    explicit WeeklyPricing(QObject *parent = nullptr) : PricingStrategy(parent) {}

    double calculatePrice(double basePrice, int days) override {
        int weeks = (days + 6) / 7;
        return basePrice * 7 * weeks * 0.9;
    }
    QString getName() override { return "Недельная"; }
};

class MonthlyPricing : public PricingStrategy {
    Q_OBJECT
public:
    explicit MonthlyPricing(QObject *parent = nullptr) : PricingStrategy(parent) {}

    double calculatePrice(double basePrice, int days) override {
        int months = (days + 29) / 30;
        return basePrice * 30 * months * 0.8;
    }
    QString getName() override { return "Месячная"; }
};

class PenaltyPricing : public PricingStrategy {
    Q_OBJECT
public:
    explicit PenaltyPricing(QObject *parent = nullptr) : PricingStrategy(parent) {}

    double calculatePrice(double basePrice, int days) override {
        return basePrice * days * 1.5;
    }
    QString getName() override { return "Штрафная"; }
};

#endif // PRICINGSTRATEGY_H
