#include "Follower.h"
#include <iostream>
#include <algorithm>

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
void Follower::update(int deltaTime, const glm::vec2& playerPos, const std::vector<Weight*> weights) {
    // PATH MANAGEMENT: Update path every 500ms
    timer -= deltaTime;
    if (pathSequence.empty() || (timer <= 0 && pathSequence.begin()->command != AICommand::ASCEND && pathSequence.begin()->command != AICommand::FALL_RIGHT && pathSequence.begin()->command != AICommand::FALL_LEFT && pathSequence.begin()->command != AICommand::FALL)) {
        // Use centers for pathfinding logic
        glm::vec2 feetPos = glm::vec2(position.x + 16, position.y + 24);
        glm::vec2 playerFeet = glm::vec2(playerPos.x + 16, playerPos.y + 16);

        pathSequence = map->getPath(feetPos, playerFeet, weights);
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
    int clampedDT = std::min<int>(deltaTime, 33);

    // cout << (current.command == AICommand::ASCEND) << ' ' << (current.command == AICommand::TRANSPORT) << ' ' << (current.command == AICommand::MOVE_RIGHT) << ' ' << (current.command == AICommand::MOVE_LEFT) << ' ' << (current.command == AICommand::CLIMB_DOWN) << ' ' << (current.command == AICommand::CLIMB_UP) << ' ' << (current.command == AICommand::FALL) << ' ' << '\n';

    switch (current.command) {
    case AICommand::ASCEND:       
        handleAscend(current, clampedDT);
        break;
    case AICommand::MOVE_LEFT:
    case AICommand::MOVE_RIGHT:   
        handleMove(current, targetDir, length, clampedDT);
        break;
    case AICommand::CLIMB_UP:
    case AICommand::CLIMB_DOWN:   
        handleClimb(current, targetDir, length, clampedDT); 
        break;
    case AICommand::FALL:         
        handleFall(current, clampedDT); 
        break;
    case AICommand::FALL_LEFT:
    case AICommand::FALL_RIGHT:   
        handleLeap(current, clampedDT); 
        break;
    case AICommand::TRANSPORT:    
        handleTransport(current); 
        return; 
        break;
    }
}

void Follower::handleAscend(PathStep& step, int dt) {
    // Absolute X-lock to prevent any side-to-side jitter during the rise
    velocity.x = 0;

    float liftSpeed = 0.25f;
    position.y -= liftSpeed * (float)dt;

    cout << position.y << ' ' << step.targetPoint.y << '\n';

    // Check arrival at the "peak"
    if (position.y <= step.targetPoint.y) {
        position.y = step.targetPoint.y;
        velocity.y = 0; // Prepare for the transition to FALL_LEFT/RIGHT
        pathSequence.erase(pathSequence.begin());
    }
}

void Follower::handleMove(PathStep& step, glm::vec2 dir, float len, int dt) {
    // --- 1. SAFE NORMALIZATION ---
    // If len is 0, glm::normalize(dir) returns NaN. 
    // We check for a small threshold (0.001f) to be safe.
    if (len > 0.001f) {
        dir = glm::normalize(dir);
    }
    else {
        dir = glm::vec2(0.0f, 0.0f);
    }

    // --- 2. ACCELERATION ---
    velocity.x += 0.005f * dir.x * (float)dt;
    velocity.y += 0.005f * dir.y * (float)dt;

    // --- 3. SAFE VELOCITY CAP ---
    float velLen = glm::length(velocity);
    if (velLen > MAX_VEL && velLen > 0.001f) {
        // Manually normalize to avoid glm::normalize double-check
        velocity = (velocity / velLen) * MAX_VEL;
    }

    // --- 4. APPLY POSITION ---
    position += velocity * (float)dt;

    // --- 5. ARRIVAL CHECK ---
    if (len < 2.0f) {
        velocity = glm::vec2(0.0f, 0.0f);
        pathSequence.erase(pathSequence.begin());
    }
}

void Follower::handleClimb(PathStep& step, glm::vec2 dir, float len, int dt) {
    // First, move horizontally toward the vine if not aligned
    float targetX = step.targetPoint.x;
    float xDiff = targetX - position.x;

    if (abs(xDiff) > 2.0f) {
        // Not yet aligned - move horizontally first
        float moveSpeed = 0.1f;
        position.x += (xDiff > 0 ? moveSpeed : -moveSpeed) * (float)dt;
        return; // Don't climb until aligned
    }

    // Now aligned - snap and climb
    position.x = targetX;
    velocity.x = 0.0f;

    float climbSpeed = 0.08f;
    velocity.y = (step.command == AICommand::CLIMB_DOWN) ? climbSpeed : -climbSpeed;
    position.y += velocity.y * (float)dt;

    if (abs(position.y - step.targetPoint.y) < 2.0f) {
        position.y = step.targetPoint.y;
        velocity.y = 0.0f;
        pathSequence.erase(pathSequence.begin());
    }
}

void Follower::handleLeap(PathStep& step, int dt) {
    int floorY;
    
    if (velocity.y >= 0 && map->collisionMoveDown(glm::ivec2(position.x + 8, position.y + 1), glm::ivec2(16, 32), &floorY))
    {
        velocity = glm::vec2(0, 0);
        pathSequence.erase(pathSequence.begin());
        return;
    }

    float power = 0.3f;
    if (abs(velocity.x) < 0.01f) {
        velocity.x = (step.command == AICommand::FALL_RIGHT) ? power : -power;
    }

    velocity.y += 0.0025f * (float)dt;
    if (velocity.y > 0.6f) velocity.y = 0.6f;

    // PREDICT next position first
    glm::vec2 nextPos = position + velocity * (float)dt;

    position = nextPos;
}

void Follower::handleFall(PathStep& step, int dt) {
    int floorY;
    velocity.x = 0;
    cout << 'a' << '\n';
    // Check if ALREADY grounded before applying any fall logic
    if (map->collisionMoveDown(glm::ivec2(position.x + 8, position.y + 1), glm::ivec2(16, 32), &floorY)) {
        // Already on ground - skip this fall step entirely
        velocity.y = 0.0f;
        pathSequence.erase(pathSequence.begin());
        return;
    }

    // Not grounded - apply gravity
    velocity.y += 0.0015f * (float)dt;
    if (velocity.y > 0.5f) velocity.y = 0.5f;

    glm::vec2 nextPos = position + velocity * (float)dt;

    if (map->collisionMoveDown(glm::ivec2(nextPos.x + 8, nextPos.y), glm::ivec2(16, 32), &floorY)) {
        this->position.y = (float)floorY;
        velocity = glm::vec2(0, 0);
        pathSequence.erase(pathSequence.begin());
        return;
    }

    position.y = nextPos.y;
}

void Follower::handleTransport(PathStep& step) {
    this->setPosition(step.targetPoint);
    this->velocity = glm::vec2(0,0);
    pathSequence.erase(pathSequence.begin());
    return;
}