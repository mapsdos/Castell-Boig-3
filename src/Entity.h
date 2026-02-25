#ifndef _ENTITY_INCLUDE
#define _ENTITY_INCLUDE

#include <glm/glm.hpp>
#include "Sprite.h"
#include "ShaderProgram.h"

class Entity {
public:
    virtual ~Entity(); // Always need a virtual destructor for abstract classes
    virtual void init(const glm::vec2& pos, ShaderProgram& program) = 0;
    virtual void update(int deltaTime) = 0;
    virtual void render(const glm::mat4& modelview) = 0;

    glm::vec2 getPosition() const;

protected:
    glm::vec2 position;
    Sprite* sprite;
    Texture spritesheet;
};

#endif