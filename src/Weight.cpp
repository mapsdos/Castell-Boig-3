#include "Weight.h"

void Weight::init(const glm::vec2& pos, ShaderProgram& program)
{
    position = pos;
    fell = false;
    spritesheet.loadFromFile("assets/images/weight-pixel-art.png", TEXTURE_PIXEL_FORMAT_RGBA);
    sprite = Sprite::createSprite(glm::vec2(16, 16), glm::vec2(1.0f, 1.0f), &spritesheet, &program);
    sprite->setPosition(position);
    // Define the animation state
    sprite->setNumberAnimations(1);
    sprite->setAnimationSpeed(0, 1);       // Prevent the infinite loop crash
    sprite->addKeyframe(0, glm::vec2(0.f, 0.f)); // Start at top-left of the image
    sprite->changeAnimation(0);            // Activate the animation
}

void Weight::render(const glm::mat4& modelview)
{
    sprite->render(modelview);
}

void Weight::push(float amount) {
    position.x += amount;
    glm::ivec2 mapPos = glm::ivec2(position.x - 32, position.y - 16);

    if (amount > 0 && map->collisionMoveRight(mapPos, glm::ivec2(16, 16))) {
        position.x -= amount;
    }
    else if (amount < 0 && map->collisionMoveLeft(mapPos, glm::ivec2(16, 16))) {
        position.x -= amount;
    }
}

void Weight::update(int deltaTime) {
    int floorY;
    glm::ivec2 mapPos = glm::ivec2(position.x - 32, position.y - 16);

    // Check if ALREADY grounded before applying gravity
    if (map->collisionMoveDown(glm::ivec2(mapPos.x, mapPos.y + 1), glm::ivec2(16, 16), &floorY)) {
        // Already on ground - snap and stop
        position.y = (float)floorY + 16;
        if (fallVelocity > 0.f)
        {
            fell = true;
        }
        fallVelocity = 0.0f;
    }
    else {
        // Not grounded - apply gravity
        fallVelocity += 0.002f * deltaTime;
        position.y += fallVelocity * deltaTime;

        if (map->collisionMoveDown(mapPos, glm::ivec2(16, 16), &floorY)) {
            position.y = (float)floorY + 16;
            fallVelocity = 0.0f;
            fell = true;
        }
    }

    sprite->setPosition(position);
    sprite->update(deltaTime);
}