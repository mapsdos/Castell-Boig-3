#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <queue>
#include <limits>
#include "TileMap.h"

struct AStarNode {
	int index;
	float fCost; // gCost + hCost

	// Priority queue is a max-heap, so we flip the comparison to make it a min-heap
	bool operator>(const AStarNode& other) const {
		return fCost > other.fCost;
	}
};


TileMap *TileMap::createTileMap(const string &levelFile, const glm::vec2 &minCoords, ShaderProgram &program)
{
	TileMap *map = new TileMap(levelFile, minCoords, program);
	
	return map;
}


TileMap::TileMap(const string &levelFile, const glm::vec2 &minCoords, ShaderProgram &program)
{
	loadLevel(levelFile);
	prepareArrays(minCoords, program);
}

TileMap::~TileMap()
{
	if(map != NULL)
		delete [] map;
	free();
}


void TileMap::render() const
{
	glEnable(GL_TEXTURE_2D);
	tilesheet.use();
	glBindVertexArray(vao);
	glEnableVertexAttribArray(posLocation);
	glEnableVertexAttribArray(texCoordLocation);
	glDrawArrays(GL_TRIANGLES, 0, 6 * nTiles);
	glDisable(GL_TEXTURE_2D);
}

void TileMap::free()
{
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vbo);
}

bool TileMap::loadLevel(const string &levelFile)
{
	ifstream fin;
	string line, tilesheetFile;
	stringstream sstream;
	char tile;
	
	fin.open(levelFile.c_str());
	if(!fin.is_open())
		return false;
	getline(fin, line);
	if(line.compare(0, 7, "TILEMAP") != 0)
		return false;
	getline(fin, line);
	sstream.str(line);
	sstream >> mapSize.x >> mapSize.y;
	getline(fin, line);
	sstream.str(line);
	sstream >> tileSize >> blockSize;
	getline(fin, line);
	sstream.str(line);
	sstream >> tilesheetFile;
	tilesheet.loadFromFile(tilesheetFile, TEXTURE_PIXEL_FORMAT_RGBA);
	tilesheet.setWrapS(GL_CLAMP_TO_EDGE);
	tilesheet.setWrapT(GL_CLAMP_TO_EDGE);
	tilesheet.setMinFilter(GL_NEAREST);
	tilesheet.setMagFilter(GL_NEAREST);
	getline(fin, line);
	sstream.str(line);
	int numDoors;
	sstream >> numDoors;
	// Load the room file. Store in a vector.
	for (int i = 0; i < numDoors; i++)
	{
		getline(fin, line);
		sstream.str(line);
		string s;
		sstream >> s;
		roomFiles.push_back(s);
	}

	getline(fin, line);
	sstream.str(line);
	sstream >> tilesheetSize.x >> tilesheetSize.y;
	tileTexSize = glm::vec2(1.f / tilesheetSize.x, 1.f / tilesheetSize.y);
	
	map = new int[mapSize.x * mapSize.y];
	for(int j=0; j<mapSize.y; j++)
	{
		for(int i=0; i<mapSize.x; i++)
		{
			fin.get(tile);
			if(tile == ' ')
				map[j*mapSize.x+i] = 0;
			else if (tile >= 'a' && tile <= 'z')
			{
				positions[tile].push_back(glm::vec2(i, j));
				map[j * mapSize.x + i] = tile;
			}
			else
				map[j*mapSize.x+i] = tile - int('0');
		}
		fin.get(tile);
#ifndef _WIN32
		fin.get(tile);
#endif
	}
	fin.close();
	
	return true;
}

