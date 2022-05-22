QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    src/main.cpp \
    src/PhoneVariableScroll.cpp

HEADERS += \
    src/CONFIG.h \
    src/PhoneVariableScroll.h \
    src/debug.h

FORMS += \
    src/PhoneVariableScroll.ui
INCLUDEPATH += keepassx/src/autotype/wayland/ libuInputPlus/
# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resource.qrc



win32:CONFIG(release, debug|release): LIBS += -L$$PWD/libuInputPlus/build/release/ -luInputPlus
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/libuInputPlus/build/debug/ -luInputPlus
else:unix: LIBS += -L$$PWD/libuInputPlus/build/ -luInputPlus

INCLUDEPATH += $$PWD/libuInputPlus/build
DEPENDPATH += $$PWD/libuInputPlus/build

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/libuInputPlus/build/release/libuInputPlus.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/libuInputPlus/build/debug/libuInputPlus.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/libuInputPlus/build/release/uInputPlus.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/libuInputPlus/build/debug/uInputPlus.lib
else:unix: PRE_TARGETDEPS += $$PWD/libuInputPlus/build/libuInputPlus.a
