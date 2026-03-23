#include "clientwindow.h"
#include "authmanager.h"
#include "carmanager.h"
#include "rentalmanager.h"
#include "car.h"
#include "user.h"
#include "rental.h"
#include "fine.h"
#include "loginwindow.h"
#include <QScrollArea>
#include <QFrame>
#include <QButtonGroup>
#include <QRadioButton>
#include <QIcon>
#include <QFont>
#include <QPalette>
#include <QHeaderView>
#include <QFile>
#include <QPainter>
#include <QPainterPath>

ClientWindow::ClientWindow(QWidget *parent)
    : QMainWindow(parent),
      mainTabWidget(nullptr),
      carsScrollArea(nullptr),
      carsContainer(nullptr),
      carsContainerLayout(nullptr),
      searchEdit(nullptr),
      brandButtonGroup(nullptr),
      brandFilterWidget(nullptr),
      brandFilterLayout(nullptr),
      rentalsScrollArea(nullptr),
      rentalsContainer(nullptr),
      rentalsContainerLayout(nullptr),
      searchRentalsEdit(nullptr),
      rentalFilterComboBox(nullptr),
      finesScrollArea(nullptr),
      finesContainer(nullptr),
      finesContainerLayout(nullptr),
      searchFinesEdit(nullptr),
      fineFilterComboBox(nullptr),
      profileNameLabel(nullptr),
      profileEmailLabel(nullptr),
      profilePhoneLabel(nullptr),
      profileLicenseLabel(nullptr) {

    setupUI();
    setWindowTitle("Система аренды автомобилей");
    setMinimumSize(1250, 850);

    setStyleSheet(
        "QMainWindow { background-color: #f5f5f5; }"
        "QGroupBox {"
        "   font-weight: bold;"
        "   border: 2px solid #ddd;"
        "   border-radius: 5px;"
        "   margin-top: 10px;"
        "   padding-top: 10px;"
        "}"
        "QGroupBox::title {"
        "   subcontrol-origin: margin;"
        "   left: 10px;"
        "   padding: 0 5px 0 5px;"
        "}"
    );

    refreshAvailableCars();
    refreshMyRentals();
    refreshMyFines();
    refreshProfile();
}

void ClientWindow::setupMyFinesTab() {
    QWidget *finesTab = new QWidget();
    QVBoxLayout *finesLayout = new QVBoxLayout(finesTab);
    finesLayout->setContentsMargins(20, 20, 20, 20);
    finesLayout->setSpacing(20);

    QLabel *titleLabel = new QLabel("Мои штрафы");
    titleLabel->setStyleSheet(
        "QLabel {"
        "   font-size: 28px;"
        "   font-weight: bold;"
        "   color: #2c3e50;"
        "   padding-bottom: 10px;"
        "   border-bottom: 2px solid #4169E1;"
        "}"
    );
    finesLayout->addWidget(titleLabel);

    QWidget *searchPanel = new QWidget();
    QHBoxLayout *searchLayout = new QHBoxLayout(searchPanel);
    searchLayout->setContentsMargins(0, 0, 0, 0);

    searchFinesEdit = new QLineEdit();
    searchFinesEdit->setPlaceholderText("Поиск по причине...");
    searchFinesEdit->setMinimumHeight(40);
    searchFinesEdit->setStyleSheet(
        "QLineEdit {"
        "   padding: 8px 15px;"
        "   border: 2px solid #ddd;"
        "   border-radius: 5px;"
        "   font-size: 14px;"
        "}"
        "QLineEdit:focus {"
        "   border: 2px solid #4169E1;"
        "}"
    );

    fineFilterComboBox = new QComboBox();
    fineFilterComboBox->addItems({"Все статусы", "Не оплаченные", "Оплаченные"});
    fineFilterComboBox->setMinimumHeight(40);
    fineFilterComboBox->setStyleSheet(
        "QComboBox {"
        "   padding: 8px 15px;"
        "   border: 2px solid #ddd;"
        "   border-radius: 5px;"
        "   font-size: 14px;"
        "   min-width: 150px;"
        "}"
        "QComboBox:focus {"
        "   border: 2px solid #4169E1;"
        "}"
    );
    connect(fineFilterComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ClientWindow::refreshMyFines);

    QPushButton *refreshFinesButton = new QPushButton("Обновить");
    refreshFinesButton->setMinimumHeight(40);
    refreshFinesButton->setStyleSheet(
        "QPushButton {"
        "   padding: 8px 20px;"
        "   border: 1px solid #ddd;"
        "   border-radius: 5px;"
        "   background-color: white;"
        "   font-weight: bold;"
        "}"
        "QPushButton:hover { background-color: #f8f9fa; }"
    );
    connect(refreshFinesButton, &QPushButton::clicked, this, &ClientWindow::refreshMyFines);

    searchLayout->addWidget(searchFinesEdit);
    searchLayout->addWidget(fineFilterComboBox);
    searchLayout->addWidget(refreshFinesButton);
    searchLayout->addStretch();

    finesLayout->addWidget(searchPanel);

    finesScrollArea = new QScrollArea();
    finesScrollArea->setWidgetResizable(true);
    finesScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    finesScrollArea->setStyleSheet("QScrollArea { border: none; background: transparent; }");

    finesContainer = new QWidget();
    finesContainerLayout = new QVBoxLayout(finesContainer);
    finesContainerLayout->setContentsMargins(0, 0, 0, 0);
    finesContainerLayout->setSpacing(15);
    finesContainerLayout->setAlignment(Qt::AlignTop);

    finesScrollArea->setWidget(finesContainer);
    finesLayout->addWidget(finesScrollArea, 1);

    mainTabWidget->addTab(finesTab, "Мои штрафы");
}

QWidget* ClientWindow::createFineCard(int fineId, int rentalId, const QString &reason,
                                     double amount, const QDate &issueDate, bool paid) {
    QWidget *card = new QWidget();
    card->setObjectName(QString("fineCard_%1").arg(fineId));
    card->setMinimumHeight(120);
    card->setStyleSheet(
        "QWidget {"
        "   background-color: white;"
        "   border: 2px solid #e0e0e0;"
        "   border-radius: 10px;"
        "   padding: 15px;"
        "}"
        "QWidget:hover {"
        "   border: 2px solid #4169E1;"
        "   background-color: #f8f9ff;"
        "}"
    );

    QVBoxLayout *cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(0, 0, 0, 0);
    cardLayout->setSpacing(10);

    QHBoxLayout *topLayout = new QHBoxLayout();

    QLabel *reasonLabel = new QLabel(reason);
    reasonLabel->setStyleSheet(
        "QLabel {"
        "   font-size: 18px;"
        "   font-weight: bold;"
        "   color: #2c3e50;"
        "}"
    );
    reasonLabel->setWordWrap(true);

    QString status = paid ? "Оплачен" : "Не оплачен";
    QLabel *statusLabel = new QLabel(status);
    QString statusColor = paid ? "#28a745" : "#dc3545";

    statusLabel->setStyleSheet(
        QString(
            "QLabel {"
            "   color: white;"
            "   background-color: %1;"
            "   padding: 5px 12px;"
            "   border-radius: 12px;"
            "   font-weight: bold;"
            "   font-size: 12px;"
            "}"
        ).arg(statusColor)
    );

    topLayout->addWidget(reasonLabel, 4);
    topLayout->addWidget(statusLabel, 1);

    cardLayout->addLayout(topLayout);

    QWidget *infoWidget = new QWidget();
    QHBoxLayout *infoLayout = new QHBoxLayout(infoWidget);
    infoLayout->setContentsMargins(0, 0, 0, 0);

    QLabel *rentalLabel = new QLabel(QString("ID аренды: #%1").arg(rentalId));
    QLabel *dateLabel = new QLabel(QString("Дата: %1").arg(issueDate.toString("dd.MM.yyyy")));

    rentalLabel->setStyleSheet("color: #666; font-size: 14px;");
    dateLabel->setStyleSheet("color: #666; font-size: 14px;");

    infoLayout->addWidget(rentalLabel);
    infoLayout->addWidget(dateLabel);
    infoLayout->addStretch();

    cardLayout->addWidget(infoWidget);

    QHBoxLayout *bottomLayout = new QHBoxLayout();

    QLabel *amountLabel = new QLabel(QString(" %1 руб.").arg(amount, 0, 'f', 2));
    amountLabel->setStyleSheet(
        "QLabel {"
        "   font-size: 16px;"
        "   font-weight: bold;"
        "   color: #dc3545;"
        "}"
    );

    QPushButton *infoButton = new QPushButton("Детали");
    infoButton->setObjectName(QString("fine_details_%1").arg(fineId));
    infoButton->setStyleSheet(
        "QPushButton {"
        "   padding: 5px 15px;"
        "   border: 1px solid #4169E1;"
        "   border-radius: 5px;"
        "   background-color: white;"
        "   color: #4169E1;"
        "   font-size: 12px;"
        "}"
        "QPushButton:hover { background-color: #f0f5ff; }"
    );
    connect(infoButton, &QPushButton::clicked, this, [this, fineId]() {
        showFineDetails(fineId);
    });

    QPushButton *payButton = new QPushButton("Оплатить");
    payButton->setObjectName(QString("fine_pay_%1").arg(fineId));
    payButton->setStyleSheet(
        QString(
            "QPushButton {"
            "   padding: 5px 15px;"
            "   border: 1px solid %1;"
            "   border-radius: 5px;"
            "   background-color: white;"
            "   color: %1;"
            "   font-size: 12px;"
            "}"
            "QPushButton:hover { background-color: %2; }"
            "QPushButton:disabled { opacity: 0.6; }"
        ).arg("#28a745").arg("#d4edda")
    );
    payButton->setEnabled(!paid);
    connect(payButton, &QPushButton::clicked, this, [this, fineId]() {
        payFine(fineId);
    });

    bottomLayout->addWidget(amountLabel);
    bottomLayout->addStretch();
    bottomLayout->addWidget(infoButton);
    bottomLayout->addWidget(payButton);

    cardLayout->addLayout(bottomLayout);

    return card;
}

void ClientWindow::setupUI() {
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(10);

    mainTabWidget = new QTabWidget();
    mainTabWidget->setStyleSheet(
        "QTabWidget::pane {"
        "   border: 1px solid #ddd;"
        "   border-radius: 5px;"
        "   background: white;"
        "}"
        "QTabBar::tab {"
        "   background: #f0f0f0;"
        "   padding: 10px 20px;"
        "   margin-right: 2px;"
        "   border-top-left-radius: 5px;"
        "   border-top-right-radius: 5px;"
        "}"
        "QTabBar::tab:selected {"
        "   background: white;"
        "   border-bottom: 3px solid #4169E1;"
        "}"
    );

    setupAvailableCarsTab();
    setupMyRentalsTab();
    setupMyFinesTab();
    setupProfileTab();

    mainLayout->addWidget(mainTabWidget);

    statusBar()->showMessage("Добро пожаловать!");
}

