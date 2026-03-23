#ifndef BRANDSDIALOG_H
#define BRANDSDIALOG_H

#include <QDialog>

class DatabaseManager;
class Car;

namespace Ui {
class brands;
}

class BrandsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit BrandsDialog(DatabaseManager *dbManager, QWidget *parent = nullptr);
    ~BrandsDialog();

    QStringList getSelectedBrands() const;

signals:
    void filtersApplied(const QStringList &brands);

private slots:
    void on_applyButton_clicked();

private:
    Ui::brands *ui;
    DatabaseManager *m_dbManager;

    void loadBrandsFromDatabase();
    void setupConnections();
};

#endif // BRANDSDIALOG_H
