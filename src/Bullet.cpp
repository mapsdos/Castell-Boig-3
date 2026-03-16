#include "Bullet.h"

void Bullet::init(const glm::vec2& pos, ShaderProgram& program, bool moveRight) {
    position = pos;
    dirRight = moveRight;
    spritesheet.loadFromFile("assets/images/money.png", TEXTURE_PIXEL_FORMAT_RGBA);
    sprite = Sprite::createSprite(glm::vec2(16, 8), glm::vec2(1.0f, 1.0f), &spritesheet, &program);
    sprite->setPosition(position);
}

void Bullet::init(const glm::vec2& pos, ShaderProgram& program) {
    init(pos, program, true);
}

void Bullet::update(int deltaTime) {
    position.x += dirRight ? (speed * deltaTime) : (-speed * deltaTime);
    sprite->setPosition(position);
    sprite->update(deltaTime);
}

void Bullet::render(const glm::mat4& modelview)
{
    sprite->render(modelview);
}