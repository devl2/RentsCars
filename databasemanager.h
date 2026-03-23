#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QFile>
#include <QDir>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QStandardPaths>
#include "user.h"
#include "car.h"
#include "rental.h"
#include "fine.h"
#include "report.h"

class DatabaseManager : public QObject {
    Q_OBJECT
public:
    static DatabaseManager* instance();

    bool initialize();
    bool backupDatabase(const QString &filePath);
    bool restoreDatabase(const QString &filePath);

    bool addUser(User *user);
    User* getUser(int id);
    User* getUserByEmail(const QString &email);
    QList<User*> getAllUsers();
    bool updateUser(User *user);
    bool deleteUser(int id);

    bool addCar(Car *car);
    Car* getCar(int id);
    QList<Car*> getAllCars();
    QList<Car*> getAvailableCars();
    QList<Car*> searchCars(const QString &brand, const QString &model,
                          double minPrice, double maxPrice, int category);
    bool updateCar(Car *car);
    bool deleteCar(int id);

    bool addRental(Rental *rental);
    Rental* getRental(int id);
    QList<Rental*> getUserRentals(int userId);
    QList<Rental*> getCarRentals(int carId);
    QList<Rental*> getActiveRentals();
    bool updateRental(Rental *rental);
    bool deleteRental(int id);

    bool addFine(Fine *fine);
    QList<Fine*> getFinesForRental(int rentalId);
    QList<Fine*> getUnpaidFines();
    bool payFine(int fineId);

    Report* generateReport(const QDate &startDate, const QDate &endDate);
    double getTotalIncome(const QDate &startDate, const QDate &endDate);
    int getRentalCount(const QDate &startDate, const QDate &endDate);

    bool exportToJson(const QString &filePath);
    bool importFromJson(const QString &filePath);

    int getNextUserId();
    int getNextCarId();
    int getNextRentalId();
    int getNextFineId();

    Q_INVOKABLE QJsonObject getAllData();
    Q_INVOKABLE void saveAllData();

signals:
    void databaseInitialized();
    void dataChanged();

private:
    DatabaseManager(QObject *parent = nullptr);
    ~DatabaseManager();

    void loadFromFile();
    void saveToFile();
    void createDefaultData();

    QList<User*> m_users;
    QList<Car*> m_cars;
    QList<Rental*> m_rentals;
    QList<Fine*> m_fines;

    QString m_dataFilePath;
    static DatabaseManager* m_instance;
};

#endif // DATABASEMANAGER_H
