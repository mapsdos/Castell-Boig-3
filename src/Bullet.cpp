#include "Bullet.h"

void Bullet::init(const glm::vec2& pos, ShaderProgram& program, bool moveRight, const string& spritePath) {
    position = pos;
    dirRight = moveRight;
    int y = 8;
    if (spritePath != "assets/images/money.png")
    {
        y = 16;
    }
    spritesheet.loadFromFile(spritePath, TEXTURE_PIXEL_FORMAT_RGBA);
    sprite = Sprite::createSprite(glm::vec2(16, y), glm::vec2(1.0f, 1.0f), &spritesheet, &program);
    sprite->setPosition(position);
}

void Bullet::init(const glm::vec2& pos, ShaderProgram& program) {
    init(pos, program, true, "assets/images/money.png");
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