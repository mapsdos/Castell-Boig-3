#ifndef _KEY_INCLUDE
#define _KEY_INCLUDE

#include "Entity.h"

class Key : public Entity {
public:
    // Key();
    // ~Key();
    // Key(Key const &)
    // Key(Key&&)
    // Key operator=(Key const &) = delete;
    // Key operator=(Key &&)
    // 
    // Ensure these signatures match Entity.h exactly
    void init(const glm::vec2& pos, ShaderProgram& program) override;
    void update(int deltaTime) override;
    void render(const glm::mat4& modelview) override;
};

#endif