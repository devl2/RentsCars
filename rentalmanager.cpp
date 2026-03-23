#include "rentalmanager.h"
#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QFile>
#include <QDate>
#include <QStandardPaths>
#include <QDir>
#include <QTextStream>
#include <algorithm>

RentalManager* RentalManager::m_instance = nullptr;
RentalManager::Deleter RentalManager::deleter;

RentalManager* RentalManager::instance() {
    if (!m_instance) {
        m_instance = new RentalManager();
    }
    return m_instance;
}

RentalManager::RentalManager(QObject *parent)
    : QObject(parent),
      m_totalIncome(0.0),
      m_totalRentalCount(0) {

    m_strategies.append(new DailyPricing());
    m_strategies.append(new WeeklyPricing());
    m_strategies.append(new MonthlyPricing());
    m_strategies.append(new PenaltyPricing());

    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(dataDir);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    m_dataFilePath = dataDir + "/rentals.json";

    loadData();

    qDebug() << "RentalManager initialized";
}

RentalManager::~RentalManager() {
    saveData();
    qDeleteAll(m_strategies);
    qDeleteAll(m_fines);
    qDeleteAll(m_rentals);
}


QList<Rental*> RentalManager::getActiveRentals() const {
    QList<Rental*> result;
    for (Rental* rental : m_rentals) {
        if (rental->status() == Rental::ACTIVE || rental->status() == Rental::OVERDUE) {
            result.append(rental);
        }
    }
    return result;
}

QList<Rental*> RentalManager::getCompletedRentals() const {
    QList<Rental*> result;
    for (Rental* rental : m_rentals) {
        if (rental->status() == Rental::COMPLETED) {
            result.append(rental);
        }
    }
    return result;
}

QList<Rental*> RentalManager::getCancelledRentals() const {
    QList<Rental*> result;
    for (Rental* rental : m_rentals) {
        if (rental->status() == Rental::CANCELLED) {
            result.append(rental);
        }
    }
    return result;
}

QList<Rental*> RentalManager::getOverdueRentals() const {
    QList<Rental*> result;
    QDate today = QDate::currentDate();
    for (Rental* rental : m_rentals) {
        if (rental->status() == Rental::ACTIVE && rental->endDate() < today) {
            result.append(rental);
        }
    }
    return result;
}

QList<Rental*> RentalManager::getRentalsByUser(int userId) const {
    QList<Rental*> result;
    for (Rental* rental : m_rentals) {
        if (rental->userId() == userId) {
            result.append(rental);
        }
    }
    return result;
}

QList<Rental*> RentalManager::getRentalsByCar(int carId) const {
    QList<Rental*> result;
    for (Rental* rental : m_rentals) {
        if (rental->carId() == carId) {
            result.append(rental);
        }
    }
    return result;
}

QList<Fine*> RentalManager::getUnpaidFines() const {
    QList<Fine*> result;
    for (Fine* fine : m_fines) {
        if (!fine->paid()) {
            result.append(fine);
        }
    }
    return result;
}


Rental* RentalManager::createRental(int userId, int carId,
                                   const QDate &startDate,
                                   const QDate &endDate,
                                   double dailyPrice,
                                   const QString &strategy) {

    try {
        validateRentalDates(startDate, endDate);

        if (!isCarAvailable(carId, startDate, endDate)) {
            emit errorOccurred("Автомобиль недоступен на выбранные даты");
            return nullptr;
        }

        int days = startDate.daysTo(endDate);
        if (days < 1) days = 1;

        double totalPrice = calculatePrice(dailyPrice, days, strategy);

        int rentalId = getNextRentalId();
        Rental* rental = new Rental(rentalId, userId, carId,
                                   startDate, endDate,
                                   totalPrice, Rental::ACTIVE, this);

        m_rentals.append(rental);
        m_totalRentalCount++;

        notifyCarManager(carId, Car::RENTED);

        qDebug() << "Rental created:" << rentalId << "User:" << userId
                 << "Car:" << carId << "Price:" << totalPrice;

        emit rentalCreated(rental);
        emit carRented(carId, rentalId);
        emit dataChanged();
        emit statisticsUpdated();

        return rental;

    } catch (const std::exception& e) {
        emit errorOccurred(QString("Ошибка создания аренды: %1").arg(e.what()));
        return nullptr;
    }
}

