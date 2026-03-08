#include "Entity.h"

Entity::~Entity() {
    if (sprite != nullptr) delete sprite;
}

glm::vec2 Entity::getPosition() const {
    return position;
}

void Entity::setPosition(glm::vec2 pos)
{
    position = pos;
}