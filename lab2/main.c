#include "raylib.h"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#define RAYMATH_IMPEMENTATION
#include "raymath.h"

#include <stdlib.h>
#include <math.h>
#include <stdio.h>

#define INF 1000000007
#define AXES_THICK 2
#define GRAPH_THICK 2
#define CIRCLE_RADUIS 4

int WINDOW_WIDTH = 1280;
int WINDOW_HEIGHT = 720;

Vector3 CAMERA_ISOMETRIC_POSITION = (Vector3){ 1.0f, 1.0f, 1.0f };
Vector3 CAMERA_SIDE_POSITION = (Vector3){ 1.0f, 0.5f, 0.0f };
Vector3 CAMERA_TOP_POSITION = (Vector3){ 0.0f, 1.0f, 0.0f };

Font font;

typedef struct {
    Vector3 a, b;
} Edge;

typedef struct {
    int cnt;
    Vector3 points[100];
} Plane;

Vector3 rotateY(Vector3 v, double angle) {
    return (Vector3) {
        v.x * cos(angle) + v.z * sin(angle),
        v.y,
        -v.x * sin(angle) + v.z * cos(angle),
    };
}

double Vector3LineDistance(Vector3 v, Vector3 w, Vector3 p) {
    double l = Vector3Distance(v, w);
    if (l == 0) return Vector3Distance(v, p);
    double t = fmax(0.0, fmin(1.0, Vector3DotProduct(Vector3Subtract(p, v), Vector3Subtract(w, v)) / l));
    Vector3 projection = Vector3Add(v, Vector3Scale(Vector3Subtract(w, v), t));
    return Vector3Distance(p, projection);
}

Vector3 origin, center;

int compare (const void* a, const void* b) {
    Vector3 vecA = Vector3Subtract(*((Vector3*) a), center);
    Vector3 vecB = Vector3Subtract(*((Vector3*) b), center);

    float angleA = Vector3Angle(origin, vecA);
    float angleB = Vector3Angle(origin, vecB);

    if (angleA < angleB) return -1;
    return 1;
}

void drawShape(Vector3* points, int len, Color color) {
    center = (Vector3) {0, 0, 0};
    for (int i = 0; i < len; i++) {
        center = Vector3Add(center, points[i]);
    } 
    center = Vector3Scale(center, 1.0/len);
    origin = Vector3Subtract(points[0], center);
    qsort(points, len, sizeof(Vector3), compare);
    for (int i = 1; i < len - 1; i++) {
        for (int j = i + 1; j < len; j++) {
            Vector3 a = Vector3Add(center, Vector3Scale(Vector3Subtract(points[j], center), 0.99));
            Vector3 b = Vector3Add(center, Vector3Scale(Vector3Subtract(points[i], center), 0.99));
            Vector3 c = Vector3Add(center, Vector3Scale(Vector3Subtract(points[0], center), 0.99));
            DrawTriangle3D(a, b, c, color);
            DrawTriangle3D(c, b, a, color);
        }
    }
}

