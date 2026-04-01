#ifndef _EXPLOSION_INCLUDE
#define _EXPLOSION_INCLUDE

#include "Entity.h"

// Explosion.h
class Explosion : public Entity {
public:
    void init(const glm::vec2& pos, ShaderProgram& program) override;
    void init(const glm::vec2& pos, Texture* tex, ShaderProgram& program);
    void update(int deltaTime) override;
    void render(const glm::mat4& modelview) override;
    bool isFinished() const { return finished; }

    Entity* clone(ShaderProgram& program) const override { return nullptr; } // Not needed for FX

private:
    Sprite* sprite;
    Texture spritesheet;
    int lifeTime;
    bool finished;
};

#endif