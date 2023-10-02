/*******************************************************************************************
*
*   raylib [core] example - Basic window (adapted for HTML5 platform)
*
*   NOTE: This example is prepared to compile for PLATFORM_WEB, and PLATFORM_DESKTOP
*   As you will notice, code structure is slightly diferent to the other examples...
*   To compile it for PLATFORM_WEB just uncomment #define PLATFORM_WEB at beginning
*
*   Example originally created with raylib 1.3, last time updated with raylib 1.3
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2015-2023 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"
#include "raymath.h"
#include <iostream>
#include <cmath>

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#define ASSETPATH "resources/"
#else
#define ASSETPATH "../resources/"
#endif

#define MAXENEMIES 30
#define MAXBLOCKSIZE 6
#define MAXHOLDING 5
#define MAXPARTICLES 10

typedef struct Vector2Int {
    int x;
    int y;
} Vector2Int;

typedef struct Block {
    Vector2Int contents[MAXBLOCKSIZE];
    float width;
    int count;
} Block;

typedef struct ParticleBurst {
    Vector2 origin;
    bool enabled[MAXPARTICLES];
    Vector2 position[MAXPARTICLES];
    Vector2 velocity[MAXPARTICLES];
    float lifetime[MAXPARTICLES];
    float size[MAXPARTICLES];
    Color color[MAXPARTICLES];
    bool active;
} ParticleBurst;


struct Enemies {
    Vector2 position[MAXENEMIES];
    Vector2 target[MAXENEMIES];
    float waitTime[MAXENEMIES];
    bool enabled [MAXENEMIES];
    Texture2D texture;
} enemies;


struct BlockPlacer {
    Block inventory[MAXHOLDING];
    int selected;
    int inventorySpot;
} BlockPlacer;

ParticleBurst ParticleSystem[20];

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
const int screenWidth = 948;
const int screenHeight = 533;

const int gridOffsetX = 35 + 20;
const int gridOffsetY = 40 + 10;

const int columns = 9;
const int rows = 7;

int grid[rows][columns];

int EnemySpawnDelay = 5;
const int BlockSpawnDelay = 3;

const int EnemyHideTime = 4;
const int EnemySpeed = 25;
const Vector2Int FinalTile = {7,6};

Texture2D Background;
Texture2D FolderBack;
Texture2D FolderFront;

float EnemyTimer;
float BlockTimer;

Vector2 TouchPosition;
int currentGesture;

//----------------------------------------------------------------------------------
// Module functions declaration
//----------------------------------------------------------------------------------
void UpdateDrawFrame(void); // Update and Draw one frame
Vector2Int PositionToGrid(Vector2 pos);
Vector2 GridToPosition(Vector2Int pos);
bool isInGrid(Vector2 pos);
void DrawEnemies();
void UpdateEnemies(float dt);
void UpdateBlocks(float dt);
void DrawBlocks();
void ManageInput();
void ShowSelection();
void DrawBlockOnGrid(Block block, Vector2Int position, bool fits);
void PlaceBlock(Block block, Vector2Int position, bool doesFit);
void UpdateGrid(float dt);
void DisplayBrokenTiles();
void DrawFolderBacks();
void DrawFolderFronts();
void CreateEnemyParticles(Vector2 origin);
void UpdateParticleSystems(float dt);
void DrawParticleSystems();

// Global Variables


//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    InitWindow(screenWidth, screenHeight, "raylib [core] example - basic window");

    enemies.texture = LoadTexture(ASSETPATH "gj.png");
    Background = LoadTexture(ASSETPATH "fullwindow.png");
    FolderBack = LoadTexture(ASSETPATH "folde-back-paper.png");
    FolderFront = LoadTexture(ASSETPATH "folder-front.png");

    int EnemyTimer = EnemySpawnDelay;
    int BlockTimer = BlockSpawnDelay;
    BlockPlacer.inventorySpot = 0;


#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(UpdateDrawFrame, 0, 1);
#else
    SetTargetFPS(60);   // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        UpdateDrawFrame();
    }
#endif

    // De-Initialization
    // Don't forget to unload the textures here.
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

//----------------------------------------------------------------------------------
// Module Functions Definition
//----------------------------------------------------------------------------------
void UpdateDrawFrame(void)
{
    // Update
    //---------------------------------------------------------------------------------
    float dt = GetFrameTime();

    ManageInput();

    UpdateEnemies(dt);
    UpdateBlocks(dt);
    UpdateGrid(dt);
    UpdateParticleSystems(dt);

    //----------------------------------------------------------------------------------

    // Draw
    //----------------------------------------------------------------------------------
    BeginDrawing();

    ClearBackground({186,186,186});

    DrawFolderBacks();
    DrawEnemies();
    DrawFolderFronts();
    DrawTexture(Background, 0, 0, WHITE);
    DrawBlocks();
    DisplayBrokenTiles();
    ShowSelection();
    DrawParticleSystems();

    EndDrawing();
    //----------------------------------------------------------------------------------
}

float GetSpawnCurve(float x) {
    return 1 - powf(1 - x, 3);
}

void DrawFolderBacks() {
    for (int i=0; i<columns; i++) {
        for (int j=0; j<rows; j++) {
            auto position = GridToPosition({i, j});
            DrawTexture(FolderBack, (int)position.x + 2, (int)position.y - 5, WHITE);
        }
    }
}

void DrawFolderFronts() {
    for (int i=0; i<columns; i++) {
        for (int j=0; j<rows; j++) {
            auto position = GridToPosition({i, j});
            DrawTexture(FolderFront, (int)position.x + 2, (int)position.y - 5, WHITE);
        }
    }
}

bool DoesBlockFit(Block block, Vector2Int position) {
    for (auto & content: block.contents) {
        if (position.x + content.x < 0 || position.x + content.x >= columns) return false;
        if (position.y + content.y < 0 || position.y + content.y >= rows) return false;

        if (grid[position.y+content.y][position.x+content.x] != 0) return false;
    }

    return true;
}

void RotateBlocks() {
    for (Block &block : BlockPlacer.inventory) {
        for (Vector2Int &content : block.contents) {
            int x = content.x;
            content.x = -content.y;
            content.y = x;
        }
    }
}

void ManageInput() {

    currentGesture = GetGestureDetected();
    auto touchPosition = GetTouchPosition(0);

    if (currentGesture == GESTURE_TAP || currentGesture == GESTURE_DOUBLETAP) {
        Rectangle touchArea = {866, 16, 20, 20};
        if (CheckCollisionPointRec(touchPosition, touchArea) && BlockPlacer.selected < 0) {
            RotateBlocks();
        }
    }

    if (currentGesture == GESTURE_HOLD || currentGesture == GESTURE_DRAG) {

        if (BlockPlacer.selected != -1) {
            return;
        }

        if (BlockPlacer.inventorySpot == 0) {
            return;
        }

        Rectangle touchArea = { 675, 42, 254, 479};

        if (CheckCollisionPointRec(touchPosition, touchArea)) {
            int item = std::round((touchPosition.y - 55) / 64);
            if (item >= 0 && item < MAXHOLDING) {
                BlockPlacer.selected = item;
            }
        }

        return;
    }

    if (currentGesture == GESTURE_NONE) {

        if (BlockPlacer.selected >= 0) {
            if (isInGrid(touchPosition)) {
                auto HoverTile = PositionToGrid(touchPosition);
                auto doesFit = DoesBlockFit(BlockPlacer.inventory[BlockPlacer.selected], HoverTile);
                PlaceBlock(BlockPlacer.inventory[BlockPlacer.selected], HoverTile, doesFit);
            }
        }

        BlockPlacer.selected = -1;
        return;
    }
}

void ShuffleDownBlocks(int selected) {
    for (int i=BlockPlacer.selected+1; i<BlockPlacer.inventorySpot; i++) {
        BlockPlacer.inventory[i - 1] = BlockPlacer.inventory[i];
    }

    BlockPlacer.inventorySpot--;
}

void CreateEnemyParticles(Vector2 origin) {
    int choice = -1;
    for (int i = 0; i < 20; i++) {
        if (ParticleSystem[i].active) continue;
        choice = i;
        break;
    }

    if (choice < 0) return;

    ParticleSystem[choice].origin = origin;
    ParticleSystem[choice].active = true;

    for (int i = 0; i < MAXPARTICLES; i++) {
        ParticleSystem[choice].color[i] = RED;
        ParticleSystem[choice].size[i] = (float)GetRandomValue(1, 4);
        ParticleSystem[choice].enabled[i] = true;
        ParticleSystem[choice].lifetime[i] = GetRandomValue(2, 5) / 10.0f;
        ParticleSystem[choice].position[i] = origin;
        ParticleSystem[choice].velocity[i] = {(float)GetRandomValue(-60,60), (float)GetRandomValue(-60,60)};
    }

}

void UpdateParticleSystems(float dt) {
    for (ParticleBurst& system : ParticleSystem) {
        if (!system.active) continue;
        bool stillAlive = false;
        for (int i = 0; i < MAXPARTICLES; i++) {

            if (!system.enabled[i]) continue;

            system.lifetime[i] -= dt;

            if (system.lifetime[i] < 0) {
                system.enabled[i] = false;
                continue;
            }

            system.position[i] = Vector2Add(system.position[i], Vector2Scale(system.velocity[i], dt));

            stillAlive = true;
        }
        if (!stillAlive) system.active = false;
    }

}

void DrawParticleSystems() {
    for (ParticleBurst& system : ParticleSystem) {
        if (!system.active) continue;
        for (int i = 0; i < MAXPARTICLES; i++) {
            if (!system.enabled[i]) continue;
            DrawCircle(system.position[i].x, system.position[i].y, system.size[i], system.color[i]);
        }
    }
}

void KillEnemy(int index) {
    enemies.enabled[index] = false;
    CreateEnemyParticles(enemies.position[index]);
}

void PlaceBlock(Block block, Vector2Int position, bool doesFit) {
    if (doesFit) {
        for (Vector2Int &content: block.contents) {
            grid[position.y + content.y][position.x + content.x] = 1000;

            for (int i=0;i<MAXENEMIES;i++) {
                auto tile = PositionToGrid(enemies.position[i]);
                if (tile.x == position.x + content.x && tile.y == position.y + content.y) {
                    KillEnemy(i);
                }
            }
        }
        ShuffleDownBlocks(BlockPlacer.selected);
    }
}

void UpdateGrid(float dt) {
    for (int i=0;i<rows;i++) {
        for (int j=0;j<columns;j++) {
            if (grid[i][j] > 0) {
                grid[i][j]--;
            }
        }
    }
}

void DisplayBrokenTiles() {
    for (int i=0;i<rows;i++) {
        for (int j=0;j<columns;j++) {
            if (grid[i][j] > 0) {
                auto pos = GridToPosition({j, i});
                //DrawRectangle(pos.x, pos.y, 48, 48, RED);
            }
        }
    }
}

void ShowSelection() {
    if (BlockPlacer.selected < 0) {
        return;
    }

    auto touchPosition = GetTouchPosition(0);

    if (!isInGrid(touchPosition)) {
        return;
    }

    auto HoverTile = PositionToGrid(touchPosition);
    auto doesFit = DoesBlockFit(BlockPlacer.inventory[BlockPlacer.selected], HoverTile);
    DrawBlockOnGrid(BlockPlacer.inventory[BlockPlacer.selected], HoverTile, doesFit);

}

Vector2Int PositionToGrid(Vector2 pos) {
    return { (int)((pos.x - gridOffsetX) / 66), (int)((pos.y - gridOffsetY) / 68)};
}

Vector2 GridToPosition(Vector2Int pos) {
    return {(pos.x*66.0f) + gridOffsetX, (pos.y*68.0f) + gridOffsetY};
}

bool isInGrid(Vector2 position) {
    return position.x >= gridOffsetX && position.x <= gridOffsetX + (66.0f * columns) &&
    position.y >= gridOffsetY && position.y <= gridOffsetY + (68 * rows);
}


void SpawnEnemy(int index) {

    int side = GetRandomValue(1, 4);

    enemies.waitTime[index] = 0;

    if (side % 2 == 0) {

        enemies.target[index] = Vector2Add(GridToPosition({((side-1)/2 > 0) ? columns - 1 : 0, GetRandomValue(0,rows - 1)}), {24,24});

        enemies.position[index].y = enemies.target[index].y;
        enemies.position[index].x = enemies.target[index].x - (((side-1)/2 > 0) ? -48.0f : 48.0f);

    } else {

        enemies.target[index] = Vector2Add(GridToPosition({GetRandomValue(0,columns - 1), ((side-1)/2 > 0) ? rows - 1 : 0}), {24,24});

        enemies.position[index].y = enemies.target[index].y -(((side-1)/2 > 0) ? -48.0f : 48.0f);
        enemies.position[index].x = enemies.target[index].x;
    }
    enemies.enabled[index] = true;
}

void GameOver() {

}

Vector2 GetNextMoveTile(int index) {
    int direction = GetRandomValue(0,1);
    Vector2Int currentTile = PositionToGrid(enemies.position[index]);

    if (currentTile.x == FinalTile.x && currentTile.y == FinalTile.y) {
        GameOver();
    }

    Vector2Int target;

    if ((direction == 1 && currentTile.x != FinalTile.x) || currentTile.y == FinalTile.y) {
        target.x = (currentTile.x > FinalTile.x) ? currentTile.x - 1 : currentTile.x + 1;
        target.y = currentTile.y;
        return Vector2Add(GridToPosition(target),{24,24});
    } else {
        target.y = (currentTile.y > FinalTile.y) ? currentTile.y - 1 : currentTile.y + 1;
        target.x = currentTile.x;
        return Vector2Add(GridToPosition(target),{24,24});
    }

}

void UpdateEnemies(float dt) {
    EnemyTimer -= dt;
    bool spawnEnemy = false;

    if (EnemyTimer <= 0) {

        EnemySpawnDelay -= 0.1;
        if (EnemySpawnDelay < 2) EnemySpawnDelay = 2;

        EnemyTimer = EnemySpawnDelay;
        spawnEnemy = true;

    }

    for (int i=0;i<MAXENEMIES;i++) {
        if (!enemies.enabled[i]) {
            if (spawnEnemy) {
                SpawnEnemy(i);
                spawnEnemy = false;
            }
            continue;
        }

        if (enemies.waitTime[i] > 0) {
            enemies.waitTime[i] -= dt;
            continue;
        }

        auto distanceVector = Vector2Subtract(enemies.target[i], enemies.position[i]);
        auto moveVector = Vector2Scale(Vector2Normalize(distanceVector), EnemySpeed * dt);

        if (Vector2LengthSqr(moveVector) >= Vector2LengthSqr(distanceVector)) {
            enemies.position[i] = enemies.target[i];
            enemies.waitTime[i] = EnemyHideTime;
            enemies.target[i] = GetNextMoveTile(i);
        }

        enemies.position[i] = Vector2Add(enemies.position[i], moveVector);
    }
}

void DrawEnemies() {
    for (int i=0;i<MAXENEMIES;i++) {
        if (!enemies.enabled[i]) continue;
        DrawTexturePro(enemies.texture, {0, 0, (float)enemies.texture.width, (float)enemies.texture.height}, {enemies.position[i].x, enemies.position[i].y, (float)enemies.texture.width, (float)enemies.texture.height}, {enemies.texture.width / 2.0f, enemies.texture.height / 2.0f}, 0, WHITE);
    }
}

Block CreateBlock() {
    int blockType = GetRandomValue(0, 2);
    switch(blockType) {
        case 0:
            return {{{1,0}, {0,0}, {-1, 0}}, 1.5, 3};
        case 1:
            return {{{1,0}, {0, 1}, {0,0}}, 2, 3};
        default:
            return {{{0,0}}, 1, 1};
    }
}

void UpdateBlocks(float dt) {

    BlockTimer -= dt;
    if (BlockTimer <= 0) {
        BlockTimer = BlockSpawnDelay;

        if (BlockPlacer.inventorySpot < MAXHOLDING) {
            BlockPlacer.inventory[BlockPlacer.inventorySpot] = CreateBlock();
            BlockPlacer.inventorySpot += 1;
        }
    }
}

void DrawBlockOnGrid(Block block, Vector2Int position, bool fits) {
    for (int i = 0;i<block.count; i++) {
        auto pos = GridToPosition({position.x + block.contents[i].x, position.y + block.contents[i].y});
        auto color = fits ? Color{ 30, 170, 30, 130 } : Color{ 170, 30 , 30, 130 };
        DrawRectangleRounded({ pos.x-4, pos.y-4, 56, 56}, 0.5, 10, color);
    }
}

void DrawBlock(Block block, Vector2 position, int size) {
    for (auto & content : block.contents) {
        DrawRectangle(content.x*size + position.x, content.y*size + position.y, size, size, GRAY);
    }
}

void DrawBlocks() {
    for (int i=0;i<MAXHOLDING;i++) {
        if (i >= BlockPlacer.inventorySpot) {
            return;
        }
        DrawBlock(BlockPlacer.inventory[i], {802.0f - BlockPlacer.inventory[i].width * 12, 55 + i*64.0f}, 24);
    }
}



