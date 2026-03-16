#ifndef _ENEMY_INCLUDE
#define _ENEMY_INCLUDE

#include "Entity.h"
#include "TileMap.h"

class Enemy : public Entity {
public:
    virtual void init(const glm::vec2& pos, ShaderProgram& program);
    virtual void update(int deltaTime) = 0; // Pure virtual: subclasses must define AI
    void render(const glm::mat4& modelview) override;

    void setTileMap(TileMap* tileMap) { map = tileMap; }

protected:
    TileMap* map;
    glm::ivec2 startPosition;
    bool moveRight = false;
};

#endif