#ifndef _WEIGHT_INCLUDE
#define _WEIGHT_INCLUDE

#include "Entity.h"
#include "TileMap.h"

class Weight : public Entity

{
public:
	void init(const glm::vec2& pos, ShaderProgram& program) override;
	void update(int deltaTime) override;
	void render(const glm::mat4& modelview) override;

	void push(float amount);
	void setTileMap(TileMap* tileMap) { map = tileMap; }

private:
	TileMap* map;
	float fallVelocity;
};

#endif