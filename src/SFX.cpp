#include "SFX.h"
#include <iostream>
#include <algorithm>

bool SFX::init() {
    // SFML no necesita inicialización especial
    std::cout << "SFX: Inicializado correctamente" << std::endl;
    return true;
}

// ===== MÚSICA =====

void SFX::playMusic(const std::string& filename, bool loop, float volume) {
    // Si ya está sonando la misma música, no recargar
    if (currentMusicFile == filename && music.getStatus() == sf::Music::Playing) {
        return;
    }

    if (!music.openFromFile(filename)) {
        std::cout << "SFX Error: No se pudo cargar la música: " << filename << std::endl;
        return;
    }

    currentMusicFile = filename;
    music.setLoop(loop);
    music.setVolume(volume * (masterVolume / 100.f) * (musicVolume / 100.f));
    music.play();

    std::cout << "SFX: Reproduciendo música: " << filename << std::endl;
}

void SFX::stopMusic() {
    if (music.getStatus() != sf::Music::Stopped) {
        music.stop();
        std::cout << "SFX: Música detenida" << std::endl;
    }
}

void SFX::pauseMusic() {
    if (music.getStatus() == sf::Music::Playing) {
        music.pause();
        std::cout << "SFX: Música pausada" << std::endl;
    }
}

void SFX::resumeMusic() {
    if (music.getStatus() == sf::Music::Paused) {
        music.play();
        std::cout << "SFX: Música reanudada" << std::endl;
    }
}

void SFX::setMusicVolume(float volume) {
    musicVolume = std::max(0.f, std::min(100.f, volume));
    music.setVolume(musicVolume * (masterVolume / 100.f));
}

bool SFX::isMusicPlaying() const {
    return music.getStatus() == sf::Music::Playing;
}

// ===== EFECTOS DE SONIDO =====

void SFX::loadSound(const std::string& name, const std::string& filename) {
    sf::SoundBuffer buffer;
    if (!buffer.loadFromFile(filename)) {
        std::cout << "SFX Error: No se pudo cargar el sonido '" << name << "' desde: " << filename << std::endl;
        return;
    }
    soundBuffers[name] = buffer;
    std::cout << "SFX: Sonido cargado: " << name << " (" << filename << ")" << std::endl;
}

void SFX::playSound(const std::string& name, float volume) {
    auto it = soundBuffers.find(name);
    if (it == soundBuffers.end()) {
        std::cout << "SFX Error: Sonido '" << name << "' no encontrado" << std::endl;
        return;
    }

    // Limpiar sonidos que ya terminaron
    cleanStoppedSounds();

    // Crear nuevo sonido
    auto sound = std::make_unique<sf::Sound>();
    sound->setBuffer(it->second);
    sound->setVolume(volume * (masterVolume / 100.f) * (soundVolume / 100.f));
    sound->play();

    activeSounds.push_back(std::move(sound));
}

void SFX::stopAllSounds() {
    for (auto& sound : activeSounds) {
        sound->stop();
    }
    activeSounds.clear();
}

void SFX::setSoundVolume(float volume) {
    soundVolume = std::max(0.f, std::min(100.f, volume));
    for (auto& sound : activeSounds) {
        sound->setVolume(soundVolume * (masterVolume / 100.f));
    }
}

// ===== VOLUMEN GLOBAL =====

void SFX::setMasterVolume(float volume) {
    masterVolume = std::max(0.f, std::min(100.f, volume));

    // Actualizar volumen de la música
    music.setVolume(musicVolume * (masterVolume / 100.f));

    // Actualizar volumen de todos los sonidos activos
    for (auto& sound : activeSounds) {
        sound->setVolume(soundVolume * (masterVolume / 100.f));
    }
}

// ===== MÉTODOS PRIVADOS =====

void SFX::cleanStoppedSounds() {
    activeSounds.erase(
        std::remove_if(activeSounds.begin(), activeSounds.end(),
            [](const std::unique_ptr<sf::Sound>& sound) {
                return sound->getStatus() == sf::Sound::Stopped;
            }),
        activeSounds.end()
    );
}