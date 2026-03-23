#include "car.h"
#include <QDebug>

Car::Car(QObject *parent)
    : QObject(parent),
      m_id(0),
      m_year(2023),
      m_pricePerDay(0.0),
      m_status(AVAILABLE),
      m_category(STANDARD) {
}

Car::Car(int id, const QString &brand, const QString &model,
         double pricePerDay, Category category, QObject *parent)
    : QObject(parent),
      m_id(id),
      m_brand(brand),
      m_model(model),
      m_year(2023),
      m_color("Черный"),
      m_pricePerDay(pricePerDay),
      m_status(AVAILABLE),
      m_category(category),
      m_imageUrl("") {
}

void Car::setBrand(const QString &brand) {
    if (m_brand != brand) {
        m_brand = brand;
        emit brandChanged();
    }
}

void Car::setModel(const QString &model) {
    if (m_model != model) {
        m_model = model;
        emit modelChanged();
    }
}

void Car::setYear(int year) {
    if (m_year != year) {
        m_year = year;
        emit yearChanged();
    }
}

void Car::setColor(const QString &color) {
    if (m_color != color) {
        m_color = color;
        emit colorChanged();
    }
}

void Car::setPricePerDay(double pricePerDay) {
    if (!qFuzzyCompare(m_pricePerDay, pricePerDay)) {
        m_pricePerDay = pricePerDay;
        emit pricePerDayChanged();
    }
}

void Car::setStatus(int status) {
    setStatus(static_cast<Status>(status));
}

void Car::setCategory(int category) {
    setCategory(static_cast<Category>(category));
}

void Car::setStatus(Status status) {
    if (m_status != status) {
        m_status = status;
        emit statusChanged();
    }
}


void Car::setCategory(Category category) {
    if (m_category != category) {
        m_category = category;
        emit categoryChanged();
    }
}

void Car::setImageUrl(const QString &imageUrl) {
    if (m_imageUrl != imageUrl) {
        m_imageUrl = imageUrl;
        emit imageUrlChanged();
    }
}

QString Car::statusString() const {
    switch (m_status) {
    case AVAILABLE:
        return "Доступен";
    case RENTED:
        return "Арендован";
    case MAINTENANCE:
        return "На обслуживании";
    default:
        return "Неизвестно";
    }
}

QString Car::categoryString() const {
    switch (m_category) {
    case ECONOMY:
        return "Эконом";
    case STANDARD:
        return "Стандарт";
    case LUXURY:
        return "Люкс";
    case SUV:
        return "Внедорожник";
    default:
        return "Неизвестно";
    }
}

QJsonObject Car::toJson() const {
    QJsonObject obj;
    obj["id"] = m_id;
    obj["brand"] = m_brand;
    obj["model"] = m_model;
    obj["year"] = m_year;
    obj["color"] = m_color;
    obj["pricePerDay"] = m_pricePerDay;
    obj["status"] = m_status;
    obj["category"] = m_category;
    obj["imageUrl"] = m_imageUrl;
    return obj;
}

Car* Car::fromJson(const QJsonObject &json, QObject *parent) {
    Car* car = new Car(parent);

    if (json.contains("id")) {
        car->m_id = json["id"].toInt();
    }

    if (json.contains("brand")) {
        car->m_brand = json["brand"].toString();
    }

    if (json.contains("model")) {
        car->m_model = json["model"].toString();
    }

    if (json.contains("year")) {
        car->m_year = json["year"].toInt();
    }

    if (json.contains("color")) {
        car->m_color = json["color"].toString();
    }

    if (json.contains("pricePerDay")) {
        car->m_pricePerDay = json["pricePerDay"].toDouble();
    }

    if (json.contains("status")) {
        car->m_status = static_cast<Status>(json["status"].toInt());
    }

    if (json.contains("category")) {
        car->m_category = static_cast<Category>(json["category"].toInt());
    }

    if (json.contains("imageUrl")) {
        car->m_imageUrl = json["imageUrl"].toString();
    }

    return car;
}
