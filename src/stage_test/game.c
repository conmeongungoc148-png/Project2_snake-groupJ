#include "game.h"
#include "food.h"
#include "snake.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

const char *HIGHSCORE_FILE = "highscore.txt";

int LoadHighScore(void) {
  FILE *file = fopen(HIGHSCORE_FILE, "r");
  if (!file)
    return 0;
  int high = 0;
  fscanf(file, "%d", &high);
  fclose(file);
  return high;
}

void SaveHighScore(int score) {
  FILE *file = fopen(HIGHSCORE_FILE, "w");
  if (file) {
    fprintf(file, "%d", score);
    fclose(file);
  }
}

void InitGameData(GameData *game) {
  game->state = STATE_START;
  game->score = 0;
  game->highScore = LoadHighScore();
  game->reverseMode = false;
  game->mode = MODE_CLASSIC;
  game->currentLevel = 1;
  game->devModeActive = false;
  game->devLevelSelect = 1;
  game->moveIntervalMs = DEFAULT_INTERVAL_MS;
  // Initial map is empty for Classic
  for (int x = 0; x < GRID_COUNT_X; x++)
    for (int y = 0; y < GRID_COUNT_Y; y++)
      game->map[x][y] = TILE_EMPTY;
}

void RestartGame(GameData *game, Snake *snake, Food *food) {
  game->score = 0;
  InitSnake(snake);
  snake->speed = 1000.0f / (float)game->moveIntervalMs;

  if (game->mode == MODE_STAGES) {
    LoadLevel(game, 1, snake);
  } else if (game->mode == MODE_CHAOS) {
    InitChaosMode(game, snake);
  } else {
    // Classic
    for (int x = 0; x < GRID_COUNT_X; x++)
      for (int y = 0; y < GRID_COUNT_Y; y++)
        game->map[x][y] = TILE_EMPTY;
  }

  InitFood(food, snake, game);
}

