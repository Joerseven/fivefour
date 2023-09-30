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
    Block onMouse;
    int currentlyHeld;
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

Texture2D MainBackground;
Texture2D SecondBackground;



int EnemyTimer;
int BlockTimer;


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

// Global Variables
Texture2D monster;

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    InitWindow(screenWidth, screenHeight, "raylib [core] example - basic window");

    enemies.texture = LoadTexture(ASSETPATH "gj.png");
    MainBackground = LoadTexture(ASSETPATH "window.png");
    SecondBackground = LoadTexture(ASSETPATH "window-block.png");

    int EnemyTimer = EnemySpawnDelay;
    int BlockTimer = BlockSpawnDelay;
    BlockPlacer.currentlyHeld = 0;
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

    UpdateEnemies(dt);
    UpdateBlocks(dt);
    //----------------------------------------------------------------------------------

    // Draw
    //----------------------------------------------------------------------------------
    BeginDrawing();

    ClearBackground(RAYWHITE);

    DrawEnemies();
    DrawBlocks();

    EndDrawing();
    //----------------------------------------------------------------------------------
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
            return {{{1,0}, {0,0}, {-1, 0}}};
        case 1:
            return {{{1,0}, {0, 1}}};
        default:
            return {{{0,0}}};
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

void DrawBlock(Block block, Vector2 position) {
    for (int i=0;i<MAXBLOCKSIZE;i++) {
        DrawRectangle(block.contents[i].x*48 + position.x, block.contents[i].y * 48 + position.y, 48, 48, GRAY);
    }
}

void DrawBlocks() {
    for (int i=0;i<MAXHOLDING;i++) {
        if (i >= BlockPlacer.inventorySpot) return;
        DrawBlock(BlockPlacer.inventory[i], {798, gridOffsetY + i*2*48.0f});
    }
}



