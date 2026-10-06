QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

TARGET = EggscellentCatch

INCLUDEPATH += . \
    src \
    src/core \
    src/entities \
    src/game \
    src/rendering \
    src/ui

SOURCES += \
    src/main.cpp \
    src/core/Pixel.cpp \
    src/core/Grid.cpp \
    src/entities/Basket.cpp \
    src/entities/Bird.cpp \
    src/entities/FallingEgg.cpp \
    src/game/GameEngine.cpp \
    src/rendering/PixelFont.cpp \
    src/rendering/GameRenderer.cpp \
    src/ui/CanvasLabel.cpp \
    src/ui/MainWindow.cpp

HEADERS += \
    src/core/Pixel.h \
    src/core/Grid.h \
    src/entities/GameTypes.h \
    src/entities/Particle.h \
    src/entities/Basket.h \
    src/entities/Bird.h \
    src/entities/FallingEgg.h \
    src/game/GameEngine.h \
    src/rendering/PixelFont.h \
    src/rendering/GameRenderer.h \
    src/ui/CanvasLabel.h \
    src/ui/MainWindow.h

FORMS += \
    src/ui/mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
