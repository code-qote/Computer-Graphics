#include "raylib.h"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#include <stdlib.h>
#include <math.h>
#include <stdio.h>

#define INF 1000000007
#define AXES_THICK 2
#define GRAPH_THICK 2
#define CIRCLE_RADUIS 4

int WINDOW_WIDTH = 1280;
int WINDOW_HEIGHT = 720;

Font font;

void drawArrow(Vector2 start, Vector2 end, char* label) {
    DrawLineEx(start, end, AXES_THICK, GRAY);
    Vector2 d = {start.x - end.x, start.y - end.y};
    double len = sqrt(d.x * d.x + d.y * d.y);
    d.x /= len;
    d.y /= len;
    double ax = d.x * sqrt(2) / 2 - d.y * sqrt(2) / 2;
    double ay = d.x * sqrt(2) / 2 + d.y * sqrt(2) / 2;
    double bx = d.x * sqrt(2) / 2 + d.y * sqrt(2) / 2;
    double by = -d.x * sqrt(2) / 2 + d.y * sqrt(2) / 2; 
    Vector2 point1 = {end.x + 10 * ax, end.y + 10 * ay};
    Vector2 point2 = {end.x + 10 * bx, end.y + 10 * by};
    DrawTriangle(point2, end, point1, GRAY);
    DrawTextEx(font, label, (Vector2){end.x - 15, end.y + 5}, font.baseSize, 1, BLACK);
}

void drawAxes() {
    Vector2 startPosVert = {(double) WINDOW_WIDTH / 2, WINDOW_HEIGHT};
    Vector2 endPosVert = {(double) WINDOW_WIDTH / 2, 0};
    drawArrow(startPosVert, endPosVert, "Y");

    Vector2 startPosHor = {0, (double) WINDOW_HEIGHT / 2};
    Vector2 endPosHor = {WINDOW_WIDTH, (double) WINDOW_HEIGHT / 2};
    drawArrow(startPosHor, endPosHor, "X");
}

Vector2* calculatePoints(double a, double k, double B, double step) {
    int len = (int) (B / step) + 2;
    Vector2* points = malloc(sizeof(Vector2) * len);
    int i = 0;
    for (double phi = 0; phi < B; phi += step) {
        double r = a * exp(k * phi);
        double x = r * cos(phi);
        double y = r * sin(phi);
        Vector2 point = {x, y};
        points[i++] = point;
    }
    double r = a * exp(k * B);
    double x = r * cos(B);
    double y = r * sin(B);
    Vector2 point = {x, y};
    points[i] = point;
    return points;
}

Vector2* detectCollisionWithAxes(Vector2* points, int len, int* resLen) {
    const double eps = 1e-3;
    Vector2* res = malloc(sizeof(Vector2) * len);
    int cur = 0;
    for (int i = 1; i < len; i++) {
        if (points[i - 1].y * points[i].y < 0) {
            double dx = points[i].x - points[i - 1].x;
            double dy = points[i].y - points[i - 1].y;
            double k = dy / dx;
            Vector2 p = {points[i].x - points[i - 1].y * k, 0};
            if (cur == 0 || !(cur - 1 >= 0 && fabs(res[cur - 1].x - p.x) < eps && fabs(res[cur - 1].y - p.y) < eps)) {
                res[cur++] = p;
            }
        }
        if (points[i - 1].x * points[i].x < 0) {
            double dx = points[i].x - points[i - 1].x;
            double dy = points[i].y - points[i - 1].y;
            double k = dy / dx;
            Vector2 p = {0, k * points[i - 1].x + points[i - 1].y};
            if (cur == 0 || !(cur - 1 >= 0 && fabs(res[cur - 1].x - p.x) < eps && fabs(res[cur - 1].y - p.y) < eps)) {
                res[cur++] = p;
            }
        }
        if (i + 1 == len) break; 
        if (fabs(points[i - 1].x) < eps && fabs(points[i - 1].y) < eps ||
            fabs(points[i].x) < eps && fabs(points[i].y) < eps) {
            Vector2 p = {0, 0};
            if (cur == 0 || !(cur - 1 >= 0 && fabs(res[cur - 1].x - p.x) < eps && fabs(res[cur - 1].y - p.y) < eps)) {
                res[cur++] = p;
            }
        }
        if (fabs(points[i - 1].x) < eps) {
            Vector2 p = {0, points[i - 1].y};
            if (cur == 0 || !(cur - 1 >= 0 && fabs(res[cur - 1].x - p.x) < eps && fabs(res[cur - 1].y - p.y) < eps)) {
                res[cur++] = p;
            }
        }
        if (fabs(points[i - 1].y) < eps) {
            Vector2 p = {points[i - 1].x, 0};
            if (cur == 0 || !(cur - 1 >= 0 && fabs(res[cur - 1].x - p.x) < eps && fabs(res[cur - 1].y - p.y) < eps)) {
                res[cur++] = p;
            }
        }
        if (fabs(points[i].x) < eps) {
            Vector2 p = {0, points[i].y};
            if (cur == 0 || !(cur - 1 >= 0 && fabs(res[cur - 1].x - p.x) < eps && fabs(res[cur - 1].y - p.y) < eps)) {
                res[cur++] = p;
            }
        }
        if (fabs(points[i].y) < eps) {
            Vector2 p = {points[i].x, 0};
            if (cur == 0 || !(cur - 1 >= 0 && fabs(res[cur - 1].x - p.x) < eps && fabs(res[cur - 1].y - p.y) < eps)) {
                res[cur++] = p;
            }
        }
    }
    *resLen = cur;
    return res;
}