void UpdateGame(GameData *game, Snake *snake, Food *food) {
  if (game->state == STATE_START) {
    if (IsKeyPressed(KEY_D) &&
        (game->mode == MODE_CHAOS || game->mode == MODE_STAGES)) {
      game->devModeActive = !game->devModeActive;
    }

    if (game->devModeActive &&
        (game->mode == MODE_CHAOS || game->mode == MODE_STAGES)) {
      if (IsKeyPressed(KEY_LEFT)) {
        game->devLevelSelect--;
        if (game->devLevelSelect < 1)
          game->devLevelSelect = 1;
      }
      if (IsKeyPressed(KEY_RIGHT)) {
        game->devLevelSelect++;
        int max = (game->mode == MODE_STAGES) ? 4 : 5;
        if (game->devLevelSelect > max)
          game->devLevelSelect = max;
      }
    }

    if (IsKeyPressed(KEY_ENTER)) {
      game->state = STATE_PLAYING;
      RestartGame(game, snake, food);

      if (game->devModeActive) {
        if (game->mode == MODE_CHAOS) {
          game->currentLevel = game->devLevelSelect;
          game->score = (game->currentLevel - 1) * 100;
          void LoadChaosStage(GameData * game, int level, Snake *snake);
          LoadChaosStage(game, game->currentLevel, snake);
        } else if (game->mode == MODE_STAGES) {
          game->currentLevel = game->devLevelSelect;
          // Jump to 400 if picking level 4, otherwise standard intervals
          game->score =
              (game->currentLevel == 4) ? 400 : (game->currentLevel - 1) * 100;
          LoadLevel(game, game->currentLevel, snake);
        }
      }
    }
    if (IsKeyPressed(KEY_M)) {
      game->reverseMode = !game->reverseMode;
    }
    if (IsKeyPressed(KEY_L)) {
      // Cycle modes: Classic -> Stages -> Chaos
      game->mode = (game->mode + 1) % 3;
    }

    // Speed adjustment (10ms increments)
    if (IsKeyPressed(KEY_RIGHT) && !game->devModeActive) {
        game->moveIntervalMs -= 10;
        if (game->moveIntervalMs < MIN_INTERVAL_MS) game->moveIntervalMs = MIN_INTERVAL_MS;
    }
    if (IsKeyPressed(KEY_LEFT) && !game->devModeActive) {
        game->moveIntervalMs += 10;
        if (game->moveIntervalMs > MAX_INTERVAL_MS) game->moveIntervalMs = MAX_INTERVAL_MS;
    }
  } else if (game->state == STATE_PLAYING) {
    HandleSnakeInput(snake);

    UpdateSnakeLogic(snake, game);

    // Check collisions
    if (CheckCollisionWithSelfOrWall(snake, game)) {
      game->state = STATE_GAME_OVER;
      if (game->score > game->highScore) {
        game->highScore = game->score;
        SaveHighScore(game->highScore);
      }
    }

    // Check food eating
    // Use lookahead so the snake eats as soon as its head enters the food tile
    int headGridX = (int)(snake->body[0].x + snake->direction.x * 0.4f);
    int headGridY = (int)(snake->body[0].y + snake->direction.y * 0.4f);

    if (headGridX == (int)food->position.x && headGridY == (int)food->position.y) {
        snake->justAteFood = true;
        game->score += 10;

        // Level Progression
        if (game->mode == MODE_STAGES) {
          if (game->currentLevel == 1 && game->score >= 100) {
            LoadLevel(game, 2, snake);
          } else if (game->currentLevel == 2 && game->score >= 200) {
            LoadLevel(game, 3, snake);
          } else if (game->currentLevel == 3 && game->score >= 400) {
            LoadLevel(game, 4, snake);
          }

          if (game->score >= 500) {
            game->state = STATE_GAME_WIN;
            if (game->score > game->highScore) {
              game->highScore = game->score;
              SaveHighScore(game->score);
            }
          }
        }

        // Chaos Mode Progression
        if (game->mode == MODE_CHAOS) {
          bool levelUp = false;
          if (game->currentLevel == 1 && game->score >= 100) {
            game->currentLevel = 2;
            levelUp = true;
          } else if (game->currentLevel == 2 && game->score >= 200) {
            game->currentLevel = 3;
            levelUp = true;
          } else if (game->currentLevel == 3 && game->score >= 300) {
            game->currentLevel = 4;
            levelUp = true;
          } else if (game->currentLevel == 4 && game->score >= 400) {
            game->currentLevel = 5;
            levelUp = true;
          }

          if (levelUp) {
            void LoadChaosStage(GameData * game, int level, Snake *snake);
            LoadChaosStage(game, game->currentLevel, snake);
            snake->invulnerabilityTimer = 3.0f; // 3 Seconds of Ghost Mode
          }

          if (game->score >= 500) {
            game->state = STATE_GAME_WIN;
            if (game->score > game->highScore) {
              game->highScore = game->score;
              SaveHighScore(game->score);
            }
          }
        }

        SpawnFood(food, snake, game);

        if (game->reverseMode) {
          ReverseSnake(snake);
        }
      }
  } else if (game->state == STATE_GAME_OVER || game->state == STATE_GAME_WIN) {
    if (IsKeyPressed(KEY_ENTER)) {
      game->state = STATE_PLAYING;
      RestartGame(game, snake, food);
    }
  }
}

