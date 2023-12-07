#include "raylib.h"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#define RAYMATH_IMPEMENTATION
#include "raymath.h"

#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <vector>
#include <string>
#include <iostream>

#define INF (float) 1000000007
#define GRAPH_THICK 2
#define CIRCLE_RADUIS 4

int WINDOW_WIDTH = 1280, WINDOW_HEIGHT = 720;

Font font;

void drawArrow(Vector2 start, Vector2 end) {
    DrawLineEx(start, end, GRAPH_THICK, RED);
    Vector2 d = {start.x - end.x, start.y - end.y};
    float len = sqrt(d.x * d.x + d.y * d.y);
    d.x /= len;
    d.y /= len;
    float ax = d.x * sqrt(2) / 2 - d.y * sqrt(2) / 2;
    float ay = d.x * sqrt(2) / 2 + d.y * sqrt(2) / 2;
    float bx = d.x * sqrt(2) / 2 + d.y * sqrt(2) / 2;
    float by = -d.x * sqrt(2) / 2 + d.y * sqrt(2) / 2; 
    Vector2 point1 = {end.x + 10 * ax, end.y + 10 * ay};
    Vector2 point2 = {end.x + 10 * bx, end.y + 10 * by};
    DrawTriangle(point2, end, point1, RED);
}

void drawPointWithLabel(Vector2 point, const char* label) {
    const int size = 15;
    Vector2 pToDraw = {point.x, point.y};
    DrawCircleV(pToDraw, CIRCLE_RADUIS, RED);
    DrawTextEx(font, label, pToDraw, font.baseSize, 1, BLACK);
}

float calcN(float u, int i, int p, const std::vector<float> &us) {
    if (p == 0) {
        return us[i] <= u && u < us[i + 1];
    }
    float a = 0, b = 0;
    if (us[i + p] - us[i] != 0) {
        a = (u - us[i]) / (us[i + p] - us[i]) * calcN(u, i, p - 1, us);
    }
    if (us[i + p + 1] - us[i + 1] != 0) {
        b = (us[i + p + 1] - u) / (us[i + p + 1] - us[i + 1]) * calcN(u, i + 1, p - 1, us);
    }
    return a + b;
}

std::vector<Vector2> calcPoints(float a, float b, int p, const std::vector<Vector2> &control_points, const std::vector<float> &us) {
    float step = 1e-3;
    std::vector<Vector2> res((b - a) / step + 1);
    int cur = 0;
    for (float u = a; u < b; u += step) {
        Vector2 point = {0, 0};
        for (int i = 0; i < control_points.size(); i++) {
            point = Vector2Add(point, Vector2Scale(control_points[i], calcN(u, i, p, us)));
        }
        res[cur++] = point;
    }
    return res;
}

int main(void)
{
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "lab7");
    SetTargetFPS(60);
    font = LoadFontEx("roboto.ttf", 20, 0, 256);

    int n = 6;
    int p = 3;

    float a = 0, b = 3;
    std::vector<float> us = {0, 0, 0, 0, 2, 2, 3, 3, 3, 3};

    std::vector<Vector2> control_points;
    std::vector<Vector2> points;

    int dragging_ind = -1;
    bool start = false;
    int cur_running = 0;

    while (!WindowShouldClose()) {

        if (!start) cur_running = 0;

        Vector2 mousePos = GetMousePosition();
        if (dragging_ind != -1) {
            control_points[dragging_ind] = mousePos;
            if (control_points.size() == n) {
                points = calcPoints(a, b, p, control_points, us);
            }
        }
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            bool added = false;
            if (control_points.size() < n) {
                bool can_add = true;
                for (auto &point : control_points) {
                    if (Vector2Distance(point, mousePos) < CIRCLE_RADUIS) {
                        can_add = false;
                        break;
                    }
                }
                if (can_add) {
                    added = true;
                    control_points.push_back(mousePos);
                    if (control_points.size() == n) {
                        points = calcPoints(a, b, p, control_points, us);
                    }
                }
            }

            if (!added) {
                for (int i = 0; i < control_points.size(); i++) {
                    if (Vector2Distance(control_points[i], mousePos) < CIRCLE_RADUIS) {
                        dragging_ind = i;
                        break;
                    }
                }
            }
        }
        
        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
            dragging_ind = -1;
        }


        BeginDrawing();
        ClearBackground(WHITE);
        WINDOW_WIDTH = GetScreenWidth();
        WINDOW_HEIGHT = GetScreenHeight();
        Rectangle bounds = {100, 100, 100, 20};
        
        for (Vector2 &point : control_points) {
            drawPointWithLabel(point, TextFormat("(%3.1lf,%3.1lf)", point.x, point.y));
        }

        if (control_points.size() == n) {
            for (int i = 0; i < points.size() - 1; i++) {
                DrawLineEx(points[i], points[i + 1], GRAPH_THICK, BLACK);
            }
        }

        if (start && control_points.size() == n) {
            cur_running %= points.size() - 1;
            Vector2 der = Vector2Scale(Vector2Normalize(Vector2Subtract(points[cur_running + 1], points[cur_running])), 50);
            drawArrow(points[cur_running], Vector2Add(points[cur_running], der));
            cur_running = (cur_running + 3) % (points.size() - 1);
        }

        if (control_points.size() == n) {
            start ^= GuiButton((Rectangle){ GetScreenWidth() * 0.078f, GetScreenHeight() * 0.139f, GetScreenWidth() * 0.078f, GetScreenHeight() * 0.041f  }, TextFormat("Start"));
        }

        EndDrawing();
    }

    CloseWindow();

    return 0;
}

