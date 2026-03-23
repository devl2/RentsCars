#include "databasemanager.h"
#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <QDate>

DatabaseManager* DatabaseManager::m_instance = nullptr;

DatabaseManager* DatabaseManager::instance() {
    if (!m_instance) {
        m_instance = new DatabaseManager();
    }
    return m_instance;
}

DatabaseManager::DatabaseManager(QObject *parent)
    : QObject(parent) {

    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(dataDir);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    m_dataFilePath = dataDir + "/rental_data.json";

    qDebug() << "DatabaseManager initialized. Data file:" << m_dataFilePath;
}

DatabaseManager::~DatabaseManager() {
    saveToFile();
    qDeleteAll(m_users);
    qDeleteAll(m_cars);
    qDeleteAll(m_rentals);
    qDeleteAll(m_fines);
}

bool DatabaseManager::initialize() {
    loadFromFile();

    if (m_users.isEmpty() && m_cars.isEmpty()) {
        createDefaultData();
        saveToFile();
        qDebug() << "Default data created";
    }

    emit databaseInitialized();
    return true;
}

bool DatabaseManager::backupDatabase(const QString &filePath) {
    return exportToJson(filePath);
}

bool DatabaseManager::restoreDatabase(const QString &filePath) {
    return importFromJson(filePath);
}


bool DatabaseManager::addUser(User *user) {
    if (!user) return false;

    for (User* u : m_users) {
        if (u->email().toLower() == user->email().toLower()) {
            qDebug() << "User with email" << user->email() << "already exists";
            return false;
        }
    }

    m_users.append(user);
    saveToFile();
    emit dataChanged();

    qDebug() << "User added:" << user->name() << "ID:" << user->id();
    return true;
}

User* DatabaseManager::getUser(int id) {
    for (User* user : m_users) {
        if (user->id() == id) {
            return user;
        }
    }
    return nullptr;
}

User* DatabaseManager::getUserByEmail(const QString &email) {
    for (User* user : m_users) {
        if (user->email().toLower() == email.toLower()) {
            return user;
        }
    }
    return nullptr;
}

QList<User*> DatabaseManager::getAllUsers() {
    return m_users;
}

bool DatabaseManager::updateUser(User *user) {
    if (!user) return false;

    for (int i = 0; i < m_users.size(); i++) {
        if (m_users[i]->id() == user->id()) {
            m_users[i]->setName(user->name());
            m_users[i]->setEmail(user->email());
            m_users[i]->setPhone(user->phone());
            m_users[i]->setLicenseNumber(user->licenseNumber());

            saveToFile();
            emit dataChanged();
            return true;
        }
    }
    return false;
}

bool DatabaseManager::deleteUser(int id) {
    for (int i = 0; i < m_users.size(); i++) {
        if (m_users[i]->id() == id) {
            User* user = m_users.takeAt(i);
            delete user;

            saveToFile();
            emit dataChanged();
            return true;
        }
    }
    return false;
}

bool DatabaseManager::addCar(Car *car) {
    if (!car) return false;

    m_cars.append(car);
    saveToFile();
    emit dataChanged();

    qDebug() << "Car added:" << car->brand() << car->model() << "ID:" << car->id();
    return true;
}

Car* DatabaseManager::getCar(int id) {
    for (Car* car : m_cars) {
        if (car->id() == id) {
            return car;
        }
    }
    return nullptr;
}

QList<Car*> DatabaseManager::getAllCars() {
    return m_cars;
}

QList<Car*> DatabaseManager::getAvailableCars() {
    QList<Car*> available;
    for (Car* car : m_cars) {
        if (car->status() == Car::AVAILABLE) {
            available.append(car);
        }
    }
    return available;
}

QList<Car*> DatabaseManager::searchCars(const QString &brand, const QString &model,
                                       double minPrice, double maxPrice, int category) {
    QList<Car*> result;

    for (Car* car : m_cars) {
        bool matches = true;

        if (!brand.isEmpty() && car->brand().toLower() != brand.toLower()) {
            matches = false;
        }

        if (!model.isEmpty() && !car->model().toLower().contains(model.toLower())) {
            matches = false;
        }

        if (car->pricePerDay() < minPrice || car->pricePerDay() > maxPrice) {
            matches = false;
        }

        if (category != -1 && car->category() != category) {
            matches = false;
        }

        if (car->status() != Car::AVAILABLE) {
            matches = false;
        }

        if (matches) {
            result.append(car);
        }
    }

    return result;
}

