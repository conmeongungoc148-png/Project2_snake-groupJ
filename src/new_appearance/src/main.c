#include "food.h"
#include "game.h"
#include "raylib.h"
#include "snake.h"

int main(void) {
  InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Snake Game");
  // InitAudioDevice(); // Removed music
  SetTargetFPS(60);

  GameData gameData = {0};
  InitGameData(&gameData);

  // Attempt to load textures. Raylib will harmlessly warn if files are missing
  // and return id = 0. The drawing logic in game.c and snake.c expects this and
  // uses fallback shapes if id == 0.
  gameData.assets.mainFont = LoadFont("assets/font/Inter-ExtraBold.otf");
  gameData.assets.headTex = LoadTexture("assets/snake/head.png");
  gameData.assets.bodyTex = LoadTexture("assets/snake/body.png");
  gameData.assets.tailTex = LoadTexture("assets/snake/body.png");
  gameData.assets.fruitTex = LoadTexture("assets/snake/9.png");
  gameData.assets.bgTex = LoadTexture("assets/background.png");
  gameData.assets.wallTex = LoadTexture("assets/snake/10.png");
  
  // Basic fallbacks
  if (gameData.assets.headTex.id == 0) gameData.assets.headTex = LoadTexture("assets/snake/head.png");
  if (gameData.assets.bodyTex.id == 0) gameData.assets.bodyTex = LoadTexture("body.png");
  if (gameData.assets.fruitTex.id == 0) gameData.assets.fruitTex = LoadTexture("food.png");
  if (gameData.assets.wallTex.id == 0) gameData.assets.wallTex = LoadTexture("food.png"); // Fallback for wall
  // gameData.assets.bgMusic = LoadMusicStream("assets/sound.mp3"); // Removed music

  Snake snake = {0};
  InitSnake(&snake);

  Food food = {0};
  InitFood(&food, &snake, &gameData);

  RestartGame(&gameData, &snake, &food);

  while (!WindowShouldClose()) {
    // UpdateMusicStream(gameData.assets.bgMusic); // Removed music
    UpdateGame(&gameData, &snake, &food);
    DrawGame(&gameData, &snake, &food);
  }

  // Clean up
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

  if (gameData.assets.mainFont.texture.id != 0)
    UnloadFont(gameData.assets.mainFont);

  // UnloadMusicStream(gameData.assets.bgMusic); // Removed music
  // CloseAudioDevice(); // Removed music
  CloseWindow();
  return 0;
}
