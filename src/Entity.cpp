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
        cout << "1: " << this << ' ' << tileMapDispl.x + position.x << ' ' << tileMapDispl.y + position.y << '\n';
        cout << "2: " << tileMapDispl.x << ' ' << tileMapDispl.y << '\n';
        cout << "3: " << position.x << ' ' << position.y << '\n';
    }
}