bool DatabaseManager::updateCar(Car *car) {
    if (!car) return false;

    for (int i = 0; i < m_cars.size(); i++) {
        if (m_cars[i]->id() == car->id()) {
            m_cars[i]->setBrand(car->brand());
            m_cars[i]->setModel(car->model());
            m_cars[i]->setYear(car->year());
            m_cars[i]->setColor(car->color());
            m_cars[i]->setPricePerDay(car->pricePerDay());
            m_cars[i]->setStatus(car->status());
            m_cars[i]->setCategory(car->category());
            //m_cars[i]->setImageUrl(car->imageUrl());

            saveToFile();
            emit dataChanged();
            return true;
        }
    }
    return false;
}

bool DatabaseManager::deleteCar(int id) {
    for (int i = 0; i < m_cars.size(); i++) {
        if (m_cars[i]->id() == id) {
            Car* car = m_cars.takeAt(i);
            delete car;

            saveToFile();
            emit dataChanged();
            return true;
        }
    }
    return false;
}


bool DatabaseManager::addRental(Rental *rental) {
    if (!rental) return false;

    m_rentals.append(rental);

    Car* car = getCar(rental->carId());
    if (car) {
        car->setStatus(Car::RENTED);
    }

    saveToFile();
    emit dataChanged();

    qDebug() << "Rental added. ID:" << rental->id()
             << "Car:" << rental->carId()
             << "User:" << rental->userId();
    return true;
}

Rental* DatabaseManager::getRental(int id) {
    for (Rental* rental : m_rentals) {
        if (rental->id() == id) {
            return rental;
        }
    }
    return nullptr;
}

QList<Rental*> DatabaseManager::getUserRentals(int userId) {
    QList<Rental*> result;
    for (Rental* rental : m_rentals) {
        if (rental->userId() == userId) {
            result.append(rental);
        }
    }
    return result;
}

QList<Rental*> DatabaseManager::getCarRentals(int carId) {
    QList<Rental*> result;
    for (Rental* rental : m_rentals) {
        if (rental->carId() == carId) {
            result.append(rental);
        }
    }
    return result;
}

QList<Rental*> DatabaseManager::getActiveRentals() {
    QList<Rental*> result;
    for (Rental* rental : m_rentals) {
        if (rental->status() == Rental::ACTIVE) {
            result.append(rental);
        }
    }
    return result;
}

bool DatabaseManager::updateRental(Rental *rental) {
    if (!rental) return false;

    for (int i = 0; i < m_rentals.size(); i++) {
        if (m_rentals[i]->id() == rental->id()) {
            m_rentals[i]->setUserId(rental->userId());
            m_rentals[i]->setCarId(rental->carId());
            m_rentals[i]->setStartDate(rental->startDate());
            m_rentals[i]->setEndDate(rental->endDate());
            m_rentals[i]->setTotalPrice(rental->totalPrice());
            m_rentals[i]->setStatus(rental->status());
            m_rentals[i]->setActualReturnDate(rental->actualReturnDate());

            saveToFile();
            emit dataChanged();
            return true;
        }
    }
    return false;
}

bool DatabaseManager::deleteRental(int id) {
    for (int i = 0; i < m_rentals.size(); i++) {
        if (m_rentals[i]->id() == id) {
            Rental* rental = m_rentals[i];
            Car* car = getCar(rental->carId());
            if (car) {
                car->setStatus(Car::AVAILABLE);
            }

            delete m_rentals.takeAt(i);

            saveToFile();
            emit dataChanged();
            return true;
        }
    }
    return false;
}


bool DatabaseManager::addFine(Fine *fine) {
    if (!fine) return false;

    m_fines.append(fine);
    saveToFile();
    emit dataChanged();

    qDebug() << "Fine added. ID:" << fine->id()
             << "Amount:" << fine->amount()
             << "Rental:" << fine->rentalId();
    return true;
}

QList<Fine*> DatabaseManager::getFinesForRental(int rentalId) {
    QList<Fine*> result;
    for (Fine* fine : m_fines) {
        if (fine->rentalId() == rentalId) {
            result.append(fine);
        }
    }
    return result;
}

QList<Fine*> DatabaseManager::getUnpaidFines() {
    QList<Fine*> result;
    for (Fine* fine : m_fines) {
        if (!fine->paid()) {
            result.append(fine);
        }
    }
    return result;
}

bool DatabaseManager::payFine(int fineId) {
    for (Fine* fine : m_fines) {
        if (fine->id() == fineId) {
            fine->setPaid(true);
            saveToFile();
            emit dataChanged();
            return true;
        }
    }
    return false;
}


