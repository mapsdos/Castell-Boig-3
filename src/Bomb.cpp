#include "Bomb.h"

Entity* Bomb::clone(ShaderProgram& program) const {
    Bomb* b = new Bomb();
    b->init(position, program);
    return b;
}

void Bomb::init(const glm::vec2& pos, ShaderProgram& program) {
    position = pos;
    position.y += 16.0f;
    spritesheet.loadFromFile("assets/images/pixel-art-bomb-off.png", TEXTURE_PIXEL_FORMAT_RGBA);
    sprite = Sprite::createSprite(glm::vec2(16, 16), glm::vec2(1.0f, 1.0f), &spritesheet, &program);
    sprite->setPosition(position);
    // Define the animation state
    sprite->setNumberAnimations(1);
    sprite->setAnimationSpeed(0, 1);       // Prevent the infinite loop crash
    sprite->addKeyframe(0, glm::vec2(0.f, 0.f)); // Start at top-left of the image
    sprite->changeAnimation(0);            // Activate the animation

    spritesheetOn.loadFromFile("assets/images/pixel-art-bombs-on.png", TEXTURE_PIXEL_FORMAT_RGBA);
    spritesheetOn.setMagFilter(GL_NEAREST); // Keep it crisp

    // UV size is 0.5 because the image is split into two frames horizontally
    spriteOn = Sprite::createSprite(glm::ivec2(16, 16), glm::vec2(0.5f, 1.0f), &spritesheetOn, &program);
    spriteOn->setPosition(position);

    spriteOn->setNumberAnimations(1);
    spriteOn->setAnimationSpeed(0, 8); // Adjust number for faster/slower flashing

    // Frame 1: Left half (X = 0.0)
    spriteOn->addKeyframe(0, glm::vec2(0.0f, 0.0f));
    // Frame 2: Right half (X = 0.5)
    spriteOn->addKeyframe(0, glm::vec2(0.5f, 0.0f));

    spriteOn->changeAnimation(0);

    isPlanted = false;
}

void Bomb::update(int deltaTime) {
    // We must update the specific sprite that is currently active
    if (isPlanted) {
        spriteOn->update(deltaTime);
        // Sync position in case the bomb moved (e.g., falling or pushed)
        spriteOn->setPosition(position);
    }
    else {
        sprite->update(deltaTime);
        sprite->setPosition(position);
    }
}

void Bomb::render(const glm::mat4& modelview)
{
    if (isPlanted)
    {
        spriteOn->render(modelview);
    }
    else sprite->render(modelview);
}