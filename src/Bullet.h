#ifndef _BULLET_INCLUDE
#define _BULLET_INCLUDE

#include "Entity.h"

class Bullet : public Entity
{
public:
	Entity* clone(ShaderProgram& program) const override;
	void init(const glm::vec2& pos, ShaderProgram& program) override;
	void init(const glm::vec2& pos, ShaderProgram& program, bool moveRight, const string& spritePath = "assets/images/money.png");
	void update(int deltaTime) override;
	void render(const glm::mat4& modelview) override;
	bool isMovingRight() const { return dirRight; }

private:
	float speed = 0.2f;
	bool dirRight;
	string bulletSpritePath;
};

#endif