#include "game.h"
#include "food.h"
#include "snake.h"
#include "stages.h"
#include <math.h>
#include <stdio.h>

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
  game->mode = MODE_CLASSIC;
  game->currentLevel = 1;
  game->score = 0;
  game->highScore = LoadHighScore();
  game->reverseMode = false;
  game->speedFPS = 8;
  game->devModeActive = false;
  game->devLevelSelect = 1;

  // Clear map initially
  for (int x = 0; x < GRID_COUNT_X; x++)
    for (int y = 0; y < GRID_COUNT_Y; y++)
      game->map[x][y] = TILE_EMPTY;
}

void RestartGame(GameData *game, Snake *snake, Food *food) {
  game->score = 0;
  InitSnake(snake);

  // Load initial map based on mode
  if (game->mode == MODE_STAGES) {
    LoadStage(game, 1);
  } else if (game->mode == MODE_CHAOS) {
    LoadChaosStage(game, 1);
  } else {
    // Classic: Clear map
    for (int x = 0; x < GRID_COUNT_X; x++)
      for (int y = 0; y < GRID_COUNT_Y; y++)
        game->map[x][y] = TILE_EMPTY;
  }

  snake->timePerMove = 1.0f / (float)game->speedFPS;
  InitFood(food, snake, game);
}

