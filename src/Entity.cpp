#include "Entity.h"
#include <iostream>

Entity::~Entity() {
    if (sprite != nullptr) delete sprite;
}

void Entity::setPosition(const glm::vec2& pos)
{
    position = pos;
    if (sprite != NULL) {
        // Match Player logic: Pixel Pos + Screen Offset
        sprite->setPosition(glm::vec2(tileMapDispl.x + position.x, tileMapDispl.y + position.y));
    }
}