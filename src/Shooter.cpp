#include "Shooter.h"
#include "Weight.h"
#include <iostream>

Entity* Shooter::clone(ShaderProgram&) const { return nullptr; }

void Shooter::init(const glm::vec2& pos, ShaderProgram& program) {
    Enemy::init(pos, program); // Call base init to set position
    movementTimer = 1000;
    isIdle = false;

    // Load your specific enemy image
    spritesheet.loadFromFile("assets/images/Mr_Krabs.png", TEXTURE_PIXEL_FORMAT_RGBA);
    sprite = Sprite::createSprite(glm::vec2(32, 32), glm::vec2(1.0f, 1.0f), &spritesheet, &program);

    sprite->setNumberAnimations(1);
    sprite->setAnimationSpeed(0, 1);       // Prevent infinite loop crash
    sprite->addKeyframe(0, glm::vec2(0.f, 0.f)); // Start at top-left of image
    sprite->changeAnimation(0);            // Activate animation
}

void Shooter::update(int deltaTime) {
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

    for (Bullet* b : bullets)
    {
        b->update(deltaTime);
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

void Shooter::render(const glm::mat4& modelview) {
    // 1. Draw Mr. Krabs himself
    Enemy::render(modelview);

    // 2. Draw all bullets that currently exist
    // If the vector is empty, this loop is skipped automatically!
    for (Bullet* b : bullets) {
        b->render(modelview);
    }
}

void Shooter::Shoot(int deltaTime, ShaderProgram& program) {
    shotTimer -= deltaTime;
    if (shotTimer <= 0) {
        shotTimer = 1500 + (rand() % 2000);

        Bullet* bullet = new Bullet();
        // Spawn slightly in front of Mr. Krabs
        glm::vec2 spawnPos = glm::vec2(position.x + tileMapDispl.x + (moveRight ? 24 : 0),
            position.y + tileMapDispl.y + 12);

        bullet->init(spawnPos, program, moveRight, "assets/images/money.png");

        // Adding it to the vector makes it "exist" for the update and render loops
        bullets.push_back(bullet);
    }
}

void Shooter::update(int deltaTime, const std::vector<Weight*> weights)
{
    int mapX = (int)position.x;
    int mapY = (int)position.y;
    bool shouldTurn = false;

    // --- 1. IDLE & BULLETS ---
    movementTimer -= deltaTime;
    if (movementTimer <= 0) {
        isIdle = !isIdle;
        movementTimer = 1000 + (rand() % 2000);
    }
    for (Bullet* b : bullets) { b->update(deltaTime); }
    if (isIdle) { sprite->update(deltaTime); return; }

    // --- 2. MOVEMENT CALCULATIONS ---
    float speed = 0.1f * deltaTime;

    // --- 3. WEIGHT COLLISION (Mirrored from Player Logic) ---
    // We use the same SCREEN offsets and hitbox widths you provided
    float pL = position.x + 32; // Using 32 based on your previous code's offset
    float pR = pL + 24;         // Shooter width (matching player)
    float pT = position.y + 16; // Adjusting for your engine's Y offset
    float pB = pT + 32;         // Shooter height

    for (Weight* w : weights) {
        float wL = w->getPosition().x;
        float wR = wL + 16;
        float wT = w->getPosition().y;
        float wB = wT + 16;

        // Vertical overlap check
        if (pB > wT && pT < wB) {
            if (moveRight) {
                // If Shooter's right side hits weight's left side
                if ((pR + speed) > wL && pL < wL) {
                    shouldTurn = true;
                    break;
                }
            }
            else {
                // If Shooter's left side hits weight's right side
                if ((pL - speed) < wR && pR > wR) {
                    shouldTurn = true;
                    break;
                }
            }
        }
    }

    // --- 4. TILE/LEDGE DETECTION ---
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

    // --- 5. EXECUTION ---
    if (shouldTurn) {
        moveRight = !moveRight;
        // Small nudge to prevent immediate re-collision
        position.x += moveRight ? 1.0f : -1.0f;
    }
    else {
        position.x += moveRight ? speed : -speed;
    }

    this->setPosition(position);
    sprite->update(deltaTime);
}