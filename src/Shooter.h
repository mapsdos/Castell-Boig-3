#ifndef _SHOOTER_INCLUDE
#define _SHOOTER_INCLUDE

#include "Enemy.h"
#include "Bullet.h"

class Shooter : public Enemy {
public:
    Entity* clone(ShaderProgram&) const override;
public:
	void init(const glm::vec2& pos, ShaderProgram& program) override;
	void update(int deltaTime) override;
	void render(const glm::mat4& modelview) override;

	void Shoot(int deltaTime, ShaderProgram& program);
	const std::vector<Bullet*>& getBullets() const { return bullets; }

private:
	int movementTimer = 1000;
	bool isIdle = false;
	int shotTimer = 2000;
	std::vector<Bullet*> bullets;
};

#endif