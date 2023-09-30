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

typedef struct Vector2Int {
    int x;
    int y;
} Vector2Int;

struct Enemies {
    Vector2 position[MAXENEMIES];
    Vector2 velocity[MAXENEMIES];
    Vector2Int gridSpace[MAXENEMIES];
    bool enabled [MAXENEMIES];
    Texture2D texture;
} enemies;

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
const int screenWidth = 948;
const int screenHeight = 533;

const int gridOffsetX = 30;
const int gridOffsetY = 35;
const int columns = 13;
const int rows = 10;
const int spawnDelay = 50;

int timer = spawnDelay;

//----------------------------------------------------------------------------------
// Module functions declaration
//----------------------------------------------------------------------------------
void UpdateDrawFrame(void); // Update and Draw one frame
Vector2Int PositionToGrid(Vector2 pos);
Vector2 GridToPosition(Vector2Int pos);
bool isInGrid(Vector2 pos);
void DrawEnemies();
void UpdateEnemies(float dt);

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
    //----------------------------------------------------------------------------------

    // Draw
    //----------------------------------------------------------------------------------
    BeginDrawing();

    ClearBackground(RAYWHITE);

    DrawEnemies();

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
    timer -= dt;
    bool spawnEnemy = false;

    if (timer <= 0) {
        timer = spawnDelay;
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



