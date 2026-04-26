#include "display.h"
#include "food.h"
#include "game.h"
#include "raylib.h"
#include "snake.h"

int main(void) {
  InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Sausage Dog Game");
  SetTargetFPS(60);

  GameData gameData = {0};
  InitGameData(&gameData);

  
  
  
  gameData.assets.mainFont = LoadFont("assets/font/04B_30__.TTF");
  gameData.assets.headTex = LoadTexture("assets/snake/head.png");
  gameData.assets.bodyTex = LoadTexture("assets/snake/body.png");
  gameData.assets.tailTex = LoadTexture("assets/snake/tail.png");
  gameData.assets.bgTex = LoadTexture("assets/snake/background.png");
  if (gameData.assets.bgTex.id == 0) {
      TraceLog(LOG_ERROR, "Failed to load background.png");
  } else {
      TraceLog(LOG_INFO, TextFormat("Loaded background.png (%dx%d)", gameData.assets.bgTex.width, gameData.assets.bgTex.height));
  }
  gameData.assets.fruitTex = LoadTexture("assets/snake/food.png");
  gameData.assets.wallTex = LoadTexture("assets/snake/wall.png");

  Snake snake = {0};
  InitSnake(&snake);

  Food food = {0};
  InitFood(&food, &snake, &gameData);

  RestartGame(&gameData, &snake, &food);

  while (!WindowShouldClose()) {
    UpdateGame(&gameData, &snake, &food);
    DrawGame(&gameData, &snake, &food);
  }

  
  if (gameData.assets.headTex.id != 0)
    UnloadTexture(gameData.assets.headTex);
  if (gameData.assets.bodyTex.id != 0)
    UnloadTexture(gameData.assets.bodyTex);
  if (gameData.assets.tailTex.id != 0)
    UnloadTexture(gameData.assets.tailTex);
  if (gameData.assets.bgTex.id != 0)
    UnloadTexture(gameData.assets.bgTex);
  if (gameData.assets.fruitTex.id != 0)
    UnloadTexture(gameData.assets.fruitTex);
  if (gameData.assets.wallTex.id != 0)
    UnloadTexture(gameData.assets.wallTex);

  if (gameData.assets.mainFont.texture.id != 0)
    UnloadFont(gameData.assets.mainFont);

  CloseWindow();
  return 0;
}
