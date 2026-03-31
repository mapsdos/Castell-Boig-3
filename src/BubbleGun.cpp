#include "BubbleGun.h"

Entity* BubbleGun::clone(ShaderProgram& program) const {
    BubbleGun* bg = new BubbleGun();
    bg->init(position, program);
    return bg;
}

void BubbleGun::init(const glm::vec2& pos, ShaderProgram& program) {
    position = pos;
    position.y += 8;
    spritesheet.loadFromFile("assets/images/bubble-blower-stick-pixel-art.png", TEXTURE_PIXEL_FORMAT_RGBA);
    sprite = Sprite::createSprite(glm::vec2(16, 16), glm::vec2(1.0f, 1.0f), &spritesheet, &program);
    sprite->setPosition(position);
    // Define the animation state
    sprite->setNumberAnimations(1);
    sprite->setAnimationSpeed(0, 1);       // Prevent the infinite loop crash
    sprite->addKeyframe(0, glm::vec2(0.f, 0.f)); // Start at top-left of the image
    sprite->changeAnimation(0);            // Activate the animation
}

void BubbleGun::update(int deltaTime) {
    sprite->update(deltaTime);
}

void BubbleGun::render(const glm::mat4& modelview)
{
    sprite->render(modelview);
}