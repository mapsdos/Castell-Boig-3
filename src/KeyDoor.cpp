#include "KeyDoor.h"

void KeyDoor::init(const glm::vec2& pos, ShaderProgram& program) {
    position = pos;
    kind = DoorType::KEYDOOR;

    spritesheet.loadFromFile("assets/images/lockedDoorClosed.png", TEXTURE_PIXEL_FORMAT_RGBA);

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