#include "InstructionsScene.h"

#include <glm/gtc/matrix_transform.hpp>

InstructionScene::InstructionScene()
{
    // You can initialize pointers to null here if you didn't in the header
    backgroundSprite = nullptr;
}

InstructionScene::~InstructionScene()
{
	// Check if the pointer exists before deleting
	if (backgroundSprite != nullptr)
	{
		delete backgroundSprite;
		backgroundSprite = nullptr; // Good practice to nullify after deletion
	}
}

void InstructionScene::init()
{
    // 1. CRITICAL: Initialize the shaders from the base class
    initShaders();

    // 2. CRITICAL: Setup the 2D coordinate system
    // If your game uses 640x480 pixels, use this:
    projection = glm::ortho(0.f, 640.f, 480.f, 0.f);
    // (If you want to use the 80x60 units from the menu, use those numbers instead)

    // 3. Load the texture and check if it actually worked
    backgroundTexture.loadFromFile("assets/images/InstructionBackground.png", TEXTURE_PIXEL_FORMAT_RGBA);

    // 4. Create the sprite
    // Match the size to your projection (640, 480)
    backgroundSprite = Sprite::createSprite(glm::ivec2(640, 480), glm::vec2(1.f, 1.f), &backgroundTexture, &texProgram);

    backgroundSprite->setNumberAnimations(1);
    backgroundSprite->addKeyframe(0, glm::vec2(0.f, 0.f));
    backgroundSprite->changeAnimation(0);
    backgroundSprite->setPosition(glm::vec2(0.f, 0.f));
}

void InstructionScene::update(int deltaTime)
{
	currentTime += deltaTime;
}

void InstructionScene::render()
{
    glm::mat4 modelview;

    // 1. Tell OpenGL to use your texture shader
    texProgram.use();

    // 2. Pass the Projection Matrix to the shader (the 80x60 space)
    texProgram.setUniformMatrix4f("projection", projection);

    // 3. Set Modelview to Identity (No camera movement/rotation for a static screen)
    modelview = glm::mat4(1.0f);
    texProgram.setUniformMatrix4f("modelview", modelview);

    // 4. Set the base color (White at 100% opacity so the texture looks normal)
    texProgram.setUniform4f("color", 1.0f, 1.0f, 1.0f, 1.0f);

    // 5. Draw the background
    if (backgroundSprite != nullptr)
    {
        backgroundSprite->render(modelview);
    }
}