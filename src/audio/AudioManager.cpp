#include "AudioManager.h"
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <algorithm>

AudioManager::AudioManager(QObject *parent)
    : QObject(parent)
{
}

void AudioManager::init()
{
    // Initialize BGM
    bgmEffect = new QSoundEffect(this);
    bgmEffect->setSource(resolveAudioUrl("bgm_cheerful.wav"));
    bgmEffect->setLoopCount(QSoundEffect::Infinite);
    bgmEffect->setVolume(muted ? 0.0f : bgmVolume);

    // Setup pools for all SFX
    // Fast repetitive events have pools > 1 to support polyphonic overlapping playback
    setupSoundPool(SoundId::CATCH_REGULAR, "sfx_catch_regular.wav", 4);
    setupSoundPool(SoundId::CATCH_GOLDEN, "sfx_catch_golden.wav", 2);
    setupSoundPool(SoundId::CATCH_GROW, "sfx_catch_grow.wav", 2);
    setupSoundPool(SoundId::CATCH_LEGENDARY, "sfx_catch_legendary.wav", 1);
    setupSoundPool(SoundId::SPLAT, "sfx_splat.wav", 3);
    setupSoundPool(SoundId::POOP_HIT, "sfx_poop_hit.wav", 2);
    setupSoundPool(SoundId::POOP_GROUND, "sfx_poop_ground.wav", 3);
    setupSoundPool(SoundId::THUNDER, "sfx_thunder.wav", 1);
    setupSoundPool(SoundId::SHOCK, "sfx_shock.wav", 1);
    setupSoundPool(SoundId::LEVEL_UP, "sfx_level_up.wav", 1);
    setupSoundPool(SoundId::GAME_OVER, "sfx_game_over.wav", 1);
    setupSoundPool(SoundId::GROW_USE, "sfx_grow_use.wav", 1);
    setupSoundPool(SoundId::COMPANION_SPAWN, "sfx_companion_spawn.wav", 1);
    setupSoundPool(SoundId::UI_CLICK, "sfx_ui_click.wav", 2);
    setupSoundPool(SoundId::BOSS_WARNING, "sfx_boss_warning.wav", 1);
    setupSoundPool(SoundId::BOSS_HIT, "sfx_boss_hit.wav", 3);
    setupSoundPool(SoundId::BOSS_DEFEAT, "sfx_boss_defeat.wav", 1);
    setupSoundPool(SoundId::WIND_GUST, "sfx_wind_gust.wav", 1);
    setupSoundPool(SoundId::FROST_FORM, "sfx_frost_form.wav", 1);
}

QUrl AudioManager::resolveAudioUrl(const QString &fileName)
{
    // 1. Check application directory /audio/
    QString appAudio = QCoreApplication::applicationDirPath() + "/audio/" + fileName;
    if (QFileInfo::exists(appAudio)) {
        return QUrl::fromLocalFile(QFileInfo(appAudio).absoluteFilePath());
    }

    // 2. Check current working directory /audio/
    QString cwdAudio = QDir::currentPath() + "/audio/" + fileName;
    if (QFileInfo::exists(cwdAudio)) {
        return QUrl::fromLocalFile(QFileInfo(cwdAudio).absoluteFilePath());
    }

    // 3. Check relative "audio/"
    if (QFileInfo::exists("audio/" + fileName)) {
        return QUrl::fromLocalFile(QFileInfo("audio/" + fileName).absoluteFilePath());
    }

    // 4. Resource fallback
    return QUrl("qrc:/audio/" + fileName);
}

void AudioManager::setupSoundPool(SoundId id, const QString &fileName, int poolSize)
{
    SoundPool pool;
    QUrl url = resolveAudioUrl(fileName);

    for (int i = 0; i < poolSize; ++i) {
        auto *effect = new QSoundEffect(this);
        effect->setSource(url);
        effect->setVolume(muted ? 0.0f : sfxVolume);
        pool.effects.push_back(effect);
    }
    pool.nextIndex = 0;
    soundPools.insert(id, pool);
}

void AudioManager::playSound(SoundId sound)
{
    if (muted) {
        return;
    }

    auto it = soundPools.find(sound);
    if (it == soundPools.end() || it->effects.isEmpty()) {
        return;
    }

    SoundPool &pool = it.value();
    QSoundEffect *effect = pool.effects[pool.nextIndex];
    pool.nextIndex = (pool.nextIndex + 1) % pool.effects.size();

    const float effVol = (muted || sfxMuted) ? 0.0f : sfxVolume;
    effect->setVolume(effVol);
    if (effVol > 0.001f) {
        effect->play();
    }
}

void AudioManager::playBGM()
{
    if (!bgmEffect) {
        return;
    }
    const float effVol = (muted || bgmMuted) ? 0.0f : bgmVolume;
    bgmEffect->setVolume(effVol);
    if (effVol > 0.001f && !bgmEffect->isPlaying()) {
        bgmEffect->play();
    }
}

void AudioManager::stopBGM()
{
    if (bgmEffect && bgmEffect->isPlaying()) {
        bgmEffect->stop();
    }
}

void AudioManager::pauseBGM()
{
    if (bgmEffect && bgmEffect->isPlaying()) {
        bgmEffect->stop();
    }
}

void AudioManager::resumeBGM()
{
    const float effVol = (muted || bgmMuted) ? 0.0f : bgmVolume;
    if (bgmEffect && effVol > 0.001f && !bgmEffect->isPlaying()) {
        bgmEffect->setVolume(effVol);
        bgmEffect->play();
    }
}

void AudioManager::setBGMVolume(float volume)
{
    bgmVolume = std::clamp(volume, 0.0f, 1.0f);
    if (bgmEffect) {
        const float effVol = (muted || bgmMuted) ? 0.0f : bgmVolume;
        bgmEffect->setVolume(effVol);
        if (effVol > 0.001f && !bgmEffect->isPlaying()) {
            bgmEffect->play();
        } else if (effVol <= 0.001f && bgmEffect->isPlaying()) {
            bgmEffect->stop();
        }
    }
}

void AudioManager::setSFXVolume(float volume)
{
    sfxVolume = std::clamp(volume, 0.0f, 1.0f);
    const float effVol = (muted || sfxMuted) ? 0.0f : sfxVolume;
    for (auto &pool : soundPools) {
        for (auto *effect : pool.effects) {
            effect->setVolume(effVol);
        }
    }
}

void AudioManager::setBGMMuted(bool mute)
{
    bgmMuted = mute;
    setBGMVolume(bgmVolume);
}

void AudioManager::setSFXMuted(bool mute)
{
    sfxMuted = mute;
    setSFXVolume(sfxVolume);
}

void AudioManager::toggleMute()
{
    setMuted(!muted);
}

void AudioManager::setMuted(bool mute)
{
    muted = mute;
    setBGMVolume(bgmVolume);
    setSFXVolume(sfxVolume);
}
