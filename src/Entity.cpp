#include "Entity.h"

Entity::~Entity() {
    if (sprite != nullptr) delete sprite;
}
