#ifndef _SCENE_INCLUDE
#define _SCENE_INCLUDE


#include <glm/glm.hpp>
#include "Texture.h"
#include "Sprite.h"
#include "ShaderProgram.h"


// Scene contains all the entities of our game.
// It is responsible for updating and render them.


class Scene
{

public:
	Scene();
	virtual ~Scene();

	virtual void init();
	virtual void update(int deltaTime);
	virtual void render();

protected:
	void initShaders();


protected:
	float currentTime;
	glm::mat4 projection;
	ShaderProgram texProgram;

};


#endif // _SCENE_INCLUDE

