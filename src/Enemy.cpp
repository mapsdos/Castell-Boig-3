#include "Enemy.h"

void Enemy::init(const glm::vec2& pos, ShaderProgram& program)
{
    position = pos;
    startPosition = pos;
    // We don't load a specific spritesheet here because 
    // each enemy type (Patroller vs Chaser) will have its own look.
}

void Enemy::render(const glm::mat4& modelview)
{
    // All enemies will use the standard Sprite render logic
    if (sprite != NULL)
        sprite->render(modelview);
}