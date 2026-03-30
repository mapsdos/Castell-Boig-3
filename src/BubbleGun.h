#ifndef _BUBBLEGUN_INCLUDE
#define _BUBBLEGUN_INCLUDE

#include "Entity.h"

class BubbleGun : public Entity
{
public:
	void init(const glm::vec2& pos, ShaderProgram& program) override;
	void update(int deltaTime) override;
	void render(const glm::mat4& modelview) override;
};

#endif