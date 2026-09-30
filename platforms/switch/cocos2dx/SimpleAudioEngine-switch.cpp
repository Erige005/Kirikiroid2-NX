// [K2S] No-op CocosDenshion::SimpleAudioEngine for Switch. It is only referenced by cocostudio
// (ComAudio component, timeline sound frames); Kirikiroid2's UI doesn't use it and all game audio goes
// through Kirikiroid2's own OpenAL backend (sound/win32/WaveImpl.cpp).
#include "audio/include/SimpleAudioEngine.h"

namespace CocosDenshion {

static SimpleAudioEngine *s_engine = nullptr;

SimpleAudioEngine *SimpleAudioEngine::getInstance() {
    if (!s_engine) s_engine = new SimpleAudioEngine();
    return s_engine;
}
void SimpleAudioEngine::end() { delete s_engine; s_engine = nullptr; }

SimpleAudioEngine::SimpleAudioEngine() {}
SimpleAudioEngine::~SimpleAudioEngine() {}

void SimpleAudioEngine::preloadBackgroundMusic(const char *) {}
void SimpleAudioEngine::playBackgroundMusic(const char *, bool) {}
void SimpleAudioEngine::stopBackgroundMusic(bool) {}
void SimpleAudioEngine::pauseBackgroundMusic() {}
void SimpleAudioEngine::resumeBackgroundMusic() {}
void SimpleAudioEngine::rewindBackgroundMusic() {}
bool SimpleAudioEngine::willPlayBackgroundMusic() { return false; }
bool SimpleAudioEngine::isBackgroundMusicPlaying() { return false; }
float SimpleAudioEngine::getBackgroundMusicVolume() { return 1.0f; }
void SimpleAudioEngine::setBackgroundMusicVolume(float) {}
float SimpleAudioEngine::getEffectsVolume() { return 1.0f; }
void SimpleAudioEngine::setEffectsVolume(float) {}
unsigned int SimpleAudioEngine::playEffect(const char *, bool, float, float, float) { return 0; }
void SimpleAudioEngine::pauseEffect(unsigned int) {}
void SimpleAudioEngine::pauseAllEffects() {}
void SimpleAudioEngine::resumeEffect(unsigned int) {}
void SimpleAudioEngine::resumeAllEffects() {}
void SimpleAudioEngine::stopEffect(unsigned int) {}
void SimpleAudioEngine::stopAllEffects() {}
void SimpleAudioEngine::preloadEffect(const char *) {}
void SimpleAudioEngine::unloadEffect(const char *) {}

} // namespace CocosDenshion