bool RentalManager::confirmRental(int rentalId) {
    Rental* rental = findRentalById(rentalId);
    if (!rental) {
        emit errorOccurred("Аренда не найдена");
        return false;
    }

    if (rental->status() != Rental::ACTIVE) {
        emit errorOccurred("Аренда уже подтверждена или отменена");
        return false;
    }

    m_totalIncome += rental->totalPrice();

    emit rentalConfirmed(rental);
    emit dataChanged();
    emit statisticsUpdated();

    return true;
}

Rental* RentalManager::getActiveRentalForCar(int carId) {

    for (Rental* rental : m_rentals) {
        if (rental->carId() == carId &&
            (rental->status() == Rental::ACTIVE || rental->status() == Rental::OVERDUE)) {
            return rental;
        }
    }

    return nullptr;
}

bool RentalManager::cancelRental(int rentalId) {
    Rental* rental = findRentalById(rentalId);
    if (!rental) {
        emit errorOccurred("Аренда не найдена");
        return false;
    }

    if (!rental->canBeCancelled()) {
        emit errorOccurred("Невозможно отменить аренду в текущем статусе");
        return false;
    }

    QDate today = QDate::currentDate();
    if (today.daysTo(rental->startDate()) < 1) {
        double penaltyAmount = rental->totalPrice() * 0.1;
        createFine(rentalId, "Штраф за позднюю отмену аренды", penaltyAmount);

        bool success = rental->cancelWithPenalty(0.1);
        if (!success) return false;
    } else {
        bool success = rental->cancel();
        if (!success) return false;
    }

    notifyCarManager(rental->carId(), Car::AVAILABLE);

    emit rentalCancelled(rental);
    emit dataChanged();

    return true;
}

bool RentalManager::returnCar(int rentalId, const QDate &actualReturnDate) {
    Rental* rental = findRentalById(rentalId);
    if (!rental) {
        emit errorOccurred("Аренда не найдена");
        return false;
    }

    if (rental->status() != Rental::ACTIVE && rental->status() != Rental::OVERDUE) {
        emit errorOccurred("Невозможно вернуть автомобиль для данной аренды");
        return false;
    }

    rental->setActualReturnDate(actualReturnDate);

    double penalty = 0.0;

    if (actualReturnDate > rental->endDate()) {
        updateRentalStatus(rental, Rental::OVERDUE);
        penalty = calculatePenalty(rentalId);

        if (penalty > 0) {
            createFine(rentalId, "Штраф за просрочку возврата", penalty);
        }
    } else {
        updateRentalStatus(rental, Rental::COMPLETED);
    }

    notifyCarManager(rental->carId(), Car::AVAILABLE);

    emit carReturned(rental, penalty);
    emit dataChanged();

    return true;
}

bool RentalManager::extendRental(int rentalId, const QDate &newEndDate) {
    Rental* rental = findRentalById(rentalId);
    if (!rental || rental->status() != Rental::ACTIVE) {
        return false;
    }

    if (newEndDate <= rental->endDate()) {
        emit errorOccurred("Новая дата окончания должна быть позже текущей");
        return false;
    }

    if (!isCarAvailable(rental->carId(), rental->endDate().addDays(1), newEndDate, rentalId)) {
        emit errorOccurred("Автомобиль недоступен на продленные даты");
        return false;
    }

    double additionalCost = calculateExtensionCost(rentalId, newEndDate);

    rental->setEndDate(newEndDate);
    rental->setTotalPrice(rental->totalPrice() + additionalCost);

    emit rentalExtended(rental, additionalCost);
    emit dataChanged();

    return true;
}


Fine* RentalManager::createFine(int rentalId, const QString &reason, double amount) {
    Rental* rental = findRentalById(rentalId);
    if (!rental) {
        return nullptr;
    }

    int fineId = getNextFineId();
    Fine* fine = new Fine(fineId, rentalId, amount, reason,
                         QDate::currentDate(), false, this);

    m_fines.append(fine);

    emit fineCreated(fine);
    emit dataChanged();

    return fine;
}

bool RentalManager::payFine(int fineId) {
    Fine* fine = findFineById(fineId);
    if (!fine || fine->paid()) {
        return false;
    }

    fine->setPaid(true);
    fine->setPaymentDate(QDate::currentDate());

    emit finePaid(fineId);
    emit dataChanged();

    return true;
}

bool RentalManager::cancelFine(int fineId) {
    Fine* fine = findFineById(fineId);
    if (!fine) {
        return false;
    }

    m_fines.removeOne(fine);
    delete fine;

    emit fineCancelled(fineId);
    emit dataChanged();

    return true;
}

