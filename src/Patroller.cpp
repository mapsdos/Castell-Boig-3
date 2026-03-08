#include "Patroller.h"

void Patroller::init(const glm::vec2& pos, ShaderProgram& program) {
    Enemy::init(pos, program); // Call base init to set position

    // Load your specific enemy image
    spritesheet.loadFromFile("assets/images/patroller.png", TEXTURE_PIXEL_FORMAT_RGBA);
    sprite = Sprite::createSprite(glm::vec2(32, 32), glm::vec2(0.25f, 0.25f), &spritesheet, &program);
    
    sprite->setNumberAnimations(1);
    sprite->setAnimationSpeed(0, 1);       // Prevent infinite loop crash
    sprite->addKeyframe(0, glm::vec2(0.f, 0.f)); // Start at top-left of image
    sprite->changeAnimation(0);            // Activate animation
}

void Patroller::update(int deltaTime) {
    // Calculate where the "feet" are. Assuming sprite is 32x32.
    glm::ivec2 pos = position;

    // We check a point slightly in front of the enemy (offset by horizontal movement)
    // and exactly at the bottom edge + 1 pixel
    int checkX = moveRight ? (pos.x + 32) : (pos.x - 1);
    int checkY = pos.y + 32; // Just below the feet

    // 1. Check for a wall in front
    if (map->collisionMoveRight(pos, glm::ivec2(32, 32)) && moveRight) {
        moveRight = false;
    }
    else if (map->collisionMoveLeft(pos, glm::ivec2(32, 32)) && !moveRight) {
        moveRight = true;
    }
    // 2. Check for the edge of a cliff
    else if (!map->hasFloorAt(glm::ivec2(checkX, checkY))) {
        moveRight = !moveRight; // Reverse direction
    }

    // Apply movement
    float speed = 0.05f * deltaTime;
    if (moveRight) position.x += speed;
    else position.x -= speed;

    sprite->setPosition(position);
    sprite->update(deltaTime);
}