#include "Stairs.h"

void Stairs::init(const glm::vec2& pos, ShaderProgram& program) {
    position = pos; // Inherited from Entity
    spritesheet.loadFromFile("assets/images/stairs.png", TEXTURE_PIXEL_FORMAT_RGBA);

    // Stairs are often 2 blocks tall (32x32 is fine, or 32x64)
    sprite = Sprite::createSprite(glm::vec2(32, 32), glm::vec2(1.0f, 1.0f), &spritesheet, &program);
    sprite->setPosition(position);

    sprite->setNumberAnimations(1);
    sprite->setAnimationSpeed(0, 1);
    sprite->addKeyframe(0, glm::vec2(0.f, 0.f));
    sprite->changeAnimation(0);
}

void Stairs::update(int deltaTime) {
    // If you want the key to rotate or bob, do it here
    sprite->update(deltaTime);
}

void Stairs::render(const glm::mat4& modelview) {
    sprite->render(modelview);
}

void Stairs::setDestination(const glm::vec2& dest)
{
    destination = dest;
}

glm::vec2 Stairs::getDestination() const 
{ 
    return destination; 
}