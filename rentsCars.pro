#-------------------------------------------------
#
# Project created by QtCreator 2025-12-19T12:07:22
#
#-------------------------------------------------

QT       += core gui widgets uitools

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = rentsCars
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

SOURCES += \
        authmanager.cpp \
        brandsdialog.cpp \
        car.cpp \
        carmanager.cpp \
        clientwindow.cpp \
        databasemanager.cpp \
        fine.cpp \
        loginwindow.cpp \
        main.cpp \
        mainwindow.cpp \
        rental.cpp \
        rentalmanager.cpp \
        report.cpp \
        user.cpp

HEADERS += \
        authmanager.h \
        brandsdialog.h \
        car.h \
        carmanager.h \
        carobserver.h \
        clientwindow.h \
        databasemanager.h \
        fine.h \
        loginwindow.h \
        mainwindow.h \
        pricingstrategy.h \
        rental.h \
        rentalmanager.h \
        report.h \
        user.h

FORMS += \
        mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
