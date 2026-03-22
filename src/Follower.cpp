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
        glm::vec2 feetPos = glm::vec2(position.x + 16, position.y + 24);
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
    int intPosY;

    // 1. Execute Command
    switch (current.command) {
    case AICommand::MOVE_LEFT:
        position.x -= moveAmount;
        break;
    case AICommand::MOVE_RIGHT:
        position.x += moveAmount;
        break;
    case AICommand::CLIMB_UP:
        position.y -= moveAmount;
        break;
    case AICommand::CLIMB_DOWN:
        // Prevent clipping: if floor is hit, stop immediately
        if (map->collisionMoveDown(glm::ivec2(position.x + 8, position.y + moveAmount), glm::vec2(16, 32), &intPosY)) {
            position.y = (float)intPosY;
            current.distance = 0;
        }
        else {
            position.y += moveAmount;
        }
        break;
    case AICommand::FALL:
    {
        // Horizontal nudge to clear ledges
        float diffX = current.targetPoint.x - position.x;
        if (abs(diffX) > 1.0f) {
            position.x += (diffX > 0 ? 1 : -1) * moveAmount;
        }
        // Vertical completion handled by targetPoint check below
        break;
    }
    case AICommand::TRANSPORT:
        this->setPosition(current.targetPoint);
        pathSequence.erase(pathSequence.begin());
        return; // Exit immediately after teleport
    }

    // 2. Update Progress
    current.distance -= moveAmount;

    // 3. Smooth Transition Check
    // Instead of snapping, we check if we've passed or reached the target coordinate
    bool reached = false;
    if (current.command == AICommand::MOVE_LEFT && position.x <= current.targetPoint.x) reached = true;
    else if (current.command == AICommand::MOVE_RIGHT && position.x >= current.targetPoint.x) reached = true;
    else if (current.command == AICommand::CLIMB_UP && position.y <= current.targetPoint.y) reached = true;
    else if (current.command == AICommand::CLIMB_DOWN && position.y >= current.targetPoint.y) reached = true;
    else if (current.command == AICommand::FALL && position.y >= current.targetPoint.y) reached = true;
    else if (current.distance <= 0) reached = true;

    if (reached) {
        // Only snap if the difference is tiny (< 2px) to keep it looking smooth
        if (glm::distance(position, current.targetPoint) < 2.0f) {
            position = current.targetPoint;
        }
        pathSequence.erase(pathSequence.begin());
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