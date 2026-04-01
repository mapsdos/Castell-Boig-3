#ifndef _BOMB_INCLUDE
#define _BOMB_INCLUDE

#include "Entity.h"

class Bomb : public Entity
{
public: 
	Entity* clone(ShaderProgram& program) const override;
	void init(const glm::vec2& pos, ShaderProgram& program) override;
	void update(int deltaTime) override;
	void render(const glm::mat4& modelview) override;

	void planted() { isPlanted = true; };

private:
	bool isPlanted;
	Sprite* spriteOn;
	Texture spritesheetOn;
};

#endif