void ClientWindow::setupProfileTab() {
    QWidget *profileTab = new QWidget();
    profileTab->setStyleSheet("background-color: #f8f9fa;");

    QHBoxLayout *mainLayout = new QHBoxLayout(profileTab);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(30);

    QWidget *leftColumn = new QWidget();
    leftColumn->setFixedWidth(350);
    leftColumn->setStyleSheet("background-color: white; border-radius: 10px; padding: 20px;");

    QVBoxLayout *leftLayout = new QVBoxLayout(leftColumn);
    leftLayout->setContentsMargins(0, 0, 0, 0);
    leftLayout->setSpacing(20);

    QLabel *profileTitle = new QLabel("Профиль");
    profileTitle->setStyleSheet(
        "QLabel {"
        "   font-size: 28px;"
        "   font-weight: bold;"
        "   color: #2c3e50;"
        "   padding-bottom: 10px;"
        "   border-bottom: 2px solid #4169E1;"
        "}"
    );
    profileTitle->setAlignment(Qt::AlignCenter);
    leftLayout->addWidget(profileTitle);

//    QLabel *avatarLabel = new QLabel();
//    avatarLabel->setPixmap(QPixmap(":/icons/avatar.png").scaled(100, 100, Qt::KeepAspectRatio, Qt::SmoothTransformation));
//    avatarLabel->setAlignment(Qt::AlignCenter);
//    avatarLabel->setStyleSheet("QLabel { border: 3px solid #4169E1; border-radius: 50px; padding: 5px; }");
//    leftLayout->addWidget(avatarLabel);

    profileNameLabel = new QLabel();
    profileNameLabel->setStyleSheet(
        "QLabel {"
        "   font-size: 22px;"
        "   font-weight: bold;"
        "   color: #2c3e50;"
        "   text-align: center;"
        "}"
    );
    profileNameLabel->setAlignment(Qt::AlignCenter);
    leftLayout->addWidget(profileNameLabel);

    QFrame *separator1 = new QFrame();
    separator1->setFrameShape(QFrame::HLine);
    separator1->setFrameShadow(QFrame::Sunken);
    separator1->setStyleSheet("background-color: #ddd; margin: 10px 0;");
    leftLayout->addWidget(separator1);

    QWidget *contactWidget = new QWidget();
    QVBoxLayout *contactLayout = new QVBoxLayout(contactWidget);
    contactLayout->setContentsMargins(0, 0, 0, 0);
    contactLayout->setSpacing(15);

    QWidget *phoneWidget = new QWidget();
    QHBoxLayout *phoneLayout = new QHBoxLayout(phoneWidget);
    phoneLayout->setContentsMargins(0, 0, 0, 0);

    QLabel *phoneIcon = new QLabel("Телефон: ");
    phoneIcon->setStyleSheet("font-size: 18px;");

    profilePhoneLabel = new QLabel();
    profilePhoneLabel->setStyleSheet("font-size: 16px; color: #333;");

    phoneLayout->addWidget(phoneIcon);
    phoneLayout->addWidget(profilePhoneLabel);
    phoneLayout->addStretch();

    contactLayout->addWidget(phoneWidget);

    QFrame *separatorContact = new QFrame();
    separatorContact->setFrameShape(QFrame::HLine);
    separatorContact->setFrameShadow(QFrame::Sunken);
    separatorContact->setStyleSheet("background-color: #eee;");
    contactLayout->addWidget(separatorContact);

    QWidget *emailWidget = new QWidget();
    QHBoxLayout *emailLayout = new QHBoxLayout(emailWidget);
    emailLayout->setContentsMargins(0, 0, 0, 0);

    QLabel *emailIcon = new QLabel("Почта: ");
    emailIcon->setStyleSheet("font-size: 18px;");

    profileEmailLabel = new QLabel();
    profileEmailLabel->setStyleSheet("font-size: 16px; color: #333;");

    emailLayout->addWidget(emailIcon);
    emailLayout->addWidget(profileEmailLabel);
    emailLayout->addStretch();

    contactLayout->addWidget(emailWidget);

    QFrame *separatorContact2 = new QFrame();
    separatorContact2->setFrameShape(QFrame::HLine);
    separatorContact2->setFrameShadow(QFrame::Sunken);
    separatorContact2->setStyleSheet("background-color: #eee;");
    contactLayout->addWidget(separatorContact2);

//    QWidget *licenseWidget = new QWidget();
//    QHBoxLayout *licenseLayout = new QHBoxLayout(licenseWidget);
//    licenseLayout->setContentsMargins(0, 0, 0, 0);

//    QLabel *licenseIcon = new QLabel("Водительское: ");
//    licenseIcon->setStyleSheet("font-size: 18px;");

//    profileLicenseLabel = new QLabel();
//    profileLicenseLabel->setStyleSheet("font-size: 16px; color: #333;");

//    licenseLayout->addWidget(licenseIcon);
//    licenseLayout->addWidget(profileLicenseLabel);
//    licenseLayout->addStretch();

//    contactLayout->addWidget(licenseWidget);

    leftLayout->addWidget(contactWidget);

    QFrame *separator2 = new QFrame();
    separator2->setFrameShape(QFrame::HLine);
    separator2->setFrameShadow(QFrame::Sunken);
    separator2->setStyleSheet("background-color: #4169E1; margin: 15px 0; height: 2px;");
    leftLayout->addWidget(separator2);

    QWidget *statsWidget = new QWidget();
    statsWidget->setStyleSheet("background-color: #f0f7ff; border-radius: 10px; padding: 15px;");

    QVBoxLayout *statsLayout = new QVBoxLayout(statsWidget);
    statsLayout->setSpacing(12);
    statsLayout->setContentsMargins(10, 10, 10, 10);

    QLabel *statsTitle = new QLabel("Ваша статистика");
    statsTitle->setStyleSheet("font-weight: bold; font-size: 18px; color: #2c3e50;");
    statsTitle->setAlignment(Qt::AlignCenter);
    statsLayout->addWidget(statsTitle);

    QWidget *tripsWidget = new QWidget();
    QHBoxLayout *tripsLayout = new QHBoxLayout(tripsWidget);
    tripsLayout->setContentsMargins(0, 0, 0, 0);

    profileTripsLabel = new QLabel("Поездок: 0");
    profileTripsLabel->setStyleSheet("font-size: 16px; color: #333; font-weight: bold;");

    tripsLayout->addWidget(profileTripsLabel);
    tripsLayout->addStretch();

    statsLayout->addWidget(tripsWidget);

    QWidget *ratingWidget = new QWidget();
    QHBoxLayout *ratingLayout = new QHBoxLayout(ratingWidget);
    ratingLayout->setContentsMargins(0, 0, 0, 0);

    profileRatingLabel = new QLabel("Рейтинг: ★★★★☆");
    profileRatingLabel->setStyleSheet("font-size: 16px; color: #333; font-weight: bold;");

    ratingLayout->addWidget(profileRatingLabel);
    ratingLayout->addStretch();

    statsLayout->addWidget(ratingWidget);

    QWidget *infoWidget = new QWidget();
    QVBoxLayout *infoLayout = new QVBoxLayout(infoWidget);
    infoLayout->setContentsMargins(0, 5, 0, 0);
    infoLayout->setSpacing(5);

    QLabel *memberSinceLabel = new QLabel("В системе с: 2024 г.");
    QLabel *reliabilityLabel = new QLabel("Надежный клиент");

    memberSinceLabel->setStyleSheet("font-size: 13px; color: #666; font-style: italic;");
    reliabilityLabel->setStyleSheet("font-size: 13px; color: #28a745;");

    infoLayout->addWidget(memberSinceLabel);
    infoLayout->addWidget(reliabilityLabel);

    statsLayout->addWidget(infoWidget);

    leftLayout->addWidget(statsWidget);
    leftLayout->addStretch();

    mainLayout->addWidget(leftColumn);

    QWidget *rightColumn = new QWidget();
    rightColumn->setStyleSheet("background-color: white; border-radius: 10px; padding: 20px;");

    QVBoxLayout *rightLayout = new QVBoxLayout(rightColumn);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(20);

    QLabel *historyTitle = new QLabel("История аренд");
    historyTitle->setStyleSheet(
        "QLabel {"
        "   font-size: 24px;"
        "   font-weight: bold;"
        "   color: #2c3e50;"
        "   padding-bottom: 10px;"
        "   border-bottom: 2px solid #4169E1;"
        "}"
    );
    rightLayout->addWidget(historyTitle);

    QTableWidget *historyTable = new QTableWidget();
    historyTable->setColumnCount(3);
    historyTable->setHorizontalHeaderLabels({"Дата", "Автомобиль", "Стоимость"});
    historyTable->horizontalHeader()->setStretchLastSection(true);
    historyTable->setAlternatingRowColors(true);
    historyTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    historyTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    historyTable->setStyleSheet(
        "QTableWidget {"
        "   border: 1px solid #ddd;"
        "   border-radius: 5px;"
        "   background-color: white;"
        "}"
        "QHeaderView::section {"
        "   background-color: #f8f9fa;"
        "   padding: 12px;"
        "   border: none;"
        "   font-weight: bold;"
        "   color: #495057;"
        "}"
        "QTableWidget::item {"
        "   padding: 10px;"
        "   border-bottom: 1px solid #eee;"
        "}"
        "QTableWidget::item:alternate {"
        "   background-color: #f8f9fa;"
        "}"
    );

    historyTable->setColumnWidth(0, 120);
    historyTable->setColumnWidth(1, 200);

    refreshHistoryTable(historyTable);

    rightLayout->addWidget(historyTable);

    QWidget *actionsWidget = new QWidget();
    QHBoxLayout *actionsLayout = new QHBoxLayout(actionsWidget);
    actionsLayout->setContentsMargins(0, 0, 0, 0);

    QPushButton *refreshHistoryButton = new QPushButton("Обновить историю");
    refreshHistoryButton->setStyleSheet(
        "QPushButton {"
        "   padding: 10px 20px;"
        "   border: 1px solid #ddd;"
        "   border-radius: 5px;"
        "   background-color: white;"
        "   font-weight: bold;"
        "}"
        "QPushButton:hover { background-color: #f8f9fa; }"
    );
    connect(refreshHistoryButton, &QPushButton::clicked, this, [this, historyTable]() {
        refreshHistoryTable(historyTable);
    });

//    QPushButton *exportHistoryButton = new QPushButton("📥 Экспорт в PDF");
//    exportHistoryButton->setStyleSheet(
//        "QPushButton {"
//        "   padding: 10px 20px;"
//        "   border: none;"
//        "   border-radius: 5px;"
//        "   background-color: #28a745;"
//        "   color: white;"
//        "   font-weight: bold;"
//        "}"
//        "QPushButton:hover { background-color: #218838; }"
//    );
//    connect(exportHistoryButton, &QPushButton::clicked, this, [this]() {
//        QMessageBox::information(this, "Экспорт", "Функция экспорта в PDF в разработке");
//    });

    actionsLayout->addWidget(refreshHistoryButton);
    actionsLayout->addStretch();
    //actionsLayout->addWidget(exportHistoryButton);

    rightLayout->addWidget(actionsWidget);
    rightLayout->addStretch();

    mainLayout->addWidget(rightColumn, 1);

    mainTabWidget->addTab(profileTab, "Мой профиль");
}

