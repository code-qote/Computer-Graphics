/*******************************************************************************************
*
*   raygui - custom sliders
*
*   DEPENDENCIES:
*       raylib 4.0  - Windowing/input management and drawing.
*       raygui 3.0  - Immediate-mode GUI controls.
*
*   COMPILATION (Windows - MinGW):
*       gcc -o $(NAME_PART).exe $(FILE_NAME) -I../../src -lraylib -lopengl32 -lgdi32 -std=c99
*
*   LICENSE: zlib/libpng
*
*   Copyright (c) 2016-2023 Ramon Santamaria (@raysan5)
*
**********************************************************************************************/

#include "raylib.h"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

//----------------------------------------------------------------------------------
// Controls Functions Declaration
//----------------------------------------------------------------------------------

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main()
{
    // Initialization
    //---------------------------------------------------------------------------------------
    int screenWidth = 800;
    int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raygui - custom sliders");

    float value = 0;
    SetTargetFPS(60);
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        // TODO: Implement required update logic
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(WHITE);

            // if (vSliderEditMode || vSliderBarEditMode) GuiLock();
            // else GuiUnlock();

            // raygui: controls drawing
            //----------------------------------------------------------------------------------
            GuiSlider((Rectangle){ 96, 48, 216, 16 }, TextFormat("%0.2f", value), NULL, &value, 0.0f, 1.0f);
            // value = GuiVerticalSlider((Rectangle){ 120, 120, 24, 192 }, TextFormat("%0.2f", value), NULL, value, 0.0f, 1.0f);
            // value = GuiVerticalSliderBar((Rectangle){ 264, 120, 24, 192 }, TextFormat("%0.2f", value), NULL, value, 0.0f, 1.0f);

            // GuiGroupBox((Rectangle){ 378, 24, 276, 312 }, "OWNING");
            // if (GuiSliderOwning((Rectangle){ 408, 48, 216, 16 }, NULL, TextFormat("%0.2f", value), &value, 0.0f, 1.0f, sliderEditMode)) sliderEditMode = !sliderEditMode;
            // if (GuiVerticalSliderOwning((Rectangle){ 432, 120, 24, 192 }, NULL, TextFormat("%0.2f", value), &value, 0.0f, 1.0f, vSliderEditMode)) vSliderEditMode = !vSliderEditMode;
            // if (GuiVerticalSliderBarOwning((Rectangle){ 576, 120, 24, 192 }, NULL, TextFormat("%0.2f", value), &value, 0.0f, 1.0f, vSliderBarEditMode)) vSliderBarEditMode = !vSliderBarEditMode;
            //----------------------------------------------------------------------------------

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
