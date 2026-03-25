#include "Follower.h"
#include <iostream>

void Follower::init(const glm::vec2& pos, ShaderProgram& program) {
    Enemy::init(pos, program);
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
    // PATH MANAGEMENT: Update path every 500ms
    timer -= deltaTime;
    if (pathSequence.empty() || timer <= 0) {
        // Use centers for pathfinding logic
        glm::vec2 feetPos = glm::vec2(position.x + 16, position.y + 24);
        glm::vec2 playerFeet = glm::vec2(playerPos.x + 16, playerPos.y + 16);

        pathSequence = map->getPath(feetPos, playerFeet);
        timer = 500;
    }
    
    // 1. Update the Brain
    updateFSM(playerPos, pathSequence);

    // 2. Execute Behavior
    if (currentState == EnemyState::TRACK)
    {
        // MOVEMENT: Follow the A* path
        if (!pathSequence.empty()) {
            followPath(deltaTime);
        }
    }
    else if (currentState == EnemyState::EXPLORE) {
        velocity *= 0.9f; // Slow down to a stop if not tracking
        position += velocity * (float)deltaTime;
    }

    // 3. Finalize
    this->setPosition(position);
    sprite->update(deltaTime);
}

void Follower::followPath(int deltaTime) {
    if (pathSequence.empty()) return;

    PathStep& current = pathSequence.front();
    glm::vec2 targetDir = current.targetPoint - this->position;
    float length = glm::length(targetDir);

    // 1. Direction & Acceleration
    if (length > 1.0f) {
        targetDir /= length;
        float factor = 0.005f;

        // Accelerate faster if falling
        if (current.command == AICommand::FALL) factor = 0.01f;

        velocity.x += factor * targetDir.x * deltaTime;
        velocity.y += factor * targetDir.y * deltaTime;

        // 2. Cap velocity (Allow higher speed for falling)
        float currentMax = MAX_VEL;
        if (current.command == AICommand::FALL) currentMax = MAX_VEL * 3.0f; 

        if (glm::length(velocity) > currentMax) {
            velocity = glm::normalize(velocity) * currentMax;
        }
    }

    // 3. Apply position
    this->position += velocity * (float)deltaTime;

    // 4. Grounding Check (The "Safety Net")
    // If we are moving down and getting close to the target block
    if (current.command == AICommand::FALL || current.command == AICommand::CLIMB_DOWN) {
        // 1. Calculate where he WILL be after this frame's movement
        float nextY = position.y + (velocity.y * (float)deltaTime);

        if (length < 64.0f) {
            int intPosY;
            // 2. Check collision at the NEXT Y position, not the current one
            if (map->collisionMoveDown(glm::ivec2(position.x + 4, nextY), glm::ivec2(32, 32), &intPosY)) {

                this->position.y = (float)intPosY; // Snap to the floor top
                velocity.y = 0;                    // Kill momentum

                // 3. Mark as arrived so the path step is erased
                pathSequence.erase(pathSequence.begin());
                velocity = glm::vec2(0, 0);
                return; // Skip the standard position += velocity at the bottom
            }
        }
    }

    // 5. Check for Arrival
    if (length < 2.0f || (current.command == AICommand::TRANSPORT)) {
        if (current.command == AICommand::TRANSPORT) {
            this->setPosition(current.targetPoint);
        }
        pathSequence.erase(pathSequence.begin());

        // Stop all movement if switching modes to prevent the "sliding" clip
        if (!pathSequence.empty() && pathSequence.front().command != current.command) {
            velocity = glm::vec2(0, 0);
        }
    }
}