void ClientWindow::setupAvailableCarsTab() {
    QWidget *carsTab = new QWidget();
    QVBoxLayout *mainCarsLayout = new QVBoxLayout(carsTab);
    mainCarsLayout->setContentsMargins(10, 10, 10, 10);
    mainCarsLayout->setSpacing(20);

    QWidget *headerRow = new QWidget();
    QHBoxLayout *headerLayout = new QHBoxLayout(headerRow);
    headerLayout->setContentsMargins(0, 0, 0, 0);
    headerLayout->setSpacing(20);

    // ЛЕВАЯ КОЛОНКА: Фильтр по маркам
    QWidget *leftColumn = new QWidget();
    leftColumn->setFixedWidth(250);
    QVBoxLayout *leftLayout = new QVBoxLayout(leftColumn); // Изменил на QVBoxLayout
    leftLayout->setContentsMargins(0, 0, 0, 0);

    QWidget *brandHeaderWidget = new QWidget();
    QHBoxLayout *brandHeaderLayout = new QHBoxLayout(brandHeaderWidget);
    brandHeaderLayout->setContentsMargins(0, 0, 0, 0);

    QLabel *brandsTitle = new QLabel("Марки");
    brandsTitle->setStyleSheet(
        "QLabel {"
        "   font-size: 24px;"
        "   font-weight: bold;"
        "   color: #333;"
        "}"
    );

    QPushButton *allBrandsButton = new QPushButton("Все");
    allBrandsButton->setFixedSize(60, 30);
    allBrandsButton->setStyleSheet(
        "QPushButton {"
        "   background-color: #4169E1;"
        "   color: white;"
        "   border: none;"
        "   border-radius: 5px;"
        "   font-size: 12px;"
        "   font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "   background-color: #3159D1;"
        "}"
    );
    connect(allBrandsButton, &QPushButton::clicked, this, [this]() {
        showAllBrandsView();
    });

    brandHeaderLayout->addWidget(brandsTitle);
    brandHeaderLayout->addStretch();
    brandHeaderLayout->addWidget(allBrandsButton);

    leftLayout->addWidget(brandHeaderWidget);

    QFrame *brandSeparator = new QFrame();
    brandSeparator->setFrameShape(QFrame::HLine);
    brandSeparator->setFrameShadow(QFrame::Sunken);
    brandSeparator->setStyleSheet("background-color: #4169E1; height: 2px;");
    brandSeparator->setFixedHeight(2);
    leftLayout->addWidget(brandSeparator);

    brandFilterWidget = new QWidget();
    brandFilterLayout = new QVBoxLayout(brandFilterWidget);
    brandFilterLayout->setContentsMargins(0, 10, 0, 10);
    brandFilterLayout->setSpacing(5);

    QRadioButton *allFilterButton = new QRadioButton("Все марки");
    allFilterButton->setChecked(true);
    allFilterButton->setStyleSheet(
        "QRadioButton {"
        "   font-size: 14px;"
        "   padding: 8px;"
        "}"
        "QRadioButton::indicator {"
        "   width: 16px;"
        "   height: 16px;"
        "}"
    );
    connect(allFilterButton, &QRadioButton::clicked, this, &ClientWindow::showAllBrands);

    brandFilterLayout->addWidget(allFilterButton);
    brandFilterLayout->addSpacing(10);

    brandButtonGroup = new QButtonGroup(this);
    brandButtonGroup->addButton(allFilterButton);

    leftLayout->addWidget(brandFilterWidget);
    leftLayout->addStretch();

    headerLayout->addWidget(leftColumn);

    QWidget *rightColumn = new QWidget();
    QVBoxLayout *rightLayout = new QVBoxLayout(rightColumn);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(15);

    QLabel *popularTitle = new QLabel("Популярные машины");
    popularTitle->setStyleSheet(
        "QLabel {"
        "   font-size: 24px;"
        "   font-weight: bold;"
        "   color: #333;"
        "   padding-bottom: 10px;"
        "}"
    );
    rightLayout->addWidget(popularTitle);

    QWidget *searchPanel = new QWidget();
    QHBoxLayout *searchLayout = new QHBoxLayout(searchPanel);
    searchLayout->setContentsMargins(0, 0, 0, 0);

    searchEdit = new QLineEdit();
    searchEdit->setPlaceholderText("Поиск по модели...");
    searchEdit->setMinimumHeight(40);
    searchEdit->setStyleSheet(
        "QLineEdit {"
        "   padding: 8px 15px;"
        "   border: 2px solid #ddd;"
        "   border-radius: 5px;"
        "   font-size: 14px;"
        "}"
        "QLineEdit:focus {"
        "   border: 2px solid #4169E1;"
        "}"
    );
    connect(searchEdit, &QLineEdit::textChanged, this, &ClientWindow::searchAvailableCars);

    searchLayout->addWidget(searchEdit);
    searchLayout->addStretch();

    rightLayout->addWidget(searchPanel);

    carsScrollArea = new QScrollArea();
    carsScrollArea->setWidgetResizable(true);
    carsScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    carsScrollArea->setStyleSheet("QScrollArea { border: none; background: transparent; }");

    carsContainer = new QWidget();
    carsContainerLayout = new QVBoxLayout(carsContainer);
    carsContainerLayout->setContentsMargins(0, 0, 0, 0);
    carsContainerLayout->setSpacing(15);
    carsContainerLayout->setAlignment(Qt::AlignTop);

    carsScrollArea->setWidget(carsContainer);
    rightLayout->addWidget(carsScrollArea, 1);

    headerLayout->addWidget(rightColumn, 1);

    mainCarsLayout->addWidget(headerRow, 1);

    mainTabWidget->addTab(carsTab, "Доступные автомобили");
}

void ClientWindow::setupMyRentalsTab() {
    QWidget *rentalsTab = new QWidget();
    QVBoxLayout *rentalsLayout = new QVBoxLayout(rentalsTab);
    rentalsLayout->setContentsMargins(20, 20, 20, 20);
    rentalsLayout->setSpacing(20);

    QLabel *titleLabel = new QLabel("Мои аренды");
    titleLabel->setStyleSheet(
        "QLabel {"
        "   font-size: 28px;"
        "   font-weight: bold;"
        "   color: #2c3e50;"
        "   padding-bottom: 10px;"
        "   border-bottom: 2px solid #4169E1;"
        "}"
    );
    rentalsLayout->addWidget(titleLabel);

    QWidget *searchPanel = new QWidget();
    QHBoxLayout *searchLayout = new QHBoxLayout(searchPanel);
    searchLayout->setContentsMargins(0, 0, 0, 0);

    searchRentalsEdit = new QLineEdit();
    searchRentalsEdit->setPlaceholderText("Поиск по машине или статусу...");
    searchRentalsEdit->setMinimumHeight(40);
    searchRentalsEdit->setStyleSheet(
        "QLineEdit {"
        "   padding: 8px 15px;"
        "   border: 2px solid #ddd;"
        "   border-radius: 5px;"
        "   font-size: 14px;"
        "}"
        "QLineEdit:focus {"
        "   border: 2px solid #4169E1;"
        "}"
    );

    rentalFilterComboBox = new QComboBox();
    rentalFilterComboBox->addItems({"Все статусы", "Активные", "Завершенные", "Отмененные", "Просроченные"});
    rentalFilterComboBox->setMinimumHeight(40);
    rentalFilterComboBox->setStyleSheet(
        "QComboBox {"
        "   padding: 8px 15px;"
        "   border: 2px solid #ddd;"
        "   border-radius: 5px;"
        "   font-size: 14px;"
        "   min-width: 150px;"
        "}"
        "QComboBox:focus {"
        "   border: 2px solid #4169E1;"
        "}"
    );
    connect(rentalFilterComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ClientWindow::refreshMyRentals);

    QPushButton *refreshRentalsButton = new QPushButton("Обновить");
    refreshRentalsButton->setMinimumHeight(40);
    refreshRentalsButton->setStyleSheet(
        "QPushButton {"
        "   padding: 8px 20px;"
        "   border: 1px solid #ddd;"
        "   border-radius: 5px;"
        "   background-color: white;"
        "   font-weight: bold;"
        "}"
        "QPushButton:hover { background-color: #f8f9fa; }"
    );
    connect(refreshRentalsButton, &QPushButton::clicked, this, &ClientWindow::refreshMyRentals);

    searchLayout->addWidget(searchRentalsEdit);
    searchLayout->addWidget(rentalFilterComboBox);
    searchLayout->addWidget(refreshRentalsButton);
    searchLayout->addStretch();

    rentalsLayout->addWidget(searchPanel);

    rentalsScrollArea = new QScrollArea();
    rentalsScrollArea->setWidgetResizable(true);
    rentalsScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    rentalsScrollArea->setStyleSheet("QScrollArea { border: none; background: transparent; }");

    rentalsContainer = new QWidget();
    rentalsContainerLayout = new QVBoxLayout(rentalsContainer);
    rentalsContainerLayout->setContentsMargins(0, 0, 0, 0);
    rentalsContainerLayout->setSpacing(15);
    rentalsContainerLayout->setAlignment(Qt::AlignTop);

    rentalsScrollArea->setWidget(rentalsContainer);
    rentalsLayout->addWidget(rentalsScrollArea, 1);

    mainTabWidget->addTab(rentalsTab, "Мои аренды");
}

void ClientWindow::extendRental(int rentalId) {
    if (rentalId == -1) {
        QMessageBox::warning(this, "Ошибка", "Аренда не найдена");
        return;
    }

    RentalManager* rentalManager = RentalManager::instance();
    if (!rentalManager) {
        QMessageBox::critical(this, "Ошибка", "Менеджер аренд не доступен");
        return;
    }

    Rental* rental = rentalManager->findRentalById(rentalId);
    if (!rental) {
        QMessageBox::warning(this, "Ошибка", "Аренда не найдена");
        return;
    }

    if (rental->status() != Rental::ACTIVE && rental->status() != Rental::OVERDUE) {
        QMessageBox::warning(this, "Ошибка",
            "Продлить можно только активные или просроченные аренды");
        return;
    }

    QDate oldEndDate = rental->endDate();
    double oldTotalPrice = rental->totalPrice();

    QDialog dialog(this);
    dialog.setWindowTitle("Продление аренды");
    dialog.setFixedSize(400, 320);
    dialog.setStyleSheet(
        "QDialog { background-color: white; }"
        "QLabel { font-size: 14px; }"
        "QPushButton {"
        "   padding: 8px 15px;"
        "   border-radius: 5px;"
        "   font-weight: bold;"
        "}"
    );

    QVBoxLayout *layout = new QVBoxLayout(&dialog);
    layout->setSpacing(15);

    QLabel *titleLabel = new QLabel("Продление аренды");
    titleLabel->setStyleSheet("font-weight: bold; font-size: 18px; color: #2c3e50; text-align: center;");
    layout->addWidget(titleLabel);

    Car* car = CarManager::instance()->getCarById(rental->carId());
    if (car) {
        QLabel *carInfoLabel = new QLabel(QString("Автомобиль: %1 %2").arg(car->brand(), car->model()));
        carInfoLabel->setStyleSheet("font-weight: bold; color: #333;");
        layout->addWidget(carInfoLabel);
    }

    QLabel *currentDatesLabel = new QLabel(
        QString("Текущий период: %1 - %2")
            .arg(rental->startDate().toString("dd.MM.yyyy"))
            .arg(oldEndDate.toString("dd.MM.yyyy")));
    layout->addWidget(currentDatesLabel);

    QLabel *currentPriceLabel = new QLabel(
        QString("Текущая стоимость: %1 руб.").arg(oldTotalPrice, 0, 'f', 2));
    layout->addWidget(currentPriceLabel);

    QFrame *separator = new QFrame();
    separator->setFrameShape(QFrame::HLine);
    separator->setStyleSheet("background-color: #ddd; margin: 10px 0;");
    layout->addWidget(separator);

    QLabel *newDateLabel = new QLabel("Новая дата окончания:");
    newDateLabel->setStyleSheet("font-weight: bold;");
    layout->addWidget(newDateLabel);

    QDateEdit *newEndDateEdit = new QDateEdit();
    newEndDateEdit->setDate(oldEndDate.addDays(1));
    newEndDateEdit->setMinimumDate(oldEndDate.addDays(1));
    newEndDateEdit->setMaximumDate(oldEndDate.addDays(30));
    newEndDateEdit->setCalendarPopup(true);
    newEndDateEdit->setDisplayFormat("dd.MM.yyyy");
    newEndDateEdit->setStyleSheet("QDateEdit { padding: 8px; border: 1px solid #ddd; border-radius: 3px; }");
    layout->addWidget(newEndDateEdit);

    QLabel *extensionInfoLabel = new QLabel();
    extensionInfoLabel->setStyleSheet("color: #666; font-size: 12px; font-style: italic;");
    layout->addWidget(extensionInfoLabel);

    QLabel *newPriceLabel = new QLabel();
    newPriceLabel->setStyleSheet("font-weight: bold; color: #27ae60; font-size: 14px;");
    layout->addWidget(newPriceLabel);

    auto updateExtensionInfo = [&]() {
        QDate newEndDate = newEndDateEdit->date();
        int extensionDays = oldEndDate.daysTo(newEndDate);

        if (extensionDays > 0) {
            double dailyPrice = oldTotalPrice / rental->rentalDays();
            double extensionCost = dailyPrice * extensionDays;
            double newTotalPrice = oldTotalPrice + extensionCost;

            extensionInfoLabel->setText(
                QString("Дополнительных дней: %1 | Стоимость продления: %2 руб.")
                    .arg(extensionDays)
                    .arg(extensionCost, 0, 'f', 2)
            );

            newPriceLabel->setText(
                QString("Новая общая стоимость: %1 руб.").arg(newTotalPrice, 0, 'f', 2)
            );
        } else {
            extensionInfoLabel->setText("Выберите дату после текущей даты окончания");
            newPriceLabel->clear();
        }
    };

    connect(newEndDateEdit, &QDateEdit::dateChanged, this, updateExtensionInfo);
    updateExtensionInfo();

    layout->addStretch();

    QHBoxLayout *buttonLayout = new QHBoxLayout();

    QPushButton *cancelButton = new QPushButton("Отмена");
    cancelButton->setStyleSheet(
        "QPushButton {"
        "   background-color: #dc3545;"
        "   color: white;"
        "}"
        "QPushButton:hover { background-color: #c82333; }"
    );

    QPushButton *confirmButton = new QPushButton("Продлить аренду");
    confirmButton->setStyleSheet(
        "QPushButton {"
        "   background-color: #28a745;"
        "   color: white;"
        "}"
        "QPushButton:hover { background-color: #218838; }"
        "QPushButton:disabled { background-color: #6c757d; }"
    );

    confirmButton->setEnabled(newEndDateEdit->date() > oldEndDate);

    connect(newEndDateEdit, &QDateEdit::dateChanged, this, [=]() {
        confirmButton->setEnabled(newEndDateEdit->date() > oldEndDate);
    });

    buttonLayout->addStretch();
    buttonLayout->addWidget(cancelButton);
    buttonLayout->addWidget(confirmButton);

    layout->addLayout(buttonLayout);

    connect(cancelButton, &QPushButton::clicked, &dialog, &QDialog::reject);
    connect(confirmButton, &QPushButton::clicked, &dialog, &QDialog::accept);

    if (dialog.exec() == QDialog::Rejected) {
        return;
    }

    QDate newEndDate = newEndDateEdit->date();
    double extensionCost = rentalManager->calculateExtensionCost(rentalId, newEndDate);
    double newTotalPrice = oldTotalPrice + extensionCost;

    int result = QMessageBox::question(this, "Подтверждение продления",
        QString("Вы уверены, что хотите продлить аренду до %1?\n\n"
               "Дополнительных дней: %2\n"
               "Стоимость продления: %3 руб.\n"
               "Новая общая стоимость: %4 руб.")
            .arg(newEndDate.toString("dd.MM.yyyy"))
            .arg(oldEndDate.daysTo(newEndDate))
            .arg(extensionCost, 0, 'f', 2)
            .arg(newTotalPrice, 0, 'f', 2),
        QMessageBox::Yes | QMessageBox::No);

    if (result == QMessageBox::Yes) {
        if (rentalManager->extendRental(rentalId, newEndDate)) {
            QMessageBox::information(this, "Успех",
                QString("Аренда успешно продлена до %1.\n"
                       "Новая общая стоимость: %2 руб.")
                    .arg(newEndDate.toString("dd.MM.yyyy"))
                    .arg(newTotalPrice, 0, 'f', 2));

            updateRentalCard(rentalId, newEndDate, newTotalPrice);

            statusBar()->showMessage("Аренда продлена", 3000);
        } else {
            QMessageBox::critical(this, "Ошибка",
                "Не удалось продлить аренду. Возможно, автомобиль недоступен на выбранные даты.");
        }
    }
}

