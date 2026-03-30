#ifndef _KEYDOOR_INCLUDE
#define _KEYDOOR_INCLUDE

#include "Door.h"

class KeyDoor : public Door
{
public:
	void init(const glm::vec2& pos, ShaderProgram& program);
};

#endif