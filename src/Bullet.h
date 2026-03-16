#ifndef _BULLET_INCLUDE
#define _BULLET_INCLUDE

#include "Entity.h"

class Bullet : public Entity
{
public:
	void init(const glm::vec2& pos, ShaderProgram& program) override;
	void init(const glm::vec2& pos, ShaderProgram& program, bool moveRight);
	void update(int deltaTime) override;
	void render(const glm::mat4& modelview) override;

private:
	float speed = 0.2f;
	bool dirRight;
};

#endif