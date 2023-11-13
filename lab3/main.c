#include "raylib.h"

#define RLIGHTS_IMPLEMENTATION
#include "rlights.h"

#define GLSL_VERSION 330

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
Vector3 CAMERA_SIDE_POSITION = (Vector3){ 1.0f, 0.5f, 0.5f };
Vector3 CAMERA_TOP_POSITION = (Vector3){ 0.0f, 1.0f, 0.0f };

Font font;

typedef struct {
    Vector3 a, b, c;
} Triangle;

Vector3 rotateY(Vector3 v, double angle) {
    return (Vector3) {
        v.x * cos(angle) + v.z * sin(angle),
        v.y,
        -v.x * sin(angle) + v.z * cos(angle),
    };
}

Vector3 calculateNormal(Vector3 a, Vector3 b, Vector3 c) {
    Vector3 dir = Vector3CrossProduct(Vector3Subtract(b, a), Vector3Subtract(c, a));
    return Vector3Normalize(dir);
}

Mesh* mesh = NULL;
Model model = { 0 };
Image image;
Shader shader;

int indVert = 0, indNorm = 0, indTex = 0;

void meshAddTriangle(Vector3 a, Vector3 b, Vector3 c) {
    Vector3 normal = calculateNormal(a, b, c);
    mesh->vertices[indVert++] = a.x;
    mesh->vertices[indVert++] = a.y;
    mesh->vertices[indVert++] = a.z;

    mesh->vertices[indVert++] = b.x;
    mesh->vertices[indVert++] = b.y;
    mesh->vertices[indVert++] = b.z;

    mesh->vertices[indVert++] = c.x;
    mesh->vertices[indVert++] = c.y;
    mesh->vertices[indVert++] = c.z;

    for (int i = 0; i < 3; i++) {
        mesh->normals[indNorm++] = normal.x;
        mesh->normals[indNorm++] = normal.y;
        mesh->normals[indNorm++] = normal.z;

        mesh->texcoords[indTex++] = 0;
        mesh->texcoords[indTex++] = 0;
    }
}

void loadMesh(int accuracy, int r, Vector3 center, Vector3 topCenter) {
    mesh = malloc(sizeof(Mesh));
    mesh->vertexCount = ((accuracy - 1) * (accuracy - 1) * 2 + (accuracy - 1) * 3 + 2) * 3;
    mesh->vertices = malloc(sizeof(float) * mesh->vertexCount * 3);
    mesh->texcoords = malloc(sizeof(float) * mesh->vertexCount * 2);
    mesh->normals = malloc(sizeof(float) * mesh->vertexCount * 3);
    Vector3 levels[accuracy][accuracy];
    double delta = PI / 2 / accuracy;
    for (int i = 0; i < accuracy; i++) {
        for (int j = 0; j < accuracy; j++) {
            double alpha = delta * i, phi = delta * j;
            levels[i][j] = (Vector3) {
                    r * cos(alpha) * cos(phi),
                    r * sin(alpha),
                    r * cos(alpha) * sin(phi)};
        }
    }
    for (int i = 0; i < accuracy - 1; i++) {
        for (int j = 0; j < accuracy - 1; j++) {
            meshAddTriangle(levels[i][j], levels[i + 1][j], levels[i + 1][j + 1]);
            meshAddTriangle(levels[i][j], levels[i + 1][j + 1], levels[i][j + 1]);
        }
        meshAddTriangle(center, levels[i + 1][0], levels[i][0]);
        meshAddTriangle(center, levels[i][accuracy - 1], levels[i + 1][accuracy - 1]);
        meshAddTriangle((Vector3) topCenter, levels[accuracy - 1][i + 1], levels[accuracy - 1][i]);
    }
    meshAddTriangle(center, topCenter, levels[accuracy - 1][0]);
    meshAddTriangle(center, levels[accuracy - 1][accuracy - 1], topCenter);
    UploadMesh(mesh, false);
}

void loadModel(int accuracy, int r, Vector3 center, Vector3 topCenter) {
    loadMesh(accuracy, r, center, topCenter);

    model = LoadModelFromMesh(*mesh);

    // Load texture
    image = GenImageColor(2, 2, WHITE); 
    Texture2D texture = LoadTextureFromImage(image);
    model.materials[0].maps[MATERIAL_MAP_METALNESS].texture = texture;

    model.materials[0].shader = shader;
    UnloadImage(image);
}