double RentalManager::calculateTotalFines(int userId) const {
    double total = 0.0;

    QList<Rental*> userRentals = getRentalsByUser(userId);

    for (Rental* rental : userRentals) {
        for (Fine* fine : m_fines) {
            if (fine->rentalId() == rental->id() && !fine->paid()) {
                total += fine->amount();
            }
        }
    }

    return total;
}

QList<Rental*> RentalManager::searchRentals(const QString &userName,
                                           const QDate &startDate,
                                           const QDate &endDate,
                                           const QString &carBrand,
                                           const QString &carModel,
                                           int status) {
    QList<Rental*> result;

    for (Rental* rental : m_rentals) {
        bool matches = true;

        if (status != -1 && rental->status() != status) {
            matches = false;
        }

        if (startDate.isValid() && rental->startDate() < startDate) {
            matches = false;
        }

        if (endDate.isValid() && rental->endDate() > endDate) {
            matches = false;
        }

        if (matches) {
            result.append(rental);
        }
    }

    return result;
}

QList<Rental*> RentalManager::findOverlappingRentals(int carId,
                                                     const QDate &startDate,
                                                     const QDate &endDate) const {
    QList<Rental*> overlapping;

    for (Rental* rental : m_rentals) {
        if (rental->carId() == carId &&
            (rental->status() == Rental::ACTIVE || rental->status() == Rental::OVERDUE) &&
            !(endDate < rental->startDate() || startDate > rental->endDate())) {
            overlapping.append(rental);
        }
    }

    return overlapping;
}

double RentalManager::calculatePrice(double dailyPrice, int days, const QString &strategy) {
    PricingStrategy* pricingStrategy = getStrategy(strategy);
    if (!pricingStrategy) {
        return dailyPrice * days;
    }
    return pricingStrategy->calculatePrice(dailyPrice, days);
}

double RentalManager::calculatePenalty(int rentalId) const {
    Rental* rental = findRentalById(rentalId);
    if (!rental || !rental->actualReturnDate().isValid()) {
        return 0.0;
    }

    int overdueDays = rental->endDate().daysTo(rental->actualReturnDate());
    if (overdueDays <= 0) {
        return 0.0;
    }

    PricingStrategy* penaltyStrategy = getStrategy("penalty");
    if (penaltyStrategy) {
        return penaltyStrategy->calculatePrice(rental->totalPrice() / rental->rentalDays(),
                                              overdueDays);
    }

    double dailyCost = rental->totalPrice() / rental->rentalDays();
    return dailyCost * 0.5 * overdueDays;
}

double RentalManager::calculateExtensionCost(int rentalId, const QDate &newEndDate) const {
    Rental* rental = findRentalById(rentalId);
    if (!rental) {
        return 0.0;
    }

    int extensionDays = rental->endDate().daysTo(newEndDate);
    if (extensionDays <= 0) {
        return 0.0;
    }

    double dailyPrice = rental->totalPrice() / rental->rentalDays();

    return dailyPrice * extensionDays;
}


double RentalManager::getIncomeForPeriod(const QDate &start, const QDate &end) const {
    double income = 0.0;

    for (Rental* rental : m_rentals) {
        if (rental->startDate() >= start &&
            rental->startDate() <= end &&
            (rental->status() == Rental::COMPLETED || rental->status() == Rental::ACTIVE)) {
            income += rental->totalPrice();
        }
    }

    return income;
}

int RentalManager::getRentalCountForPeriod(const QDate &start, const QDate &end) const {
    int count = 0;

    for (Rental* rental : m_rentals) {
        if (rental->startDate() >= start && rental->startDate() <= end) {
            count++;
        }
    }

    return count;
}

QMap<QString, double> RentalManager::getIncomeByCarCategory() const {
    QMap<QString, double> incomeByCategory;

    for (Rental* rental : m_rentals) {
        if (rental->status() == Rental::COMPLETED || rental->status() == Rental::ACTIVE) {
            QString category = "STANDARD";
            incomeByCategory[category] += rental->totalPrice();
        }
    }

    return incomeByCategory;
}