void ClientWindow::updateRentalCard(int rentalId, const QDate &newEndDate, double newTotalPrice) {
    for (QWidget *card : rentalCards) {
        QPushButton *detailsButton = card->findChild<QPushButton*>(QString("details_%1").arg(rentalId));
        if (detailsButton) {

            QLayout *cardLayout = card->layout();
            if (cardLayout) {
                QWidget *specsWidget = nullptr;
                for (int i = 0; i < cardLayout->count(); i++) {
                    QLayoutItem *item = cardLayout->itemAt(i);
                    if (item && item->widget() && item->widget()->objectName().contains("specs")) {
                        specsWidget = item->widget();
                        break;
                    }
                }

                if (specsWidget) {
                    QLabel *datesLabel = specsWidget->findChild<QLabel*>();
                    if (datesLabel) {
                        QString currentText = datesLabel->text();
                        QStringList parts = currentText.split(" - ");
                        if (parts.size() == 2) {
                            QString startDateStr = parts[0].replace("📅 ", "");
                            datesLabel->setText(QString("📅 %1 - %2")
                                .arg(startDateStr)
                                .arg(newEndDate.toString("dd.MM.yyyy")));
                        }
                    }
                }

                QWidget *bottomWidget = nullptr;
                for (int i = 0; i < cardLayout->count(); i++) {
                    QLayoutItem *item = cardLayout->itemAt(i);
                    if (item && item->widget()) {
                        QWidget *widget = item->widget();
                        QLabel *priceLabel = widget->findChild<QLabel*>();
                        if (priceLabel && priceLabel->text().contains("руб.")) {
                            priceLabel->setText(QString("%1 руб.").arg(newTotalPrice, 0, 'f', 2));
                            break;
                        }
                    }
                }
            }
            break;
        }
    }
}

QWidget* ClientWindow::createRentalCard(int rentalId, const QString &carName,
                                       const QString &brand, const QString &model,
                                       const QDate &startDate, const QDate &endDate,
                                       const QString &status, double totalPrice,
                                       const QString &carColor, int carYear) {
    QWidget *card = new QWidget();
    card->setObjectName("rentalCard");
    card->setMinimumHeight(280);
    card->setStyleSheet(
        "QWidget#rentalCard {"
        "   background-color: white;"
        "   border: 2px solid #e0e0e0;"
        "   border-radius: 10px;"
        "}"
        "QWidget#rentalCard:hover {"
        "   border: 2px solid #4169E1;"
        "   background-color: #f8f9ff;"
        "}"
    );

    QVBoxLayout *cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(15, 15, 15, 15);
    cardLayout->setSpacing(10);

    QWidget *imageWidget = new QWidget();
    imageWidget->setFixedHeight(120);
    QHBoxLayout *imageLayout = new QHBoxLayout(imageWidget);
    imageLayout->setContentsMargins(0, 0, 0, 0);

    QLabel *carImageLabel = new QLabel();
    carImageLabel->setAlignment(Qt::AlignCenter);
    carImageLabel->setFixedHeight(110);
    setupCarPlaceholder(carImageLabel, brand, carColor);

    imageLayout->addWidget(carImageLabel);
    cardLayout->addWidget(imageWidget);

    QHBoxLayout *topInfoLayout = new QHBoxLayout();
    QLabel *brandLabel = new QLabel(brand);
    brandLabel->setStyleSheet(
        "QLabel {"
        "   font-size: 20px;"
        "   font-weight: bold;"
        "   color: #2c3e50;"
        "}"
    );

    QLabel *statusLabel = new QLabel(status);
    QString statusColor;
    if (status == "Активная") statusColor = "#28a745";
    else if (status == "Завершена") statusColor = "#6c757d";
    else if (status == "Отменена") statusColor = "#dc3545";
    else if (status == "Просрочена") statusColor = "#ffc107";
    else statusColor = "#17a2b8";

    statusLabel->setStyleSheet(
        QString(
            "QLabel {"
            "   color: white;"
            "   background-color: %1;"
            "   padding: 5px 12px;"
            "   border-radius: 12px;"
            "   font-weight: bold;"
            "   font-size: 12px;"
            "}"
        ).arg(statusColor)
    );

    topInfoLayout->addWidget(brandLabel);
    topInfoLayout->addStretch();
    topInfoLayout->addWidget(statusLabel);
    cardLayout->addLayout(topInfoLayout);

    QLabel *modelLabel = new QLabel(model);
    modelLabel->setStyleSheet(
        "QLabel {"
        "   font-size: 16px;"
        "   color: #34495e;"
        "   font-weight: bold;"
        "}"
    );
    cardLayout->addWidget(modelLabel);

    QWidget *specsWidget = new QWidget();
    QVBoxLayout *specsLayout = new QVBoxLayout(specsWidget);
    specsLayout->setContentsMargins(0, 0, 0, 0);
    specsLayout->setSpacing(5);

    QWidget *specsRow1 = new QWidget();
    QHBoxLayout *specsRow1Layout = new QHBoxLayout(specsRow1);
    specsRow1Layout->setContentsMargins(0, 0, 0, 0);
    QLabel *datesLabel = new QLabel(QString("📅 %1 - %2")
        .arg(startDate.toString("dd.MM.yyyy"))
        .arg(endDate.toString("dd.MM.yyyy")));
    specsRow1Layout->addWidget(datesLabel);
    specsRow1Layout->addStretch();
    specsLayout->addWidget(specsRow1);

    QWidget *specsRow2 = new QWidget();
    QHBoxLayout *specsRow2Layout = new QHBoxLayout(specsRow2);
    specsRow2Layout->setContentsMargins(0, 0, 0, 0);
    QLabel *colorLabel = new QLabel(carColor);
    QLabel *yearLabel = new QLabel(QString::number(carYear) + " г.");
    colorLabel->setStyleSheet("QLabel { color: #666; font-size: 13px; }");
    yearLabel->setStyleSheet("QLabel { color: #666; font-size: 13px; }");
    specsRow2Layout->addWidget(colorLabel);
    specsRow2Layout->addStretch();
    specsRow2Layout->addWidget(yearLabel);
    specsLayout->addWidget(specsRow2);

    cardLayout->addWidget(specsWidget);

    QWidget *bottomWidget = new QWidget();
    QHBoxLayout *bottomLayout = new QHBoxLayout(bottomWidget);
    bottomLayout->setContentsMargins(0, 0, 0, 0);

    QLabel *priceLabel = new QLabel(QString("%1 руб.").arg(totalPrice, 0, 'f', 2));
    priceLabel->setStyleSheet(
        "QLabel {"
        "   font-size: 18px;"
        "   font-weight: bold;"
        "   color: #27ae60;"
        "}"
    );

    QPushButton *completeButton = nullptr;
    if (status == "Активная" || status == "Просрочена") {
        completeButton = new QPushButton("Завершить");
        completeButton->setObjectName(QString("complete_%1").arg(rentalId));
        completeButton->setMinimumWidth(80);
        completeButton->setMinimumHeight(35);
        completeButton->setStyleSheet(
            "QPushButton {"
            "   border: 1px solid #20c997;"
            "   border-radius: 5px;"
            "   background-color: white;"
            "   color: #20c997;"
            "   font-size: 12px;"
            "   padding: 8px 12px;"
            "   font-weight: bold;"
            "}"
            "QPushButton:hover {"
            "   background-color: #20c997;"
            "   color: white;"
            "}"
        );
        connect(completeButton, &QPushButton::clicked, this, [this, rentalId]() {
            completeRental(rentalId);
        });
    } else {
        QPushButton *detailsButton = new QPushButton("Детали");
        detailsButton->setObjectName(QString("details_%1").arg(rentalId));
        detailsButton->setMinimumWidth(80);
        detailsButton->setMinimumHeight(35);
        detailsButton->setStyleSheet(
            "QPushButton {"
            "   border: 1px solid #4169E1;"
            "   border-radius: 5px;"
            "   background-color: white;"
            "   color: #4169E1;"
            "   font-size: 12px;"
            "   padding: 8px 12px;"
            "}"
            "QPushButton:hover { background-color: #f0f5ff; }"
        );
        connect(detailsButton, &QPushButton::clicked, this, [this, rentalId]() {
            showRentalDetails(rentalId);
        });
        completeButton = detailsButton;
    }

    QPushButton *extendButton = nullptr;
    if (status == "Активная" || status == "Просрочена") {
        extendButton = new QPushButton("Продлить");
        extendButton->setObjectName(QString("extend_%1").arg(rentalId));
        extendButton->setMinimumWidth(80);
        extendButton->setMinimumHeight(35);
        extendButton->setStyleSheet(
            "QPushButton {"
            "   border: 1px solid #ffc107;"
            "   border-radius: 5px;"
            "   background-color: white;"
            "   color: #ffc107;"
            "   font-size: 12px;"
            "   padding: 8px 12px;"
            "   font-weight: bold;"
            "}"
            "QPushButton:hover {"
            "   background-color: #ffc107;"
            "   color: white;"
            "}"
        );
        connect(extendButton, &QPushButton::clicked, this, [this, rentalId]() {
            extendRental(rentalId);
        });
    }

    QPushButton *cancelButton = nullptr;
    if (status == "Активная" || status == "Просрочена") {
        cancelButton = new QPushButton("Отменить");
        cancelButton->setObjectName(QString("cancel_%1").arg(rentalId));
        cancelButton->setMinimumWidth(80);
        cancelButton->setMinimumHeight(35);
        cancelButton->setStyleSheet(
            "QPushButton {"
            "   border: 1px solid #dc3545;"
            "   border-radius: 5px;"
            "   background-color: white;"
            "   color: #dc3545;"
            "   font-size: 12px;"
            "   padding: 8px 12px;"
            "}"
            "QPushButton:hover {"
            "   background-color: #dc3545;"
            "   color: white;"
            "}"
            "QPushButton:disabled {"
            "   opacity: 0.5;"
            "}"
        );
        connect(cancelButton, &QPushButton::clicked, this, [this, rentalId]() {
            cancelMyRental(rentalId);
        });
    }

    bottomLayout->addWidget(priceLabel);
    bottomLayout->addStretch();

    if (completeButton) {
        bottomLayout->addWidget(completeButton);
    }
    if (extendButton) {
        bottomLayout->addWidget(extendButton);
    }
    if (cancelButton) {
        bottomLayout->addWidget(cancelButton);
    }

    cardLayout->addWidget(bottomWidget);

    return card;
}

