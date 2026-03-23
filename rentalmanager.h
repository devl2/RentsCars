#ifndef RENTALMANAGER_H
#define RENTALMANAGER_H

#include <QObject>
#include <QList>
#include <QDate>
#include <QMap>
#include "rental.h"
#include "car.h"
#include "user.h"
#include "fine.h"
#include "pricingstrategy.h"

class RentalManager : public QObject {
    Q_OBJECT

public:
    static RentalManager* instance();

    QList<Rental*> getAllRentals() const { return m_rentals; }
    QList<Rental*> getActiveRentals() const;
    QList<Rental*> getCompletedRentals() const;
    QList<Rental*> getCancelledRentals() const;
    QList<Rental*> getOverdueRentals() const;
    QList<Rental*> getRentalsByUser(int userId) const;
    QList<Rental*> getRentalsByCar(int carId) const;
    QList<Fine*> getAllFines() const { return m_fines; }
    QList<Fine*> getUnpaidFines() const;

    Rental* createRental(int userId, int carId,
                        const QDate &startDate,
                        const QDate &endDate,
                        double dailyPrice,
                        const QString &strategy = "daily");

    Rental* getActiveRentalForCar(int carId);

    bool confirmRental(int rentalId);
    bool cancelRental(int rentalId);
    bool returnCar(int rentalId, const QDate &actualReturnDate);
    bool extendRental(int rentalId, const QDate &newEndDate);

    Fine* createFine(int rentalId, const QString &reason, double amount);
    bool payFine(int fineId);
    bool cancelFine(int fineId);
    double calculateTotalFines(int userId) const;

    QList<Rental*> searchRentals(const QString &userName = "",
                                const QDate &startDate = QDate(),
                                const QDate &endDate = QDate(),
                                const QString &carBrand = "",
                                const QString &carModel = "",
                                int status = -1);

    QList<Rental*> findOverlappingRentals(int carId, const QDate &startDate, const QDate &endDate) const;

    double calculatePrice(double dailyPrice, int days, const QString &strategy);
    double calculatePenalty(int rentalId) const;
    double calculateExtensionCost(int rentalId, const QDate &newEndDate) const;

    double getTotalIncome() const { return m_totalIncome; }
    int getTotalRentalCount() const { return m_totalRentalCount; }

    double getIncomeForPeriod(const QDate &start, const QDate &end) const;
    int getRentalCountForPeriod(const QDate &start, const QDate &end) const;

    QMap<QString, double> getIncomeByCarCategory() const;
    QMap<QString, int> getRentalCountByMonth() const;
    QMap<int, double> getTopUsersBySpending(int limit = 10) const;

    QString generateDetailedReport(const QDate &start, const QDate &end) const;
    QString generateFinancialReport(const QDate &start, const QDate &end) const;

    void addPricingStrategy(PricingStrategy* strategy);
    QList<QString> getAvailableStrategies() const;
    PricingStrategy* getStrategy(const QString &name) const;

    bool exportToJson(const QString &filePath);
    bool importFromJson(const QString &filePath);
    bool exportToCsv(const QString &filePath);
    bool importFromCsv(const QString &filePath);

    void updateOverdueRentals();
    void cleanupOldData(int daysOld = 365);
    bool loadData();
    bool saveData();

    Rental* findRentalById(int id) const;
    Fine* findFineById(int id) const;

signals:
    void rentalCreated(Rental* rental);
    void rentalConfirmed(Rental* rental);
    void rentalCancelled(Rental* rental);
    void rentalExtended(Rental* rental, double additionalCost);
    void carReturned(Rental* rental, double penalty);
    void carRented(int carId, int rentalId);
    void rentalStatusChanged(int rentalId, int oldStatus, int newStatus);

    void fineCreated(Fine* fine);
    void finePaid(int fineId);
    void fineCancelled(int fineId);

    void dataChanged();
    void statisticsUpdated();

    void errorOccurred(const QString &errorMessage);

private:
    RentalManager(QObject *parent = nullptr);
    ~RentalManager();

    RentalManager(const RentalManager&) = delete;
    RentalManager& operator=(const RentalManager&) = delete;

    int getNextRentalId() const;
    int getNextFineId() const;

    bool isCarAvailable(int carId, const QDate &startDate, const QDate &endDate, int excludeRentalId = -1) const;
    void updateRentalStatus(Rental* rental, Rental::Status newStatus);
    void notifyCarManager(int carId, int status);

    void updateStatistics();
    void validateRentalDates(const QDate &startDate, const QDate &endDate) const;

    QList<Rental*> m_rentals;
    QList<Fine*> m_fines;
    QList<PricingStrategy*> m_strategies;

    double m_totalIncome;
    int m_totalRentalCount;

    QString m_dataFilePath;

    static RentalManager* m_instance;

    class Deleter {
    public:
        ~Deleter() {
            if (RentalManager::m_instance) {
                delete RentalManager::m_instance;
                RentalManager::m_instance = nullptr;
            }
        }
    };
    static Deleter deleter;
};

#endif // RENTALMANAGER_H