Report* DatabaseManager::generateReport(const QDate &startDate, const QDate &endDate) {
    Report* report = new Report(startDate, endDate);

    double totalIncome = 0;
    int rentalCount = 0;

    for (Rental* rental : m_rentals) {
        if (rental->startDate() >= startDate &&
            rental->endDate() <= endDate &&
            rental->status() == Rental::COMPLETED) {

            totalIncome += rental->totalPrice();
            rentalCount++;
            report->addRental(rental->totalPrice());
        }
    }

    report->calculateStatistics();
    return report;
}

double DatabaseManager::getTotalIncome(const QDate &startDate, const QDate &endDate) {
    double total = 0;
    for (Rental* rental : m_rentals) {
        if (rental->startDate() >= startDate &&
            rental->endDate() <= endDate &&
            rental->status() == Rental::COMPLETED) {
            total += rental->totalPrice();
        }
    }
    return total;
}

int DatabaseManager::getRentalCount(const QDate &startDate, const QDate &endDate) {
    int count = 0;
    for (Rental* rental : m_rentals) {
        if (rental->startDate() >= startDate &&
            rental->endDate() <= endDate &&
            rental->status() == Rental::COMPLETED) {
            count++;
        }
    }
    return count;
}


bool DatabaseManager::exportToJson(const QString &filePath) {
    QJsonObject root;

    QJsonArray usersArray;
    for (User* user : m_users) {
        usersArray.append(user->toJson());
    }
    root["users"] = usersArray;

    QJsonArray carsArray;
    for (Car* car : m_cars) {
        carsArray.append(car->toJson());
    }
    root["cars"] = carsArray;

    QJsonArray rentalsArray;
    for (Rental* rental : m_rentals) {
        rentalsArray.append(rental->toJson());
    }
    root["rentals"] = rentalsArray;

    QJsonArray finesArray;
    for (Fine* fine : m_fines) {
        finesArray.append(fine->toJson());
    }
    root["fines"] = finesArray;

    QJsonDocument doc(root);
    QFile file(filePath);

    if (!file.open(QIODevice::WriteOnly)) {
        qDebug() << "Failed to export to:" << filePath;
        return false;
    }

    file.write(doc.toJson());
    file.close();

    qDebug() << "Data exported to:" << filePath;
    return true;
}

bool DatabaseManager::importFromJson(const QString &filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Failed to import from:" << filePath;
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

    qDeleteAll(m_users);
    qDeleteAll(m_cars);
    qDeleteAll(m_rentals);
    qDeleteAll(m_fines);
    m_users.clear();
    m_cars.clear();
    m_rentals.clear();
    m_fines.clear();

    if (root.contains("users")) {
        QJsonArray usersArray = root["users"].toArray();
        for (const QJsonValue &value : usersArray) {
            User* user = User::fromJson(value.toObject(), this);
            if (user) {
                m_users.append(user);
            }
        }
    }

    if (root.contains("cars")) {
        QJsonArray carsArray = root["cars"].toArray();
        for (const QJsonValue &value : carsArray) {
            Car* car = Car::fromJson(value.toObject(), this);
            if (car) {
                m_cars.append(car);
            }
        }
    }

    if (root.contains("rentals")) {
        QJsonArray rentalsArray = root["rentals"].toArray();
        for (const QJsonValue &value : rentalsArray) {
            Rental* rental = Rental::fromJson(value.toObject(), this);
            if (rental) {
                m_rentals.append(rental);
            }
        }
    }

    if (root.contains("fines")) {
        QJsonArray finesArray = root["fines"].toArray();
        for (const QJsonValue &value : finesArray) {
            Fine* fine = Fine::fromJson(value.toObject(), this);
            if (fine) {
                m_fines.append(fine);
            }
        }
    }

    saveToFile();
    emit dataChanged();

    qDebug() << "Data imported from:" << filePath
             << "Users:" << m_users.size()
             << "Cars:" << m_cars.size()
             << "Rentals:" << m_rentals.size()
             << "Fines:" << m_fines.size();

    return true;
}


int DatabaseManager::getNextUserId() {
    int maxId = 0;
    for (User* user : m_users) {
        if (user->id() > maxId) {
            maxId = user->id();
        }
    }
    return maxId + 1;
}

int DatabaseManager::getNextCarId() {
    int maxId = 0;
    for (Car* car : m_cars) {
        if (car->id() > maxId) {
            maxId = car->id();
        }
    }
    return maxId + 1;
}

int DatabaseManager::getNextRentalId() {
    int maxId = 0;
    for (Rental* rental : m_rentals) {
        if (rental->id() > maxId) {
            maxId = rental->id();
        }
    }
    return maxId + 1;
}