void ClientWindow::completeRental(int rentalId) {
    if (rentalId == -1) {
        QMessageBox::warning(this, "Ошибка", "Аренда не найдена");
        return;
    }

    RentalManager* rentalManager = RentalManager::instance();
    if (!rentalManager) {
        QMessageBox::critical(this, "Ошибка", "Менеджер аренд не доступен");
        return;
    }

    Rental* rental = rentalManager->findRentalById(rentalId);
    if (!rental) {
        QMessageBox::warning(this, "Ошибка", "Аренда не найдена");
        return;
    }

    if (rental->status() != Rental::ACTIVE && rental->status() != Rental::OVERDUE) {
        QMessageBox::warning(this, "Ошибка",
            "Завершить можно только активные или просроченные аренды");
        return;
    }

    QDialog dialog(this);
    dialog.setWindowTitle("Завершение аренды");
    dialog.setFixedSize(400, 300);
    dialog.setStyleSheet(
        "QDialog { background-color: white; }"
        "QLabel { font-size: 14px; }"
        "QPushButton {"
        "   padding: 8px 15px;"
        "   border-radius: 5px;"
        "   font-weight: bold;"
        "}"
    );

    QVBoxLayout *layout = new QVBoxLayout(&dialog);
    layout->setSpacing(15);

    QLabel *titleLabel = new QLabel("Завершение аренды");
    titleLabel->setStyleSheet("font-weight: bold; font-size: 18px; color: #2c3e50; text-align: center;");
    layout->addWidget(titleLabel);

    Car* car = CarManager::instance()->getCarById(rental->carId());
    if (car) {
        QLabel *carInfoLabel = new QLabel(QString("Автомобиль: %1 %2").arg(car->brand(), car->model()));
        carInfoLabel->setStyleSheet("font-weight: bold; color: #333;");
        layout->addWidget(carInfoLabel);
    }

    QLabel *datesLabel = new QLabel(
        QString("Период аренды: %1 - %2")
            .arg(rental->startDate().toString("dd.MM.yyyy"))
            .arg(rental->endDate().toString("dd.MM.yyyy")));
    layout->addWidget(datesLabel);

    QLabel *priceLabel = new QLabel(
        QString("Стоимость аренды: %1 руб.").arg(rental->totalPrice(), 0, 'f', 2));
    layout->addWidget(priceLabel);

    QFrame *separator = new QFrame();
    separator->setFrameShape(QFrame::HLine);
    separator->setStyleSheet("background-color: #ddd; margin: 10px 0;");
    layout->addWidget(separator);

    QLabel *returnDateLabel = new QLabel("Дата фактического возврата:");
    returnDateLabel->setStyleSheet("font-weight: bold;");
    layout->addWidget(returnDateLabel);

    QDateEdit *actualReturnDateEdit = new QDateEdit();
    actualReturnDateEdit->setDate(QDate::currentDate());
    actualReturnDateEdit->setMinimumDate(rental->startDate());
    actualReturnDateEdit->setMaximumDate(QDate::currentDate().addDays(30));
    actualReturnDateEdit->setCalendarPopup(true);
    actualReturnDateEdit->setDisplayFormat("dd.MM.yyyy");
    actualReturnDateEdit->setStyleSheet("QDateEdit { padding: 8px; border: 1px solid #ddd; border-radius: 3px; }");
    layout->addWidget(actualReturnDateEdit);

    QLabel *overdueLabel = new QLabel();
    overdueLabel->setStyleSheet("color: #dc3545; font-weight: bold;");
    layout->addWidget(overdueLabel);

    auto updateOverdueInfo = [&]() {
        QDate actualReturnDate = actualReturnDateEdit->date();

        if (actualReturnDate > rental->endDate()) {
            int overdueDays = rental->endDate().daysTo(actualReturnDate);
            double penalty = rentalManager->calculatePenalty(rentalId);

            overdueLabel->setText(
                QString("⚠️ Просрочка: %1 день(дня/дней)\n"
                       "Штраф за просрочку: %2 руб.")
                    .arg(overdueDays)
                    .arg(penalty, 0, 'f', 2)
            );
        } else {
            overdueLabel->setText("✅ Возврат в срок");
        }
    };

    connect(actualReturnDateEdit, &QDateEdit::dateChanged, this, updateOverdueInfo);
    updateOverdueInfo();

    layout->addStretch();

    QHBoxLayout *buttonLayout = new QHBoxLayout();

    QPushButton *cancelButton = new QPushButton("Отмена");
    cancelButton->setStyleSheet(
        "QPushButton {"
        "   background-color: #6c757d;"
        "   color: white;"
        "}"
        "QPushButton:hover { background-color: #5a6268; }"
    );

    QPushButton *confirmButton = new QPushButton("Завершить аренду");
    confirmButton->setStyleSheet(
        "QPushButton {"
        "   background-color: #20c997;"
        "   color: white;"
        "}"
        "QPushButton:hover { background-color: #17a589; }"
    );

    buttonLayout->addStretch();
    buttonLayout->addWidget(cancelButton);
    buttonLayout->addWidget(confirmButton);

    layout->addLayout(buttonLayout);

    connect(cancelButton, &QPushButton::clicked, &dialog, &QDialog::reject);
    connect(confirmButton, &QPushButton::clicked, &dialog, &QDialog::accept);

    if (dialog.exec() == QDialog::Rejected) {
        return;
    }

    QDate actualReturnDate = actualReturnDateEdit->date();

    QString confirmationText;
    if (actualReturnDate > rental->endDate()) {
        double penalty = rentalManager->calculatePenalty(rentalId);
        confirmationText = QString("Вы уверены, что хотите завершить аренду?\n\n"
                                 "Дата возврата: %1 (просрочка: %2 дня)\n"
                                 "Штраф за просрочку: %3 руб.\n"
                                 "Общая сумма к оплате: %4 руб.")
            .arg(actualReturnDate.toString("dd.MM.yyyy"))
            .arg(rental->endDate().daysTo(actualReturnDate))
            .arg(penalty, 0, 'f', 2)
            .arg(rental->totalPrice() + penalty, 0, 'f', 2);
    } else {
        confirmationText = QString("Вы уверены, что хотите завершить аренду?\n\n"
                                 "Дата возврата: %1\n"
                                 "Сумма к оплате: %2 руб.")
            .arg(actualReturnDate.toString("dd.MM.yyyy"))
            .arg(rental->totalPrice(), 0, 'f', 2);
    }

    int result = QMessageBox::question(this, "Подтверждение завершения",
        confirmationText,
        QMessageBox::Yes | QMessageBox::No);

    if (result == QMessageBox::Yes) {
        if (rentalManager->returnCar(rentalId, actualReturnDate)) {
            QMessageBox::information(this, "Успех",
                "Аренда успешно завершена.\n"
                "Автомобиль возвращен и снова доступен для аренды.");

            refreshMyRentals();
            refreshAvailableCars();
            statusBar()->showMessage("Аренда завершена", 3000);
        } else {
            QMessageBox::critical(this, "Ошибка",
                "Не удалось завершить аренду. Обратитесь к администратору.");
        }
    }
}

void ClientWindow::showRentalDetails(int rentalId) {
    RentalManager* rentalManager = RentalManager::instance();
    if (!rentalManager) return;

    User* currentUser = AuthManager::instance()->currentUser();
    if (!currentUser) return;

    QList<Rental*> myRentals = rentalManager->getRentalsByUser(currentUser->id());

    Rental* rental = nullptr;
    for (Rental* r : myRentals) {
        if (r->id() == rentalId) {
            rental = r;
            break;
        }
    }

    if (!rental) {
        QMessageBox::warning(this, "Ошибка", "Аренда не найдена");
        return;
    }

    Car* car = CarManager::instance()->getCarById(rental->carId());

    QString statusStr;
    switch (rental->status()) {
        case Rental::ACTIVE:
            statusStr = "Активная";
            break;
        case Rental::COMPLETED:
            statusStr = "Завершена";
            break;
        case Rental::CANCELLED:
            statusStr = "Отменена";
            break;
        case Rental::OVERDUE:
            statusStr = "Просрочена";
            break;
        default:
            statusStr = "Неизвестно";
    }

    QString details = QString(
        "Детали аренды #%1\n\n"
        "Автомобиль: %2\n"
        "Период: %3 - %4\n"
        "Стоимость: %5 руб.\n"
        "Статус: %6"
    ).arg(rentalId)
     .arg(car ? car->fullName() : "Неизвестно")
     .arg(rental->startDate().toString("dd.MM.yyyy"))
     .arg(rental->endDate().toString("dd.MM.yyyy"))
     .arg(rental->totalPrice(), 0, 'f', 2)
     .arg(statusStr);

    if (rental->actualReturnDate().isValid()) {
        details += QString("\nДата возврата: %1")
            .arg(rental->actualReturnDate().toString("dd.MM.yyyy"));
    }

    QMessageBox::information(this, "Детали аренды", details);
}

void ClientWindow::clearRentalCards() {
    if (rentalsContainer->layout()) {
        QLayout *layout = rentalsContainer->layout();
        QLayoutItem *item;
        while ((item = layout->takeAt(0)) != nullptr) {
            if (item->widget()) {
                item->widget()->deleteLater();
            }
            delete item;
        }
        delete layout;
    }

    for (QWidget *card : rentalCards) {
        card->deleteLater();
    }
    rentalCards.clear();
}

void ClientWindow::refreshHistoryTable(QTableWidget *historyTable) {
    User* currentUser = AuthManager::instance()->currentUser();
    if (!currentUser) return;

    RentalManager* rentalManager = RentalManager::instance();
    if (!rentalManager) return;

    QList<Rental*> userRentals = rentalManager->getRentalsByUser(currentUser->id());

    QList<Rental*> completedRentals;
    for (Rental* rental : userRentals) {
        if (rental->status() == Rental::COMPLETED) {
            completedRentals.append(rental);
        }
    }

    historyTable->setRowCount(completedRentals.size());

    for (int i = 0; i < completedRentals.size(); i++) {
        Rental* rental = completedRentals[i];

        Car* car = CarManager::instance()->getCarById(rental->carId());
        QString carInfo = car ? QString("%1 %2").arg(car->brand(), car->model()) : "Неизвестно";

        QDate endDate = rental->actualReturnDate().isValid() ?
                       rental->actualReturnDate() : rental->endDate();

        historyTable->setItem(i, 0, new QTableWidgetItem(endDate.toString("dd.MM.yyyy")));
        historyTable->setItem(i, 1, new QTableWidgetItem(carInfo));
        historyTable->setItem(i, 2, new QTableWidgetItem(QString::number(rental->totalPrice(), 'f', 2) + " руб."));
    }

    historyTable->sortItems(0, Qt::DescendingOrder);

    updateProfileStats();
}

void ClientWindow::updateProfileStats() {
    User* currentUser = AuthManager::instance()->currentUser();
    if (!currentUser) return;

    RentalManager* rentalManager = RentalManager::instance();
    if (!rentalManager) return;

    QList<Rental*> userRentals = rentalManager->getRentalsByUser(currentUser->id());

    int completedTrips = 0;
    for (Rental* rental : userRentals) {
        if (rental->status() == Rental::COMPLETED) {
            completedTrips++;
        }
    }

    if (profileTripsLabel) {
        profileTripsLabel->setText(QString("Поездок: %1").arg(completedTrips));
    }

    QString rating;
    if (completedTrips <= 0) {
        rating = "★☆☆☆☆";
    } else if (completedTrips <= 2) {
        rating = "★★☆☆☆";
    } else if (completedTrips <= 5) {
        rating = "★★★☆☆";
    } else if (completedTrips <= 10) {
        rating = "★★★★☆";
    } else {
        rating = "★★★★★";
    }

    if (profileRatingLabel) {
        profileRatingLabel->setText(QString("Рейтинг: %1").arg(rating));
    }
}

