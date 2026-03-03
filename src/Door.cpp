#include "Door.h"

// In Key.cpp
void Door::init(const glm::vec2& pos, ShaderProgram& program) {
    position = pos;

    spritesheet.loadFromFile("assets/images/doorClosed.png", TEXTURE_PIXEL_FORMAT_RGBA);

    // 1.0f, 1.0f uses the whole image as one frame
    sprite = Sprite::createSprite(glm::vec2(32, 32), glm::vec2(1.0f, 1.0f), &spritesheet, &program);
    sprite->setPosition(position);

    // Define the animation state
    sprite->setNumberAnimations(1);
    sprite->setAnimationSpeed(0, 1);       // Prevent the infinite loop crash
    sprite->addKeyframe(0, glm::vec2(0.f, 0.f)); // Start at top-left of the image
    sprite->changeAnimation(0);            // Activate the animation

    spritesheetOpened.loadFromFile("assets/images/doorOpened.png", TEXTURE_PIXEL_FORMAT_RGBA);

    // 1.0f, 1.0f uses the whole image as one frame
    opened = Sprite::createSprite(glm::vec2(32, 32), glm::vec2(1.0f, 1.0f), &spritesheetOpened, &program);
    opened->setPosition(position);

    // Define the animation state
    opened->setNumberAnimations(1);
    opened->setAnimationSpeed(0, 1);       // Prevent the infinite loop crash
    opened->addKeyframe(0, glm::vec2(0.f, 0.f)); // Start at top-left of the image
    opened->changeAnimation(0);            // Activate the animation
}

void Door::update(int deltaTime) {
    // If you want the key to rotate or bob, do it here
    sprite->update(deltaTime);
}

void Door::render(const glm::mat4& modelview)
{
    if (isOpen && opened != nullptr) {
        opened->render(modelview); // Render the opened sprite
    }
    else {
        sprite->render(modelview); // Render the closed sprite (from Entity)
    }
}

void Door::setRoom(LevelScene* setRoom)
{
    room = setRoom;
}

LevelScene* Door::getRoom() const
{
    return room;
}