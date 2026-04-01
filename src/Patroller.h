#ifndef _PATROLER_INCLUDE
#define _PATROLER_INCLUDE

#include "Enemy.h"

enum PatrollerAnims {
	PATRICK_STAND_RIGHT, PATRICK_STAND_LEFT,
	PATRICK_WALK_RIGHT, PATRICK_WALK_LEFT
};

class Patroller : public Enemy {
public:
    Entity* clone(ShaderProgram&) const override;
public:
	void init(const glm::vec2& pos, ShaderProgram& program) override;
	void update(int deltaTime) override;
	void update(int deltaTime, const std::vector<Weight*> weights);

private:
	int movementTimer = 1000;
	bool isIdle = false;
	int spriteWidth = 28;
	int spriteHeight = 60;
	void updateAnimation();
};

#endif