#include "Explosion.h"

// Explosion.cpp
void Explosion::init(const glm::vec2& pos, ShaderProgram& program) {
    // 1. Load texture (Make sure path is exactly correct)
    // Loading every time is slow; consider making the texture a member of LevelScene 
    // and passing a pointer to it instead to avoid disk I/O freezes.
    spritesheet.loadFromFile("assets/images/explosion.png", TEXTURE_PIXEL_FORMAT_RGBA);

    // 2. Sprite Setup
    sprite = Sprite::createSprite(glm::ivec2(64, 64), glm::vec2(1.0f / 7.0f, 1.0f), &spritesheet, &program);
    sprite->setNumberAnimations(1);
    sprite->addKeyframe(0, glm::vec2(0.f, 0.f)); // ... add all 7 frames
    sprite->changeAnimation(0);

    sprite->setPosition(pos);
    lifeTime = 0;
    finished = false;
}

void Explosion::init(const glm::vec2& pos, Texture* tex, ShaderProgram& program) {
    // 0.1428f is 1/7th of the texture width
    sprite = Sprite::createSprite(glm::ivec2(64, 64), glm::vec2(0.1428f, 1.0f), tex, &program);
    sprite->setNumberAnimations(1);

    // Add ALL 7 frames
    for (int i = 0; i < 7; i++) {
        sprite->addKeyframe(0, glm::vec2(i * 0.1428f, 0.0f));
    }

    // Set a specific speed (e.g., 14 frames per second)
    sprite->setAnimationSpeed(0, 14);
    sprite->changeAnimation(0);

    sprite->setPosition(pos);
    lifeTime = 0;
    finished = false;
}

void Explosion::update(int deltaTime) {
    lifeTime += deltaTime;
    sprite->update(deltaTime);

    // 3. DO NOT LOOP. Just check if time is up.
    if (lifeTime > 500) { // Explosion lasts 0.5 seconds
        finished = true;
    }
}

void Explosion::render(const glm::mat4& modelview) {
    if (sprite != nullptr) {
        sprite->render(modelview);
    }
}