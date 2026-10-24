#ifndef MOTIONSENSOR_H
#define MOTIONSENSOR_H

#include <algorithm>
#include <cmath>

#if defined(Q_OS_ANDROID)
#include <android/sensor.h>
#include <android/looper.h>
#include <QGuiApplication>
#include <QScreen>

class MotionSensor
{
public:
    static MotionSensor &instance()
    {
        static MotionSensor s;
        return s;
    }

    void start()
    {
        if (m_running)
        {
            return;
        }

        m_sensorManager = ASensorManager_getInstance();
        if (!m_sensorManager)
        {
            return;
        }

        m_accelerometer = ASensorManager_getDefaultSensor(m_sensorManager, ASENSOR_TYPE_ACCELEROMETER);
        if (!m_accelerometer)
        {
            return;
        }

        ALooper *looper = ALooper_prepare(ALOOPER_PREPARE_ALLOW_NON_CALLBACKS);
        if (!looper)
        {
            looper = ALooper_forThread();
        }
        if (!looper)
        {
            return;
        }

        m_eventQueue = ASensorManager_createEventQueue(m_sensorManager, looper, 1, nullptr, nullptr);
        if (!m_eventQueue)
        {
            return;
        }

        ASensorEventQueue_enableSensor(m_eventQueue, m_accelerometer);
        ASensorEventQueue_setEventRate(m_eventQueue, m_accelerometer, 20000); // 20ms = 50Hz
        m_running = true;
    }

    void stop()
    {
        if (!m_running)
        {
            return;
        }
        if (m_eventQueue && m_accelerometer)
        {
            ASensorEventQueue_disableSensor(m_eventQueue, m_accelerometer);
        }
        if (m_sensorManager && m_eventQueue)
        {
            ASensorManager_destroyEventQueue(m_sensorManager, m_eventQueue);
            m_eventQueue = nullptr;
        }
        m_running = false;
    }

    bool isRunning() const { return m_running; }

    // Returns normalized tilt in range [-1.0, 1.0]: negative = left, positive = right
    float getTiltX(bool isLandscape = true)
    {
        if (!m_running)
        {
            start();
        }

        if (m_eventQueue)
        {
            ASensorEvent event;
            while (ASensorEventQueue_hasEvents(m_eventQueue) > 0 &&
                   ASensorEventQueue_getEvents(m_eventQueue, &event, 1) > 0)
            {
                if (event.type == ASENSOR_TYPE_ACCELEROMETER)
                {
                    m_accX = event.acceleration.x;
                    m_accY = event.acceleration.y;
                    m_accZ = event.acceleration.z;
                }
            }
        }

        // Determine orientation tilt:
        // In landscape (standard game view):
        // Tilting left/right maps to -accY (or +accY depending on 90 vs 270 deg)
        float rawTilt = 0.0f;
        QScreen *screen = QGuiApplication::primaryScreen();
        Qt::ScreenOrientation orient = screen ? screen->orientation() : Qt::LandscapeOrientation;

        if (orient == Qt::InvertedLandscapeOrientation)
        {
            rawTilt = -m_accY;
        }
        else if (orient == Qt::PortraitOrientation)
        {
            rawTilt = m_accX;
        }
        else if (orient == Qt::InvertedPortraitOrientation)
        {
            rawTilt = -m_accX;
        }
        else
        {
            // Standard Landscape
            rawTilt = m_accY;
        }

        // Apply deadzone and scaling:
        // Gravity is ~9.8 m/s^2; ~3.8 m/s^2 (~23 degree tilt) corresponds to full speed
        const float maxTiltAccel = 3.8f;
        float norm = rawTilt / maxTiltAccel;
        norm = std::clamp(norm, -1.0f, 1.0f);

        // Deadzone to prevent hand tremor jitter
        if (std::abs(norm) < 0.07f)
        {
            return 0.0f;
        }

        return norm;
    }

private:
    MotionSensor() = default;
    ~MotionSensor() { stop(); }

    bool m_running{false};
    ASensorManager *m_sensorManager{nullptr};
    const ASensor *m_accelerometer{nullptr};
    ASensorEventQueue *m_eventQueue{nullptr};
    float m_accX{0.0f};
    float m_accY{0.0f};
    float m_accZ{0.0f};
};

#else

// Desktop / non-Android mock
class MotionSensor
{
public:
    static MotionSensor &instance()
    {
        static MotionSensor s;
        return s;
    }

    void start() { m_running = true; }
    void stop() { m_running = false; }
    bool isRunning() const { return m_running; }

    float getTiltX(bool /*isLandscape*/ = true)
    {
        return m_simulatedTilt;
    }

    void setSimulatedTilt(float t)
    {
        m_simulatedTilt = std::clamp(t, -1.0f, 1.0f);
    }

private:
    MotionSensor() = default;
    bool m_running{false};
    float m_simulatedTilt{0.0f};
};

#endif // Q_OS_ANDROID

#endif // MOTIONSENSOR_H
