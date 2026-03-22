#include "Follower.h"
#include <iostream>

void Follower::init(const glm::vec2& pos, ShaderProgram& program) {
    Enemy::init(pos, program);
    speed = 0.05f; // Ensure speed is initialized
    timer = 0;

    spritesheet.loadFromFile("assets/images/plankton-png.png", TEXTURE_PIXEL_FORMAT_RGBA);
    sprite = Sprite::createSprite(glm::vec2(32, 32), glm::vec2(1.0f, 1.0f), &spritesheet, &program);

    sprite->setNumberAnimations(1);
    sprite->setAnimationSpeed(0, 1);
    sprite->addKeyframe(0, glm::vec2(0.f, 0.f));
    sprite->changeAnimation(0);
}

// 1. FIX THE OVERRIDE: This satisfies the compiler/base class
void Follower::update(int deltaTime) {
    // We shouldn't really use this for a Follower, 
    // but we can't delete it. LevelScene calls the version with playerPos anyway.
}

// 2. THE ACTUAL LOGIC

void Follower::update(int deltaTime, const glm::vec2& playerPos) {
    // 1. Path Management
    timer -= deltaTime;
    if (pathSequence.empty() || timer <= 0) {
        glm::vec2 feetPos = glm::vec2(position.x + 16, position.y + 16);
        glm::vec2 playerFeet = glm::vec2(playerPos.x + 16, playerPos.y + 16);
        pathSequence = map->getPath(feetPos, playerFeet);
        timer = 500; // Fixed: Use '=' not '=='
        // cout << position.x << " " << position.y << "\n";
    }

    // 2. Gravity Logic
    if (!pathSequence.empty()) {
        AICommand cmd = pathSequence.front().command;
        // Only apply gravity if we are moving horizontally or explicitly falling.
        // DO NOT apply it during CLIMB or TRANSPORT, or he will sink/float.
        if (cmd == AICommand::MOVE_LEFT || cmd == AICommand::MOVE_RIGHT || cmd == AICommand::FALL) {
            applyGravity();
        }
    }
    else {
        // If no path exists, he should still fall to the nearest floor
        applyGravity();
    }

    // 3. AI Movement
    followPath(deltaTime);

    // 4. Finalize
    this->setPosition(position);
    sprite->update(deltaTime);
}

void Follower::followPath(int deltaTime) {
    if (pathSequence.empty()) return;

    PathStep& current = pathSequence.front();
    float moveAmount = speed * deltaTime;
    glm::vec2 size = glm::vec2(24, 32); // Slightly thinner than 32 to avoid "snagging"

    switch (current.command) {
    case AICommand::MOVE_LEFT:
        // Only move if there is NO collision to the left
        if (!map->collisionMoveLeft(glm::ivec2(position.x - moveAmount, position.y), size)) {
            position.x -= moveAmount;
        }
        else {
            current.distance = 0; // Blocked! Force next tile or recalculation
        }
        break;

    case AICommand::MOVE_RIGHT:
        if (!map->collisionMoveRight(glm::ivec2(position.x + moveAmount, position.y), size)) {
            position.x += moveAmount;
        }
        else {
            current.distance = 0; // Blocked!
        }
        break;

    case AICommand::CLIMB_UP:
        // Ensure he doesn't clip through a ceiling
        position.y -= moveAmount;
        break;

    case AICommand::CLIMB_DOWN:
    {
        int intPosY;
        // Check if there's a floor before moving down
        if (map->collisionMoveDown(glm::ivec2(position.x + 4, position.y + moveAmount), size, &intPosY)) {
            position.y = (float)intPosY;
            current.distance = 0; // We hit the floor, stop climbing
        }
        else {
            position.y += moveAmount;
        }
        break;
    }

    case AICommand::FALL:
    {
        float diffX = current.targetPoint.x - position.x;
        // Nudge horizontally
        if (abs(diffX) > 1.0f) {
            float nudge = (diffX > 0 ? 1 : -1) * moveAmount;
            // Check collision before nudging off ledge
            if (nudge > 0 && !map->collisionMoveRight(glm::ivec2(position.x + nudge, position.y), size))
                position.x += nudge;
            else if (nudge < 0 && !map->collisionMoveLeft(glm::ivec2(position.x + nudge, position.y), size))
                position.x += nudge;
        }

        // Finish if we reached the Y or if gravity has landed us
        if (position.y >= (current.targetPoint.y - 4.0f) || current.distance <= 0) {
            current.distance = 0;
        }
        break;
    }
    case AICommand::TRANSPORT:
        this->setPosition(glm::vec2(current.targetPoint.x, current.targetPoint.y - 16));
        current.distance = 0; // Force immediate completion
        moveAmount = 0;
        pathSequence.clear();
        break;
    }

    current.distance -= moveAmount;
    if (current.distance <= 0) {
        if (!pathSequence.empty()) pathSequence.erase(pathSequence.begin());
    }
}

void Follower::applyGravity() {
    // 1. Vine Safety Check
    int tileAtCenter = map->getTileIdAt(glm::ivec2(position.x + 16, position.y + 16));
    if (tileAtCenter == 3) return;

    float FALL_STEP = 2.0f;
    int intPosY;

    // 2. Check if there is floor JUST below us
    // Narrow the collision width (position.x + 12, width 8) so he falls off ledges easier
    if (map->collisionMoveDown(glm::ivec2(position.x + 12, position.y + FALL_STEP), glm::ivec2(8, 32), &intPosY)) {
        position.y = (float)intPosY; // Snap to floor
    }
    else {
        position.y += FALL_STEP; // Actually move down in the air
    }
}