#ifndef _FOLLOWER_INCLUDE
#define _FOLLOWER_INCLUDE

#include "Enemy.h"

class Follower : public Enemy
{
public:
	void init(const glm::vec2& pos, ShaderProgram& program) override;
	void update(int deltaTime) override;
	void update(int deltaTime, const glm::vec2& playerPos);
	void followPath(int deltaTime);
private:

	void handleAscend(PathStep& step, int dt);
	void handleMove(PathStep& step, glm::vec2 dir, float len, int dt);
	void handleClimb(PathStep& step, glm::vec2 dir, float len, int dt);
	void handleLeap(PathStep& step, int dt);
	void handleFall(PathStep& step, int dt);
	void handleTransport(PathStep& step);

	int timer;
	std::vector<PathStep> pathSequence;
};

#endif