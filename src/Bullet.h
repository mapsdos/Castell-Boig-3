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
	
	glm::ivec2 getSize() const { return bulletSize; }

private:
	float speed = 0.15f;
	bool dirRight;
	string bulletSpritePath;
	glm::ivec2 bulletSize;
};

#endif