void ClientWindow::refreshProfile() {
    User* currentUser = AuthManager::instance()->currentUser();
    if (!currentUser) return;

    if (profileNameLabel) profileNameLabel->setText(currentUser->name());
    if (profileEmailLabel) profileEmailLabel->setText(currentUser->email());
    if (profilePhoneLabel) profilePhoneLabel->setText(currentUser->phone().isEmpty() ? "Не указан" : currentUser->phone());
    if (profileLicenseLabel) profileLicenseLabel->setText(currentUser->licenseNumber().isEmpty() ? "Не указан" : currentUser->licenseNumber());

    updateProfileStats();

    statusBar()->showMessage("Профиль обновлен", 3000);
}

void ClientWindow::setupCarPlaceholder(QLabel *label, const QString &brand, const QString &color) {
    label->setAlignment(Qt::AlignCenter);

    QColor bgColor;

    QString brandLower = brand.toLower();
    if (brandLower.contains("bmw") || brandLower.contains("mercedes") || brandLower.contains("audi"))
        bgColor = QColor(0, 102, 204); // синий для премиум
    else if (brandLower.contains("toyota") || brandLower.contains("honda") || brandLower.contains("nissan"))
        bgColor = QColor(204, 0, 0); // красный для японских
    else if (brandLower.contains("ford") || brandLower.contains("chevrolet") || brandLower.contains("dodge"))
        bgColor = QColor(0, 153, 0); // зеленый для американских
    else if (brandLower.contains("volkswagen") || brandLower.contains("skoda") || brandLower.contains("seat"))
        bgColor = QColor(51, 51, 51); // черный для немецких
    else if (brandLower.contains("renault") || brandLower.contains("peugeot") || brandLower.contains("citroen"))
        bgColor = QColor(255, 153, 0); // оранжевый для французских
    else if (brandLower.contains("hyundai") || brandLower.contains("kia"))
        bgColor = QColor(102, 0, 204); // фиолетовый для корейских
    else
        bgColor = QColor(102, 102, 102); // серый по умолчанию

    QPixmap placeholder(200, 110);
    placeholder.fill(bgColor);

    QPainter painter(&placeholder);
    painter.setRenderHint(QPainter::Antialiasing);

    QPainterPath path;
    path.addRoundedRect(QRectF(0, 0, 200, 110), 8, 8);
    painter.setClipPath(path);
    painter.drawPixmap(0, 0, placeholder);

    painter.setPen(Qt::white);
    QFont font = painter.font();
    font.setPointSize(48);
    painter.setFont(font);
    painter.drawText(placeholder.rect(), Qt::AlignCenter, "");

    painter.setPen(Qt::white);
    font.setPointSize(12);
    font.setBold(true);
    painter.setFont(font);
    painter.drawText(QRect(0, 75, 200, 30), Qt::AlignCenter, brand);

    label->setPixmap(placeholder);
    label->setStyleSheet("border-radius: 8px;");
}

