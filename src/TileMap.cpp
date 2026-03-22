#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <queue>
#include "TileMap.h"


using namespace std;


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

std::vector<PathStep> TileMap::getPath(glm::vec2 posE, glm::vec2 posP)
{
	// 1. Convert pixel positions to Tile coordinates
	int startX = static_cast<int>(posE.x / tileSize);
	int startY = static_cast<int>(posE.y / tileSize);
	int targetX = static_cast<int>(posP.x / tileSize);
	int targetY = static_cast<int>(posP.y / tileSize);

	if (startX < 0 || startX >= mapSize.x || startY < 0 || startY >= mapSize.y ||
		targetX < 0 || targetX >= mapSize.x || targetY < 0 || targetY >= mapSize.y) {
		return {}; // Return empty path if anyone is outside the map
	}

	int startIdx = startY * mapSize.x + startX;
	int targetIdx = targetY * mapSize.x + targetX;

	if (startIdx == targetIdx) return {};

	std::queue<int> nxtPos;
	std::vector<int> parent(mapSize.x * mapSize.y, -1); // To reconstruct path
	std::vector<bool> visited(mapSize.x * mapSize.y, false);

	nxtPos.push(startIdx);
	visited[startIdx] = true;

	bool found = false;
	int rec = 0;
	while (!nxtPos.empty() && rec < 10000)
	{
		rec++;
		int curr = nxtPos.front();
		nxtPos.pop();

		if (curr == targetIdx) {
			found = true;
			break;
		}

		std::vector<int> neighbors;
		int tileVal = map[curr];
		int x = curr % mapSize.x;
		int y = curr / mapSize.x;

		// 1. Check for floor existence
		bool hasFloorBelow = false;
		int downIdx = curr + mapSize.x;
		if (downIdx < mapSize.x * mapSize.y) {
			// We consider it a "floor" if the tile below is a wall (1) or another vine (3)
			if (map[downIdx] == 1 || map[downIdx] == 3) {
				hasFloorBelow = true;
			}
		}

		// 2. Priority Logic

		// CONDITION: IF ON VINE (3rd Priority)
		// Vines allow free vertical movement regardless of floors
		if (tileVal == 3) {
			// Up
			if (y > 0) {
				int up = curr - mapSize.x;
				if (map[up] == 3 || map[up] == 0) neighbors.push_back(up);
			}
			// Down
			if (downIdx < mapSize.x * mapSize.y) {
				if (map[downIdx] == 3 || map[downIdx] == 0 || map[downIdx] == 1) neighbors.push_back(downIdx);
			}
		}

		// CONDITION: FLOOR vs FALLING (1st & 2nd Priority)
		if (hasFloorBelow || tileVal == 3) {
			// If on a floor (or vine), he can move Left & Right
			if (x > 0) neighbors.push_back(curr - 1);
			if (x < mapSize.x - 1) neighbors.push_back(curr + 1);
		}
		else {
			// No floor and not on a vine: MUST fall down. 
			// We don't add Left/Right here, so the path is forced straight down.
			if (downIdx < mapSize.x * mapSize.y) {
				neighbors.push_back(downIdx);
			}
		}

		// CONDITION: STAIRS (4th Priority)
		if (tileVal >= 'a' && tileVal <= 'z') {
			char stairChar = (char)tileVal;
			const std::vector<glm::vec2>& pair = positions.at(stairChar);
			for (const glm::vec2& p : pair) {
				int stairIdx = (int)p.y * mapSize.x + (int)p.x;
				if (stairIdx != curr) neighbors.push_back(stairIdx);
			}
		}

		// Final validation and push to queue
		for (int next : neighbors) {
			// Ensure we don't walk into walls (1) and haven't visited this tile
			if (!visited[next] && map[next] != 1) {
				visited[next] = true;
				parent[next] = curr;
				nxtPos.push(next);
			}
		}
	}

	// 3. Reconstruct the path from target back to start
	std::vector<PathStep> sequence;
	if (found) {
		std::vector<int> indices;
		int currIdx = targetIdx;
		while (currIdx != -1) {
			indices.push_back(currIdx);
			currIdx = parent[currIdx];
		}
		std::reverse(indices.begin(), indices.end());

		// Inside the reconstruction loop of TileMap::getPath
		for (size_t i = 0; i < indices.size() - 1; ++i) {
			int from = indices[i];
			int to = indices[i + 1];

			int x1 = from % mapSize.x;
			int y1 = from / mapSize.x;
			int x2 = to % mapSize.x;
			int y2 = to / mapSize.x;

			PathStep step;

			// Dimension Correction:
			// To center Plankton's lower 24px wide area on a 16px tile:
			// X: (x * 16) + (16/2) - (24/2) = (x * 16) - 4
			// Y: (y * 16) + 16 (bottom of tile) - 32 (height of sprite) = (y * 16) - 16
			step.targetPoint = glm::vec2((x2 * tileSize) - 4.0f, (y2 * tileSize) - 16.0f);

			step.distance = (float)tileSize;

			// Use absolute tile coordinate differences for stair detection
			int xDiff = abs(x2 - x1);
			int yDiff = abs(y2 - y1);

			// 1. STAIR CASE (Restored your original check)
			if (xDiff > 1 || yDiff > 1) {
				step.command = AICommand::TRANSPORT;
				step.distance = 0;
				// For transport, we must use your specific door/stair offset
				step.targetPoint = glm::vec2(x2 * tileSize, (y2 * tileSize) - 16.0f);
			}
			// 2. VERTICAL MOVEMENT
			else if (y2 > y1) {
				if (map[to] == 3 || map[from] == 3) {
					step.command = AICommand::CLIMB_DOWN;
				}
				else {
					step.command = AICommand::FALL;
					// Restored your specific fall distance multiplier
					step.distance = (float)tileSize * 1.2f;
				}
			}
			else if (y2 < y1) {
				step.command = AICommand::CLIMB_UP;
			}
			// 3. HORIZONTAL MOVEMENT
			else if (x2 > x1) step.command = AICommand::MOVE_RIGHT;
			else if (x2 < x1) step.command = AICommand::MOVE_LEFT;

			sequence.push_back(step);
		}
	}
	for (auto ps : sequence)
	{
		cout << ps.targetPoint.x << " " << ps.targetPoint.y << " " << (ps.command == AICommand::TRANSPORT) << "\n";
	}
	cout << "\n";
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