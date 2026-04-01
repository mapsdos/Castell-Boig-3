#include "Shooter.h"
#include "Weight.h"
#include <iostream>

Entity* Shooter::clone(ShaderProgram&) const { return nullptr; }

void Shooter::init(const glm::vec2& pos, ShaderProgram& program) {
    Enemy::init(pos, program);
    position = pos;
    movementTimer = 1000;
    isIdle = false;
    isShooting = false;
    shootAnimTimer = 0;

    // Squidward spritesheet: 10 frames of 14x31 pixels
    float frameWidth = 14.0f;
    float frameHeight = 31.0f;
    int numFrames = 10;
    float frameWidthUV = 1.0f / numFrames;
    float frameHeightUV = 1.0f;

    // Display size (scaled ~1.3x - slightly larger than player 24x32)
    spriteWidth = 18;
    spriteHeight = 40;

    spritesheet.loadFromFile("assets/images/enemies/squidward/squidward.png", TEXTURE_PIXEL_FORMAT_RGBA);
    sprite = Sprite::createSprite(
        glm::ivec2(spriteWidth, spriteHeight),
        glm::vec2(frameWidthUV, frameHeightUV),
        &spritesheet,
        &program
    );

    sprite->setNumberAnimations(6);

    // SQUIDWARD_STAND_RIGHT (frame 0)
    sprite->setAnimationSpeed(SQUIDWARD_STAND_RIGHT, 8);
    sprite->addKeyframe(SQUIDWARD_STAND_RIGHT, glm::vec2(0.0f * frameWidthUV, 0.0f));

    // SQUIDWARD_STAND_LEFT (frame 1)
    sprite->setAnimationSpeed(SQUIDWARD_STAND_LEFT, 8);
    sprite->addKeyframe(SQUIDWARD_STAND_LEFT, glm::vec2(1.0f * frameWidthUV, 0.0f));

    // SQUIDWARD_WALK_RIGHT (frames 2,3,4)
    sprite->setAnimationSpeed(SQUIDWARD_WALK_RIGHT, 8);
    sprite->addKeyframe(SQUIDWARD_WALK_RIGHT, glm::vec2(2.0f * frameWidthUV, 0.0f));
    sprite->addKeyframe(SQUIDWARD_WALK_RIGHT, glm::vec2(3.0f * frameWidthUV, 0.0f));
    sprite->addKeyframe(SQUIDWARD_WALK_RIGHT, glm::vec2(4.0f * frameWidthUV, 0.0f));

    // SQUIDWARD_WALK_LEFT (frames 5,6,7)
    sprite->setAnimationSpeed(SQUIDWARD_WALK_LEFT, 8);
    sprite->addKeyframe(SQUIDWARD_WALK_LEFT, glm::vec2(5.0f * frameWidthUV, 0.0f));
    sprite->addKeyframe(SQUIDWARD_WALK_LEFT, glm::vec2(6.0f * frameWidthUV, 0.0f));
    sprite->addKeyframe(SQUIDWARD_WALK_LEFT, glm::vec2(7.0f * frameWidthUV, 0.0f));

    // SQUIDWARD_SHOOT_RIGHT (frame 8)
    sprite->setAnimationSpeed(SQUIDWARD_SHOOT_RIGHT, 8);
    sprite->addKeyframe(SQUIDWARD_SHOOT_RIGHT, glm::vec2(8.0f * frameWidthUV, 0.0f));

    // SQUIDWARD_SHOOT_LEFT (frame 9)
    sprite->setAnimationSpeed(SQUIDWARD_SHOOT_LEFT, 8);
    sprite->addKeyframe(SQUIDWARD_SHOOT_LEFT, glm::vec2(9.0f * frameWidthUV, 0.0f));

    sprite->changeAnimation(SQUIDWARD_STAND_RIGHT);
}

void Shooter::updateAnimation() {
    if (isShooting) {
        int targetAnim = moveRight ? SQUIDWARD_SHOOT_RIGHT : SQUIDWARD_SHOOT_LEFT;
        if (sprite->animation() != targetAnim) {
            sprite->changeAnimation(targetAnim);
        }
        return;
    }
    
    int targetAnim;
    if (isIdle) {
        targetAnim = moveRight ? SQUIDWARD_STAND_RIGHT : SQUIDWARD_STAND_LEFT;
    } else {
        targetAnim = moveRight ? SQUIDWARD_WALK_RIGHT : SQUIDWARD_WALK_LEFT;
    }
    if (sprite->animation() != targetAnim) {
        sprite->changeAnimation(targetAnim);
    }
}

