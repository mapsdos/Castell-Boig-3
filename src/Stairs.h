#ifndef _STAIRS_INCLUDE
#define _STAIRS_INCLUDE

#include "Entity.h"

class Stairs : public Entity {
public:
    Entity* clone(ShaderProgram&) const override;
public:
    void init(const glm::vec2& pos, ShaderProgram& program) override;
    void update(int deltaTime) override;
    void render(const glm::mat4& modelview) override;

    void setDestination(const glm::vec2& dest);
    glm::vec2 getDestination() const;

private:
    glm::vec2 destination;
};

#endif