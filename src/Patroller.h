#ifndef _PATROLER_INCLUDE
#define _PATROLER_INCLUDE

#include "Enemy.h"

class Patroller : public Enemy {
public:
    Entity* clone(ShaderProgram&) const override;
public:
	void init(const glm::vec2& pos, ShaderProgram& program) override;
	void update(int deltaTime) override;

private:
	int movementTimer = 1000;
	bool isIdle = false;
};

#endif