int main(void) {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "lab3");
    SetTargetFPS(60);
    font = LoadFontEx("roboto.ttf", 20, 0, 256);

    Vector3 center = {0, 0, 0};
    Vector3 target = {0.5, 0.5, 0.5};
    
    Camera camera = { 0 };
    camera.position = CAMERA_ISOMETRIC_POSITION; 
    camera.target = target; 
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };         
    camera.fovy = 45.0f;                                
    camera.projection = CAMERA_PERSPECTIVE;             

    double dphi = 0.01;
    int rotate = 0, zoomOut = 0, zoomIn = 0;
    double zoomC = 5;
    Vector3 curPosition = CAMERA_ISOMETRIC_POSITION; 

    float accuracy = 100;
    double r = 1;
    Vector3 topCenter = {0, r, 0};
    
    // Load shader
    shader = LoadShader("resources/lighting.vs", "resources/lighting.fs");
    shader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(shader, "viewPos");

    loadModel(accuracy, r, center, topCenter);

    int ambientLoc = GetShaderLocation(shader, "ambient");
    SetShaderValue(shader, ambientLoc, (float[4]){ 0.1f, 0.1f, 0.1f, 1.0f }, SHADER_UNIFORM_VEC4);

    Light light = CreateLight(LIGHT_POINT, (Vector3) {1, 0.5, 1}, topCenter, WHITE, shader); 
    light.enabled = false;
    UpdateLightValues(shader, light);

    double angle = 0;
    while (!WindowShouldClose()) {
        int last_accuracy = accuracy;
        int top = 0, side = 0, isometric = 0, lightToggled = 0;
        rotate ^= GuiButton((Rectangle){ GetScreenWidth() * 0.078, GetScreenHeight() * 0.139, GetScreenWidth() * 0.078, GetScreenHeight() * 0.041  }, TextFormat("Rotate"));
        GuiSlider((Rectangle){ GetScreenWidth() * 0.078, GetScreenHeight() * 0.208, GetScreenWidth() * 0.078, GetScreenHeight() * 0.041  }, TextFormat("accuracy: %0.0f", accuracy), NULL, &accuracy, 5, 100);
        zoomOut = GuiButton((Rectangle){ GetScreenWidth() * 0.078, GetScreenHeight() * 0.277, GetScreenWidth() * 0.078, GetScreenHeight() * 0.041  }, TextFormat("Zoom Out"));
        zoomIn = GuiButton((Rectangle){ GetScreenWidth() * 0.078, GetScreenHeight() * 0.347, GetScreenWidth() * 0.078, GetScreenHeight() * 0.041  }, TextFormat("Zoom In"));
        top = GuiButton((Rectangle){ GetScreenWidth() * 0.078, GetScreenHeight() * 0.416, GetScreenWidth() * 0.078, GetScreenHeight() * 0.041  }, TextFormat("Top"));
        side = GuiButton((Rectangle){ GetScreenWidth() * 0.078, GetScreenHeight() * 0.486, GetScreenWidth() * 0.078, GetScreenHeight() * 0.041  }, TextFormat("Side"));
        isometric = GuiButton((Rectangle){ GetScreenWidth() * 0.078, GetScreenHeight() * 0.555, GetScreenWidth() * 0.078, GetScreenHeight() * 0.041  }, TextFormat("Isometric"));
        lightToggled = GuiButton((Rectangle){ GetScreenWidth() * 0.078, GetScreenHeight() * 0.625, GetScreenWidth() * 0.078, GetScreenHeight() * 0.041  }, TextFormat("Light"));

        if (last_accuracy != accuracy) {
            indVert = 0; indTex = 0; indNorm = 0;
            UnloadModel(model);
            loadModel(accuracy, r, center, topCenter);
        }

        if (lightToggled) {
            light.enabled = !light.enabled;
        }
        UpdateLightValues(shader, light);

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
                if (rotate) {
                    angle += dphi;
                    if (angle > 2 * PI) angle -= 2 * PI;
                }
                DrawModelEx(model, center, (Vector3) {0, 1, 0}, angle * 180 / PI, (Vector3) {1, 1, 1}, WHITE);
                DrawGrid(10, 1);
            EndMode3D();
            DrawFPS(0, 0);
        EndDrawing();
    }

    UnloadShader(shader);
    UnloadModel(model);
    CloseWindow();

    return 0;
}