void Shooter::update(int deltaTime) {
    int mapX = (int)position.x;
    int mapY = (int)position.y;
    bool shouldTurn = false;

    // Handle shooting animation timer
    if (isShooting) {
        shootAnimTimer -= deltaTime;
        if (shootAnimTimer <= 0) {
            isShooting = false;
        }
    }

    // --- 1. IDLE LOGIC ---
    movementTimer -= deltaTime;
    if (movementTimer <= 0) {
        isIdle = !isIdle;
        movementTimer = 1000 + (rand() % 2000);
    }

    for (Bullet* b : bullets) {
        b->update(deltaTime);
    }

    if (isIdle || isShooting) {
        updateAnimation();
        sprite->update(deltaTime);
        return;
    }

    // --- 2. COLLISION & LEDGE DETECTION ---
    int groundCheckY = mapY + spriteHeight;
    
    if (moveRight) {
        int rightEdge = mapX + spriteWidth;
        // Wall check at mid-height
        if (map->getTileIdAt(glm::ivec2(rightEdge, mapY + spriteHeight / 2)) == 1) {
            shouldTurn = true;
        }
        // Ledge check
        else {
            int tileBelow = map->getTileIdAt(glm::ivec2(rightEdge - 1, groundCheckY));
            if (tileBelow != 1 && tileBelow != 6) {
                shouldTurn = true;
            }
        }
    }
    else {
        // Wall check at mid-height
        if (map->getTileIdAt(glm::ivec2(mapX - 1, mapY + spriteHeight / 2)) == 1) {
            shouldTurn = true;
        }
        // Ledge check
        else {
            int tileBelow = map->getTileIdAt(glm::ivec2(mapX, groundCheckY));
            if (tileBelow != 1 && tileBelow != 6) {
                shouldTurn = true;
            }
        }
    }

    // --- 3. MOVEMENT EXECUTION ---
    if (shouldTurn) {
        moveRight = !moveRight;
    }
    else {
        float speed = 0.04f * deltaTime;
        position.x += moveRight ? speed : -speed;
    }

    this->setPosition(position);
    updateAnimation();
    sprite->update(deltaTime);
}

void Shooter::render(const glm::mat4& modelview) {
    Enemy::render(modelview);
    for (Bullet* b : bullets) {
        b->render(modelview);
    }
}

void Shooter::Shoot(int deltaTime, ShaderProgram& program) {
    shotTimer -= deltaTime;
    if (shotTimer <= 0) {
        shotTimer = 1500 + (rand() % 2000);

        // Start shooting animation
        isShooting = true;
        shootAnimTimer = 300;

        Bullet* bullet = new Bullet();
        // Spawn in front of Squidward (28x62 sprite)
        glm::vec2 spawnPos = glm::vec2(
            position.x + tileMapDispl.x + (moveRight ? spriteWidth : -16),
            position.y + tileMapDispl.y + spriteHeight / 3
        );

        bullet->init(spawnPos, program, moveRight, "assets/images/enemies/squidward/clarinet.png");
        bullets.push_back(bullet);
    }
}

void Shooter::update(int deltaTime, const std::vector<Weight*> weights)
{
    int mapX = (int)position.x;
    int mapY = (int)position.y;
    bool shouldTurn = false;

    // Handle shooting animation timer
    if (isShooting) {
        shootAnimTimer -= deltaTime;
        if (shootAnimTimer <= 0) {
            isShooting = false;
        }
    }

    // --- 1. IDLE & BULLETS ---
    movementTimer -= deltaTime;
    if (movementTimer <= 0) {
        isIdle = !isIdle;
        movementTimer = 1000 + (rand() % 2000);
    }
    for (Bullet* b : bullets) { b->update(deltaTime); }
    if (isIdle || isShooting) { 
        updateAnimation();
        sprite->update(deltaTime); 
        return; 
    }

    // --- 2. MOVEMENT CALCULATIONS ---
    float speed = 0.04f * deltaTime;

    // Hitbox for Squidward (28x62)
    float pL = position.x;
    float pR = pL + spriteWidth;
    float pT = position.y;
    float pB = pT + spriteHeight;

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

    // --- 3. TILE/LEDGE DETECTION ---
    if (!shouldTurn) {
        int groundCheckY = mapY + spriteHeight;
        
        if (moveRight) {
            int rightEdge = mapX + spriteWidth;
            if (map->getTileIdAt(glm::ivec2(rightEdge, mapY + spriteHeight / 2)) == 1) {
                shouldTurn = true;
            }
            else {
                int tileBelow = map->getTileIdAt(glm::ivec2(rightEdge - 1, groundCheckY));
                if (tileBelow != 1 && tileBelow != 6) {
                    shouldTurn = true;
                }
            }
        }
        else {
            if (map->getTileIdAt(glm::ivec2(mapX - 1, mapY + spriteHeight / 2)) == 1) {
                shouldTurn = true;
            }
            else {
                int tileBelow = map->getTileIdAt(glm::ivec2(mapX, groundCheckY));
                if (tileBelow != 1 && tileBelow != 6) {
                    shouldTurn = true;
                }
            }
        }
    }

    // --- 4. EXECUTION ---
    if (shouldTurn) {
        moveRight = !moveRight;
    }
    else {
        position.x += moveRight ? speed : -speed;
    }

    this->setPosition(position);
    updateAnimation();
    sprite->update(deltaTime);
}