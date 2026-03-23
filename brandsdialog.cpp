#include "brandsdialog.h"
#include "ui_brands.h"
#include "databasemanager.h"
#include "car.h"

#include <QCheckBox>
#include <QVBoxLayout>
#include <QDebug>
#include <QPushButton>
#include <QLabel>

BrandsDialog::BrandsDialog(DatabaseManager *dbManager, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::brands),
    m_dbManager(dbManager)
{
    ui->setupUi(this);
    setWindowTitle("Фильтр автомобилей");

    loadBrandsFromDatabase();

    setupConnections();
}

BrandsDialog::~BrandsDialog()
{
    delete ui;
}

void BrandsDialog::loadBrandsFromDatabase()
{
    if (!m_dbManager) {
        qWarning() << "DatabaseManager not initialized!";
        return;
    }

    QList<Car*> cars = m_dbManager->getAllCars();

    QSet<QString> uniqueBrands;
    for (Car *car : cars) {
        uniqueBrands.insert(car->brand());
    }

    if (ui->verticalLayout) {
        while (ui->verticalLayout->count() > 6) {
            QLayoutItem *item = ui->verticalLayout->takeAt(6);
            if (item) {
                if (item->widget()) {
                    delete item->widget();
                }
                delete item;
            }
        }

        for (const QString &brand : uniqueBrands) {
            bool exists = false;
            if (brand.toUpper() == "AUDI" || brand == "Audi") exists = true;
            if (brand.toUpper() == "BMW") exists = true;
            if (brand.toUpper() == "TOYOTA" || brand == "Toyota") exists = true;
            if (brand.toUpper() == "MERS" || brand == "Mers" ||
                brand.toUpper() == "MERCEDES") exists = true;

            if (!exists) {
                QCheckBox *checkBox = new QCheckBox(brand, ui->scrollAreaWidgetContents);
                checkBox->setObjectName(brand + "Check");
                ui->verticalLayout->insertWidget(ui->verticalLayout->count() - 1, checkBox);
            }
        }
    }
}

QStringList BrandsDialog::getSelectedBrands() const
{
    QStringList selectedBrands;

    if (ui->audiCheck && ui->audiCheck->isChecked()) {
        selectedBrands.append("Audi");
    }
    if (ui->bmwCheck && ui->bmwCheck->isChecked()) {
        selectedBrands.append("BMW");
    }
    if (ui->toyotaCheck && ui->toyotaCheck->isChecked()) {
        selectedBrands.append("Toyota");
    }
    if (ui->mersCheck && ui->mersCheck->isChecked()) {
        selectedBrands.append("Mers");
    }

    if (ui->verticalLayout) {
        for (int i = 6; i < ui->verticalLayout->count() - 1; i++) {
            QCheckBox *checkBox = qobject_cast<QCheckBox*>(
                ui->verticalLayout->itemAt(i)->widget()
            );
            if (checkBox && checkBox->isChecked()) {
                selectedBrands.append(checkBox->text());
            }
        }
    }

    return selectedBrands;
}

void BrandsDialog::on_applyButton_clicked()
{
    QStringList brands = getSelectedBrands();
    emit filtersApplied(brands);
    accept();
}

void BrandsDialog::setupConnections()
{
    if (ui->applyButton) {
        connect(ui->applyButton, &QPushButton::clicked,
                this, &BrandsDialog::on_applyButton_clicked);
    }
}
