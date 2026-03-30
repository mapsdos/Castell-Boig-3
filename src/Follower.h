#ifndef _FOLLOWER_INCLUDE
#define _FOLLOWER_INCLUDE

#include "Enemy.h"

class Follower : public Enemy {
public:
    Entity* clone(ShaderProgram&) const override;
public:
	void init(const glm::vec2& pos, ShaderProgram& program) override;
	void update(int deltaTime) override;
	void update(int deltaTime, const glm::vec2& playerPos);
	void followPath(int deltaTime);
private:
	int timer;
	std::vector<PathStep> pathSequence;
};

#endif