int DatabaseManager::getNextFineId() {
    int maxId = 0;
    for (Fine* fine : m_fines) {
        if (fine->id() > maxId) {
            maxId = fine->id();
        }
    }
    return maxId + 1;
}

QJsonObject DatabaseManager::getAllData() {
    QJsonObject root;

    QJsonArray usersArray;
    for (User* user : m_users) {
        usersArray.append(user->toJson());
    }
    root["users"] = usersArray;

    QJsonArray carsArray;
    for (Car* car : m_cars) {
        carsArray.append(car->toJson());
    }
    root["cars"] = carsArray;

    QJsonArray rentalsArray;
    for (Rental* rental : m_rentals) {
        rentalsArray.append(rental->toJson());
    }
    root["rentals"] = rentalsArray;

    QJsonArray finesArray;
    for (Fine* fine : m_fines) {
        finesArray.append(fine->toJson());
    }
    root["fines"] = finesArray;

    return root;
}

void DatabaseManager::saveAllData() {
    saveToFile();
}

void DatabaseManager::loadFromFile() {
    QFile file(m_dataFilePath);
    if (!file.exists()) {
        qDebug() << "Data file does not exist:" << m_dataFilePath;
        return;
    }

    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Failed to open data file:" << file.errorString();
        return;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull()) {
        qDebug() << "Invalid JSON in data file";
        return;
    }

    QJsonObject root = doc.object();

    if (root.contains("users")) {
        QJsonArray usersArray = root["users"].toArray();
        for (const QJsonValue &value : usersArray) {
            User* user = User::fromJson(value.toObject(), this);
            if (user) {
                m_users.append(user);
            }
        }
    }

    if (root.contains("cars")) {
        QJsonArray carsArray = root["cars"].toArray();
        for (const QJsonValue &value : carsArray) {
            Car* car = Car::fromJson(value.toObject(), this);
            if (car) {
                m_cars.append(car);
            }
        }
    }

    if (root.contains("rentals")) {
        QJsonArray rentalsArray = root["rentals"].toArray();
        for (const QJsonValue &value : rentalsArray) {
            Rental* rental = Rental::fromJson(value.toObject(), this);
            if (rental) {
                m_rentals.append(rental);
            }
        }
    }

    if (root.contains("fines")) {
        QJsonArray finesArray = root["fines"].toArray();
        for (const QJsonValue &value : finesArray) {
            Fine* fine = Fine::fromJson(value.toObject(), this);
            if (fine) {
                m_fines.append(fine);
            }
        }
    }

    qDebug() << "Data loaded from file:"
             << "Users:" << m_users.size()
             << "Cars:" << m_cars.size()
             << "Rentals:" << m_rentals.size()
             << "Fines:" << m_fines.size();
}

void DatabaseManager::saveToFile() {
    QJsonObject root = getAllData();

    QJsonDocument doc(root);
    QFile file(m_dataFilePath);

    if (!file.open(QIODevice::WriteOnly)) {
        qDebug() << "Failed to save data file:" << file.errorString();
        return;
    }

    file.write(doc.toJson());
    file.close();

    qDebug() << "Data saved to file:" << m_dataFilePath;
}

void DatabaseManager::createDefaultData() {
    qDebug() << "Creating default data...";

    User* admin = new User(1, "Администратор", "admin@mail.com", User::ADMIN, this);
    admin->setPhone("+79991234567");
    m_users.append(admin);

    User* user1 = new User(2, "Иван Иванов", "user@mail.com", User::CLIENT, this);
    user1->setPhone("+79998765432");
    user1->setLicenseNumber("AB123456");
    m_users.append(user1);

    User* user2 = new User(3, "Мария Петрова", "maria@mail.com", User::CLIENT, this);
    user2->setPhone("+79997654321");
    user2->setLicenseNumber("CD789012");
    m_users.append(user2);

    m_cars.append(new Car(1, "BMW", "M5", 5000.0, Car::LUXURY, this));
    m_cars.append(new Car(2, "Audi", "A6", 4000.0, Car::LUXURY, this));
    m_cars.append(new Car(3, "Toyota", "Camry", 1500.0, Car::ECONOMY, this));
    m_cars.append(new Car(4, "Mercedes", "E-Class", 4500.0, Car::LUXURY, this));
    m_cars.append(new Car(5, "Volkswagen", "Passat", 2000.0, Car::STANDARD, this));
    m_cars.append(new Car(6, "Toyota", "Land Cruiser", 6000.0, Car::SUV, this));

    qDebug() << "Default data created:"
             << "Users:" << m_users.size()
             << "Cars:" << m_cars.size();
}
