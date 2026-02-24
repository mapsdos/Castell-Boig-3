#include <iostream>
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>
#include "MenuScene.h"
#include "Game.h"

#define SCREEN_X 32
#define SCREEN_Y 16

#define INIT_PLAYER_X_TILES 4
#define INIT_PLAYER_Y_TILES 25


MenuScene::MenuScene()
{
    for (int i = 0; i < 3; i++) buttonSprites[i] = nullptr;
}

MenuScene::~MenuScene()
{
    for (int i = 0; i < 3; i++) {
        if (buttonSprites[i] != nullptr) delete buttonSprites[i];
    }
}


void MenuScene::init() {
    initShaders(); // Call the base Scene shader setup
    projection = glm::ortho(0.f, float(SCREEN_WIDTH), float(SCREEN_HEIGHT), 0.f);

    setupButtons();
}

void MenuScene::setupButtons() {
    // 1. Load the spritesheet containing the button graphics
    // Ensure this file exists in your assets folder!
    if (!buttonSheet.loadFromFile("assets/images/menu_buttons.png", TEXTURE_PIXEL_FORMAT_RGBA)) {
        std::cout << "ERROR: Could not load menu_buttons.png!" << std::endl;
    }

    // 2. Define layout constants
    float startX = SCREEN_WIDTH / 2.0f - 64.0f; // Centered (128 width / 2)
    float startY = 150.0f;
    float padding = 80.0f;

    for (int i = 0; i < 3; i++) {
        // We assume the spritesheet has 3 rows (Play, Instructions, Credits)
        // sizeInSpritesheet = vec2(1.0, 0.33) means one full width, 1/3 height
        buttonSprites[i] = Sprite::createSprite(glm::vec2(128, 64), glm::vec2(1.0f, 0.33f), &buttonSheet, &texProgram);

        // Position each button vertically
        buttonSprites[i]->setPosition(glm::vec2(startX, startY + (i * padding)));

        // Select the correct frame from the spritesheet
        buttonSprites[i]->setNumberAnimations(1);
        buttonSprites[i]->addKeyframe(0, glm::vec2(0.0f, i * 0.33f));
        buttonSprites[i]->changeAnimation(0);
    }
}

void MenuScene::update(int deltaTime) {
    if (Game::instance().isMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT))
    {
        glm::ivec2 mPos = Game::instance().getMousePos();

        // Button 1: Play (Example coordinates)
        if (mPos.x >= 250 && mPos.x <= 378 && mPos.y >= 150 && mPos.y <= 214)
        {
            Game::instance().changeState(PLAYING);
        }

        // Button 2: Instructions (Example coordinates)
        if (mPos.x >= 250 && mPos.x <= 378 && mPos.y >= 150 && mPos.y <= 214)
        {
            // Logic to open instructions
        }

        // Button 3: Credits (Example coordinates)
        if (mPos.x >= 250 && mPos.x <= 378 && mPos.y >= 150 && mPos.y <= 214)
        {
            // Logic to open Credits
        }
    }
}

void MenuScene::render()
{
    glm::mat4 modelview;
    texProgram.use();
    texProgram.setUniformMatrix4f("projection", projection);
    texProgram.setUniform4f("color", 1.0f, 1.0f, 1.0f, 1.0f);

    glActiveTexture(GL_TEXTURE0);

    for (int i = 0; i < 3; i++) {
        if (buttonSprites[i] != nullptr) buttonSprites[i]->render(modelview);
    }
}