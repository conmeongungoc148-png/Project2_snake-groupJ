#ifndef GAME_H
#define GAME_H

#include "raylib.h"

// Screen and Grid setup
#define GRID_SIZE 30
#define GRID_COUNT_X 30
#define GRID_COUNT_Y 30
#define TOOLBAR_HEIGHT 60
#define WINDOW_WIDTH (GRID_SIZE * GRID_COUNT_X)
#define WINDOW_HEIGHT ((GRID_SIZE * GRID_COUNT_Y) + TOOLBAR_HEIGHT)

// Game Modes
typedef enum {
  MODE_CLASSIC = 0,
  MODE_STAGES,
  MODE_CHAOS
} GameMode;

// Tile types for the game map (matched with stages.h)
typedef enum {
    TILE_EMPTY = 0,
    TILE_WALL,
    TILE_PORTAL
} TileType;

// Game states
typedef enum { STATE_SCOREBOARD = 0, STATE_START, STATE_PLAYING, STATE_GAME_OVER, STATE_GAME_WIN, STATE_NAME_INPUT } GameState;

// Textures loaded from assets
typedef struct {
  Texture2D headTex;
  Texture2D bodyTex;
  Texture2D tailTex;
  Texture2D bgTex;
  Texture2D fruitTex;
  Texture2D wallTex; // New for stages
  Font mainFont;
  Music bgMusic;
} GameAssets;

typedef struct {
  char name[16];
  int score;
} ScoreEntry;

// Shared Game Context
typedef struct GameData {
  GameState state;
  GameMode mode;
  int currentLevel;
  int score;
  ScoreEntry highScores[10];
  char currentName[16];
  int nameLength;
  TileType map[GRID_COUNT_X][GRID_COUNT_Y];
  Vector2 portals[2];
  bool reverseMode;
  bool isPaused;
  GameAssets assets;
} GameData;

// Forward declarations
typedef struct Snake Snake;
typedef struct Food Food;

void InitGameData(GameData *game);
void UpdateGame(GameData *game, Snake *snake, Food *food);
void DrawGame(GameData *game, Snake *snake, Food *food);
void RestartGame(GameData *game, Snake *snake, Food *food);
void LoadScoreboard(GameData *game);
void SaveScoreboard(GameData *game);

#endif // GAME_H
