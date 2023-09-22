QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17
DESTDIR = bin

APP_NAME = "CVW66-Kneeboard"
APP_DISPLAY_NAME="DCS_Kneeboard_creator_for_CVW66"
APP_VERSION="1.0.0.1"
ORGANIZATION_NAME="CVW-66"
ORGANIZATION_DOMAIN="https://www.cvw-66.com"

DEFINES += \
    APP_NAME=\\\"$$APP_NAME\\\" \
    APP_DISPLAY_NAME=\\\"$$APP_DISPLAY_NAME\\\" \
    APP_VERSION=\\\"$$APP_VERSION\\\" \
    ORGANIZATION_NAME=\\\"$$ORGANIZATION_NAME\\\" \
    ORGANIZATION_DOMAIN=\\\"$$ORGANIZATION_DOMAIN\\\"

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    aboutkneebuilder.cpp \
    editdocumentwdg.cpp \
    kneeboardapp.cpp \
    main.cpp \
    kneeboardmainwnd.cpp \
    previewwidget.cpp

HEADERS += \
    aboutkneebuilder.h \
    editdocumentwdg.h \
    kneeboardapp.h \
    kneeboardmainwnd.h \
    previewwidget.h

FORMS += \
    aboutkneebuilder.ui \
    editdocumentwdg.ui \
    kneeboardmainwnd.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    CVW_66_yellow_triangleCVN73.png \
    Release.txt \
    kneeboard.ico \
    kneeboard.ini \
    kneeboarddb.xml

RESOURCES += \
    kneeboard.qrc

RC_ICONS = kneeboard.ico
