#ifndef _ENEMY_INCLUDE
#define _ENEMY_INCLUDE

#include "Entity.h"
#include "TileMap.h"

enum class EnemyState { EXPLORE, TRACK, FLEE, ATTACK };

class Enemy : public Entity {
public:
    virtual void init(const glm::vec2& pos, ShaderProgram& program);
    virtual void update(int deltaTime) = 0; // Pure virtual: subclasses must define AI
    void render(const glm::mat4& modelview) override;

    void setTileMap(TileMap* tileMap) { map = tileMap; }
    void updateFSM(const glm::vec2& playerPos, std::vector<PathStep> pathSequence);

    int getHeight() { return spriteHeight; };
    int getWidth() { return spriteWidth; };

protected:
    TileMap* map;
    EnemyState currentState = EnemyState::EXPLORE;
    glm::vec2 velocity = glm::vec2(0.0f, 0.0f); // Add this!
    float MAX_VEL = 0.1f;
    float detectionRange = 1500.0f;
    bool moveRight = false;
    int spriteWidth;
    int spriteHeight;
};

#endif