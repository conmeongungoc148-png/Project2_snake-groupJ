#ifndef GAME_H
#define GAME_H

#include "raylib.h"

// Screen and Grid setup
#define GRID_SIZE 30
#define GRID_COUNT_X 30
#define GRID_COUNT_Y 30
#define WINDOW_WIDTH (GRID_SIZE * GRID_COUNT_X)
#define WINDOW_HEIGHT (GRID_SIZE * GRID_COUNT_Y)

// Game states
typedef enum { STATE_START = 0, STATE_PLAYING, STATE_GAME_OVER } GameState;

// Textures loaded from assets
typedef struct {
  Texture2D headTex;
  Texture2D bodyTex;
  Texture2D tailTex;
  Texture2D bgTex;
  Texture2D fruitTex;
  Font mainFont;
} GameAssets;

// Shared Game Context
typedef struct {
  GameState state;
  int score;
  int highScore;
  GameAssets assets;
} GameData;

// Forward declarations
typedef struct Snake Snake;
typedef struct Food Food;

void InitGameData(GameData *game);
void UpdateGame(GameData *game, Snake *snake, Food *food);
void DrawGame(GameData *game, Snake *snake, Food *food);
void RestartGame(GameData *game, Snake *snake, Food *food);
int LoadHighScore(void);
void SaveHighScore(int score);

#endif // GAME_H
