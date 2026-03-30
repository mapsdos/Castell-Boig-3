#ifndef _DOOR_INCLUDE
#define _DOOR_INCLUDE

#include "Entity.h"

class LevelScene;
enum DoorType {OPENDOOR = 0, KEYDOOR = 1};

class Door : public Entity
{
public:
    // Ensure these signatures match Entity.h exactly
    void init(const glm::vec2& pos, ShaderProgram& program) override;
    void update(int deltaTime) override;
    void render(const glm::mat4& modelview) override;

    void setRoom(LevelScene* setRoom);
    LevelScene* getRoom() const;

    void setOpened(bool open) { isOpen = open; }
    bool isOpened() const { return isOpen; }
    DoorType getKind() { return kind; };
    
protected:
    bool isOpen = false;
    Sprite* opened;
    Texture spritesheetOpened;
    LevelScene* room;
    DoorType kind;
};

#endif