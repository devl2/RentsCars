#include "carmanager.h"
#include "rentalmanager.h"
#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QFile>
#include <QStandardPaths>
#include <QDir>
#include <QDateTime>

CarManager* CarManager::m_instance = nullptr;

CarManager* CarManager::instance() {
    if (!m_instance) {
        m_instance = new CarManager();
    }
    return m_instance;
}

CarManager::CarManager(QObject *parent)
    : QObject(parent) {

    loadCars();
    qDebug() << "CarManager initialized";
}

CarManager::~CarManager() {
    saveCars();
    qDeleteAll(m_cars);
}

QList<Car*> CarManager::getAvailableCars() const {
    QList<Car*> availableCars;
    RentalManager* rentalManager = RentalManager::instance();

    if (!rentalManager) {
        qDebug() << "RentalManager not available";
        return m_cars;
    }

    for (Car* car : m_cars) {
        Rental* activeRental = rentalManager->getActiveRentalForCar(car->id());

        qDebug() << "Car ID:" << car->id()
                 << "Brand:" << car->brand()
                 << "Model:" << car->model()
                 << "Has active rental:" << (activeRental != nullptr);

        if (!activeRental) {
            availableCars.append(car);
            qDebug() << "  -> Available";
        } else {
            qDebug() << "  -> NOT Available (Rental ID:" << activeRental->id()
                     << "Status:" << activeRental->status() << ")";
        }
    }

    qDebug() << "Total available cars:" << availableCars.size();
    return availableCars;
}

QList<Car*> CarManager::getRentedCars() const {
    QList<Car*> rented;
    for (Car* car : m_cars) {
        if (car->statusEnum() == Car::RENTED) {
            rented.append(car);
        }
    }
    return rented;
}

QList<Car*> CarManager::getMaintenanceCars() const {
    QList<Car*> maintenance;
    for (Car* car : m_cars) {
        if (car->statusEnum() == Car::MAINTENANCE) {
            maintenance.append(car);
        }
    }
    return maintenance;
}

Car* CarManager::getCarById(int id) const {
    for (Car* car : m_cars) {
        if (car->id() == id) {
            return car;
        }
    }
    return nullptr;
}

QList<Car*> CarManager::searchCars(const QString &brand, const QString &model,
                                 double minPrice, double maxPrice, int category) {
    QList<Car*> result;

    for (Car* car : m_cars) {
        bool matches = true;

        if (!brand.isEmpty() && brand != "Все марки" &&
            car->brand().toLower() != brand.toLower()) {
            matches = false;
        }

        if (!model.isEmpty() && model != "Все модели" &&
            !car->model().toLower().contains(model.toLower())) {
            matches = false;
        }

        if (car->pricePerDay() < minPrice || car->pricePerDay() > maxPrice) {
            matches = false;
        }

        if (category != -1 && car->category() != category) {
            matches = false;
        }

        if (car->statusEnum() != Car::AVAILABLE) {
            matches = false;
        }

        if (matches) {
            result.append(car);
        }
    }

    return result;
}

bool CarManager::addCar(const QString &brand, const QString &model,
                       double pricePerDay, int category, int year,
                       const QString &color, const QString &imageUrl) {

    int newId = 1;
    if (!m_cars.isEmpty()) {
        int maxId = 0;
        for (Car* car : m_cars) {
            if (car->id() > maxId) {
                maxId = car->id();
            }
        }
        newId = maxId + 1;
    }

    Car* car = new Car(newId, brand, model, pricePerDay,
                      static_cast<Car::Category>(category), this);
    car->setYear(year);
    car->setColor(color);
    car->setImageUrl(imageUrl);

    m_cars.append(car);

    emit carsChanged();
    emit carAdded(car);

    qDebug() << "Car added:" << brand << model << "ID:" << newId;
    return true;
}

bool CarManager::updateCar(int id, const QString &brand, const QString &model,
                          double pricePerDay, int category, int year,
                          const QString &color, const QString &imageUrl) {

    Car* car = getCarById(id);
    if (!car) {
        return false;
    }

    car->setBrand(brand);
    car->setModel(model);
    car->setPricePerDay(pricePerDay);
    car->setCategory(static_cast<Car::Category>(category));
    car->setYear(year);
    car->setColor(color);
    car->setImageUrl(imageUrl);

    emit carsChanged();
    emit carUpdated(car);

    return true;
}

bool CarManager::deleteCar(int id) {
    for (int i = 0; i < m_cars.size(); i++) {
        if (m_cars[i]->id() == id) {
            Car* car = m_cars.takeAt(i);
            delete car;

            emit carsChanged();
            emit carDeleted(id);

            return true;
        }
    }
    return false;
}

bool CarManager::setCarStatus(int carId, int status) {
    Car* car = getCarById(carId);
    if (!car) {
        return false;
    }

    car->setStatus(static_cast<Car::Status>(status));

    emit carsChanged();
    emit carStatusChanged(carId, status);

    return true;
}

bool CarManager::rentCar(int carId) {
    return setCarStatus(carId, Car::RENTED);
}

bool CarManager::returnCar(int carId) {
    return setCarStatus(carId, Car::AVAILABLE);
}

bool CarManager::sendToMaintenance(int carId) {
    return setCarStatus(carId, Car::MAINTENANCE);
}

bool CarManager::returnFromMaintenance(int carId) {
    return setCarStatus(carId, Car::AVAILABLE);
}

