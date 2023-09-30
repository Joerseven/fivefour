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
#include <iostream>

//#define PLATFORM_WEB



#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#define ASSETPATH "resources/"
#else
#define ASSETPATH "../resources/"
#endif

typedef struct Vector2Int {
    int x;
    int y;
} Vector2Int;

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
const int screenWidth = 948;
const int screenHeight = 533;

const int gridOffsetX = 30;
const int gridOffsetY = 35;

//----------------------------------------------------------------------------------
// Module functions declaration
//----------------------------------------------------------------------------------
void UpdateDrawFrame(void); // Update and Draw one frame
Vector2Int PositionToGrid(Vector2 pos);
Vector2 GridToPosition(Vector2Int pos);
bool isInGrid(Vector2 pos);

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
    monster = LoadTexture(ASSETPATH "testasset.png");


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

    //----------------------------------------------------------------------------------

    // Draw
    //----------------------------------------------------------------------------------
    BeginDrawing();

    ClearBackground(RAYWHITE);

    DrawText("Congrats! You created your first window!", 190, 200, 20, LIGHTGRAY);
    DrawTexturePro(monster, {0, 0, (float)monster.width, (float)monster.height},
                   {screenWidth / 2.0f, screenHeight / 2.0f, (float)monster.width, (float)monster.height },
                   {monster.width/2.0f, monster.height/2.0f}, 0.0f, WHITE);

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
    return position.x >= gridOffsetX && position.x <= gridOffsetX + (48 * 13) &&
    position.y >= gridOffsetY && position.y <= gridOffsetY + (48 * 10);
}

