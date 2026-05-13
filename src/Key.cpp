#include "Key.h"

// In Key.cpp
void Key::init(const glm::vec2& pos, ShaderProgram& program) {
    position = pos;
    spritesheet.loadFromFile("assets/images/pixel-key.png", TEXTURE_PIXEL_FORMAT_RGBA);

    // Key image is 802x512, aspect ratio ~1.57:1
    // Render at 20x13 to maintain proportions and fit the game scale
    // Hitbox will match visual size
    sprite = Sprite::createSprite(glm::vec2(20, 13), glm::vec2(1.0f, 1.0f), &spritesheet, &program);
    sprite->setPosition(position);

    // Define the animation state
    sprite->setNumberAnimations(1);
    sprite->setAnimationSpeed(0, 1);       // Prevent the infinite loop crash
    sprite->addKeyframe(0, glm::vec2(0.f, 0.f)); // Start at top-left of the image
    sprite->changeAnimation(0);            // Activate the animation
}

void Key::update(int deltaTime) {
    // If you want the key to rotate or bob, do it here
    sprite->update(deltaTime);
}

void Key::render(const glm::mat4& modelview) {
    sprite->render(modelview);
}

Entity* Key::clone(ShaderProgram& program) const {
    Key* newKey = new Key();
    newKey->init(this->position, program);
    // Si hay más estado relevante, copiarlo aquí
    return newKey;
}