void drawPointWithLabel(Vector2 point, Vector2 center, double scale, const char* label) {
    const int size = 15;
    Vector2 pToDraw = {center.x + point.x * scale, center.y - point.y * scale};
    DrawCircleV(pToDraw, CIRCLE_RADUIS, RED);
    DrawTextEx(font, label, pToDraw, font.baseSize, 1, BLACK);
}

void drawGraph(double a, double k, double B) {
    const double step = 1e-3;
    Vector2 center = {(double) WINDOW_WIDTH / 2, (double) WINDOW_HEIGHT / 2};
    int len = (int) (B / step) + 2;
    Vector2* points = calculatePoints(a, k, B, step);
    double min_x = INF, min_y = INF, max_x = -INF, max_y = -INF;
    for (int i = 0; i < len; i++) {
        min_x = fmin(min_x, points[i].x);
        min_y = fmin(min_y, points[i].y);
        max_x = fmax(max_x, points[i].x);
        max_y = fmax(max_y, points[i].y);
    }
    min_x = floor(min_x); min_y = floor(min_y); max_x = ceil(max_x); max_y = ceil(max_y);
    double scale = 1;
    scale = fmin(WINDOW_WIDTH / (fmax(fabs(max_x), fabs(min_x))) / 2, WINDOW_HEIGHT / (fmax(fabs(max_y), fabs(min_y))) / 2) * 0.8;

    for (int i = 1; i < len; i++) {
        Vector2 prev = {center.x + points[i - 1].x * scale, center.y - points[i - 1].y * scale};
        Vector2 cur = {center.x + points[i].x * scale, center.y - points[i].y * scale};
        DrawLineEx(prev, cur, GRAPH_THICK, RED);
    }

    int collisionPointsLen = 0;
    Vector2* collisionPoints = detectCollisionWithAxes(points, len, &collisionPointsLen);
    for (int i = 0; i < collisionPointsLen; i++) {
        drawPointWithLabel(collisionPoints[i], center, scale, TextFormat("(%3.1lf,%3.1lf)", collisionPoints[i].x, collisionPoints[i].y));
    }

    free(points);
    free(collisionPoints);
}


int main(void)
{
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "lab1");
    SetTargetFPS(60);
    font = LoadFontEx("roboto.ttf", 20, 0, 256);

    float a = 5, k = 1, B = PI / 2;

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(WHITE);
        WINDOW_WIDTH = GetScreenWidth();
        WINDOW_HEIGHT = GetScreenHeight();
        drawAxes();
        Rectangle bounds = {100, 100, 100, 20};
        drawGraph(a, k, B);
        GuiSlider((Rectangle){ WINDOW_WIDTH * 0.075, WINDOW_HEIGHT * 0.067, WINDOW_WIDTH * 0.17, WINDOW_HEIGHT * 0.022 }, TextFormat("a: %0.2f", a), NULL, &a, 5, 100);
        GuiSlider((Rectangle){ WINDOW_WIDTH * 0.075, WINDOW_HEIGHT * 0.1, WINDOW_WIDTH * 0.17, WINDOW_HEIGHT * 0.022 }, TextFormat("k: %0.2f", k), NULL, &k, 0, 1.5);
        GuiSlider((Rectangle){ WINDOW_WIDTH * 0.075, WINDOW_HEIGHT * 0.14, WINDOW_WIDTH * 0.17, WINDOW_HEIGHT * 0.022 }, TextFormat("B: %0.2f", B), NULL, &B, 0.1, 5 * PI);
        EndDrawing();
    }

    CloseWindow();

    return 0;
}

