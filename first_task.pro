QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    control_panel.cpp \
    load_form_no_vector.cpp \
    load_form_vector.cpp \
    main.cpp \
    mainwindow.cpp\
    restraints_form.cpp


HEADERS += \
    control_panel.h \
    load_form_no_vector.h \
    load_form_vector.h \
    mainwindow.h \
    restraints_form.h

FORMS += \
    control_panel.ui \
    load_form_no_vector.ui \
    load_form_vector.ui \
    mainwindow.ui \
    restraints_form.ui



# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
