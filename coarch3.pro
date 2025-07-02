QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    assemblerwindow.cpp \
    cpu.cpp \
    encoder.cpp \
    instruction.cpp \
    main.cpp \
    mainwindow.cpp \
    memory.cpp \
    parser.cpp \
    pseudo.cpp \
    register_file.cpp \
    simulatorwindow.cpp \
    symbol_table.cpp

HEADERS += \
    assemblerwindow.h \
    comm.h \
    common.h \
    cpu.h \
    encoder.h \
    instruction.h \
    mainwindow.h \
    memory.h \
    parser.h \
    pseudo.h \
    register_file.h \
    simulatorwindow.h \
    symbol_table.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
