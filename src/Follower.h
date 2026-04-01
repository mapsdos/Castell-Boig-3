#ifndef _FOLLOWER_INCLUDE
#define _FOLLOWER_INCLUDE

#include "Enemy.h"
#include "Weight.h"

enum FollowerAnims {
	PLANKTON_STAND_RIGHT, PLANKTON_STAND_LEFT,
	PLANKTON_WALK_RIGHT, PLANKTON_WALK_LEFT,
	PLANKTON_STAIRS
};

class Follower : public Enemy {
public:
    Entity* clone(ShaderProgram&) const override;
public:
	void init(const glm::vec2& pos, ShaderProgram& program) override;
	void update(int deltaTime) override;
	void update(int deltaTime, const glm::vec2& playerPos, const std::vector<Weight*> weights);
	void followPath(int deltaTime);

private:

	void handleMove(PathStep& step, glm::vec2 dir, float len, int dt);
	void handleClimb(PathStep& step, glm::vec2 dir, float len, int dt);
	void handleFall(PathStep& step, int dt);
	void handleTransport(PathStep& step);
	void updateAnimation();
	void handleAscend(PathStep& step, int dt);
	void handleLeap(PathStep& step, int dt);

	int timer;
	bool isClimbing = false;
	bool lastMoveRight = true;
	std::vector<PathStep> pathSequence;
};

#endif