QString RentalManager::generateDetailedReport(const QDate &start, const QDate &end) const {
    QString report;
    QTextStream stream(&report);

    stream << "Детальный отчет по арендам\n";
    stream << "============================\n";
    stream << "Период: " << start.toString("dd.MM.yyyy")
           << " - " << end.toString("dd.MM.yyyy") << "\n\n";

    int totalCount = getRentalCountForPeriod(start, end);
    double totalIncome = getIncomeForPeriod(start, end);
    int activeCount = 0;
    int completedCount = 0;

    stream << "Общая статистика:\n";
    stream << "-----------------\n";
    stream << "Всего аренд: " << totalCount << "\n";
    stream << "Общий доход: " << QString::number(totalIncome, 'f', 2) << " руб.\n\n";

    stream << "Список аренд:\n";
    stream << "-------------\n";

    int counter = 1;
    for (Rental* rental : m_rentals) {
        if (rental->startDate() >= start && rental->startDate() <= end) {
            stream << counter++ << ". ID: " << rental->id()
                   << " | Автомобиль: " << rental->carId()
                   << " | Клиент: " << rental->userId()
                   << " | Период: " << rental->startDate().toString("dd.MM.yyyy")
                   << " - " << rental->endDate().toString("dd.MM.yyyy")
                   << " | Стоимость: " << QString::number(rental->totalPrice(), 'f', 2)
                   << " руб. | Статус: " << rental->status() << "\n";

            if (rental->status() == Rental::ACTIVE) activeCount++;
            if (rental->status() == Rental::COMPLETED) completedCount++;
        }
    }

    stream << "\nАктивные аренды: " << activeCount << "\n";
    stream << "Завершенные аренды: " << completedCount << "\n";
    stream << "Средний чек: "
           << (totalCount > 0 ? QString::number(totalIncome / totalCount, 'f', 2) : "0.00")
           << " руб.\n";

    return report;
}

void RentalManager::addPricingStrategy(PricingStrategy* strategy) {
    if (!strategy || getStrategy(strategy->getName())) {
        return;
    }
    m_strategies.append(strategy);
}

QList<QString> RentalManager::getAvailableStrategies() const {
    QList<QString> strategyNames;
    for (PricingStrategy* strategy : m_strategies) {
        strategyNames.append(strategy->getName());
    }
    return strategyNames;
}

PricingStrategy* RentalManager::getStrategy(const QString &name) const {
    for (PricingStrategy* strategy : m_strategies) {
        if (strategy->getName().toLower() == name.toLower()) {
            return strategy;
        }
    }
    return nullptr;
}

bool RentalManager::exportToJson(const QString &filePath) {
    QJsonObject root;
    QJsonArray rentalsArray;
    QJsonArray finesArray;

    for (Rental* rental : m_rentals) {
        rentalsArray.append(rental->toJson());
    }

    for (Fine* fine : m_fines) {
        finesArray.append(fine->toJson());
    }

    root["rentals"] = rentalsArray;
    root["fines"] = finesArray;
    root["totalIncome"] = m_totalIncome;
    root["totalRentalCount"] = m_totalRentalCount;

    QJsonDocument doc(root);
    QFile file(filePath);

    if (!file.open(QIODevice::WriteOnly)) {
        emit errorOccurred("Не удалось открыть файл для записи: " + file.errorString());
        return false;
    }

    file.write(doc.toJson());
    file.close();

    qDebug() << "Data exported to JSON:" << filePath;
    return true;
}

bool RentalManager::importFromJson(const QString &filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        emit errorOccurred("Не удалось открыть файл для чтения: " + file.errorString());
        return false;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull() || !doc.isObject()) {
        emit errorOccurred("Некорректный JSON файл");
        return false;
    }

    QJsonObject root = doc.object();

    qDeleteAll(m_rentals);
    m_rentals.clear();
    qDeleteAll(m_fines);
    m_fines.clear();

    if (root.contains("rentals") && root["rentals"].isArray()) {
        QJsonArray rentalsArray = root["rentals"].toArray();
        for (const QJsonValue &value : rentalsArray) {
            Rental* rental = Rental::fromJson(value.toObject(), this);
            if (rental) {
                m_rentals.append(rental);
            }
        }
    }

    if (root.contains("fines") && root["fines"].isArray()) {
        QJsonArray finesArray = root["fines"].toArray();
        for (const QJsonValue &value : finesArray) {
            Fine* fine = Fine::fromJson(value.toObject(), this);
            if (fine) {
                m_fines.append(fine);
            }
        }
    }

    if (root.contains("totalIncome")) {
        m_totalIncome = root["totalIncome"].toDouble();
    }

    if (root.contains("totalRentalCount")) {
        m_totalRentalCount = root["totalRentalCount"].toInt();
    }

    emit dataChanged();
    emit statisticsUpdated();

    qDebug() << "Data imported from JSON:" << filePath
             << "Rentals:" << m_rentals.size()
             << "Fines:" << m_fines.size();

    return true;
}

