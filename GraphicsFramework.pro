QT += widgets
QT += multimedia
QT += serialport

CONFIG += gui
CONFIG += c++11

SOURCES += \
    main.cpp \
    vec2d.cpp \
    graphics.cpp \
    vec3d.cpp \
    filedownloader.cpp \
    plugin.cpp \
    serialportreader.cpp \
    networkclient.cpp \
    networkplugin.cpp \
    networkserver.cpp

HEADERS  += \
    graphics.h \
    vec2d.h \
    graphics.h \
    window.h \
    vec3d.h \
    filedownloader.h \
    plugin.h \
    serialportreader.h \
    networkclient.h \
    networkplugin.h \
    networkserver.h

