#include "Bullet.h"

Entity* Bullet::clone(ShaderProgram& program) const {
    Bullet* b = new Bullet();
    b->init(position, program, dirRight, bulletSpritePath);
    return b;
}


void Bullet::init(const glm::vec2& pos, ShaderProgram& program, bool moveRight, const string& spritePath) {
    position = pos;
    dirRight = moveRight;
    bulletSpritePath = spritePath;
    
    // Clarinet sprite
    if (spritePath == "assets/images/enemies/squidward/clarinet.png") {
        // Clarinet: single image ~24x6
        bulletSize = glm::ivec2(24, 6);
        
        spritesheet.loadFromFile(spritePath, TEXTURE_PIXEL_FORMAT_RGBA);
        sprite = Sprite::createSprite(bulletSize, glm::vec2(1.0f, 1.0f), &spritesheet, &program);
        
        sprite->setNumberAnimations(1);
        sprite->setAnimationSpeed(0, 1);
        sprite->addKeyframe(0, glm::vec2(0.0f, 0.0f));
        sprite->changeAnimation(0);
    }
    else {
        // Default bullet (money, etc)
        int h = (spritePath == "assets/images/money.png") ? 8 : 16;
        bulletSize = glm::ivec2(16, h);
        
        spritesheet.loadFromFile(spritePath, TEXTURE_PIXEL_FORMAT_RGBA);
        sprite = Sprite::createSprite(bulletSize, glm::vec2(1.0f, 1.0f), &spritesheet, &program);
        
        sprite->setNumberAnimations(1);
        sprite->setAnimationSpeed(0, 1);
        sprite->addKeyframe(0, glm::vec2(0.f, 0.f));
        sprite->changeAnimation(0);
    }
    
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