int main(void)
{
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "lab2");
    SetTargetFPS(60);
    font = LoadFontEx("roboto.ttf", 20, 0, 256);

    Vector3 center = {0, 0, 0};
    FILE* fin = fopen("planes.txt", "r");
    int edgesCnt;
    fscanf(fin, "%d", &edgesCnt);
    Edge edges[edgesCnt];
    for (int i = 0; i < edgesCnt; i++) {
        fscanf(fin, "%f %f %f %f %f %f",
                &edges[i].a.x,
                &edges[i].a.y,
                &edges[i].a.z,
                &edges[i].b.x,
                &edges[i].b.y,
                &edges[i].b.z);
        center = Vector3Add(center, Vector3Add(edges[i].a, edges[i].b));
    }
    center = Vector3Scale(center, 1.0/(edgesCnt * 2));
    int planesCnt;
    fscanf(fin, "%d", &planesCnt);
    Plane planes[planesCnt];
    for (int i = 0; i < planesCnt; i++) {
        int n;
        fscanf(fin, "%d", &n);
        planes[i].cnt = n;
        for (int j = 0; j < n; j++) {
            fscanf(fin, "%f %f %f", &planes[i].points[j].x, &planes[i].points[j].y, &planes[i].points[j].z);
        }
    }
    fclose(fin);

    
    int len = sizeof(edges) / sizeof(Edge);
    double diameter = -10000000;


    for (int i = 0; i < edgesCnt; i++) {
        for (int j = i + 1; j < edgesCnt; j++) {
            diameter = fmax(diameter, Vector3Distance(edges[i].a, edges[j].a));
            diameter = fmax(diameter, Vector3Distance(edges[i].a, edges[j].b));
            diameter = fmax(diameter, Vector3Distance(edges[i].b, edges[j].a));
            diameter = fmax(diameter, Vector3Distance(edges[i].b, edges[j].b));
        }
    }
    
    Camera camera = { 0 };
    camera.position = CAMERA_ISOMETRIC_POSITION; 
    camera.target = center; 
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };         
    camera.fovy = 45.0f;                                
    camera.projection = CAMERA_PERSPECTIVE;             

    double dphi = 0.01;
    int rotate = 0, showLines = 1, zoomOut = 0, zoomIn = 0;
    double zoomC = diameter;
    Vector3 curPosition = CAMERA_ISOMETRIC_POSITION; 
    while (!WindowShouldClose()) {
        int top = 0, side = 0, isometric = 0;
        rotate ^= GuiButton((Rectangle){ GetScreenWidth() * 0.078, GetScreenHeight() * 0.139, GetScreenWidth() * 0.078, GetScreenHeight() * 0.041  }, TextFormat("Rotate"));
        showLines ^= GuiButton((Rectangle){ GetScreenWidth() * 0.078, GetScreenHeight() * 0.208, GetScreenWidth() * 0.078, GetScreenHeight() * 0.041  }, TextFormat("Show Lines"));
        zoomOut = GuiButton((Rectangle){ GetScreenWidth() * 0.078, GetScreenHeight() * 0.277, GetScreenWidth() * 0.078, GetScreenHeight() * 0.041  }, TextFormat("Zoom Out"));
        zoomIn = GuiButton((Rectangle){ GetScreenWidth() * 0.078, GetScreenHeight() * 0.347, GetScreenWidth() * 0.078, GetScreenHeight() * 0.041  }, TextFormat("Zoom In"));
        top = GuiButton((Rectangle){ GetScreenWidth() * 0.078, GetScreenHeight() * 0.416, GetScreenWidth() * 0.078, GetScreenHeight() * 0.041  }, TextFormat("Top"));
        side = GuiButton((Rectangle){ GetScreenWidth() * 0.078, GetScreenHeight() * 0.486, GetScreenWidth() * 0.078, GetScreenHeight() * 0.041  }, TextFormat("Side"));
        isometric = GuiButton((Rectangle){ GetScreenWidth() * 0.078, GetScreenHeight() * 0.555, GetScreenWidth() * 0.078, GetScreenHeight() * 0.041  }, TextFormat("Isometric"));

        if (rotate) {
            for (int i = 0; i < edgesCnt; i++) {
                edges[i].a = Vector3Add(center, rotateY(Vector3Add(edges[i].a, Vector3Scale(center, -1)), dphi));
                edges[i].b = Vector3Add(center, rotateY(Vector3Add(edges[i].b, Vector3Scale(center, -1)), dphi));
            }
            for (int i = 0; i < planesCnt; i++) {
                for (int j = 0; j < planes[i].cnt; j++) {
                    planes[i].points[j] = Vector3Add(center, rotateY(Vector3Add(planes[i].points[j], Vector3Scale(center, -1)), dphi));
                }
            }
        }
        
        if (top) {
            curPosition = CAMERA_TOP_POSITION;
        } else if (side) {
            curPosition = CAMERA_SIDE_POSITION;
        } else if (isometric) {
            curPosition = CAMERA_ISOMETRIC_POSITION;
        }

        if (zoomOut) {
            zoomC += 0.5;
        }

        if (zoomIn) {
            zoomC -= 0.5;
        }
        
        camera.position = Vector3Add(center, Vector3Scale(Vector3Subtract(curPosition, center), zoomC));


        BeginDrawing();
            ClearBackground(WHITE);
            WINDOW_WIDTH = GetScreenWidth();
            WINDOW_HEIGHT = GetScreenHeight();
            BeginMode3D(camera);
                if (!showLines) {
                    for (int i = 0; i < planesCnt; i++) {
                       drawShape(planes[i].points, planes[i].cnt, WHITE);
                    } 
                }
                for (int i = 0; i < len; i++) {
                    DrawLine3D(edges[i].a, edges[i].b, RED);
                }
            EndMode3D();
        EndDrawing();
    }

    CloseWindow();

    return 0;
}