QWidget* ClientWindow::createCarCard(int carId, const QString &brand, const QString &model,
                                    const QString &category, double price, const QString &color, int year) {
    QWidget *card = new QWidget();
    card->setObjectName("carCard");
    card->setMinimumHeight(250);
    card->setStyleSheet(
        "QWidget#carCard {"
        "   background-color: white;"
        "   border: 2px solid #e0e0e0;"
        "   border-radius: 10px;"
        "}"
        "QWidget#carCard:hover {"
        "   border: 2px solid #4169E1;"
        "   background-color: #f8f9ff;"
        "}"
    );

    QVBoxLayout *cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(15, 15, 15, 15);
    cardLayout->setSpacing(10);

    QWidget *imageWidget = new QWidget();
    imageWidget->setFixedHeight(120);
    QHBoxLayout *imageLayout = new QHBoxLayout(imageWidget);
    imageLayout->setContentsMargins(0, 0, 0, 0);

    QLabel *carImageLabel = new QLabel();
    carImageLabel->setAlignment(Qt::AlignCenter);
    carImageLabel->setFixedHeight(110);

    CarManager* carManager = CarManager::instance();
    Car* car = carManager ? carManager->getCarById(carId) : nullptr;

    if (car && !car->imageUrl().isEmpty()) {
        QString imagePath = car->imageUrl();

        if (imagePath.startsWith("file://")) {
            imagePath = imagePath.mid(7);
        }

        if (QFile::exists(imagePath)) {
            QPixmap carPixmap(imagePath);
            if (!carPixmap.isNull()) {
                carPixmap = carPixmap.scaled(200, 110, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                carImageLabel->setPixmap(carPixmap);
                carImageLabel->setStyleSheet("border-radius: 8px;");
            } else {
                setupCarPlaceholder(carImageLabel, brand, color);
            }
        } else if (imagePath.startsWith("http://") || imagePath.startsWith("https://")) {
            setupCarPlaceholder(carImageLabel, brand, color);
        } else if (imagePath.startsWith("qrc:/") || imagePath.startsWith(":/")) {
            QPixmap carPixmap(imagePath);
            if (!carPixmap.isNull()) {
                carPixmap = carPixmap.scaled(200, 110, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                carImageLabel->setPixmap(carPixmap);
                carImageLabel->setStyleSheet("border-radius: 8px;");
            } else {
                setupCarPlaceholder(carImageLabel, brand, color);
            }
        } else {
            setupCarPlaceholder(carImageLabel, brand, color);
        }
    } else {
        setupCarPlaceholder(carImageLabel, brand, color);
    }

    imageLayout->addWidget(carImageLabel);
    cardLayout->addWidget(imageWidget);

    QHBoxLayout *topInfoLayout = new QHBoxLayout();

    QLabel *brandLabel = new QLabel(brand);
    brandLabel->setStyleSheet(
        "QLabel {"
        "   font-size: 20px;"
        "   font-weight: bold;"
        "   color: #2c3e50;"
        "}"
    );

    QLabel *categoryLabel = new QLabel(category);
    categoryLabel->setStyleSheet(
        "QLabel {"
        "   font-size: 12px;"
        "   color: white;"
        "   background-color: #7f8c8d;"
        "   padding: 3px 8px;"
        "   border-radius: 10px;"
        "}"
    );

    topInfoLayout->addWidget(brandLabel);
    topInfoLayout->addStretch();
    topInfoLayout->addWidget(categoryLabel);

    cardLayout->addLayout(topInfoLayout);

    QLabel *modelLabel = new QLabel(model);
    modelLabel->setStyleSheet(
        "QLabel {"
        "   font-size: 16px;"
        "   color: #34495e;"
        "   font-weight: bold;"
        "}"
    );
    cardLayout->addWidget(modelLabel);

    QWidget *specsWidget = new QWidget();
    QVBoxLayout *specsLayout = new QVBoxLayout(specsWidget);
    specsLayout->setContentsMargins(0, 0, 0, 0);
    specsLayout->setSpacing(5);

    QWidget *specsRow1 = new QWidget();
    QHBoxLayout *specsRow1Layout = new QHBoxLayout(specsRow1);
    specsRow1Layout->setContentsMargins(0, 0, 0, 0);

    QLabel *colorLabel = new QLabel(color);
    QLabel *yearLabel = new QLabel(QString::number(year) + " г.");

    colorLabel->setStyleSheet("QLabel { color: #666; font-size: 13px; }");
    yearLabel->setStyleSheet("QLabel { color: #666; font-size: 13px; }");

    specsRow1Layout->addWidget(colorLabel);
    specsRow1Layout->addStretch();
    specsRow1Layout->addWidget(yearLabel);
    specsLayout->addWidget(specsRow1);

    QWidget *specsRow2 = new QWidget();
    QHBoxLayout *specsRow2Layout = new QHBoxLayout(specsRow2);
    specsRow2Layout->setContentsMargins(0, 0, 0, 0);

    QLabel *pricePerHourLabel = new QLabel(QString("%1 руб/час").arg(price, 0, 'f', 2));
    pricePerHourLabel->setStyleSheet("QLabel { color: #666; font-size: 13px; }");

    specsRow2Layout->addWidget(pricePerHourLabel);
    specsRow2Layout->addStretch();
    specsLayout->addWidget(specsRow2);

    cardLayout->addWidget(specsWidget);

    QWidget *bottomWidget = new QWidget();
    QHBoxLayout *bottomLayout = new QHBoxLayout(bottomWidget);
    bottomLayout->setContentsMargins(0, 0, 0, 0);

    QLabel *priceLabel = new QLabel(QString("%1 руб/день").arg(price, 0, 'f', 2));
    priceLabel->setStyleSheet(
        "QLabel {"
        "   font-size: 18px;"
        "   font-weight: bold;"
        "   color: #27ae60;"
        "}"
    );

    QPushButton *rentButton = new QPushButton("Арендовать");
    rentButton->setMinimumWidth(120);
    rentButton->setMinimumHeight(35);
    rentButton->setStyleSheet(
        "QPushButton {"
        "   background: qlineargradient(x1:0, y1:0, x2:1, y2:1,"
        "                               stop:0 #4169E1, stop:1 #8A2BE2);"
        "   color: white;"
        "   font-weight: bold;"
        "   font-size: 14px;"
        "   border: none;"
        "   border-radius: 5px;"
        "   padding: 8px 15px;"
        "}"
        "QPushButton:hover {"
        "   background: qlineargradient(x1:0, y1:0, x2:1, y2:1,"
        "                               stop:0 #3159D1, stop:1 #7A1BD2);"
        "}"
        "QPushButton:pressed {"
        "   background: qlineargradient(x1:0, y1:0, x2:1, y2:1,"
        "                               stop:0 #2159C1, stop:1 #6A0BC2);"
        "}"
    );

    connect(rentButton, &QPushButton::clicked, this, [this, carId]() {
        rentCar(carId);
    });

    bottomLayout->addWidget(priceLabel);
    bottomLayout->addStretch();
    bottomLayout->addWidget(rentButton);

    cardLayout->addWidget(bottomWidget);

    return card;
}

void ClientWindow::clearCarCards() {
    for (QWidget *card : carCards) {
        carsContainerLayout->removeWidget(card);
        card->deleteLater();
    }
    carCards.clear();

    QLayoutItem* item;
    while ((item = carsContainerLayout->takeAt(0)) != nullptr) {
        if (item->widget()) {
            item->widget()->deleteLater();
        }
        delete item;
    }
}

void ClientWindow::refreshAvailableCars() {
    CarManager* carManager = CarManager::instance();
    if (!carManager) {
        qDebug() << "CarManager не инициализирован";
        return;
    }

    QList<Car*> availableCars = carManager->getAvailableCars();

    clearCarCards();

    QSet<QString> uniqueBrands;

    for (Car* car : availableCars) {
        QWidget *card = createCarCard(
            car->id(),
            car->brand(),
            car->model(),
            car->categoryString(),
            car->pricePerDay(),
            car->color(),
            car->year()
        );

        carCards.append(card);
        carsContainerLayout->addWidget(card);

        uniqueBrands.insert(car->brand());
    }

    QFrame *separator = new QFrame();
    separator->setFrameShape(QFrame::HLine);
    separator->setFrameShadow(QFrame::Sunken);
    separator->setStyleSheet("background-color: #ddd;");
    separator->setFixedHeight(2);
    carsContainerLayout->addWidget(separator);

    updateBrandFilter();

    statusBar()->showMessage(QString("Доступно автомобилей: %1").arg(availableCars.size()), 3000);
}

void ClientWindow::refreshMyRentals() {
    User* currentUser = AuthManager::instance()->currentUser();
    if (!currentUser) {
        qDebug() << "Пользователь не авторизован";
        return;
    }

    RentalManager* rentalManager = RentalManager::instance();
    if (!rentalManager) {
        qDebug() << "RentalManager не инициализирован";
        return;
    }

    QList<Rental*> myRentals = rentalManager->getRentalsByUser(currentUser->id());

    clearRentalCards();

    QString filterText = rentalFilterComboBox->currentText();
    QString searchText = searchRentalsEdit->text().toLower();

    QGridLayout *gridLayout = new QGridLayout(rentalsContainer);
    gridLayout->setContentsMargins(0, 0, 0, 0);
    gridLayout->setSpacing(15);
    gridLayout->setAlignment(Qt::AlignTop);

    int row = 0;
    int col = 0;
    const int columns = 2;

    for (Rental* rental : myRentals) {
        Car* car = CarManager::instance()->getCarById(rental->carId());
        if (!car) continue;

        QString carName = car->fullName();
        QString brand = car->brand();
        QString model = car->model();
        QString carColor = car->color();
        int carYear = car->year();

        QString statusStr;
        switch (rental->status()) {
            case Rental::ACTIVE: statusStr = "Активная"; break;
            case Rental::COMPLETED: statusStr = "Завершена"; break;
            case Rental::CANCELLED: statusStr = "Отменена"; break;
            case Rental::OVERDUE: statusStr = "Просрочена"; break;
            default: statusStr = "Неизвестно";
        }

        if (filterText == "Все статусы" || filterText == "Отмененные") {
        } else {
            if (statusStr == "Отменена") {
                continue;
            }
        }

        if (filterText != "Все статусы" && filterText != "Отмененные") {
            if (filterText == "Активные" && statusStr != "Активная") continue;
            if (filterText == "Завершенные" && statusStr != "Завершена") continue;
            if (filterText == "Просроченные" && statusStr != "Просрочена") continue;
        } else if (filterText == "Отмененные") {
            if (statusStr != "Отменена") continue;
        }

        if (!searchText.isEmpty()) {
            if (!carName.toLower().contains(searchText) &&
                !brand.toLower().contains(searchText) &&
                !model.toLower().contains(searchText) &&
                !statusStr.toLower().contains(searchText)) {
                continue;
            }
        }

        QWidget *card = createRentalCard(
            rental->id(),
            carName,
            brand,
            model,
            rental->startDate(),
            rental->endDate(),
            statusStr,
            rental->totalPrice(),
            carColor,
            carYear
        );

        if (statusStr == "Отменена") {
            card->setStyleSheet(
                "QWidget#rentalCard {"
                "   background-color: #f8f9fa;"
                "   border: 2px solid #dee2e6;"
                "   border-radius: 10px;"
                "   opacity: 0.8;"
                "}"
                "QWidget#rentalCard:hover {"
                "   border: 2px solid #6c757d;"
                "   background-color: #e9ecef;"
                "}"
            );
        }

        rentalCards.append(card);
        gridLayout->addWidget(card, row, col);

        col++;
        if (col >= columns) {
            col = 0;
            row++;
        }
    }

    gridLayout->setRowStretch(row + 1, 1);
    for (int i = 0; i < columns; i++) {
        gridLayout->setColumnStretch(i, 1);
    }

    if (rentalCards.isEmpty()) {
        delete rentalsContainer->layout();

        QVBoxLayout *verticalLayout = new QVBoxLayout(rentalsContainer);
        verticalLayout->setContentsMargins(0, 0, 0, 0);

        QString message;
        if (filterText == "Отмененные") {
            message = "У вас нет отмененных аренд";
        } else if (filterText != "Все статусы") {
            message = QString("У вас нет аренд со статусом \"%1\"").arg(filterText);
        } else {
            message = "У вас пока нет аренд";
        }

        QLabel *noRentalsLabel = new QLabel(message);
        noRentalsLabel->setStyleSheet(
            "QLabel {"
            "   color: #7f8c8d;"
            "   font-size: 16px;"
            "   font-style: italic;"
            "   padding: 40px;"
            "   text-align: center;"
            "}"
        );
        noRentalsLabel->setAlignment(Qt::AlignCenter);
        verticalLayout->addWidget(noRentalsLabel);
        verticalLayout->addStretch();
    }

    statusBar()->showMessage(QString("Мои аренды: %1").arg(rentalCards.size()), 3000);
}

void ClientWindow::refreshMyFines() {
    User* currentUser = AuthManager::instance()->currentUser();
    if (!currentUser) return;

    RentalManager* rentalManager = RentalManager::instance();
    if (!rentalManager) return;

    QList<Rental*> myRentals = rentalManager->getRentalsByUser(currentUser->id());
    QList<Fine*> allFines = rentalManager->getAllFines();

    QList<Fine*> myFines;
    for (Fine* fine : allFines) {
        for (Rental* rental : myRentals) {
            if (fine->rentalId() == rental->id()) {
                myFines.append(fine);
                break;
            }
        }
    }

    clearFineCards();

    QString filterText = fineFilterComboBox->currentText();
    QString searchText = searchFinesEdit->text().toLower();

    for (Fine* fine : myFines) {
        bool paid = fine->paid();

        if (filterText != "Все статусы") {
            if (filterText == "Не оплаченные" && paid) continue;
            if (filterText == "Оплаченные" && !paid) continue;
        }

        if (!searchText.isEmpty()) {
            if (!fine->reason().toLower().contains(searchText)) {
                continue;
            }
        }

        QWidget *card = createFineCard(
            fine->id(),
            fine->rentalId(),
            fine->reason(),
            fine->amount(),
            fine->issueDate(),
            paid
        );

        fineCards.append(card);
        finesContainerLayout->addWidget(card);
    }

    if (fineCards.isEmpty()) {
        QLabel *noFinesLabel = new QLabel("У вас пока нет штрафов");
        noFinesLabel->setStyleSheet(
            "QLabel {"
            "   color: #7f8c8d;"
            "   font-size: 16px;"
            "   font-style: italic;"
            "   padding: 40px;"
            "   text-align: center;"
            "}"
        );
        noFinesLabel->setAlignment(Qt::AlignCenter);
        finesContainerLayout->addWidget(noFinesLabel);
    }

    finesContainerLayout->addStretch();

    statusBar()->showMessage(QString("Мои штрафы: %1").arg(fineCards.size()), 3000);
}

void ClientWindow::clearFineCards() {
    for (QWidget *card : fineCards) {
        finesContainerLayout->removeWidget(card);
        card->deleteLater();
    }
    fineCards.clear();
}

void ClientWindow::showFineDetails(int fineId) {
    RentalManager* rentalManager = RentalManager::instance();
    if (!rentalManager) return;

    User* currentUser = AuthManager::instance()->currentUser();
    if (!currentUser) return;

    QList<Rental*> myRentals = rentalManager->getRentalsByUser(currentUser->id());

    QList<Fine*> allFines = rentalManager->getAllFines();

    Fine* fine = nullptr;
    Rental* relatedRental = nullptr;

    for (Fine* f : allFines) {
        if (f->id() == fineId) {
            for (Rental* rental : myRentals) {
                if (rental->id() == f->rentalId()) {
                    fine = f;
                    relatedRental = rental;
                    break;
                }
            }
            if (fine) break;
        }
    }

    if (!fine || !relatedRental) {
        QMessageBox::warning(this, "Ошибка", "Штраф не найден");
        return;
    }

    Car* car = CarManager::instance()->getCarById(relatedRental->carId());

    QString details = QString(
        "Детали штрафа #%1\n\n"
        "Автомобиль: %2\n"
        "ID аренды: #%3\n"
        "Период аренды: %4 - %5\n"
        "Причина штрафа: %6\n"
        "Сумма: %7 руб.\n"
        "Дата выдачи: %8\n"
        "Статус: %9"
    ).arg(fineId)
     .arg(car ? car->fullName() : "Неизвестно")
     .arg(fine->rentalId())
     .arg(relatedRental->startDate().toString("dd.MM.yyyy"))
     .arg(relatedRental->endDate().toString("dd.MM.yyyy"))
     .arg(fine->reason())
     .arg(fine->amount(), 0, 'f', 2)
     .arg(fine->issueDate().toString("dd.MM.yyyy"))
     .arg(fine->paid() ? "Оплачен" : "Не оплачен");

    QMessageBox::information(this, "Детали штрафа", details);
}


void ClientWindow::updateBrandFilter() {
    CarManager* carManager = CarManager::instance();
    if (!carManager) return;

    QList<Car*> availableCars = carManager->getAvailableCars();
    if (availableCars.isEmpty()) {
        return;
    }

    QRadioButton* existingAllBrandsButton = nullptr;

    for (int i = 0; i < brandFilterLayout->count(); i++) {
        QLayoutItem* item = brandFilterLayout->itemAt(i);
        if (item && item->widget()) {
            QRadioButton* button = qobject_cast<QRadioButton*>(item->widget());
            if (button && button->text() == "Все марки") {
                existingAllBrandsButton = button;
                break;
            }
        }
    }

    QSet<QString> buttonsToRemove = {"Audi", "BMW", "Ford", "Mercedes",
                                     "Toyota", "Hyundai", "Kia", "Volkswagen",
                                     "Skoda", "Audi", "... и еще"};

    QList<QLayoutItem*> itemsToRemove;
    for (int i = 0; i < brandFilterLayout->count(); i++) {
        QLayoutItem* item = brandFilterLayout->itemAt(i);
        if (item && item->widget()) {
            QString widgetText;
            QRadioButton* button = qobject_cast<QRadioButton*>(item->widget());
            if (button) {
                widgetText = button->text();
                if (widgetText != "Все марки" && buttonsToRemove.contains(widgetText)) {
                    itemsToRemove.append(item);
                    brandButtonGroup->removeButton(button);
                }
            } else if (qobject_cast<QLabel*>(item->widget())) {
                itemsToRemove.append(item);
            }
        }
    }

    for (QLayoutItem* item : itemsToRemove) {
        if (item->widget()) {
            brandFilterLayout->removeWidget(item->widget());
            item->widget()->deleteLater();
        }
        delete item;
    }

    brandButtons.clear();

    if (!existingAllBrandsButton) {
        existingAllBrandsButton = new QRadioButton("Все марки");
        existingAllBrandsButton->setChecked(true);
        existingAllBrandsButton->setStyleSheet(
            "QRadioButton { font-size: 14px; padding: 8px; }"
            "QRadioButton::indicator { width: 16px; height: 16px; }"
        );

        connect(existingAllBrandsButton, &QRadioButton::clicked, this, [this]() {
            showAllBrands();
        });

        brandFilterLayout->addWidget(existingAllBrandsButton);
        brandButtonGroup->addButton(existingAllBrandsButton);
    }

    QSet<QString> uniqueBrands;
    for (Car* car : availableCars) {
        uniqueBrands.insert(car->brand());
    }

    QStringList sortedBrands = uniqueBrands.values();
    std::sort(sortedBrands.begin(), sortedBrands.end());

    int maxBrandsToShow = 4;
    int brandsCount = std::min(sortedBrands.size(), maxBrandsToShow);

    for (int i = 0; i < brandsCount; i++) {
        const QString &brand = sortedBrands[i];
        QRadioButton *brandButton = new QRadioButton(brand);
        brandButton->setStyleSheet(
            "QRadioButton { font-size: 14px; padding: 8px; }"
            "QRadioButton::indicator { width: 16px; height: 16px; }"
        );

        connect(brandButton, &QRadioButton::clicked, this, [this, brand]() {
            filterByBrand(brand);
        });

        brandFilterLayout->addWidget(brandButton);
        brandButtons.append(brandButton);
        brandButtonGroup->addButton(brandButton);
    }

    if (sortedBrands.size() > maxBrandsToShow) {
        QLabel *moreLabel = new QLabel(QString("... и еще %1").arg(sortedBrands.size() - maxBrandsToShow));
        moreLabel->setStyleSheet(
            "QLabel { font-size: 12px; color: #666; font-style: italic; padding-left: 8px; }"
        );
        brandFilterLayout->addWidget(moreLabel);
    }

    brandFilterLayout->addSpacing(10);
}

void ClientWindow::showAllBrandsView() {
    QDialog allBrandsDialog(this);
    allBrandsDialog.setWindowTitle("Все марки автомобилей");
    allBrandsDialog.setMinimumSize(400, 500);
    allBrandsDialog.setStyleSheet(
        "QDialog {"
        "   background-color: white;"
        "}"
    );

    QVBoxLayout *dialogLayout = new QVBoxLayout(&allBrandsDialog);
    dialogLayout->setContentsMargins(20, 20, 20, 20);
    dialogLayout->setSpacing(15);

    QLabel *titleLabel = new QLabel("Все марки автомобилей");
    titleLabel->setStyleSheet(
        "QLabel {"
        "   font-size: 24px;"
        "   font-weight: bold;"
        "   color: #2c3e50;"
        "   padding-bottom: 10px;"
        "   border-bottom: 2px solid #4169E1;"
        "}"
    );
    dialogLayout->addWidget(titleLabel);

    CarManager* carManager = CarManager::instance();
    if (!carManager) return;

    QList<Car*> availableCars = carManager->getAvailableCars();
    QSet<QString> uniqueBrands;

    for (Car* car : availableCars) {
        uniqueBrands.insert(car->brand());
    }

    QStringList sortedBrands = uniqueBrands.values();
    std::sort(sortedBrands.begin(), sortedBrands.end());

    QWidget *brandsContainer = new QWidget();
    QVBoxLayout *brandsLayout = new QVBoxLayout(brandsContainer);
    brandsLayout->setContentsMargins(0, 0, 0, 0);
    brandsLayout->setSpacing(5);

    for (const QString &brand : sortedBrands) {
        QPushButton *brandButton = new QPushButton(brand);
        brandButton->setStyleSheet(
            "QPushButton {"
            "   text-align: left;"
            "   padding: 12px 15px;"
            "   border: 1px solid #ddd;"
            "   border-radius: 5px;"
            "   background-color: white;"
            "   font-size: 14px;"
            "}"
            "QPushButton:hover {"
            "   background-color: #f0f5ff;"
            "   border: 1px solid #4169E1;"
            "}"
        );
        connect(brandButton, &QPushButton::clicked, this, [this, brand, &allBrandsDialog]() {
            filterByBrand(brand);
            allBrandsDialog.accept();
        });
        brandsLayout->addWidget(brandButton);
    }

    QScrollArea *scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setWidget(brandsContainer);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setStyleSheet("QScrollArea { border: none; }");

    dialogLayout->addWidget(scrollArea, 1);

    QPushButton *closeButton = new QPushButton("Закрыть");
    closeButton->setStyleSheet(
        "QPushButton {"
        "   padding: 10px 25px;"
        "   border: none;"
        "   border-radius: 5px;"
        "   background-color: #4169E1;"
        "   color: white;"
        "   font-weight: bold;"
        "}"
        "QPushButton:hover { background-color: #3159D1; }"
    );
    connect(closeButton, &QPushButton::clicked, &allBrandsDialog, &QDialog::accept);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget(closeButton);

    dialogLayout->addLayout(buttonLayout);

    allBrandsDialog.exec();
}

void ClientWindow::rentCar(int carId) {
    if (carId == -1) {
        QMessageBox::warning(this, "Ошибка", "Выберите автомобиль для аренды");
        return;
    }

    QDialog dialog(this);
    dialog.setWindowTitle("Аренда автомобиля");
    dialog.setFixedSize(350, 250);
    dialog.setStyleSheet(
        "QDialog { background-color: white; }"
        "QLabel { font-size: 14px; }"
        "QPushButton {"
        "   padding: 8px 15px;"
        "   border-radius: 5px;"
        "   font-weight: bold;"
        "}"
    );

    QVBoxLayout *layout = new QVBoxLayout(&dialog);

    QLabel *infoLabel = new QLabel("Выберите даты аренды:");
    infoLabel->setStyleSheet("font-weight: bold; font-size: 16px; color: #333;");
    layout->addWidget(infoLabel);

    QFormLayout *formLayout = new QFormLayout();
    formLayout->setSpacing(15);

    QDateEdit *startDateEdit = new QDateEdit();
    startDateEdit->setDate(QDate::currentDate().addDays(1));
    startDateEdit->setMinimumDate(QDate::currentDate().addDays(1));
    startDateEdit->setCalendarPopup(true);
    startDateEdit->setStyleSheet("QDateEdit { padding: 8px; border: 1px solid #ddd; border-radius: 3px; }");

    QDateEdit *endDateEdit = new QDateEdit();
    endDateEdit->setDate(QDate::currentDate().addDays(3));
    endDateEdit->setMinimumDate(QDate::currentDate().addDays(2));
    endDateEdit->setCalendarPopup(true);
    endDateEdit->setStyleSheet("QDateEdit { padding: 8px; border: 1px solid #ddd; border-radius: 3px; }");

    formLayout->addRow("Дата начала:", startDateEdit);
    formLayout->addRow("Дата окончания:", endDateEdit);

    layout->addLayout(formLayout);
    layout->addSpacing(20);

    QPushButton *okButton = new QPushButton("Арендовать");
    okButton->setStyleSheet(
        "QPushButton {"
        "   background-color: #28a745;"
        "   color: white;"
        "}"
        "QPushButton:hover { background-color: #218838; }"
    );

    QPushButton *cancelButton = new QPushButton("Отмена");
    cancelButton->setStyleSheet(
        "QPushButton {"
        "   background-color: #dc3545;"
        "   color: white;"
        "}"
        "QPushButton:hover { background-color: #c82333; }"
    );

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);

    layout->addLayout(buttonLayout);

    connect(okButton, &QPushButton::clicked, &dialog, &QDialog::accept);
    connect(cancelButton, &QPushButton::clicked, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Rejected) {
        return;
    }

    QDate startDate = startDateEdit->date();
    QDate endDate = endDateEdit->date();

    if (startDate >= endDate) {
        QMessageBox::warning(this, "Ошибка", "Дата окончания должна быть позже даты начала");
        return;
    }

    Car* car = CarManager::instance()->getCarById(carId);
    if (!car) {
        QMessageBox::critical(this, "Ошибка", "Автомобиль не найден");
        return;
    }

    User* currentUser = AuthManager::instance()->currentUser();
    RentalManager* rentalManager = RentalManager::instance();

    Rental* rental = rentalManager->createRental(currentUser->id(), carId,
                                                startDate, endDate,
                                                car->pricePerDay(), "daily");

    if (rental) {
        QMessageBox::information(this, "✅ Успех",
            QString("Аренда успешно создана!\n\n"
                   "Автомобиль: %1 %2\n"
                   "Стоимость: %3 руб.\n"
                   "Период: %4 - %5")
            .arg(car->brand())
            .arg(car->model())
            .arg(rental->totalPrice(), 0, 'f', 2)
            .arg(startDate.toString("dd.MM.yyyy"))
            .arg(endDate.toString("dd.MM.yyyy")));

        refreshAvailableCars();
        refreshMyRentals();
    } else {
        QMessageBox::critical(this, "Ошибка", "Не удалось создать аренду");
    }
}

void ClientWindow::cancelMyRental(int rentalId) {
    if (rentalId == -1) {
        QMessageBox::warning(this, "Ошибка", "Аренда не найдена");
        return;
    }

    RentalManager* rentalManager = RentalManager::instance();
    if (!rentalManager) {
        QMessageBox::critical(this, "Ошибка", "Менеджер аренд не доступен");
        return;
    }

    User* currentUser = AuthManager::instance()->currentUser();
    if (!currentUser) {
        QMessageBox::critical(this, "Ошибка", "Пользователь не авторизован");
        return;
    }

    QList<Rental*> userRentals = rentalManager->getRentalsByUser(currentUser->id());
    Rental* rental = nullptr;

    for (Rental* r : userRentals) {
        if (r->id() == rentalId) {
            rental = r;
            break;
        }
    }

    if (!rental) {
        QMessageBox::warning(this, "Ошибка", "Аренда не найдена или вам не принадлежит");
        return;
    }

    QString statusStr;
    switch (rental->status()) {
        case Rental::ACTIVE: statusStr = "Активная"; break;
        case Rental::COMPLETED: statusStr = "Завершена"; break;
        case Rental::CANCELLED: statusStr = "Отменена"; break;
        case Rental::OVERDUE: statusStr = "Просрочена"; break;
        default: statusStr = "Неизвестно";
    }

    if (statusStr == "Завершена" || statusStr == "Отменена") {
        QMessageBox::warning(this, "Ошибка",
            QString("Невозможно отменить аренду в статусе \"%1\"").arg(statusStr));
        return;
    }

    Car* car = CarManager::instance()->getCarById(rental->carId());
    QString carInfo = car ? car->fullName() : "Неизвестный автомобиль";

    QDate today = QDate::currentDate();
    QString warningText = "";
    if (today.daysTo(rental->startDate()) < 1) {
        double penaltyAmount = rental->totalPrice() * 0.1;
        warningText = QString("\n\n⚠️ Внимание! При отмене менее чем за 24 часа "
                              "будет применен штраф: %1 руб. (10%% от стоимости)")
                      .arg(penaltyAmount, 0, 'f', 2);
    }

    int result = QMessageBox::question(this, "Подтверждение отмены",
        QString("Вы уверены, что хотите отменить аренду?\n\n"
               "Автомобиль: %1\n"
               "Период: %2 - %3\n"
               "Стоимость: %4 руб.%5")
            .arg(carInfo)
            .arg(rental->startDate().toString("dd.MM.yyyy"))
            .arg(rental->endDate().toString("dd.MM.yyyy"))
            .arg(rental->totalPrice(), 0, 'f', 2)
            .arg(warningText),
        QMessageBox::Yes | QMessageBox::No);

    if (result == QMessageBox::Yes) {
        if (rentalManager->cancelRental(rentalId)) {
            QMessageBox::information(this, "Успех",
                "Аренда успешно отменена.\nАвтомобиль снова доступен для аренды.");

            refreshMyRentals();
            refreshAvailableCars();

            statusBar()->showMessage("Аренда отменена", 3000);
        } else {
            QMessageBox::critical(this, "Ошибка",
                "Не удалось отменить аренду. Возможно, автомобиль уже недоступен.");
        }
    }
}

void ClientWindow::payFine(int fineId) {
    if (fineId == -1) {
        QMessageBox::warning(this, "Ошибка", "Выберите штраф для оплаты");
        return;
    }

    RentalManager* rentalManager = RentalManager::instance();
    if (!rentalManager) return;

    QList<Fine*> allFines = rentalManager->getAllFines();

    Fine* fine = nullptr;
    for (Fine* f : allFines) {
        if (f->id() == fineId) {
            fine = f;
            break;
        }
    }

    if (!fine) {
        QMessageBox::warning(this, "Ошибка", "Штраф не найден");
        return;
    }

    if (fine->paid()) {
        QMessageBox::information(this, "Информация", "Этот штраф уже оплачен");
        return;
    }

    int result = QMessageBox::question(this, "Подтверждение оплаты",
        QString("Оплатить штраф на сумму %1 руб.?\n\nПричина: %2")
            .arg(fine->amount(), 0, 'f', 2)
            .arg(fine->reason()),
        QMessageBox::Yes | QMessageBox::No);

    if (result == QMessageBox::Yes) {
        if (rentalManager->payFine(fineId)) {
            QMessageBox::information(this, "Успех", "Штраф успешно оплачен");
            refreshMyFines();
        } else {
            QMessageBox::critical(this, "Ошибка", "Не удалось оплатить штраф");
        }
    }
}

void ClientWindow::filterByBrand(const QString &brand) {
    CarManager* carManager = CarManager::instance();
    if (!carManager) return;

    QList<Car*> allCars = carManager->getAvailableCars();

    for (int i = 0; i < carCards.size(); i++) {
        Car* car = allCars[i];
        bool visible = (car->brand() == brand);
        carCards[i]->setVisible(visible);
    }

    statusBar()->showMessage(QString("Показаны автомобили марки: %1").arg(brand), 3000);
}

void ClientWindow::showAllBrands() {
    for (QWidget *card : carCards) {
        card->setVisible(true);
    }

    statusBar()->showMessage("Показаны все автомобили", 3000);
}

void ClientWindow::searchAvailableCars() {
    QString searchText = searchEdit->text().toLower();

    CarManager* carManager = CarManager::instance();
    if (!carManager) return;

    QList<Car*> allCars = carManager->getAvailableCars();

    for (int i = 0; i < carCards.size(); i++) {
        Car* car = allCars[i];
        bool matches = true;

        if (!searchText.isEmpty()) {
            QString searchIn = car->model().toLower();
            if (!searchIn.contains(searchText)) {
                matches = false;
            }
        }

        carCards[i]->setVisible(matches);
    }
}

void ClientWindow::onLogout() {
    int result = QMessageBox::question(this, "Выход",
        "Вы уверены, что хотите выйти из системы?",
        QMessageBox::Yes | QMessageBox::No);

    if (result == QMessageBox::Yes) {
        AuthManager::instance()->logout();
        close();

        LoginWindow *loginWindow = new LoginWindow();
        loginWindow->show();
    }
}
