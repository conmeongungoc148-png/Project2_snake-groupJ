#include "raylib.h"
#include "game.h"
#include "snake.h"
#include "food.h"

int main(void) {
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Snake Game");
    SetTargetFPS(60);

    GameData gameData = {0};
    InitGameData(&gameData);
    
    // Attempt to load textures. Raylib will harmlessly warn if files are missing and return id = 0.
    // The drawing logic in game.c and snake.c expects this and uses fallback shapes if id == 0.
    gameData.assets.headTex = LoadTexture("assets/snake/head.png");
    gameData.assets.bodyTex = LoadTexture("assets/snake/body.png");
    gameData.assets.tailTex = LoadTexture("assets/snake/tail.png");
    gameData.assets.bgTex = LoadTexture("assets/background/board.jpg");
    gameData.assets.fruitTex = LoadTexture("assets/background/fruit.png");

    Snake snake = {0};
    InitSnake(&snake);

    Food food = {0};
    InitFood(&food, &snake);

    RestartGame(&gameData, &snake, &food);

    while (!WindowShouldClose()) {
        UpdateGame(&gameData, &snake, &food);
        DrawGame(&gameData, &snake, &food);
    }

    // Clean up
    if (gameData.assets.headTex.id != 0) UnloadTexture(gameData.assets.headTex);
    if (gameData.assets.bodyTex.id != 0) UnloadTexture(gameData.assets.bodyTex);
    if (gameData.assets.tailTex.id != 0) UnloadTexture(gameData.assets.tailTex);
    if (gameData.assets.bgTex.id != 0) UnloadTexture(gameData.assets.bgTex);
    if (gameData.assets.fruitTex.id != 0) UnloadTexture(gameData.assets.fruitTex);

    CloseWindow();
    return 0;
}
