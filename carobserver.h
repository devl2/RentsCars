#ifndef CAROBSERVER_H
#define CAROBSERVER_H

#include <QObject>
#include <QList>

class CarObserver {
public:
    virtual ~CarObserver() = default;
    virtual void onCarStatusChanged(int carId, int newStatus) = 0;
    virtual void onCarRented(int carId) = 0;
    virtual void onCarReturned(int carId) = 0;
};

class CarObservable : public QObject {
    Q_OBJECT
public:
    void addObserver(CarObserver* observer) {
        m_observers.append(observer);
    }

    void removeObserver(CarObserver* observer) {
        m_observers.removeAll(observer);
    }

    void notifyCarStatusChanged(int carId, int newStatus) {
        for (auto observer : m_observers) {
            observer->onCarStatusChanged(carId, newStatus);
        }
        emit carStatusChanged(carId, newStatus);
    }

    void notifyCarRented(int carId) {
        for (auto observer : m_observers) {
            observer->onCarRented(carId);
        }
        emit carRented(carId);
    }

    void notifyCarReturned(int carId) {
        for (auto observer : m_observers) {
            observer->onCarReturned(carId);
        }
        emit carReturned(carId);
    }

signals:
    void carStatusChanged(int carId, int newStatus);
    void carRented(int carId);
    void carReturned(int carId);

private:
    QList<CarObserver*> m_observers;
};

#endif // CAROBSERVER_H
