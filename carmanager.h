#ifndef CARMANAGER_H
#define CARMANAGER_H

#include <QObject>
#include <QList>
#include "car.h"
#include "carobserver.h"

#include "databasemanager.h"

class CarManager : public QObject, public CarObserver {
    Q_OBJECT

public:
    static CarManager* instance();

    QList<Car*> getAllCars() const { return m_cars; }
    QList<Car*> getAvailableCars() const;
    QList<Car*> getRentedCars() const;
    QList<Car*> getMaintenanceCars() const;

    Car* getCarById(int id) const;
    QList<Car*> searchCars(const QString &brand = "",
                          const QString &model = "",
                          double minPrice = 0.0,
                          double maxPrice = 1000000.0,
                          int category = -1);

    bool addCar(const QString &brand, const QString &model,
               double pricePerDay, int category, int year = 0,
               const QString &color = "", const QString &imageUrl = "");

    bool updateCar(int id, const QString &brand, const QString &model,
                  double pricePerDay, int category, int year = 0,
                  const QString &color = "", const QString &imageUrl = "");

    bool deleteCar(int id);

    bool setCarStatus(int carId, int status);
    bool rentCar(int carId);
    bool returnCar(int carId);
    bool sendToMaintenance(int carId);
    bool returnFromMaintenance(int carId);

    int getCarsCount() const { return m_cars.size(); }
    int getAvailableCarsCount() const;
    int getRentedCarsCount() const;
    int getMaintenanceCarsCount() const;

    double getTotalValue() const;
    double getAveragePrice() const;

    void onCarStatusChanged(int carId, int newStatus) override;
    void onCarRented(int carId) override;
    void onCarReturned(int carId) override;

    void createTestCars();

    bool loadCars();
    bool saveCars();

    bool exportToJson(const QString &filePath);
    bool importFromJson(const QString &filePath);

signals:
    void carsChanged();
    void carAdded(Car* car);
    void carUpdated(Car* car);
    void carDeleted(int carId);
    void carStatusChanged(int carId, int status);

private:
    CarManager(QObject *parent = nullptr);
    ~CarManager();

    CarManager(const CarManager&) = delete;
    CarManager& operator=(const CarManager&) = delete;

    QList<Car*> m_cars;
    // DatabaseManager* m_dbManager;

    static CarManager* m_instance;
};

#endif // CARMANAGER_H
