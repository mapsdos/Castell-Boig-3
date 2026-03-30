#ifndef _CLOCK_INCLUDE
#define _CLOCK_INCLUDE

#include "Entity.h"

class Clock : public Entity
{
public:
	void init(const glm::vec2& pos, ShaderProgram& program) override;
	void update(int deltaTime) override;
	void render(const glm::mat4& modelview) override;
};

#endif