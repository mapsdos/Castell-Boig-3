#ifndef _INSTRUCTIONSCENE_INCLUDE
#define _INSTRUCTIONSCENE_INCLUDE

#include "Scene.h"

class InstructionScene : public Scene
{
public:
	InstructionScene();
	~InstructionScene();

	void init() override;
	void update(int deltaTime) override;
	void render() override;

private:
	Texture backgroundTexture;
	Sprite* backgroundSprite = nullptr;

};


#endif