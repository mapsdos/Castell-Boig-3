#include "Patroller.h"
#include "Weight.h"
#include <iostream>

Entity* Patroller::clone(ShaderProgram&) const { return nullptr; }

void Patroller::init(const glm::vec2& pos, ShaderProgram& program) {
    Enemy::init(pos, program);
    position = pos;
    movementTimer = 1000;
    isIdle = false;

    // Patrick spritesheet: 8 frames of 14x30 pixels
    float frameWidth = 14.0f;
    float frameHeight = 30.0f;
    int numFrames = 8;
    float frameWidthUV = 1.0f / numFrames;
    float frameHeightUV = 1.0f;

    // Display size (scaled ~1.3x - slightly larger than player 24x32)
    spriteWidth = 18;
    spriteHeight = 38;

    spritesheet.loadFromFile("assets/images/enemies/patrick/patrick.png", TEXTURE_PIXEL_FORMAT_RGBA);
    sprite = Sprite::createSprite(
        glm::ivec2(spriteWidth, spriteHeight),
        glm::vec2(frameWidthUV, frameHeightUV),
        &spritesheet,
        &program
    );

    sprite->setNumberAnimations(4);

    // PATRICK_STAND_RIGHT (frame 0)
    sprite->setAnimationSpeed(PATRICK_STAND_RIGHT, 8);
    sprite->addKeyframe(PATRICK_STAND_RIGHT, glm::vec2(0.0f * frameWidthUV, 0.0f));

    // PATRICK_STAND_LEFT (frame 1)
    sprite->setAnimationSpeed(PATRICK_STAND_LEFT, 8);
    sprite->addKeyframe(PATRICK_STAND_LEFT, glm::vec2(1.0f * frameWidthUV, 0.0f));

    // PATRICK_WALK_RIGHT (frames 2,3,4)
    sprite->setAnimationSpeed(PATRICK_WALK_RIGHT, 8);
    sprite->addKeyframe(PATRICK_WALK_RIGHT, glm::vec2(2.0f * frameWidthUV, 0.0f));
    sprite->addKeyframe(PATRICK_WALK_RIGHT, glm::vec2(3.0f * frameWidthUV, 0.0f));
    sprite->addKeyframe(PATRICK_WALK_RIGHT, glm::vec2(4.0f * frameWidthUV, 0.0f));

    // PATRICK_WALK_LEFT (frames 5,6,7)
    sprite->setAnimationSpeed(PATRICK_WALK_LEFT, 8);
    sprite->addKeyframe(PATRICK_WALK_LEFT, glm::vec2(5.0f * frameWidthUV, 0.0f));
    sprite->addKeyframe(PATRICK_WALK_LEFT, glm::vec2(6.0f * frameWidthUV, 0.0f));
    sprite->addKeyframe(PATRICK_WALK_LEFT, glm::vec2(7.0f * frameWidthUV, 0.0f));

    sprite->changeAnimation(PATRICK_STAND_RIGHT);
}

void Patroller::updateAnimation() {
    int targetAnim;
    if (isIdle) {
        targetAnim = moveRight ? PATRICK_STAND_RIGHT : PATRICK_STAND_LEFT;
    } else {
        targetAnim = moveRight ? PATRICK_WALK_RIGHT : PATRICK_WALK_LEFT;
    }
    if (sprite->animation() != targetAnim) {
        sprite->changeAnimation(targetAnim);
    }
}

void Patroller::update(int deltaTime) {
    int mapX = (int)position.x;
    int mapY = (int)position.y;
    bool shouldTurn = false;

    // --- 1. IDLE LOGIC ---
    movementTimer -= deltaTime;
    if (movementTimer <= 0) {
        isIdle = !isIdle;
        movementTimer = 1000 + (rand() % 2000);
    }

    if (isIdle) {
        updateAnimation();
        sprite->update(deltaTime);
        return;
    }

    // --- 2. COLLISION & LEDGE DETECTION ---
    // Use center-bottom checks for more reliable detection
    int checkY = mapY + spriteHeight - 1; // Bottom of sprite
    int groundCheckY = mapY + spriteHeight; // One pixel below
    
    if (moveRight) {
        int rightEdge = mapX + spriteWidth;
        // Wall check at mid-height
        if (map->getTileIdAt(glm::ivec2(rightEdge, mapY + spriteHeight / 2)) == 1) {
            shouldTurn = true;
        }
        // Ledge check: is there ground under our right foot?
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
        // Ledge check: is there ground under our left foot?
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
    if (isIdle) { 
        updateAnimation();
        sprite->update(deltaTime); 
        return; 
    }

    float speed = 0.04f * deltaTime;

    // Hitbox for Patrick (28x60)
    float pL = position.x + tileMapDispl.x;
    float pR = pL + spriteWidth + tileMapDispl.x;
    float pT = position.y + tileMapDispl.y;
    float pB = pT + spriteHeight + tileMapDispl.y;

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
        int checkY = mapY + spriteHeight - 1;
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