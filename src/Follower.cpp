#include "Follower.h"
#include "ShaderProgram.h"
Entity* Follower::clone(ShaderProgram&) const { return nullptr; }

#include <iostream>
#include <algorithm>

void Follower::init(const glm::vec2& pos, ShaderProgram& program) {
    Enemy::init(pos, program);
    position = pos;
    timer = 0;
    isClimbing = false;
    lastMoveRight = true;

    // Plankton spritesheet: 10 frames of 11x15 pixels
    float frameWidth = 11.0f;
    float frameHeight = 15.0f;
    int numFrames = 10;
    float frameWidthUV = 1.0f / numFrames;
    float frameHeightUV = 1.0f;

    // Display size (about half of player 24x32, slightly bigger)
    spriteWidth = 16;
    spriteHeight = 20;

    spritesheet.loadFromFile("assets/images/enemies/plankton/plankton.png", TEXTURE_PIXEL_FORMAT_RGBA);
    sprite = Sprite::createSprite(
        glm::ivec2(spriteWidth, spriteHeight),
        glm::vec2(frameWidthUV, frameHeightUV),
        &spritesheet,
        &program
    );

    sprite->setNumberAnimations(5);

    // PLANKTON_STAND_RIGHT (frame 0)
    sprite->setAnimationSpeed(PLANKTON_STAND_RIGHT, 8);
    sprite->addKeyframe(PLANKTON_STAND_RIGHT, glm::vec2(0.0f * frameWidthUV, 0.0f));

    // PLANKTON_STAND_LEFT (frame 1)
    sprite->setAnimationSpeed(PLANKTON_STAND_LEFT, 8);
    sprite->addKeyframe(PLANKTON_STAND_LEFT, glm::vec2(1.0f * frameWidthUV, 0.0f));

    // PLANKTON_WALK_RIGHT (frames 2,3,4)
    sprite->setAnimationSpeed(PLANKTON_WALK_RIGHT, 8);
    sprite->addKeyframe(PLANKTON_WALK_RIGHT, glm::vec2(2.0f * frameWidthUV, 0.0f));
    sprite->addKeyframe(PLANKTON_WALK_RIGHT, glm::vec2(3.0f * frameWidthUV, 0.0f));
    sprite->addKeyframe(PLANKTON_WALK_RIGHT, glm::vec2(4.0f * frameWidthUV, 0.0f));

    // PLANKTON_WALK_LEFT (frames 5,6,7)
    sprite->setAnimationSpeed(PLANKTON_WALK_LEFT, 8);
    sprite->addKeyframe(PLANKTON_WALK_LEFT, glm::vec2(5.0f * frameWidthUV, 0.0f));
    sprite->addKeyframe(PLANKTON_WALK_LEFT, glm::vec2(6.0f * frameWidthUV, 0.0f));
    sprite->addKeyframe(PLANKTON_WALK_LEFT, glm::vec2(7.0f * frameWidthUV, 0.0f));

    // PLANKTON_STAIRS (frames 8,9)
    sprite->setAnimationSpeed(PLANKTON_STAIRS, 8);
    sprite->addKeyframe(PLANKTON_STAIRS, glm::vec2(8.0f * frameWidthUV, 0.0f));
    sprite->addKeyframe(PLANKTON_STAIRS, glm::vec2(9.0f * frameWidthUV, 0.0f));

    sprite->changeAnimation(PLANKTON_STAND_RIGHT);
}

void Follower::updateAnimation() {
    int targetAnim;
    
    if (isClimbing) {
        targetAnim = PLANKTON_STAIRS;
    }
    else if (velocity.x > 0.01f) {
        lastMoveRight = true;
        targetAnim = PLANKTON_WALK_RIGHT;
    }
    else if (velocity.x < -0.01f) {
        lastMoveRight = false;
        targetAnim = PLANKTON_WALK_LEFT;
    }
    else {
        targetAnim = lastMoveRight ? PLANKTON_STAND_RIGHT : PLANKTON_STAND_LEFT;
    }
    
    if (sprite->animation() != targetAnim) {
        sprite->changeAnimation(targetAnim);
    }
}

void Follower::update(int deltaTime) {
    // Empty - LevelScene calls the version with playerPos
}

void Follower::update(int deltaTime, const glm::vec2& playerPos, const std::vector<Weight*> weights) {
    // PATH MANAGEMENT: Update path every 500ms
    timer -= deltaTime;
    if (pathSequence.empty() || (timer <= 0 && pathSequence.begin()->command != AICommand::ASCEND && pathSequence.begin()->command != AICommand::FALL_RIGHT && pathSequence.begin()->command != AICommand::FALL_LEFT && pathSequence.begin()->command != AICommand::FALL)) {
        // Use centers for pathfinding logic
        glm::vec2 feetPos = glm::vec2(position.x + spriteWidth / 2, position.y + spriteHeight - 8);
        glm::vec2 playerFeet = glm::vec2(playerPos.x + 16, playerPos.y + 16);

        pathSequence = map->getPath(feetPos, playerFeet, weights);
        timer = 500;
    }

    // Update the Brain
    updateFSM(playerPos, pathSequence);

    // Execute Behavior
    if (currentState == EnemyState::TRACK) {
        if (!pathSequence.empty()) {
            followPath(deltaTime);
        }
    }
    else if (currentState == EnemyState::EXPLORE) {
        velocity *= 0.9f;
        position += velocity * (float)deltaTime;
    }

    this->setPosition(position);
    updateAnimation();
    sprite->update(deltaTime);
}

