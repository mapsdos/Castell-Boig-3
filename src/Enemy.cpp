#include "Enemy.h"

void Enemy::init(const glm::vec2& pos, ShaderProgram& program)
{
    tileMapDispl = pos;
    // We don't load a specific spritesheet here because 
    // each enemy type (Patroller vs Chaser) will have its own look.
}

void Enemy::render(const glm::mat4& modelview)
{
    // All enemies will use the standard Sprite render logic
    if (sprite != NULL)
        sprite->render(modelview);
}

void Enemy::updateFSM(const glm::vec2& playerPos, std::vector<PathStep> pathSequence) {
    float dist = 0.0f;
    for (auto step : pathSequence)
    {
        dist += step.distance;
    }

    switch (currentState) {
    case EnemyState::EXPLORE:
        if (dist < detectionRange) currentState = EnemyState::TRACK;
        break;
    case EnemyState::TRACK:
        if (dist > detectionRange * 1.5f) currentState = EnemyState::EXPLORE; // Explore
        if (dist < 32.0f) currentState = EnemyState::TRACK; // Attack
        break;
    case EnemyState::ATTACK:
        if (dist > 40.0f) currentState = EnemyState::TRACK;
        break;
    }
}