void RentalManager::updateOverdueRentals() {
    QDate today = QDate::currentDate();
    bool updated = false;

    for (Rental* rental : m_rentals) {
        if (rental->status() == Rental::ACTIVE && rental->endDate() < today) {
            updateRentalStatus(rental, Rental::OVERDUE);
            updated = true;
        }
    }

    if (updated) {
        emit dataChanged();
    }
}

void RentalManager::cleanupOldData(int daysOld) {
    QDate cutoffDate = QDate::currentDate().addDays(-daysOld);
    bool cleaned = false;

    for (int i = m_rentals.size() - 1; i >= 0; i--) {
        Rental* rental = m_rentals[i];
        if (rental->status() == Rental::COMPLETED &&
            rental->actualReturnDate() < cutoffDate) {
            m_rentals.removeAt(i);
            delete rental;
            cleaned = true;
        }
    }

    for (int i = m_fines.size() - 1; i >= 0; i--) {
        Fine* fine = m_fines[i];
        if (fine->paid() && fine->paymentDate() < cutoffDate) {
            m_fines.removeAt(i);
            delete fine;
            cleaned = true;
        }
    }

    if (cleaned) {
        emit dataChanged();
    }
}

bool RentalManager::loadData() {
    return importFromJson(m_dataFilePath);
}

bool RentalManager::saveData() {
    return exportToJson(m_dataFilePath);
}

int RentalManager::getNextRentalId() const {
    int maxId = 0;
    for (Rental* rental : m_rentals) {
        if (rental->id() > maxId) {
            maxId = rental->id();
        }
    }
    return maxId + 1;
}

int RentalManager::getNextFineId() const {
    int maxId = 0;
    for (Fine* fine : m_fines) {
        if (fine->id() > maxId) {
            maxId = fine->id();
        }
    }
    return maxId + 1;
}

Rental* RentalManager::findRentalById(int id) const {
    for (Rental* rental : m_rentals) {
        if (rental->id() == id) {
            return rental;
        }
    }
    return nullptr;
}

Fine* RentalManager::findFineById(int id) const {
    for (Fine* fine : m_fines) {
        if (fine->id() == id) {
            return fine;
        }
    }
    return nullptr;
}

bool RentalManager::isCarAvailable(int carId, const QDate &startDate,
                                  const QDate &endDate, int excludeRentalId) const {
    for (Rental* rental : m_rentals) {
        if (rental->id() == excludeRentalId) {
            continue;
        }

        if (rental->carId() == carId &&
            (rental->status() == Rental::ACTIVE || rental->status() == Rental::OVERDUE) &&
            !(endDate < rental->startDate() || startDate > rental->endDate())) {
            return false;
        }
    }
    return true;
}

void RentalManager::updateRentalStatus(Rental* rental, Rental::Status newStatus) {
    if (!rental || rental->status() == newStatus) {
        return;
    }

    Rental::Status oldStatus = rental->status();
    rental->setStatus(newStatus);

    emit rentalStatusChanged(rental->id(), oldStatus, newStatus);
}

void RentalManager::notifyCarManager(int carId, int status) {
    qDebug() << "Car" << carId << "status changed to:" << status;
}

void RentalManager::updateStatistics() {
    m_totalIncome = 0.0;
    for (Rental* rental : m_rentals) {
        if (rental->status() == Rental::COMPLETED || rental->status() == Rental::ACTIVE) {
            m_totalIncome += rental->totalPrice();
        }
    }

    m_totalRentalCount = m_rentals.size();
}

void RentalManager::validateRentalDates(const QDate &startDate, const QDate &endDate) const {
    QDate today = QDate::currentDate();

    if (!startDate.isValid() || !endDate.isValid()) {
        throw std::runtime_error("Некорректные даты");
    }

    if (startDate < today) {
        throw std::runtime_error("Дата начала не может быть в прошлом");
    }

    if (endDate < startDate) {
        throw std::runtime_error("Дата окончания должна быть позже даты начала");
    }

    if (startDate.daysTo(endDate) > 365) {
        throw std::runtime_error("Максимальный срок аренды - 1 год");
    }
}