void TileMap::prepareArrays(const glm::vec2 &minCoords, ShaderProgram &program)
{
	int tile;
	glm::vec2 posTile, texCoordTile[2], halfTexel;
	vector<float> vertices;
	
	nTiles = 0;
	halfTexel = glm::vec2(0.5f / tilesheet.width(), 0.5f / tilesheet.height());
	for(int j=0; j<mapSize.y; j++)
	{
		for(int i=0; i<mapSize.x; i++)
		{
			tile = map[j * mapSize.x + i];
			switch (tile)
			{
			case 2:
			case 0:
			case 4:
			case 5:
				break;
			default:
			{
				if (tile == 6)
				{
					tile = 2;
				}
				// Non-empty tile
				nTiles++;
				posTile = glm::vec2(minCoords.x + i * tileSize, minCoords.y + j * tileSize);
				texCoordTile[0] = glm::vec2(float((tile - 1) % tilesheetSize.x) / tilesheetSize.x, float((tile - 1) / tilesheetSize.x) / tilesheetSize.y);
				texCoordTile[1] = texCoordTile[0] + tileTexSize;
				//texCoordTile[0] += halfTexel;
				texCoordTile[1] -= halfTexel;
				// First triangle
				vertices.push_back(posTile.x); vertices.push_back(posTile.y);
				vertices.push_back(texCoordTile[0].x); vertices.push_back(texCoordTile[0].y);
				vertices.push_back(posTile.x + blockSize); vertices.push_back(posTile.y);
				vertices.push_back(texCoordTile[1].x); vertices.push_back(texCoordTile[0].y);
				vertices.push_back(posTile.x + blockSize); vertices.push_back(posTile.y + blockSize);
				vertices.push_back(texCoordTile[1].x); vertices.push_back(texCoordTile[1].y);
				// Second triangle
				vertices.push_back(posTile.x); vertices.push_back(posTile.y);
				vertices.push_back(texCoordTile[0].x); vertices.push_back(texCoordTile[0].y);
				vertices.push_back(posTile.x + blockSize); vertices.push_back(posTile.y + blockSize);
				vertices.push_back(texCoordTile[1].x); vertices.push_back(texCoordTile[1].y);
				vertices.push_back(posTile.x); vertices.push_back(posTile.y + blockSize);
				vertices.push_back(texCoordTile[0].x); vertices.push_back(texCoordTile[1].y);
			}
			break;
			}
		}
	}

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, 24 * nTiles * sizeof(float), &vertices[0], GL_STATIC_DRAW);
	posLocation = program.bindVertexAttribute("position", 2, 4*sizeof(float), 0);
	texCoordLocation = program.bindVertexAttribute("texCoord", 2, 4*sizeof(float), (void *)(2*sizeof(float)));
}

// Collision tests for axis aligned bounding boxes.
// Method collisionMoveDown also corrects Y coordinate if the box is
// already intersecting a tile below.

bool TileMap::collisionMoveLeft(const glm::ivec2 &pos, const glm::ivec2 &size) const
{
	int x, y0, y1;
	
	x = pos.x / tileSize;
	y0 = pos.y / tileSize;
	y1 = (pos.y + size.y - 1) / tileSize;
	for(int y=y0; y<=y1; y++)
	{
		if(map[y*mapSize.x+x] == 1)
			return true;
	}
	
	return false;
}

bool TileMap::collisionMoveRight(const glm::ivec2 &pos, const glm::ivec2 &size) const
{
	int x, y0, y1;
	
	x = (pos.x + size.x - 1) / tileSize;
	y0 = pos.y / tileSize;
	y1 = (pos.y + size.y - 1) / tileSize;
	for(int y=y0; y<=y1; y++)
	{
		if(map[y*mapSize.x+x] == 1)
			return true;
	}
	
	return false;
}

bool TileMap::collisionMoveDown(const glm::ivec2& pos, const glm::ivec2& size, int* posY) const
{
	int x0, x1, y;

	x0 = pos.x / tileSize;
	x1 = (pos.x + size.x - 1) / tileSize;
	y = (pos.y + size.y - 1) / tileSize;
	for (int x = x0; x <= x1; x++)
	{
		if (map[y * mapSize.x + x] == 1)
		{
			*posY = tileSize * y - size.y;
			return true;
		}
	}

	return false;
}

int TileMap::getTileIdAt(const glm::ivec2& pos) const {
	// Convert pixel coordinates to tile coordinates
	int x = pos.x / tileSize;
	int y = pos.y / tileSize;

	// Safety check for map boundaries
	if (x < 0 || x >= mapSize.x || y < 0 || y >= mapSize.y)
		return EMPTY;

	return map[y * mapSize.x + x];
}

glm::ivec2 TileMap::getMapSize() const
{
	return mapSize;
}

int TileMap::getTileSize() const
{
	return tileSize;
}

std::map<char, std::vector<glm::vec2>> const & TileMap::getPositionsOfStairs() const
{
	return positions;
}

vector<string> const & TileMap::getRoomFiles() const
{
	return roomFiles;
}