void DrawGame(GameData *game, Snake *snake, Food *food) {
  BeginDrawing();
  ClearBackground(RAYWHITE);

  // Draw Background if loaded
  if (game->assets.bgTex.id != 0) {
    DrawTexturePro(game->assets.bgTex,
                   (Rectangle){0, 0, (float)game->assets.bgTex.width,
                               (float)game->assets.bgTex.height},
                   (Rectangle){0, 0, WINDOW_WIDTH, WINDOW_HEIGHT},
                   (Vector2){0, 0}, 0.0f, WHITE);
  } else {
    ClearBackground(DARKGRAY);
  }

  // Draw Grid (Light overlay)
  Color gridColor = (Color){255, 255, 255, 40}; // Semi-transparent white
  for (int i = 0; i <= WINDOW_WIDTH; i += GRID_SIZE) {
    DrawLine(i, 0, i, WINDOW_HEIGHT, gridColor);
  }
  for (int i = 0; i <= WINDOW_HEIGHT; i += GRID_SIZE) {
    DrawLine(0, i, WINDOW_WIDTH, i, gridColor);
  }

  // Draw entities
  if (game->state == STATE_PLAYING || game->state == STATE_GAME_OVER) {
    // Draw Walls
    for (int x = 0; x < GRID_COUNT_X; x++) {
      for (int y = 0; y < GRID_COUNT_Y; y++) {
        if (game->map[x][y] == TILE_WALL) {
          if (game->assets.wallTex.id != 0) {
            Rectangle source = {0, 0, (float)game->assets.wallTex.width,
                                (float)game->assets.wallTex.height};
            Rectangle dest = {(float)x * GRID_SIZE, (float)y * GRID_SIZE,
                              (float)GRID_SIZE, (float)GRID_SIZE};
            DrawTexturePro(game->assets.wallTex, source, dest, (Vector2){0},
                           0.0f, WHITE);
          } else {
            DrawRectangle(x * GRID_SIZE, y * GRID_SIZE, GRID_SIZE, GRID_SIZE,
                          DARKGRAY);
            DrawRectangleLines(x * GRID_SIZE, y * GRID_SIZE, GRID_SIZE,
                               GRID_SIZE, GRAY);
          }
        } else if (game->map[x][y] == TILE_PORTAL) {
          // Draw glowing portals
          float time = (float)GetTime();
          float pulse = (sinf(time * 5.0f) + 1.0f) * 0.2f +
                        0.8f; // Pulse between 0.8 and 1.2
          Color portalColor =
              (game->portals[0].x == x && game->portals[0].y == y) ? SKYBLUE
                                                                   : ORANGE;

          Vector2 center = {x * GRID_SIZE + GRID_SIZE / 2.0f,
                            y * GRID_SIZE + GRID_SIZE / 2.0f};
          DrawCircleGradient((int)center.x, (int)center.y,
                             (GRID_SIZE / 2.0f) * pulse, portalColor,
                             (Color){0, 0, 0, 0});
          DrawCircleLines((int)center.x, (int)center.y,
                          (GRID_SIZE / 2.0f) * pulse, WHITE);
        }
      }
    }

    DrawFood(food, game->assets.fruitTex);
    DrawSnake(snake, game->assets.headTex, game->assets.bodyTex,
              game->assets.tailTex);
  }

  // Draw UI
  // Draw "Best Score" at the top right on ALL screens (improved visibility)
  DrawRectangle(WINDOW_WIDTH - 160, 5, 155, 30,
                (Color){0, 0, 0, 100}); // Shadow box
  DrawText(TextFormat("BEST: %d", game->highScore), WINDOW_WIDTH - 150, 10, 20,
           GOLD);

  if (game->state == STATE_START) {
    DrawText("SNAKE GAME", WINDOW_WIDTH / 2 - MeasureText("SNAKE GAME", 50) / 2,
             WINDOW_HEIGHT / 2 - 70, 50, GREEN);
    DrawText("Press ENTER to start",
             WINDOW_WIDTH / 2 - MeasureText("Press ENTER to start", 20) / 2,
             WINDOW_HEIGHT / 2, 20, WHITE);

    const char *modeText = game->reverseMode
                               ? "Reverse Mode: ON (M to toggle)"
                               : "Reverse Mode: OFF (M to toggle)";
    Color modeColor = game->reverseMode ? GOLD : LIGHTGRAY;
    DrawText(modeText, WINDOW_WIDTH / 2 - MeasureText(modeText, 18) / 2,
             WINDOW_HEIGHT / 2 + 40, 18, modeColor);

    const char *stageText = "Mode: UNKNOWN";
    Color stageColor = WHITE;
    if (game->mode == MODE_CLASSIC) {
      stageText = "Mode: CLASSIC (L to toggle)";
      stageColor = LIGHTGRAY;
    } else if (game->mode == MODE_STAGES) {
      stageText = "Mode: STAGES (L to toggle)";
      stageColor = LIME;
    } else if (game->mode == MODE_CHAOS) {
      stageText = "Mode: CHAOS (L to toggle)";
      stageColor = MAGENTA;
    }

    DrawText(stageText, WINDOW_WIDTH / 2 - MeasureText(stageText, 18) / 2,
             WINDOW_HEIGHT / 2 + 70, 18, stageColor);

    if (game->mode == MODE_CHAOS || game->mode == MODE_STAGES) {
      const char *devPrompt =
          game->devModeActive ? "Dev Mode: ON [D]" : "Dev Mode: OFF [D]";
      Color devColor = game->devModeActive ? GOLD : DARKGRAY;
      DrawText(devPrompt, WINDOW_WIDTH / 2 - MeasureText(devPrompt, 18) / 2,
               WINDOW_HEIGHT / 2 + 100, 18, devColor);

      if (game->devModeActive) {
        if (game->mode == MODE_CHAOS) {
          const char *stageNames[] = {"Stage 1", "Stage 2 (Corners)",
                                      "Stage 3 (Pillars)", "Stage 4 (Extreme)",
                                      "Final Stage (Castle)"};
          int scores[] = {0, 100, 200, 300, 400};
          const char *selectText = TextFormat(
              "< %s | Start at %d pts >", stageNames[game->devLevelSelect - 1],
              scores[game->devLevelSelect - 1]);
          DrawText(selectText,
                   WINDOW_WIDTH / 2 - MeasureText(selectText, 16) / 2,
                   WINDOW_HEIGHT / 2 + 130, 16, YELLOW);
        } else if (game->mode == MODE_STAGES) {
          const char *stageNames[] = {"Level 1", "Level 2 (Corners)",
                                      "Level 3 (Pillars)", "Level 4 (Castle)"};
          int scores[] = {0, 100, 200, 400};
          int sel = game->devLevelSelect > 4 ? 4 : game->devLevelSelect;
          const char *selectText = TextFormat(
              "< %s | Start at %d pts >", stageNames[sel - 1], scores[sel - 1]);
          DrawText(selectText,
                   WINDOW_WIDTH / 2 - MeasureText(selectText, 16) / 2,
                   WINDOW_HEIGHT / 2 + 130, 16, YELLOW);
        }
      }
    }

    // Draw Speed Level
    // Calculate a display level based on intervals (1 to 27)
    int displayLevel = (MAX_INTERVAL_MS - game->moveIntervalMs) / 10 + 1;
    const char* speedName = "Beginner";
    if (game->moveIntervalMs <= 100) speedName = "Intermediate";
    if (game->moveIntervalMs <= 60) speedName = "Expert";
    if (game->moveIntervalMs <= 40) speedName = "Insane";

    const char* speedText = TextFormat("Speed Level %d (%s): %d ms/tile", 
                                        displayLevel, speedName, game->moveIntervalMs);
    DrawText(speedText, WINDOW_WIDTH / 2 - MeasureText(speedText, 16) / 2,
             WINDOW_HEIGHT / 2 + 160, 16, SKYBLUE);
    DrawText("< Left/Right to adjust speed >", 
             WINDOW_WIDTH / 2 - MeasureText("< Left/Right to adjust speed >", 14) / 2,
             WINDOW_HEIGHT / 2 + 185, 14, DARKGRAY);

  } else if (game->state == STATE_PLAYING) {
    DrawRectangle(5, 5, 150, 30, (Color){0, 0, 0, 100}); // Score shadow box
    DrawText(TextFormat("Score: %d", game->score), 10, 10, 20, WHITE);
  } else if (game->state == STATE_GAME_OVER) {
    DrawText("GAME OVER!", WINDOW_WIDTH / 2 - MeasureText("GAME OVER!", 50) / 2,
             WINDOW_HEIGHT / 2 - 50, 50, RED);
    DrawText(TextFormat("Final Score: %d", game->score),
             WINDOW_WIDTH / 2 -
                 MeasureText(TextFormat("Final Score: %d", game->score), 20) /
                     2,
             WINDOW_HEIGHT / 2 + 10, 20, WHITE);
    DrawText("Press ENTER to try again",
             WINDOW_WIDTH / 2 - MeasureText("Press ENTER to try again", 20) / 2,
             WINDOW_HEIGHT / 2 + 40, 20, LIGHTGRAY);
  } else if (game->state == STATE_GAME_WIN) {
    DrawText("CHAMPION!", WINDOW_WIDTH / 2 - MeasureText("CHAMPION!", 60) / 2,
             WINDOW_HEIGHT / 2 - 70, 60, GOLD);
    DrawText("You conquered the Castle!",
             WINDOW_WIDTH / 2 -
                 MeasureText("You conquered the Castle!", 20) / 2,
             WINDOW_HEIGHT / 2, 20, WHITE);
    DrawText("Chaos Mode Clear (500 pts)",
             WINDOW_WIDTH / 2 -
                 MeasureText("Chaos Mode Clear (500 pts)", 18) / 2,
             WINDOW_HEIGHT / 2 + 30, 18, SKYBLUE);
    DrawText("Press ENTER to play again",
             WINDOW_WIDTH / 2 -
                 MeasureText("Press ENTER to play again", 20) / 2,
             WINDOW_HEIGHT / 2 + 70, 20, LIGHTGRAY);
  }

  EndDrawing();
}
void LoadLevel(GameData *game, int level, Snake *snake) {
  game->currentLevel = level;
  // Clear map
  for (int x = 0; x < GRID_COUNT_X; x++) {
    for (int y = 0; y < GRID_COUNT_Y; y++) {
      game->map[x][y] = TILE_EMPTY;
    }
  }

  if (level == 2) {
    // Corners Box layout (Adjusted for 30x30)
    int size = 7;
    for (int i = 0; i < size; i++) {
      game->map[i][0] = TILE_WALL;
      game->map[0][i] = TILE_WALL;
      game->map[GRID_COUNT_X - 1 - i][0] = TILE_WALL;
      game->map[GRID_COUNT_X - 1][i] = TILE_WALL;
      game->map[i][GRID_COUNT_Y - 1] = TILE_WALL;
      game->map[0][GRID_COUNT_Y - 1 - i] = TILE_WALL;
      game->map[GRID_COUNT_X - 1 - i][GRID_COUNT_Y - 1] = TILE_WALL;
      game->map[GRID_COUNT_X - 1][GRID_COUNT_Y - 1 - i] = TILE_WALL;
    }
  } else if (level == 3) {
    // Four Pillars layout (Adjusted for 30x30)
    int cx = GRID_COUNT_X / 2;
    int cy = GRID_COUNT_Y / 2;
    for (int x = cx - 6; x <= cx - 4; x++)
      for (int y = cy - 5; y <= cy - 3; y++)
        game->map[x][y] = TILE_WALL;
    for (int x = cx + 4; x <= cx + 6; x++)
      for (int y = cy - 5; y <= cy - 3; y++)
        game->map[x][y] = TILE_WALL;
    for (int x = cx - 6; x <= cx - 4; x++)
      for (int y = cy + 3; y <= cy + 5; y++)
        game->map[x][y] = TILE_WALL;
    for (int x = cx + 4; x <= cx + 6; x++)
      for (int y = cy + 3; y <= cy + 5; y++)
        game->map[x][y] = TILE_WALL;
  } else if (level == 4) {
    // Stage 4: THE CASTLE (Moved here from Chaos Stage 5)
    // 1. Outer Shell
    for (int x = 0; x < GRID_COUNT_X; x++) {
      for (int y = 0; y < GRID_COUNT_Y; y++) {
        bool isBorder =
            (x < 2 || x >= GRID_COUNT_X - 2 || y < 2 || y >= GRID_COUNT_Y - 2);
        int mid = GRID_COUNT_X / 2;
        bool isGateX = (x >= mid - 2 && x <= mid + 1);
        bool isGateY = (y >= mid - 2 && y <= mid + 1);
        if (isBorder) {
          if ((x < 2 || x >= GRID_COUNT_X - 2) && isGateY)
            continue;
          if ((y < 2 || y >= GRID_COUNT_Y - 2) && isGateX)
            continue;
          game->map[x][y] = TILE_WALL;
        }
      }
    }
    // 2. Inner Brackets
    int offset = 7, len = 4;
    for (int i = 0; i < len; i++) {
      game->map[offset + i][offset] = TILE_WALL;
      game->map[offset][offset + i] = TILE_WALL;
      game->map[GRID_COUNT_X - 1 - offset - i][offset] = TILE_WALL;
      game->map[GRID_COUNT_X - 1 - offset][offset + i] = TILE_WALL;
      game->map[offset + i][GRID_COUNT_Y - 1 - offset] = TILE_WALL;
      game->map[offset][GRID_COUNT_Y - 1 - offset - i] = TILE_WALL;
      game->map[GRID_COUNT_X - 1 - offset - i][GRID_COUNT_Y - 1 - offset] =
          TILE_WALL;
      game->map[GRID_COUNT_X - 1 - offset][GRID_COUNT_Y - 1 - offset - i] =
          TILE_WALL;
    }
  }
}

