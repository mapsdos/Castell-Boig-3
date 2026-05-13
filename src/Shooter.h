#ifndef _SHOOTER_INCLUDE
#define _SHOOTER_INCLUDE

#include "Enemy.h"
#include "Bullet.h"

enum ShooterAnims {
	SQUIDWARD_STAND_RIGHT, SQUIDWARD_STAND_LEFT,
	SQUIDWARD_WALK_RIGHT, SQUIDWARD_WALK_LEFT,
	SQUIDWARD_SHOOT_RIGHT, SQUIDWARD_SHOOT_LEFT
};

class Shooter : public Enemy {
public:
    Entity* clone(ShaderProgram&) const override;
public:
	void init(const glm::vec2& pos, ShaderProgram& program) override;
	void update(int deltaTime) override;
	void render(const glm::mat4& modelview) override;

	void update(int deltaTime, const std::vector<Weight*> weights);

	void Shoot(int deltaTime, ShaderProgram& program);
	std::vector<Bullet*>& getBullets() { return bullets; }
	const std::vector<Bullet*>& getBullets() const { return bullets; };

private:
	void updateAnimation();
	int movementTimer = 1000;
	bool isIdle = false;
	int shotTimer = 2000;
	bool isShooting = false;
	int shootAnimTimer = 0;
	std::vector<Bullet*> bullets;
};

#endif