#ifndef _SHOOTER_INCLUDE
#define _SHOOTER_INCLUDE

#include "Enemy.h"
#include "Bullet.h"

class Shooter : public Enemy
{
public:
	void init(const glm::vec2& pos, ShaderProgram& program) override;
	void update(int deltaTime) override;
	void render(const glm::mat4& modelview) override;

	void update(int deltaTime, const std::vector<Weight*> weights);

	void Shoot(int deltaTime, ShaderProgram& program);

private:
	int movementTimer = 1000;
	bool isIdle = false;
	int shotTimer = 2000;
	std::vector<Bullet*> bullets;
};

#endif