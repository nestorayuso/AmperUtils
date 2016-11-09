#-------------------------------------------------
#
# Project created by QtCreator 2016-03-22T12:21:49
#
#-------------------------------------------------

QT       += core gui

QT       += serialport network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = AmperUtils
TEMPLATE = app


SOURCES += main.cpp\
        maindialog.cpp \
    dti.cpp \
    dti_device.cpp \
    amper_dev.cpp \
    tarif_edit.cpp \
    par_edit.cpp \
    advanced_edit.cpp \
    settings.cpp \
    supdate.cpp \
    autocal_edit.cpp \
    archive_view.cpp \
#    logger.cpp \
    model_arch_data.cpp \
    model_arch_search.cpp \
    model_arch_event.cpp \
    model_arch_data_profile.cpp \
    widget_itgr.cpp \
    widget_meter_curr.cpp \
    widget_tariff_edit.cpp \
    MainWindow.cpp \
    DTI_url.cpp \
    widget_meter_arch_data_setttings.cpp \
    dialog_cur_value.cpp

HEADERS  += maindialog.h \
    dti.h \
    dti_device.h \
    amper_dev.h \
    tarif_edit.h \
    par_edit.h \
    advanced_edit.h \
    settings.h \
    supdate.h \
    autocal_edit.h \
    archive_view.h \
    logger.h \
    meter.h \
    model_arch_data.h \
    model_arch_search.h \
    model_arch_event.h \
    model_arch_data_profile.h \
    widget_itgr.h \
    widget_meter_curr.h \
    tariff.h \
    widget_tariff_edit.h \
    MainWindow.h \
    DTI_url.h \
    widget_meter_arch_data_setttings.h \
    dialog_cur_value.h

FORMS    += maindialog.ui \
    tarif_edit.ui \
    par_edit.ui \
    advanced_edit.ui \
    settings.ui \
    supdate.ui \
    autocal_edit.ui \
    archive_view.ui \
    widget_itgr.ui \
    widget_meter_curr.ui \
    widget_tariff_edit.ui \
    MainWindow.ui \
    widget_meter_arch_data_setttings.ui \
    dialog_cur_value.ui

RESOURCES += \
    amperutils.qrc

RC_FILE = rc_file.rc
