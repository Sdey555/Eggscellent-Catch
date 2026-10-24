#ifndef AUDIOMANAGER_H
#define AUDIOMANAGER_H

#include <QObject>
#include <QSoundEffect>
#include <QUrl>
#include <QMap>
#include <QVector>
#include <QString>
#include "GameTypes.h"

class AudioManager : public QObject
{
    Q_OBJECT

public:
    explicit AudioManager(QObject *parent = nullptr);
    ~AudioManager() override = default;

    void init();

    // Sound effect & BGM playback
    void playSound(SoundId sound);
    void playBGM();
    void stopBGM();
    void pauseBGM();
    void resumeBGM();

    // Volume & Mute control
    void setBGMVolume(float volume);
    void setSFXVolume(float volume);
    float getBGMVolume() const { return bgmVolume; }
    float getSFXVolume() const { return sfxVolume; }

    void toggleMute();
    bool isMuted() const { return muted; }
    void setMuted(bool mute);

private:
    QUrl resolveAudioUrl(const QString &fileName);
    void setupSoundPool(SoundId id, const QString &fileName, int poolSize = 1);

    bool muted{false};
    float bgmVolume{0.38f};
    float sfxVolume{0.75f};

    QSoundEffect *bgmEffect{nullptr};

    struct SoundPool {
        QVector<QSoundEffect*> effects;
        int nextIndex{0};
    };
    QMap<SoundId, SoundPool> soundPools;
};

#endif // AUDIOMANAGER_H