void Follower::followPath(int deltaTime) {
    if (pathSequence.empty()) return;

    PathStep& current = pathSequence.front();
    glm::vec2 targetDir = current.targetPoint - this->position;
    float length = glm::length(targetDir);
    int clampedDT = std::min<int>(deltaTime, 33);

    // Check if we're climbing
    isClimbing = (current.command == AICommand::CLIMB_UP || current.command == AICommand::CLIMB_DOWN);

    switch (current.command) {
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
    case AICommand::TRANSPORT:    
        handleTransport(current); 
        return;
    case AICommand::ASCEND:
        handleAscend(current, clampedDT);
	    break;
    case AICommand::FALL_LEFT:
    case AICommand::FALL_RIGHT:
        handleLeap(current, clampedDT);
        break;
    default:
        break;
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
    isClimbing = false;
    
    if (len > 0.001f) {
        dir = glm::normalize(dir);
    }
    else {
        dir = glm::vec2(0.0f, 0.0f);
    }

    // Only move horizontally - no vertical movement during walk
    velocity.x += 0.005f * dir.x * (float)dt;

    float velLen = abs(velocity.x);
    if (velLen > MAX_VEL) {
        velocity.x = (velocity.x > 0 ? MAX_VEL : -MAX_VEL);
    }

    float newX = position.x + velocity.x * (float)dt;
    
    // Check wall collision before moving
    bool blocked = false;
    if (velocity.x < 0) {
        // Moving left - check left edge
        int tileLeft = map->getTileIdAt(glm::ivec2((int)newX, (int)(position.y + spriteHeight / 2)));
        if (tileLeft == 1) blocked = true;
    } else if (velocity.x > 0) {
        // Moving right - check right edge
        int tileRight = map->getTileIdAt(glm::ivec2((int)(newX + spriteWidth), (int)(position.y + spriteHeight / 2)));
        if (tileRight == 1) blocked = true;
    }
    
    if (!blocked) {
        position.x = newX;
    } else {
        velocity.x = 0;
        // Skip this step if blocked
        pathSequence.erase(pathSequence.begin());
        return;
    }

    // Check if reached target horizontally
    float xDist = abs(position.x - step.targetPoint.x);
    if (xDist < 2.0f) {
        velocity = glm::vec2(0.0f, 0.0f);
        pathSequence.erase(pathSequence.begin());
    }
}

void Follower::handleClimb(PathStep& step, glm::vec2 dir, float len, int dt) {
    isClimbing = true;
    
    float targetX = step.targetPoint.x;
    float xDiff = targetX - position.x;

    if (abs(xDiff) > 2.0f) {
        float moveSpeed = 0.1f;
        position.x += (xDiff > 0 ? moveSpeed : -moveSpeed) * (float)dt;
        return;
    }

    position.x = targetX;
    velocity.x = 0.0f;

    float climbSpeed = 0.08f;
    velocity.y = (step.command == AICommand::CLIMB_DOWN) ? climbSpeed : -climbSpeed;
    position.y += velocity.y * (float)dt;

    if (abs(position.y - step.targetPoint.y) < 2.0f) {
        position.y = step.targetPoint.y;
        velocity.y = 0.0f;
        pathSequence.erase(pathSequence.begin());
        
        // Check if next command is also climb - keep isClimbing true to avoid animation flicker
        if (!pathSequence.empty()) {
            AICommand nextCmd = pathSequence.front().command;
            if (nextCmd != AICommand::CLIMB_UP && nextCmd != AICommand::CLIMB_DOWN) {
                isClimbing = false;
            }
        } else {
            isClimbing = false;
        }
    }
}

void Follower::handleFall(PathStep& step, int dt) {
    int floorY;
    velocity.x = 0;
    isClimbing = false;
    
    if (map->collisionMoveDown(glm::ivec2(position.x + spriteWidth/4, position.y + 1), glm::ivec2(spriteWidth/2, spriteHeight), &floorY)) {
        velocity.y = 0.0f;
        pathSequence.erase(pathSequence.begin());
        return;
    }

    velocity.y += 0.0015f * (float)dt;
    if (velocity.y > 0.5f) velocity.y = 0.5f;

    glm::vec2 nextPos = position + velocity * (float)dt;

    if (map->collisionMoveDown(glm::ivec2(nextPos.x + spriteWidth/4, nextPos.y), glm::ivec2(spriteWidth/2, spriteHeight), &floorY)) {
        this->position.y = (float)floorY;
        velocity = glm::vec2(0, 0);
        pathSequence.erase(pathSequence.begin());
        return;
    }

    position.y = nextPos.y;
}

void Follower::handleTransport(PathStep& step) {
    isClimbing = false;
    // targetPoint.y is calculated for 32px sprites (offset -16 from tile position)
    // For Plankton (20px), we need to adjust: targetY = tileY + tileSize - spriteHeight
    // Since targetPoint.y = tileY - 16 (for 32px), we add back (32 - spriteHeight) = (32 - 20) = 12
    glm::vec2 adjustedPos = step.targetPoint;
    adjustedPos.y = step.targetPoint.y + (32 - spriteHeight);
    this->setPosition(adjustedPos);
    this->velocity = glm::vec2(0,0);
    pathSequence.erase(pathSequence.begin());
}
