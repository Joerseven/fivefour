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

typedef struct Vector2Int {
    int x;
    int y;
} Vector2Int;

typedef struct Block {
    Vector2Int contents[MAXBLOCKSIZE];
    float width;
    int count;
} Block;


struct Enemies {
    Vector2 position[MAXENEMIES];
    Vector2 velocity[MAXENEMIES];
    Vector2Int gridSpace[MAXENEMIES];
    bool enabled [MAXENEMIES];
    Texture2D texture;
} enemies;


struct BlockPlacer {
    Block inventory[MAXHOLDING];
    int selected;
    int inventorySpot;
} BlockPlacer;

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
const int screenWidth = 948;
const int screenHeight = 533;

const int gridOffsetX = 30;
const int gridOffsetY = 35;

const int columns = 13;
const int rows = 10;

const int EnemySpawnDelay = 50;
const int BlockSpawnDelay = 300;

Texture2D Background;

int EnemyTimer;
int BlockTimer;

Vector2 TouchPosition;
int currentGuesture;

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
    //----------------------------------------------------------------------------------

    // Draw
    //----------------------------------------------------------------------------------
    BeginDrawing();

    ClearBackground((Color){186,186,186});

    DrawEnemies();
    DrawTexture(Background, 0, 0, WHITE);
    DrawBlocks();
    ShowSelection();

    EndDrawing();
    //----------------------------------------------------------------------------------
}

bool DoesBlockFit(Block block, Vector2Int position) {
    for (auto & content: block.contents) {
        if (position.x + content.x < 0 || position.x + content.x >= columns) return false;
        if (position.y + content.y < 0 || position.y + content.y >= rows) return false;
    }
    return true;
}

void ManageInput() {

    currentGuesture = GetGestureDetected();



    if (currentGuesture == GESTURE_HOLD || currentGuesture == GESTURE_DRAG) {

        if (BlockPlacer.selected != -1) {
            return;
        }

        Rectangle touchArea = { 675, 42, 254, 479};
        auto touchPosition = GetTouchPosition(0);

        if (CheckCollisionPointRec(touchPosition, touchArea)) {
            int item = std::round((touchPosition.y - 55) / 64);
            if (item >= 0 && item < MAXHOLDING) {
                BlockPlacer.selected = item;
            }
        }

        return;
    }

    if (currentGuesture == GESTURE_NONE) {
        std::cout << "Nothing" << std::endl;

        if (BlockPlacer.selected >= 0) {
            auto touchPosition = GetTouchPosition(0);
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

void PlaceBlock(Block block, Vector2Int position, bool doesFit) {
    if (doesFit) {
        ShuffleDownBlocks(BlockPlacer.selected);
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
    return { (int)((pos.x - gridOffsetX) / 48), (int)((pos.y - gridOffsetY) / 48)};
}

Vector2 GridToPosition(Vector2Int pos) {
    return {(pos.x*48.0f) + gridOffsetX, (pos.y*48.0f) + gridOffsetY};
}

bool isInGrid(Vector2 position) {
    return position.x >= gridOffsetX && position.x <= gridOffsetX + (48 * columns) &&
    position.y >= gridOffsetY && position.y <= gridOffsetY + (48 * rows);
}


void SpawnEnemy(int index) {

    int side = GetRandomValue(1, 4);

    if (side % 2 == 0) {
        enemies.position[index].y = GetRandomValue(gridOffsetY, gridOffsetY + (48 * rows));
        enemies.position[index].x = gridOffsetX + (48 * columns)*((side-1)/2);

    } else {
        enemies.position[index].x = GetRandomValue(gridOffsetX, gridOffsetX + (48 * columns));
        enemies.position[index].y = gridOffsetY + (48 * rows)*((side-1)/2);
    }

    auto target = GridToPosition({6,3});
    auto mag = Vector2Normalize({target.x - enemies.position[index].x, target.y - enemies.position[index].y});

    enemies.velocity[index].x = mag.x;
    enemies.velocity[index].y = mag.y;

    enemies.enabled[index] = true;

}

void UpdateEnemies(float dt) {
    EnemyTimer -= dt;
    bool spawnEnemy = false;

    if (EnemyTimer <= 0) {
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

        enemies.position[i] = Vector2Add(enemies.position[i], enemies.velocity[i]);
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
        DrawRectangle(pos.x, pos.y, 48, 48, fits ? (Color){30,170,30,130} : (Color){170, 30 , 30, 130});
    }
}

void DrawBlock(Block block, Vector2 position, int size) {
    for (auto & content : block.contents) {
        DrawRectangle(content.x*size + position.x, content.y*size + position.y, size, size, GRAY);
    }
}

void DrawBlocks() {
    for (int i=0;i<MAXHOLDING;i++) {
        if (i >= BlockPlacer.inventorySpot) return;
        DrawBlock(BlockPlacer.inventory[i], {802.0f - BlockPlacer.inventory[i].width * 12, 55 + i*64.0f}, 24);
    }
}