void UpdateGame(GameData *game, Snake *snake, Food *food) {
  if (game->state == STATE_START) {
    if (IsKeyPressed(KEY_L)) {
      game->mode = (game->mode + 1) % 3;
      game->devLevelSelect = 1; // Reset selection on mode change
    }
    if (IsKeyPressed(KEY_M)) {
      game->reverseMode = !game->reverseMode;
    }
    if (IsKeyPressed(KEY_K)) {
      game->speedFPS += 4;
      if (game->speedFPS > 20)
        game->speedFPS = 4;
    }

    // Hidden Dev Mode Toggle
    if (IsKeyPressed(KEY_D) && (game->mode == MODE_STAGES || game->mode == MODE_CHAOS)) {
      game->devModeActive = !game->devModeActive;
    }

    // Dev Level Selection
    if (game->devModeActive) {
      if (IsKeyPressed(KEY_LEFT)) {
        game->devLevelSelect--;
        if (game->devLevelSelect < 1) game->devLevelSelect = 1;
      }
      if (IsKeyPressed(KEY_RIGHT)) {
        game->devLevelSelect++;
        int max = (game->mode == MODE_STAGES) ? 4 : 5;
        if (game->devLevelSelect > max) game->devLevelSelect = max;
      }
    }

    if (IsKeyPressed(KEY_ENTER)) {
      game->state = STATE_PLAYING;
      RestartGame(game, snake, food);

      // Apply Dev Selection
      if (game->devModeActive) {
          game->currentLevel = game->devLevelSelect;
          if (game->mode == MODE_STAGES) {
              int scores[] = {0, 100, 200, 350};
              game->score = scores[game->currentLevel - 1];
              LoadStage(game, game->currentLevel);
          } else if (game->mode == MODE_CHAOS) {
              game->score = (game->currentLevel - 1) * 100;
              LoadChaosStage(game, game->currentLevel);
          }
          // Fix: Respawn food after the new map is loaded to ensure it's not in a wall
          SpawnFood(food, snake, game);
      }

      // Initial Invincibility for Stage/Chaos
      if (game->mode != MODE_CLASSIC) {
          snake->invincibilityTimer = 2.0f;
      }
    }
  } else if (game->state == STATE_PLAYING) {
    HandleSnakeInput(snake);

    // Decrement Invincibility Timer
    if (snake->invincibilityTimer > 0) {
        snake->invincibilityTimer -= GetFrameTime();
    }

    snake->moveTimer += GetFrameTime();
    if (snake->moveTimer >= snake->timePerMove) {
      snake->moveTimer = 0.0f;
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
      if (snake->body[0].x == food->position.x &&
          snake->body[0].y == food->position.y) {
        snake->justAteFood = true;
        game->score += 10;

        // Level Progression
        if (game->mode == MODE_STAGES) {
          int oldLevel = game->currentLevel;
          if (game->currentLevel == 1 && game->score >= 100)
            LoadStage(game, 2);
          else if (game->currentLevel == 2 && game->score >= 200)
            LoadStage(game, 3);
          else if (game->currentLevel == 3 && game->score >= 350)
            LoadStage(game, 4);

          // Trigger pulse of invincibility on level transition
          if (game->currentLevel != oldLevel) {
              snake->invincibilityTimer = 2.0f;
          }

          if (game->score >= 500)
            game->state = STATE_GAME_WIN;
        } else if (game->mode == MODE_CHAOS) {
          if (game->score % 100 == 0) {
            LoadChaosStage(game, (game->score / 100) + 1);
            snake->invincibilityTimer = 2.0f;
          }
          if (game->score >= 500)
            game->state = STATE_GAME_WIN;
        }

        if (game->state == STATE_PLAYING) {
          SpawnFood(food, snake, game);
          if (game->reverseMode) {
            ReverseSnake(snake);
            // Pause for 0.2 seconds by setting the timer back
            snake->moveTimer = -0.2555f;
          }
        }
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
  Color arenaColor = (Color){160, 220, 160, 255}; // Màu xanh lá nhẹ
  ClearBackground(arenaColor);

  // Draw Background if loaded
  if (game->assets.bgTex.id != 0) {
    DrawTexturePro(game->assets.bgTex,
                   (Rectangle){0, 0, (float)game->assets.bgTex.width,
                               (float)game->assets.bgTex.height},
                   (Rectangle){0, 0, WINDOW_WIDTH, WINDOW_HEIGHT},
                   (Vector2){0, 0}, 0.0f, WHITE);
  } else {
    ClearBackground(arenaColor);
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
  if (game->state == STATE_PLAYING || game->state == STATE_GAME_OVER ||
      game->state == STATE_GAME_WIN) {
    // Draw Walls and Portals
    for (int x = 0; x < GRID_COUNT_X; x++) {
      for (int y = 0; y < GRID_COUNT_Y; y++) {
        if (game->map[x][y] == TILE_WALL) {
          DrawRectangle(x * GRID_SIZE, y * GRID_SIZE, GRID_SIZE, GRID_SIZE,
                        DARKGRAY);
          DrawRectangleLines(x * GRID_SIZE, y * GRID_SIZE, GRID_SIZE, GRID_SIZE,
                             GRAY);
        } else if (game->map[x][y] == TILE_PORTAL) {
          float time = (float)GetTime();
          float pulse = (sinf(time * 5.0f) + 1.0f) * 0.2f + 0.8f;
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
  if (game->state == STATE_START) {
    Vector2 titlePos = {
        WINDOW_WIDTH / 2.0f -
            MeasureTextEx(game->assets.mainFont, "SNAKE GAME", 50, 2).x / 2.0f,
        WINDOW_HEIGHT / 2.0f - 50};
    DrawTextEx(game->assets.mainFont, "SNAKE GAME", titlePos, 50, 2, DARKGREEN);

    Vector2 subPos = {
        WINDOW_WIDTH / 2.0f -
            MeasureTextEx(game->assets.mainFont, "Press ENTER to start", 20, 1)
                    .x /
                2.0f,
        WINDOW_HEIGHT / 2.0f};
    DrawTextEx(game->assets.mainFont, "Press ENTER to start", subPos, 20, 1,
               DARKGRAY);

    // Draw Mode Info
    const char *modeTexts[] = {"MODE: CLASSIC (Key L)", "MODE: STAGES (Key L)",
                               "MODE: CHAOS (Key L)"};
    Color modeColors[] = {GRAY, LIME, MAGENTA};
    Vector2 modePos = {
        WINDOW_WIDTH / 2.0f -
            MeasureTextEx(game->assets.mainFont, modeTexts[game->mode], 18, 1)
                    .x /
                2.0f,
        WINDOW_HEIGHT / 2.0f + 40};
    DrawTextEx(game->assets.mainFont, modeTexts[game->mode], modePos, 18, 1,
               modeColors[game->mode]);

    const char *revText = game->reverseMode ? "REVERSE MODE: ON (Key M)"
                                            : "REVERSE MODE: OFF (Key M)";
    Vector2 revPos = {
        WINDOW_WIDTH / 2.0f -
            MeasureTextEx(game->assets.mainFont, revText, 18, 1).x / 2.0f,
        WINDOW_HEIGHT / 2.0f + 70};
    DrawTextEx(game->assets.mainFont, revText, revPos, 18, 1,
               game->reverseMode ? GOLD : DARKGRAY);

    const char *speedText = TextFormat("SPEED: %d FPS (Key K)", game->speedFPS);
    Vector2 speedPos = {
        WINDOW_WIDTH / 2.0f -
            MeasureTextEx(game->assets.mainFont, speedText, 18, 1).x / 2.0f,
        WINDOW_HEIGHT / 2.0f + 100};
    DrawTextEx(game->assets.mainFont, speedText, speedPos, 18, 1, SKYBLUE);

    // Hidden Dev Mode UI
    if (game->devModeActive) {
      const char *devText = TextFormat("DEV MODE ACTIVE - LEVEL: %d", game->devLevelSelect);
      Vector2 devPos = {
          WINDOW_WIDTH / 2.0f - MeasureTextEx(game->assets.mainFont, devText, 16, 1).x / 2.0f,
          WINDOW_HEIGHT / 2.0f + 130};
      DrawTextEx(game->assets.mainFont, devText, devPos, 16, 1, YELLOW);
      
      const char *promptText = "< LEFT / RIGHT TO CHANGE >";
      Vector2 promptPos = {
          WINDOW_WIDTH / 2.0f - MeasureTextEx(game->assets.mainFont, promptText, 12, 1).x / 2.0f,
          WINDOW_HEIGHT / 2.0f + 155};
      DrawTextEx(game->assets.mainFont, promptText, promptPos, 12, 1, GOLD);
    }
  } else if (game->state == STATE_PLAYING) {
    DrawTextEx(game->assets.mainFont, TextFormat("Score: %d", game->score),
               (Vector2){10, 10}, 25, 1, DARKGREEN);
    DrawTextEx(game->assets.mainFont, TextFormat("Best: %d", game->highScore),
               (Vector2){10, 40}, 20, 1, GRAY);
  } else if (game->state == STATE_GAME_OVER) {
    Vector2 overPos = {
        WINDOW_WIDTH / 2.0f -
            MeasureTextEx(game->assets.mainFont, "GAME OVER!", 50, 2).x / 2.0f,
        WINDOW_HEIGHT / 2.0f - 50};
    DrawTextEx(game->assets.mainFont, "GAME OVER!", overPos, 50, 2, RED);

    Vector2 scorePos = {
        WINDOW_WIDTH / 2.0f -
            MeasureTextEx(game->assets.mainFont,
                          TextFormat("Final Score: %d", game->score), 20, 1)
                    .x /
                2.0f,
        WINDOW_HEIGHT / 2.0f + 10};
    DrawTextEx(game->assets.mainFont,
               TextFormat("Final Score: %d", game->score), scorePos, 20, 1,
               BLACK);

    Vector2 restartPos = {WINDOW_WIDTH / 2.0f -
                              MeasureTextEx(game->assets.mainFont,
                                            "Press ENTER to try again", 20, 1)
                                      .x /
                                  2.0f,
                          WINDOW_HEIGHT / 2.0f + 40};
    DrawTextEx(game->assets.mainFont, "Press ENTER to try again", restartPos,
               20, 1, DARKGRAY);
  } else if (game->state == STATE_GAME_WIN) {
    Vector2 winPos = {
        WINDOW_WIDTH / 2.0f -
            MeasureTextEx(game->assets.mainFont, "CHAMPION!", 60, 2).x / 2.0f,
        WINDOW_HEIGHT / 2.0f - 50};
    DrawTextEx(game->assets.mainFont, "CHAMPION!", winPos, 60, 2, GOLD);

    Vector2 subWinPos = {WINDOW_WIDTH / 2.0f -
                             MeasureTextEx(game->assets.mainFont,
                                           "You conquered the game!", 20, 1)
                                     .x /
                                 2.0f,
                         WINDOW_HEIGHT / 2.0f + 10};
    DrawTextEx(game->assets.mainFont, "You conquered the game!", subWinPos, 20,
               1, DARKGREEN);

    Vector2 restartPos = {WINDOW_WIDTH / 2.0f -
                              MeasureTextEx(game->assets.mainFont,
                                            "Press ENTER to play again", 20, 1)
                                      .x /
                                  2.0f,
                          WINDOW_HEIGHT / 2.0f + 50};
    DrawTextEx(game->assets.mainFont, "Press ENTER to play again", restartPos,
               20, 1, DARKGRAY);
  }

  EndDrawing();
}
