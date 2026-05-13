#pragma once
#include <SFML/Audio.hpp>       // ✅ Correcto
#include <SFML/System.hpp>
#include <string>
#include <map>
#include <vector>
#include <memory>

class SFX {
public:
    // Singleton pattern
    static SFX& instance() {
        static SFX instance;
        return instance;
    }

    // No copiable
    SFX(const SFX&) = delete;
    SFX& operator=(const SFX&) = delete;

    // Inicialización
    bool init();

    // Música
    void playMusic(const std::string& filename, bool loop = true, float volume = 50.f);
    void stopMusic();
    void pauseMusic();
    void resumeMusic();
    void setMusicVolume(float volume);
    bool isMusicPlaying() const;

    // Efectos de sonido
    void loadSound(const std::string& name, const std::string& filename);
    void playSound(const std::string& name, float volume = 100.f);
    void stopAllSounds();
    void setSoundVolume(float volume);

    // Volumen global
    void setMasterVolume(float volume);
    float getMasterVolume() const { return masterVolume; }

private:
    SFX() = default;
    ~SFX() = default;

    // Música
    sf::Music music;
    std::string currentMusicFile;
    float musicVolume = 50.f;

    // Efectos de sonido
    std::map<std::string, sf::SoundBuffer> soundBuffers;
    std::vector<std::unique_ptr<sf::Sound>> activeSounds;
    float soundVolume = 100.f;

    // Volumen global
    float masterVolume = 35.f;

    // Limpiar sonidos que ya terminaron
    void cleanStoppedSounds();
};