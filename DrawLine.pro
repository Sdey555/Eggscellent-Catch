TEMPLATE = app
TARGET = EggscellentCatch

QT += core gui widgets multimedia

CONFIG += c++17

INCLUDEPATH += \
    "$$PWD" \
    "$$PWD/src" \
    "$$PWD/src/core" \
    "$$PWD/src/entities" \
    "$$PWD/src/game" \
    "$$PWD/src/rendering" \
    "$$PWD/src/audio" \
    "$$PWD/src/ui"

SOURCES += \
    src/main.cpp \
    src/core/Pixel.cpp \
    src/core/Grid.cpp \
    src/entities/Basket.cpp \
    src/entities/Bird.cpp \
    src/entities/Boss.cpp \
    src/entities/FallingEgg.cpp \
    src/game/GameEngine.cpp \
    src/rendering/PixelFont.cpp \
    src/rendering/GameRenderer.cpp \
    src/audio/AudioManager.cpp \
    src/ui/CanvasLabel.cpp \
    src/ui/MainWindow.cpp

HEADERS += \
    src/core/Pixel.h \
    src/core/Grid.h \
    src/core/MotionSensor.h \
    src/entities/GameTypes.h \
    src/entities/Particle.h \
    src/entities/Basket.h \
    src/entities/Bird.h \
    src/entities/Boss.h \
    src/entities/FallingEgg.h \
    src/game/GameEngine.h \
    src/rendering/PixelFont.h \
    src/rendering/GameRenderer.h \
    src/audio/AudioManager.h \
    src/ui/CanvasLabel.h \
    src/ui/MainWindow.h

RESOURCES += \
    audio.qrc

FORMS += \
    src/ui/mainwindow.ui

# Default rules for deployment.
android {
    ANDROID_ABIS = arm64-v8a
    QT_ANDROID_ABIS = arm64-v8a
    LIBS += -landroid
}
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
