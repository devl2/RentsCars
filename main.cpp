#include <QApplication>
#include <QMainWindow>
#include <QMessageBox>
#include <QStyleFactory>
#include <QDebug>
#include <QSharedPointer>

#include "authmanager.h"
#include "carmanager.h"
#include "rentalmanager.h"
#include "databasemanager.h"
#include "user.h"
#include "car.h"
#include "rental.h"
#include "fine.h"
#include "report.h"
#include "pricingstrategy.h"

#include "loginwindow.h"
#include "mainwindow.h"
#include "clientwindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    app.setOrganizationName("CarRental");
    app.setApplicationName("RentCar");
    app.setApplicationVersion("1.0.0");
    app.setStyle(QStyleFactory::create("Fusion"));

    DatabaseManager::instance()->initialize();
    AuthManager* authManager = AuthManager::instance();
    CarManager* carManager = CarManager::instance();
    RentalManager* rentalManager = RentalManager::instance();

    if (!authManager || !carManager || !rentalManager) {
        QMessageBox::critical(nullptr, "Ошибка",
                            "Не удалось инициализировать менеджеры приложения");
        return -1;
    }

    LoginWindow loginWindow;

    static ClientWindow *clientWindow = nullptr;
    static MainWindow *mainWindow = nullptr;

    QObject::connect(&loginWindow, &LoginWindow::loginSuccessful,
                    [&]() {
                        User* currentUser = AuthManager::instance()->currentUser();
                        if (!currentUser) {
                            return;
                        }

                        loginWindow.hide();

                        if (currentUser->isAdmin()) {
                            if (clientWindow) {
                                clientWindow->close();
                                clientWindow->deleteLater();
                                clientWindow = nullptr;
                            }

                            if (!mainWindow) {
                                mainWindow = new MainWindow();
                                mainWindow->setAttribute(Qt::WA_DeleteOnClose);

                                QObject::connect(mainWindow, &MainWindow::destroyed,
                                               [&]() {
                                                    mainWindow = nullptr;
                                               });
                            }

                            mainWindow->show();
                            mainWindow->activateWindow();
                        } else {
                            if (mainWindow) {
                                mainWindow->hide();
                            }

                            if (!clientWindow) {
                                clientWindow = new ClientWindow();
                                clientWindow->setAttribute(Qt::WA_DeleteOnClose);

                                QObject::connect(clientWindow, &ClientWindow::destroyed,
                                               [&]() {
                                                    clientWindow = nullptr;
                                               });
                            }

                            clientWindow->show();
                            clientWindow->activateWindow();
                        }
                    });

    QObject::connect(&loginWindow, &LoginWindow::loginCancelled,
                    &app, &QApplication::quit);

    loginWindow.show();

    int result = app.exec();

    // Удаляем окна если они еще существуют
    if (mainWindow) {
        mainWindow->deleteLater();
        mainWindow = nullptr;
    }

    if (clientWindow) {
        clientWindow->deleteLater();
        clientWindow = nullptr;
    }

    if (authManager) authManager->saveUsersToFile();
    if (carManager) carManager->saveCars();
    if (rentalManager) rentalManager->saveData();

    return result;
}