std::vector<PathStep> TileMap::getPath(glm::vec2 posE, glm::vec2 posP) {
	int startX = static_cast<int>(posE.x / tileSize);
	int startY = static_cast<int>(posE.y / tileSize);
	int targetX = static_cast<int>(posP.x / tileSize);
	int targetY = static_cast<int>(posP.y / tileSize);

	if (startX < 0 || startX >= mapSize.x || startY < 0 || startY >= mapSize.y ||
		targetX < 0 || targetX >= mapSize.x || targetY < 0 || targetY >= mapSize.y) {
		return {};
	}

	int startIdx = startY * mapSize.x + startX;
	int targetIdx = targetY * mapSize.x + targetX;
	if (startIdx == targetIdx) return {};

	// A* Data Structures
	std::priority_queue<AStarNode, std::vector<AStarNode>, std::greater<AStarNode>> openSet;
	std::vector<float> gCost(mapSize.x * mapSize.y, (std::numeric_limits<float>::max)());
	std::vector<int> parent(mapSize.x * mapSize.y, -1);

	// Initial Node
	gCost[startIdx] = 0;
	float hStart = (float)(abs(startX - targetX) + abs(startY - targetY)); // Manhattan
	openSet.push({ startIdx, hStart });

	bool found = false;
	int iterations = 0;

	while (!openSet.empty() && iterations < 10000) {
		iterations++;
		int curr = openSet.top().index;
		openSet.pop();

		if (curr == targetIdx) {
			found = true;
			break;
		}

		int x = curr % mapSize.x;
		int y = curr / mapSize.x;
		int tileVal = map[curr];

		// --- ENEMY MOVEMENT RULES (Your Original Logic) ---
		std::vector<int> neighbors;
		int downIdx = curr + mapSize.x;
		bool hasFloorBelow = (downIdx < mapSize.x * mapSize.y) && (map[downIdx] == 1 || map[downIdx] == 3);

		if (tileVal == 3) { // Vine logic
			if (y > 0) neighbors.push_back(curr - mapSize.x);
			if (downIdx < mapSize.x * mapSize.y) neighbors.push_back(downIdx);
		}

		if (hasFloorBelow || tileVal == 3) { // Walking logic
			if (x > 0) neighbors.push_back(curr - 1);
			if (x < mapSize.x - 1) neighbors.push_back(curr + 1);
		}
		else if (downIdx < mapSize.x * mapSize.y) { // Forced Fall logic
			neighbors.push_back(downIdx);
		}

		if (tileVal >= 'a' && tileVal <= 'z') { // Stairs logic
			for (const glm::vec2& p : positions.at((char)tileVal)) {
				int stairIdx = (int)p.y * mapSize.x + (int)p.x;
				if (stairIdx != curr) neighbors.push_back(stairIdx);
			}
		}

		// --- A* COST CALCULATION ---
		for (int next : neighbors) {
			if (map[next] == 1) continue; // Wall check

			float tentativeGCost = gCost[curr] + 1.0f; // Each tile step costs 1

			if (tentativeGCost < gCost[next]) {
				parent[next] = curr;
				gCost[next] = tentativeGCost;

				// h(N): Manhattan distance to target
				float hNext = (float)(abs((next % mapSize.x) - targetX) +
					abs((next / mapSize.x) - targetY));

				openSet.push({ next, gCost[next] + hNext });
			}
		}
	}

	// --- RECONSTRUCTION (Restored your specific logic) ---
	std::vector<PathStep> sequence;
	if (found) {
		std::vector<int> indices;
		for (int c = targetIdx; c != -1; c = parent[c]) indices.push_back(c);
		std::reverse(indices.begin(), indices.end());

		for (size_t i = 0; i < indices.size() - 1; ++i) {
			int from = indices[i], to = indices[i + 1];
			int x1 = from % mapSize.x, y1 = from / mapSize.x;
			int x2 = to % mapSize.x, y2 = to / mapSize.x;

			PathStep step;
			// Your exact -4, -16 offset for centering
			step.targetPoint = glm::vec2((x2 * tileSize) - 4.0f, (y2 * tileSize) - 16.0f);
			step.distance = (float)tileSize;

			if (abs(x2 - x1) > 1 || abs(y2 - y1) > 1) { // STAIRS
				step.command = AICommand::TRANSPORT;
				step.targetPoint = glm::vec2(x2 * tileSize, (y2 * tileSize) - 16.0f);
				step.distance = 0;
			}
			else if (y2 > y1) {
				step.command = (map[to] == 3 || map[from] == 3) ? AICommand::CLIMB_DOWN : AICommand::FALL;
				if (step.command == AICommand::FALL) step.distance = (float)tileSize * 1.2f;
			}
			else if (y2 < y1) {
				step.command = AICommand::CLIMB_UP;
			}
			else if (x2 > x1) {
				step.command = AICommand::MOVE_RIGHT;
			}
			else if (x2 < x1) {
				step.command = AICommand::MOVE_LEFT;
			}
			sequence.push_back(step);
		}
		if (!sequence.empty()) {
			sequence.back().targetPoint = glm::vec2(posP.x - 16.0f, posP.y - 16.0f);
		}
	}
	return sequence;
}

bool TileMap::hasFloorAt(const glm::ivec2& pixelPos) const {
	// 1. Convert pixel coordinates to tile coordinates
	int tileX = pixelPos.x / tileSize;
	int tileY = pixelPos.y / tileSize;

	// 2. Safety bounds check (don't check tiles outside the map)
	if (tileX < 0 || tileX >= mapSize.x || tileY < 0 || tileY >= mapSize.y) {
		return false;
	}

	// 3. Get the tile ID at this location
	// map is your 1D or 2D array of tile IDs
	int tileId = map[tileY * mapSize.x + tileX];

	// 4. Return true if the tile is NOT empty (usually 0 is sky/empty)
	return (tileId > 0);
}