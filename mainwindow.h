#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidgetItem>
#include <loginwindow.h>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class DatabaseManager;
class BrandsDialog;
class Car;
class User;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_loginButton_clicked();
    void on_registerButton_clicked();
    void on_logoutButton_clicked();
    void on_profileButton_clicked();
    void on_backButton_clicked();

    void on_filterButton_clicked();
    void on_brandList_itemClicked(QListWidgetItem *item);

    void onFiltersApplied(const QStringList &brands);

    void on_carItemClicked(QListWidgetItem *item);

    void onLoginSuccess(User *user);
    void onLoginFailed(const QString &error);

    void updateAvailableCars();
    void updateUserRentals();

private:
    Ui::MainWindow *ui;
    DatabaseManager *m_dbManager;
    BrandsDialog *m_brandsDialog;
    User *m_currentUser;
    LoginWindow *m_loginWindow;

    void setupConnections();
    void initializeDatabase();

    void loadCars();
    void updateCarList(const QList<Car*> &cars);
    void displayCarDetails(Car *car);
    void clearCarDetails();

    void setupUserInterface(User *user);
    void clearUserInterface();

    void filterCarsByBrand(const QString &brand);
    void filterCarsByBrands(const QStringList &brands);

    void showMessage(const QString &title, const QString &message, bool isError = false);
    void switchToPage(int pageIndex);

    enum PageIndex {
        LOGIN_PAGE = 0,
        REGISTER_PAGE = 1,
        MAIN_PAGE = 2,
        PROFILE_PAGE = 3,
        CAR_DETAILS_PAGE = 4
    };
};

#endif // MAINWINDOW_H
