#ifndef CLIENTWINDOW_H
#define CLIENTWINDOW_H

#include <QMainWindow>
#include <QTabWidget>
#include <QTableWidget>
#include <QPushButton>
#include <QListWidget>
#include <QGroupBox>
#include <QDateEdit>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QToolBar>
#include <QScrollArea>
#include <QFrame>
#include <QHeaderView>
#include <QPixmap>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QMessageBox>
#include <QInputDialog>
#include <QAction>
#include <QDialog>
#include <QDate>
#include <QDebug>
#include <QStatusBar>
#include <QButtonGroup>
#include <QRadioButton>

class ClientWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit ClientWindow(QWidget *parent = nullptr);

private slots:
    void onLogout();
    void refreshAvailableCars();
    void refreshMyRentals();
    void refreshMyFines();
    void refreshProfile();
    void refreshHistoryTable(QTableWidget *historyTable);
    void updateProfileStats();

    void rentCar(int carId = -1);
    void cancelMyRental(int rentalId = -1);
    void payFine(int fineId = -1);

    void searchAvailableCars();
    void filterByBrand(const QString &brand);
    void showAllBrands();

private:
    void setupUI();
    void setupToolbar();
    void setupAvailableCarsTab();
    void setupMyRentalsTab();
    void setupMyFinesTab();
    void setupProfileTab();
    void showAllBrandsView();
    void setupCarPlaceholder(QLabel *label, const QString &brand, const QString &color);


    QWidget* createCarCard(int carId, const QString &brand, const QString &model,
                          const QString &category, double price, const QString &color, int year);

    QWidget* createRentalCard(int rentalId, const QString &carName,
                             const QString &brand, const QString &model,
                             const QDate &startDate, const QDate &endDate,
                             const QString &status, double totalPrice,
                             const QString &carColor, int carYear);

    QWidget* createFineCard(int fineId, int rentalId, const QString &reason,
                           double amount, const QDate &issueDate, bool paid);

    void clearCarCards();
    void clearRentalCards();
    void clearFineCards();
    void updateBrandFilter();
    void showRentalDetails(int rentalId);
    void showFineDetails(int fineId);
    void extendRental(int rentalId);
    void updateRentalCard(int rentalId, const QDate &newEndDate, double newTotalPrice);
    void completeRental(int rentalId);

    QTabWidget *mainTabWidget;

    QScrollArea *carsScrollArea;
    QWidget *carsContainer;
    QVBoxLayout *carsContainerLayout;
    QLineEdit *searchEdit;
    QButtonGroup *brandButtonGroup;
    QWidget *brandFilterWidget;
    QVBoxLayout *brandFilterLayout;

    // Мои аренды
    QScrollArea *rentalsScrollArea;
    QWidget *rentalsContainer;
    QVBoxLayout *rentalsContainerLayout;
    QLineEdit *searchRentalsEdit;
    QComboBox *rentalFilterComboBox;

    // Мои штрафы
    QScrollArea *finesScrollArea;
    QWidget *finesContainer;
    QVBoxLayout *finesContainerLayout;
    QLineEdit *searchFinesEdit;
    QComboBox *fineFilterComboBox;

    // Профиль
    QLabel *profileNameLabel;
    QLabel *profileEmailLabel;
    QLabel *profilePhoneLabel;
    QLabel *profileLicenseLabel;
    QLabel *profileTripsLabel;
    QLabel *profileRatingLabel;

    QList<QWidget*> carCards;
    QList<QRadioButton*> brandButtons;
    QList<QWidget*> rentalCards;
    QList<QWidget*> fineCards;
};

#endif // CLIENTWINDOW_H
