#include "Patroller.h"
#include "Weight.h"
#include <iostream>

Entity* Patroller::clone(ShaderProgram&) const { return nullptr; }

void Patroller::init(const glm::vec2& pos, ShaderProgram& program) {
    Enemy::init(pos, program); // Call base init to set position
    movementTimer = 1000;
    isIdle = false;

    // Load your specific enemy image
    spritesheet.loadFromFile("assets/images/squidward-dancing-pixel.png", TEXTURE_PIXEL_FORMAT_RGBA);
    sprite = Sprite::createSprite(glm::vec2(32, 32), glm::vec2(1.0f, 1.0f), &spritesheet, &program);
    
    sprite->setNumberAnimations(1);
    sprite->setAnimationSpeed(0, 1);       // Prevent infinite loop crash
    sprite->addKeyframe(0, glm::vec2(0.f, 0.f)); // Start at top-left of image
    sprite->changeAnimation(0);            // Activate animation
}

void Patroller::update(int deltaTime) {
    int mapX = (int)position.x;
    int mapY = (int)position.y;
    bool shouldTurn = false;

    // --- 1. IDLE LOGIC ---
    movementTimer -= deltaTime;
    if (movementTimer <= 0) {
        isIdle = !isIdle; // Toggle between walking and stopping
        // Random time: 1-3 seconds
        movementTimer = 1000 + (rand() % 2000);
    }

    if (isIdle) {
        sprite->update(deltaTime);
        return; // Skip movement logic while idle
    }

    // --- 2. COLLISION & LEDGE DETECTION ---
    if (moveRight) {
        // Wall check (Torso level)
        if (map->getTileIdAt(glm::ivec2(mapX + 32, mapY + 16)) == 1) {
            shouldTurn = true;
        }
        // Ledge check: Check tile under where the right edge will be
        // mapY + 32 is the row directly beneath the 32x32 sprite
        else if (map->getTileIdAt(glm::ivec2(mapX + 31, mapY + 32)) != 1 && map->getTileIdAt(glm::ivec2(mapX + 31, mapY + 32)) != 6) {
            shouldTurn = true;
        }
    }
    else {
        // Wall check (Torso level)
        if (map->getTileIdAt(glm::ivec2(mapX - 1, mapY + 16)) == 1) {
            shouldTurn = true;
        }
        // Ledge check: Check tile under where the left edge is
        else if (map->getTileIdAt(glm::ivec2(mapX, mapY + 32)) != 1 && map->getTileIdAt(glm::ivec2(mapX, mapY + 32)) != 6) {
            shouldTurn = true;
        }
    }

    // --- 3. MOVEMENT EXECUTION ---
    if (shouldTurn) {
        moveRight = !moveRight;
        position.x += moveRight ? 1.0f : -1.0f;
    }
    else {
        float speed = 0.1f * deltaTime;
        position.x += moveRight ? speed : -speed;
    }

    this->setPosition(position);
    sprite->update(deltaTime);
}

void Patroller::update(int deltaTime, const std::vector<Weight*> weights)
{
    int mapX = (int)position.x;
    int mapY = (int)position.y;
    bool shouldTurn = false;

    movementTimer -= deltaTime;
    if (movementTimer <= 0) {
        isIdle = !isIdle;
        movementTimer = 1000 + (rand() % 2000);
    }
    if (isIdle) { sprite->update(deltaTime); return; }

    float speed = 0.1f * deltaTime;

    // Using the Player's specific hitbox logic
    float pL = position.x + 32;
    float pR = pL + 24;
    float pT = position.y + 16;
    float pB = pT + 32;

    for (Weight* w : weights) {
        float wL = w->getPosition().x;
        float wR = wL + 16;
        float wT = w->getPosition().y;
        float wB = wT + 16;

        if (pB > wT && pT < wB) {
            if (moveRight) {
                if ((pR + speed) > wL && pL < wL) {
                    shouldTurn = true;
                    break;
                }
            }
            else {
                if ((pL - speed) < wR && pR > wR) {
                    shouldTurn = true;
                    break;
                }
            }
        }
    }

    if (!shouldTurn) {
        if (moveRight) {
            if (map->getTileIdAt(glm::ivec2(mapX + 32, mapY + 16)) == 1) shouldTurn = true;
            else if (map->getTileIdAt(glm::ivec2(mapX + 31, mapY + 32)) != 1 &&
                map->getTileIdAt(glm::ivec2(mapX + 31, mapY + 32)) != 6) shouldTurn = true;
        }
        else {
            if (map->getTileIdAt(glm::ivec2(mapX - 1, mapY + 16)) == 1) shouldTurn = true;
            else if (map->getTileIdAt(glm::ivec2(mapX, mapY + 32)) != 1 &&
                map->getTileIdAt(glm::ivec2(mapX, mapY + 32)) != 6) shouldTurn = true;
        }
    }

    if (shouldTurn) {
        moveRight = !moveRight;
        position.x += moveRight ? 1.0f : -1.0f;
    }
    else {
        position.x += moveRight ? speed : -speed;
    }

    this->setPosition(position);
    sprite->update(deltaTime);
}