void LoadChaosStage(GameData *game, int level, Snake *snake) {
  // Clear map
  for (int x = 0; x < GRID_COUNT_X; x++) {
    for (int y = 0; y < GRID_COUNT_Y; y++) {
      game->map[x][y] = TILE_EMPTY;
    }
  }

  if (level == 1) {
    // Chaos Stage 1: Random Walls (30)
    int wallCount = 30;
    while (wallCount > 0) {
      int rx = GetRandomValue(2, GRID_COUNT_X - 3);
      int ry = GetRandomValue(2, GRID_COUNT_Y - 3);
      if (game->map[rx][ry] == TILE_EMPTY) {
        game->map[rx][ry] = TILE_WALL;
        wallCount--;
      }
    }
  } else if (level == 2) {
    // Chaos Stage 2: Corners (Re-use existing layout)
    LoadLevel(game, 2, snake);
    game->currentLevel = 2; // Restore level tracker
  } else if (level == 3) {
    // Chaos Stage 3: Pillars (Re-use existing layout)
    LoadLevel(game, 3, snake);
    game->currentLevel = 3; // Restore level tracker
  } else if (level == 4) {
    // Chaos Stage 4: Extreme Chaos (60 random walls)
    int wallCount = 60;
    while (wallCount > 0) {
      int rx = GetRandomValue(3, GRID_COUNT_X - 4);
      int ry = GetRandomValue(3, GRID_COUNT_Y - 4);
      if (game->map[rx][ry] == TILE_EMPTY) {
        game->map[rx][ry] = TILE_WALL;
        wallCount--;
      }
    }
  } else if (level == 5) {
    // FINAL STAGE: The Castle (from user image)
    // 1. Outer Shell (2 tiles thick with gates)
    for (int x = 0; x < GRID_COUNT_X; x++) {
      for (int y = 0; y < GRID_COUNT_Y; y++) {
        bool isBorder =
            (x < 2 || x >= GRID_COUNT_X - 2 || y < 2 || y >= GRID_COUNT_Y - 2);
        int mid = GRID_COUNT_X / 2;
        bool isGateX = (x >= mid - 2 && x <= mid + 1);
        bool isGateY = (y >= mid - 2 && y <= mid + 1);
        if (isBorder) {
          if ((x < 2 || x >= GRID_COUNT_X - 2) && isGateY)
            continue;
          if ((y < 2 || y >= GRID_COUNT_Y - 2) && isGateX)
            continue;
          game->map[x][y] = TILE_WALL;
        }
      }
    }
    // 2. Inner Brackets
    int offset = 7, len = 4;
    for (int i = 0; i < len; i++) {
      game->map[offset + i][offset] = TILE_WALL;
      game->map[offset][offset + i] = TILE_WALL;
      game->map[GRID_COUNT_X - 1 - offset - i][offset] = TILE_WALL;
      game->map[GRID_COUNT_X - 1 - offset][offset + i] = TILE_WALL;
      game->map[offset + i][GRID_COUNT_Y - 1 - offset] = TILE_WALL;
      game->map[offset][GRID_COUNT_Y - 1 - offset - i] = TILE_WALL;
      game->map[GRID_COUNT_X - 1 - offset - i][GRID_COUNT_Y - 1 - offset] =
          TILE_WALL;
      game->map[GRID_COUNT_X - 1 - offset][GRID_COUNT_Y - 1 - offset - i] =
          TILE_WALL;
    }
  }

  // Portal Pair (Always refresh portals for chaos stages)
  int p = 0;
  while (p < 2) {
    int rx = GetRandomValue(3, GRID_COUNT_X - 4);
    int ry = GetRandomValue(3, GRID_COUNT_Y - 4);
    if (game->map[rx][ry] == TILE_EMPTY) {
      game->map[rx][ry] = TILE_PORTAL;
      game->portals[p] = (Vector2){(float)rx, (float)ry};
      p++;
    }
  }
}

void InitChaosMode(GameData *game, Snake *snake) {
  game->currentLevel = 1;
  LoadChaosStage(game, 1, snake);
}