int CarManager::getAvailableCarsCount() const {
    int count = 0;
    for (Car* car : m_cars) {
        if (car->statusEnum() == Car::AVAILABLE) {
            count++;
        }
    }
    return count;
}

int CarManager::getRentedCarsCount() const {
    int count = 0;
    for (Car* car : m_cars) {
        if (car->statusEnum() == Car::RENTED) {
            count++;
        }
    }
    return count;
}

int CarManager::getMaintenanceCarsCount() const {
    int count = 0;
    for (Car* car : m_cars) {
        if (car->statusEnum() == Car::MAINTENANCE) {
            count++;
        }
    }
    return count;
}

double CarManager::getTotalValue() const {
    double total = 0.0;
    for (Car* car : m_cars) {
        total += car->pricePerDay();
    }
    return total;
}

double CarManager::getAveragePrice() const {
    if (m_cars.isEmpty()) return 0.0;
    return getTotalValue() / m_cars.size();
}

void CarManager::onCarStatusChanged(int carId, int newStatus) {
    setCarStatus(carId, newStatus);
}

void CarManager::onCarRented(int carId) {
    rentCar(carId);
}

void CarManager::onCarReturned(int carId) {
    returnCar(carId);
}

void CarManager::createTestCars() {
    qDebug() << "Creating test cars...";

    qDeleteAll(m_cars);
    m_cars.clear();

    addCar("Toyota", "Camry", 1500.0, Car::ECONOMY, 2022, "Серый", "");
    addCar("Hyundai", "Solaris", 1200.0, Car::ECONOMY, 2021, "Белый", "");
    addCar("Kia", "Rio", 1300.0, Car::ECONOMY, 2023, "Черный", "");

    addCar("Volkswagen", "Passat", 2000.0, Car::STANDARD, 2022, "Синий", "");
    addCar("Skoda", "Octavia", 1900.0, Car::STANDARD, 2021, "Красный", "");
    addCar("Ford", "Mondeo", 1800.0, Car::STANDARD, 2020, "Серебристый", "");

    addCar("BMW", "M5", 5000.0, Car::LUXURY, 2023, "Черный", "");
    addCar("Mercedes", "E-Class", 4500.0, Car::LUXURY, 2022, "Белый", "");
    addCar("Audi", "A6", 4000.0, Car::LUXURY, 2023, "Серый", "");

    addCar("Toyota", "Land Cruiser", 6000.0, Car::SUV, 2023, "Черный", "");
    addCar("BMW", "X5", 5500.0, Car::SUV, 2022, "Синий", "");
    addCar("Mercedes", "GLE", 5200.0, Car::SUV, 2023, "Белый", "");

    qDebug() << "Test cars created. Total:" << m_cars.size();
}

bool CarManager::loadCars() {
    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(dataDir);
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    QString filePath = dataDir + "/cars.json";
    QFile file(filePath);

    if (!file.exists()) {
        createTestCars();
        saveCars();
        return true;
    }

    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Failed to open cars file:" << file.errorString();
        createTestCars();
        return false;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull()) {
        qDebug() << "Invalid JSON in cars file";
        createTestCars();
        return false;
    }

    QJsonObject root = doc.object();
    QJsonArray carsArray = root["cars"].toArray();

    qDeleteAll(m_cars);
    m_cars.clear();

    for (const QJsonValue &value : carsArray) {
        Car* car = Car::fromJson(value.toObject(), this);
        if (car) {
            m_cars.append(car);
        }
    }

    qDebug() << "Cars loaded from file. Count:" << m_cars.size();
    emit carsChanged();

    return true;
}

bool CarManager::saveCars() {
    QJsonArray carsArray;
    for (Car* car : m_cars) {
        carsArray.append(car->toJson());
    }

    QJsonObject root;
    root["cars"] = carsArray;
    root["lastModified"] = QDateTime::currentDateTime().toString(Qt::ISODate);

    QJsonDocument doc(root);

    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(dataDir);
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    QString filePath = dataDir + "/cars.json";
    QFile file(filePath);

    if (!file.open(QIODevice::WriteOnly)) {
        qDebug() << "Failed to save cars to file:" << file.errorString();
        return false;
    }

    file.write(doc.toJson());
    file.close();

    qDebug() << "Cars saved to file. Count:" << m_cars.size();
    return true;
}

bool CarManager::exportToJson(const QString &filePath) {
    QJsonArray carsArray;
    for (Car* car : m_cars) {
        carsArray.append(car->toJson());
    }

    QJsonObject root;
    root["cars"] = carsArray;
    root["exportDate"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    root["totalCars"] = m_cars.size();

    QJsonDocument doc(root);
    QFile file(filePath);

    if (!file.open(QIODevice::WriteOnly)) {
        qDebug() << "Failed to export cars to:" << filePath;
        return false;
    }

    file.write(doc.toJson());
    file.close();

    qDebug() << "Cars exported to:" << filePath << "Count:" << m_cars.size();
    return true;
}

bool CarManager::importFromJson(const QString &filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Failed to import cars from:" << filePath;
        return false;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull()) {
        qDebug() << "Invalid JSON file";
        return false;
    }

    QJsonObject root = doc.object();
    QJsonArray carsArray = root["cars"].toArray();

    qDeleteAll(m_cars);
    m_cars.clear();

    for (const QJsonValue &value : carsArray) {
        Car* car = Car::fromJson(value.toObject(), this);
        if (car) {
            m_cars.append(car);
        }
    }

    emit carsChanged();
    qDebug() << "Cars imported from:" << filePath << "Count:" << m_cars.size();

    saveCars();

    return true;
}
