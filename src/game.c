#include "game.h"
#include "food.h"
#include "snake.h"
#include "stages.h"
#include <math.h>
#include <stdio.h>

const char *SCOREBOARD_FILE = "scoreboard.txt";

void LoadScoreboard(GameData *game) {
  for (int i = 0; i < 10; i++) {
    game->highScores[i].score = 0;
    snprintf(game->highScores[i].name, 16, "Player");
  }
  FILE *file = fopen(SCOREBOARD_FILE, "r");
  if (!file)
    return;
  for (int i = 0; i < 10; i++) {
    if (fscanf(file, "%15s %d", game->highScores[i].name,
               &game->highScores[i].score) != 2) {
      break;
    }
  }
  fclose(file);
}

void SaveScoreboard(GameData *game) {
  FILE *file = fopen(SCOREBOARD_FILE, "w");
  if (!file)
    return;
  for (int i = 0; i < 10; i++) {
    fprintf(file, "%s %d\n", game->highScores[i].name,
            game->highScores[i].score);
  }
  fclose(file);
}

void InitGameData(GameData *game) {
  game->state = STATE_START;
  game->isPaused = false;
  game->mode = MODE_CLASSIC;
  game->currentLevel = 1;
  game->score = 0;
  LoadScoreboard(game);
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
  game->isPaused = false;
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
    if (IsKeyPressed(KEY_D) &&
        (game->mode == MODE_STAGES || game->mode == MODE_CHAOS)) {
      game->devModeActive = !game->devModeActive;
    }

    // Dev Level Selection
    if (game->devModeActive) {
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
        // Respawn food after the new map is loaded to ensure it's not in a wall
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
        if (game->score > game->highScores[9].score) {
          game->state = STATE_NAME_INPUT;
          game->nameLength = 0;
          game->currentName[0] = '\0';
        } else {
          game->state = STATE_GAME_OVER;
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

          if (game->score >= 500) {
            if (game->score > game->highScores[9].score) {
              game->state = STATE_NAME_INPUT;
              game->nameLength = 0;
              game->currentName[0] = '\0';
            } else {
              game->state = STATE_GAME_WIN;
            }
          }
        } else if (game->mode == MODE_CHAOS) {
          if (game->score % 100 == 0) {
            LoadChaosStage(game, (game->score / 100) + 1);
            snake->invincibilityTimer = 2.0f;
          }
          if (game->score >= 500) {
            if (game->score > game->highScores[9].score) {
              game->state = STATE_NAME_INPUT;
              game->nameLength = 0;
              game->currentName[0] = '\0';
            } else {
              game->state = STATE_GAME_WIN;
            }
          }
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
  } else if (game->state == STATE_NAME_INPUT) {
    int key = GetCharPressed();
    while (key > 0) {
      if ((key >= 32) && (key <= 125) && (game->nameLength < 15)) {
        game->currentName[game->nameLength] = (char)key;
        game->currentName[game->nameLength + 1] = '\0';
        game->nameLength++;
      }
      key = GetCharPressed();
    }
    if (IsKeyPressed(KEY_BACKSPACE)) {
      game->nameLength--;
      if (game->nameLength < 0)
        game->nameLength = 0;
      game->currentName[game->nameLength] = '\0';
    }
    if (IsKeyPressed(KEY_ENTER)) {
      int insertIndex = 9;
      while (insertIndex >= 0 &&
             game->score > game->highScores[insertIndex].score) {
        insertIndex--;
      }
      insertIndex++;

      for (int i = 9; i > insertIndex; i--) {
        game->highScores[i] = game->highScores[i - 1];
      }

      game->highScores[insertIndex].score = game->score;
      if (game->nameLength == 0) {
        snprintf(game->highScores[insertIndex].name, 16, "Unknown");
      } else {
        snprintf(game->highScores[insertIndex].name, 16, "%s",
                 game->currentName);
      }

      SaveScoreboard(game);

      if (game->score >= 500 &&
          (game->mode == MODE_STAGES || game->mode == MODE_CHAOS)) {
        game->state = STATE_GAME_WIN;
      } else {
        game->state = STATE_GAME_OVER;
      }
    }
  } else if (game->state == STATE_GAME_OVER || game->state == STATE_GAME_WIN) {
    if (IsKeyPressed(KEY_ENTER)) {
      game->state = STATE_PLAYING;
      RestartGame(game, snake, food);
    }
  }

  // Handle Scoreboard Transition from Menu
  if (game->state == STATE_START) {
    bool clickedSb = false;
    bool clickedStart = false;
    bool clickedMode = false;
    bool clickedRev = false;

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
      Vector2 mousePos = GetMousePosition();

      Vector2 startTextSize =
          MeasureTextEx(game->assets.mainFont, "START GAME", 20, 1);
      Vector2 startPos = {WINDOW_WIDTH / 2.0f - startTextSize.x / 2.0f,
                          WINDOW_HEIGHT / 2.0f};
      Rectangle startRect = {startPos.x - 20, startPos.y - 10,
                             startTextSize.x + 40, startTextSize.y + 20};
      if (CheckCollisionPointRec(mousePos, startRect))
        clickedStart = true;

      const char *modeTexts[] = {"MODE: CLASSIC", "MODE: STAGES",
                                 "MODE: CHAOS"};
      Vector2 modeTextSize =
          MeasureTextEx(game->assets.mainFont, modeTexts[game->mode], 18, 1);
      Vector2 modePos = {WINDOW_WIDTH / 2.0f - modeTextSize.x / 2.0f,
                         WINDOW_HEIGHT / 2.0f + 40};
      Rectangle modeRect = {modePos.x - 20, modePos.y - 10, modeTextSize.x + 40,
                            modeTextSize.y + 20};
      if (CheckCollisionPointRec(mousePos, modeRect))
        clickedMode = true;

      const char *revText =
          game->reverseMode ? "REVERSE MODE: ON" : "REVERSE MODE: OFF";
      Vector2 revTextSize =
          MeasureTextEx(game->assets.mainFont, revText, 18, 1);
      Vector2 revPos = {WINDOW_WIDTH / 2.0f - revTextSize.x / 2.0f,
                        WINDOW_HEIGHT / 2.0f + 70};
      Rectangle revRect = {revPos.x - 20, revPos.y - 10, revTextSize.x + 40,
                           revTextSize.y + 20};
      if (CheckCollisionPointRec(mousePos, revRect))
        clickedRev = true;

      const char *speedText = TextFormat("SPEED: %d FPS", game->speedFPS);
      Vector2 speedTextSize =
          MeasureTextEx(game->assets.mainFont, speedText, 18, 1);
      Vector2 speedPos = {WINDOW_WIDTH / 2.0f - speedTextSize.x / 2.0f,
                          WINDOW_HEIGHT / 2.0f + 100};
      Rectangle speedRect = {speedPos.x - 20, speedPos.y - 10,
                             speedTextSize.x + 40, speedTextSize.y + 20};
      if (CheckCollisionPointRec(mousePos, speedRect)) {
        // Logic handled by manual click check for now
      }

      Vector2 sbTextSize =
          MeasureTextEx(game->assets.mainFont, "SCOREBOARD", 18, 1);
      Vector2 sbHintPos = {WINDOW_WIDTH / 2.0f - sbTextSize.x / 2.0f,
                           WINDOW_HEIGHT / 2.0f + 130};
      Rectangle sbRect = {sbHintPos.x - 20, sbHintPos.y - 10, sbTextSize.x + 40,
                          sbTextSize.y + 20};
      if (CheckCollisionPointRec(mousePos, sbRect))
        clickedSb = true;
    }

    if (clickedSb || IsKeyPressed(KEY_H)) {
      game->state = STATE_SCOREBOARD;
    }
    if (clickedMode || IsKeyPressed(KEY_L)) {
      game->mode = (game->mode + 1) % 3;
      game->devLevelSelect = 1;
    }
    if (clickedRev || IsKeyPressed(KEY_M)) {
      game->reverseMode = !game->reverseMode;
    }

    // speed update logic
    if (CheckCollisionPointRec(GetMousePosition(),
                               (Rectangle){WINDOW_WIDTH / 2.0f - 50,
                                           WINDOW_HEIGHT / 2.0f + 90, 100,
                                           40})) {
      // Hover state handled in Draw
    }

    // Simplified speed logic
    bool speedTrigger = IsKeyPressed(KEY_K);
    // Note: the mouse click is handled via the separate block below for better
    // precision if needed, but for simplicity let's just check the boolean we
    // set
    if (speedTrigger) {
      game->speedFPS += 4;
      if (game->speedFPS > 20)
        game->speedFPS = 4;
    }
    // Re-handling speed click more robustly
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
      Vector2 mPos = GetMousePosition();
      Vector2 sTextSize =
          MeasureTextEx(game->assets.mainFont,
                        TextFormat("SPEED: %d FPS", game->speedFPS), 18, 1);
      Rectangle sRect = {WINDOW_WIDTH / 2.0f - sTextSize.x / 2.0f - 20,
                         WINDOW_HEIGHT / 2.0f + 90, sTextSize.x + 40, 40};
      if (CheckCollisionPointRec(mPos, sRect)) {
        game->speedFPS += 4;
        if (game->speedFPS > 20)
          game->speedFPS = 4;
      }
    }

    // Hidden Dev Mode Toggle
    if (IsKeyPressed(KEY_D)) {
      game->devModeActive = !game->devModeActive;
    }

    // Dev Level Selection
    if (game->devModeActive) {
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

    if (clickedStart || IsKeyPressed(KEY_ENTER)) {
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
        SpawnFood(food, snake, game);
      }

      if (game->mode != MODE_CLASSIC) {
        snake->invincibilityTimer = 2.0f;
      }
    }
  } else if (game->state == STATE_PLAYING) {
    bool clickedPause = false;
    bool clickedMenu = false;

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
      Vector2 mousePos = GetMousePosition();

      Vector2 pauseTextSize =
          MeasureTextEx(game->assets.mainFont,
                        game->isPaused ? "[ RESUME ]" : "[ PAUSE ]", 20, 1);
      Vector2 pausePos = {WINDOW_WIDTH / 2.0f - pauseTextSize.x / 2.0f,
                          TOOLBAR_HEIGHT / 2.0f - pauseTextSize.y / 2.0f};
      Rectangle pauseRect = {pausePos.x - 10, pausePos.y - 5,
                             pauseTextSize.x + 20, pauseTextSize.y + 10};
      if (CheckCollisionPointRec(mousePos, pauseRect))
        clickedPause = true;

      Vector2 menuTextSize =
          MeasureTextEx(game->assets.mainFont, "[ MENU ]", 20, 1);
      Vector2 menuPos = {WINDOW_WIDTH - menuTextSize.x - 20,
                         TOOLBAR_HEIGHT / 2.0f - menuTextSize.y / 2.0f};
      Rectangle menuRect = {menuPos.x - 10, menuPos.y - 5, menuTextSize.x + 20,
                            menuTextSize.y + 10};
      if (CheckCollisionPointRec(mousePos, menuRect))
        clickedMenu = true;
    }

    if (IsKeyPressed(KEY_P) || clickedPause) {
      game->isPaused = !game->isPaused;
    }
    if (clickedMenu) {
      game->state = STATE_START;
    }

    if (!game->isPaused) {
      HandleSnakeInput(snake);

      if (snake->invincibilityTimer > 0) {
        snake->invincibilityTimer -= GetFrameTime();
      }

      snake->moveTimer += GetFrameTime();
      if (snake->moveTimer >= snake->timePerMove) {
        snake->moveTimer = 0.0f;
        UpdateSnakeLogic(snake, game);

        if (CheckCollisionWithSelfOrWall(snake, game)) {
          if (game->score > game->highScores[9].score) {
            game->state = STATE_NAME_INPUT;
            game->nameLength = 0;
            game->currentName[0] = '\0';
          } else {
            game->state = STATE_GAME_OVER;
          }
        }

        if (snake->body[0].x == food->position.x &&
            snake->body[0].y == food->position.y) {
          snake->justAteFood = true;
          game->score += 10;

          if (game->mode == MODE_STAGES) {
            int oldLevel = game->currentLevel;
            if (game->currentLevel == 1 && game->score >= 100)
              LoadStage(game, 2);
            else if (game->currentLevel == 2 && game->score >= 200)
              LoadStage(game, 3);
            else if (game->currentLevel == 3 && game->score >= 350)
              LoadStage(game, 4);

            if (game->currentLevel != oldLevel)
              snake->invincibilityTimer = 2.0f;

            if (game->score >= 500) {
              if (game->score > game->highScores[9].score) {
                game->state = STATE_NAME_INPUT;
                game->nameLength = 0;
                game->currentName[0] = '\0';
              } else {
                game->state = STATE_GAME_WIN;
              }
            }
          } else if (game->mode == MODE_CHAOS) {
            if (game->score % 100 == 0) {
              LoadChaosStage(game, (game->score / 100) + 1);
              snake->invincibilityTimer = 2.0f;
            }
            if (game->score >= 500) {
              if (game->score > game->highScores[9].score) {
                game->state = STATE_NAME_INPUT;
                game->nameLength = 0;
                game->currentName[0] = '\0';
              } else {
                game->state = STATE_GAME_WIN;
              }
            }
          }

          if (game->state == STATE_PLAYING) {
            SpawnFood(food, snake, game);
            if (game->reverseMode) {
              ReverseSnake(snake);
              snake->moveTimer = -0.2555f;
            }
          }
        }
      }
    }
  } else if (game->state == STATE_NAME_INPUT) {
    int key = GetCharPressed();
    while (key > 0) {
      if ((key >= 32) && (key <= 125) && (game->nameLength < 15)) {
        game->currentName[game->nameLength] = (char)key;
        game->currentName[game->nameLength + 1] = '\0';
        game->nameLength++;
      }
      key = GetCharPressed();
    }
    if (IsKeyPressed(KEY_BACKSPACE)) {
      game->nameLength--;
      if (game->nameLength < 0)
        game->nameLength = 0;
      game->currentName[game->nameLength] = '\0';
    }
    if (IsKeyPressed(KEY_ENTER)) {
      int insertIndex = 9;
      while (insertIndex >= 0 &&
             game->score > game->highScores[insertIndex].score) {
        insertIndex--;
      }
      insertIndex++;

      for (int i = 9; i > insertIndex; i--) {
        game->highScores[i] = game->highScores[i - 1];
      }

      game->highScores[insertIndex].score = game->score;
      if (game->nameLength == 0) {
        snprintf(game->highScores[insertIndex].name, 16, "Unknown");
      } else {
        snprintf(game->highScores[insertIndex].name, 16, "%s",
                 game->currentName);
      }

      SaveScoreboard(game);

      if (game->score >= 500 &&
          (game->mode == MODE_STAGES || game->mode == MODE_CHAOS)) {
        game->state = STATE_GAME_WIN;
      } else {
        game->state = STATE_GAME_OVER;
      }
    }
  } else if (game->state == STATE_GAME_OVER || game->state == STATE_GAME_WIN) {
    if (IsKeyPressed(KEY_ENTER)) {
      game->state = STATE_PLAYING;
      RestartGame(game, snake, food);
    }
  }

  // Handle Scoreboard Transition from Menu
  if (game->state == STATE_START) {
    if (IsKeyPressed(KEY_H)) {
      game->state = STATE_SCOREBOARD;
    }
  } else if (game->state == STATE_SCOREBOARD) {
    bool clickedBack = false;
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
      Vector2 mousePos = GetMousePosition();
      Vector2 backTextSize = MeasureTextEx(game->assets.mainFont,
                                           "Press H or ENTER to return", 20, 1);
      Vector2 subPos = {WINDOW_WIDTH / 2.0f - backTextSize.x / 2.0f,
                        WINDOW_HEIGHT - 60.0f};
      Rectangle backRect = {subPos.x - 20, subPos.y - 10, backTextSize.x + 40,
                            backTextSize.y + 20};
      if (CheckCollisionPointRec(mousePos, backRect)) {
        clickedBack = true;
      }
    }
    if (clickedBack || IsKeyPressed(KEY_H) || IsKeyPressed(KEY_ENTER) ||
        IsKeyPressed(KEY_ESCAPE)) {
      game->state = STATE_START;
    }
  }
}

void DrawGame(GameData *game, Snake *snake, Food *food) {
  BeginDrawing();
  Color arenaColor = (Color){160, 220, 160, 255}; // Màu xanh lá nhẹ
  ClearBackground(arenaColor);

  Camera2D cam = {0};
  cam.offset = (Vector2){0, TOOLBAR_HEIGHT};
  cam.zoom = 1.0f;

  if (game->state == STATE_PLAYING || game->state == STATE_GAME_OVER ||
      game->state == STATE_GAME_WIN) {
    BeginMode2D(cam);
  }

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

  // Draw Grid exactly inside the 30x30 arena
  Color gridColor = (Color){255, 255, 255, 100}; // White at 100 alpha (approx 50% opacity of 255)
  for (int i = 0; i <= GRID_COUNT_X; i++) {
    DrawLine(i * GRID_SIZE, 0, i * GRID_SIZE, GRID_COUNT_Y * GRID_SIZE, gridColor);
  }
  for (int i = 0; i <= GRID_COUNT_Y; i++) {
    DrawLine(0, i * GRID_SIZE, GRID_COUNT_X * GRID_SIZE, i * GRID_SIZE, gridColor);
  }

  // Draw entities
  if (game->state == STATE_PLAYING || game->state == STATE_GAME_OVER ||
      game->state == STATE_GAME_WIN) {
    // Draw Walls and Portals
    for (int x = 0; x < GRID_COUNT_X; x++) {
      for (int y = 0; y < GRID_COUNT_Y; y++) {
        if (game->map[x][y] == TILE_WALL) {
          if (game->assets.wallTex.id != 0) {
            DrawTexturePro(game->assets.wallTex,
                           (Rectangle){0, 0, (float)game->assets.wallTex.width,
                                       (float)game->assets.wallTex.height},
                           (Rectangle){(float)x * GRID_SIZE,
                                       (float)y * GRID_SIZE, (float)GRID_SIZE,
                                       (float)GRID_SIZE},
                           (Vector2){0, 0}, 0.0f, WHITE);
          } else {
            DrawRectangle(x * GRID_SIZE, y * GRID_SIZE, GRID_SIZE, GRID_SIZE,
                          DARKGRAY);
            DrawRectangleLines(x * GRID_SIZE, y * GRID_SIZE, GRID_SIZE,
                               GRID_SIZE, GRAY);
          }
        }
      }
    }

    DrawFood(food, game->assets.fruitTex);
    DrawSnake(snake, game->assets.headTex, game->assets.bodyTex,
              game->assets.tailTex);
  }

  if (game->state == STATE_PLAYING || game->state == STATE_GAME_OVER ||
      game->state == STATE_GAME_WIN) {
    EndMode2D();

    // Draw Toolbar Space
    DrawRectangle(0, 0, WINDOW_WIDTH, TOOLBAR_HEIGHT, (Color){30, 30, 30, 255});
    DrawLine(0, TOOLBAR_HEIGHT, WINDOW_WIDTH, TOOLBAR_HEIGHT, WHITE);
  }

  // Draw UI
  if (game->state == STATE_SCOREBOARD) {
    Vector2 titlePos = {
        WINDOW_WIDTH / 2.0f -
            MeasureTextEx(game->assets.mainFont, "TOP 10 SCOREBOARD", 40, 2).x /
                2.0f,
        50};
    DrawTextEx(game->assets.mainFont, "TOP 10 SCOREBOARD", titlePos, 40, 2, (Color){230, 250, 240, 255});

    for (int i = 0; i < 10; i++) {
      char entryText[64];
      snprintf(entryText, 64, "%2d. %-15s %5d", i + 1, game->highScores[i].name,
               game->highScores[i].score);
      Vector2 entryPos = {
          WINDOW_WIDTH / 2.0f -
              MeasureTextEx(game->assets.mainFont, entryText, 25, 1).x / 2.0f,
          120.0f + i * 35.0f};
      DrawTextEx(game->assets.mainFont, entryText, entryPos, 25, 1,
                 (Color){230, 250, 240, 255});
    }

    Vector2 backTextSize =
        MeasureTextEx(game->assets.mainFont, "RETURN", 20, 1);
    Vector2 subPos = {WINDOW_WIDTH / 2.0f - backTextSize.x / 2.0f,
                      WINDOW_HEIGHT - 60.0f};

    Rectangle backRect = {subPos.x - 20, subPos.y - 10, backTextSize.x + 40,
                          backTextSize.y + 20};
    Vector2 mousePos = GetMousePosition();
    bool isHovering = CheckCollisionPointRec(mousePos, backRect);

    DrawRectangleRounded(backRect, 0.3f, 10,
                         isHovering ? (Color){230, 250, 240, 50} : BLANK);
    if (isHovering)
      DrawRectangleRoundedLines(backRect, 0.3f, 10, 2,
                                (Color){230, 250, 240, 255});

    DrawTextEx(game->assets.mainFont, "RETURN", subPos, 20, 1,
               isHovering ? (Color){230, 250, 240, 255}
                          : (Color){200, 220, 210, 255});

  } else if (game->state == STATE_START) {
    Vector2 titlePos = {
        WINDOW_WIDTH / 2.0f -
            MeasureTextEx(game->assets.mainFont, "SNAKE GAME", 75, 2).x / 2.0f,
        WINDOW_HEIGHT / 3.0f - 35}; // Adjusted position for size 75
    DrawTextEx(game->assets.mainFont, "SNAKE GAME", titlePos, 75, 2, (Color){135, 206, 250, 255});

    Vector2 mousePos = GetMousePosition();

    // Start Button
    Vector2 startTextSize =
        MeasureTextEx(game->assets.mainFont, "START GAME", 20, 1);
    Vector2 startPos = {WINDOW_WIDTH / 2.0f - startTextSize.x / 2.0f,
                        WINDOW_HEIGHT / 2.0f};
    Rectangle startRect = {startPos.x - 20, startPos.y - 10,
                           startTextSize.x + 40, startTextSize.y + 20};
    bool startHovering = CheckCollisionPointRec(mousePos, startRect);
    DrawRectangleRounded(startRect, 0.3f, 10,
                         startHovering ? (Color){230, 250, 240, 50} : BLANK);
    if (startHovering)
      DrawRectangleRoundedLines(startRect, 0.3f, 10, 2,
                                (Color){230, 250, 240, 255});
    DrawTextEx(game->assets.mainFont, "START GAME", startPos, 20, 1,
               startHovering ? BLACK : (Color){230, 250, 240, 255});

    // Draw Mode Info
    const char *modeTexts[] = {"MODE: CLASSIC", "MODE: STAGES", "MODE: CHAOS"};
    Color modeColors[] = {(Color){200, 220, 210, 255}, LIME, MAGENTA};
    Vector2 modeTextSize =
        MeasureTextEx(game->assets.mainFont, modeTexts[game->mode], 18, 1);
    Vector2 modePos = {WINDOW_WIDTH / 2.0f - modeTextSize.x / 2.0f,
                       WINDOW_HEIGHT / 2.0f + 40};
    Rectangle modeRect = {modePos.x - 20, modePos.y - 10, modeTextSize.x + 40,
                          modeTextSize.y + 20};
    bool modeHovering = CheckCollisionPointRec(mousePos, modeRect);
    DrawRectangleRounded(modeRect, 0.3f, 10,
                         modeHovering ? (Color){230, 250, 240, 50} : BLANK);
    if (modeHovering)
      DrawRectangleRoundedLines(modeRect, 0.3f, 10, 2, modeColors[game->mode]);
    DrawTextEx(game->assets.mainFont, modeTexts[game->mode], modePos, 18, 1,
               modeHovering ? BLACK : modeColors[game->mode]);

    // Draw Reverse string
    const char *revText =
        game->reverseMode ? "REVERSE MODE: ON" : "REVERSE MODE: OFF";
    Vector2 revTextSize = MeasureTextEx(game->assets.mainFont, revText, 18, 1);
    Vector2 revPos = {WINDOW_WIDTH / 2.0f - revTextSize.x / 2.0f,
                      WINDOW_HEIGHT / 2.0f + 70};
    Rectangle revRect = {revPos.x - 20, revPos.y - 10, revTextSize.x + 40,
                         revTextSize.y + 20};
    bool revHovering = CheckCollisionPointRec(mousePos, revRect);
    DrawRectangleRounded(revRect, 0.3f, 10,
                         revHovering ? (Color){230, 250, 240, 50} : BLANK);
    if (revHovering)
      DrawRectangleRoundedLines(
          revRect, 0.3f, 10, 2,
          game->reverseMode ? GOLD : (Color){230, 250, 240, 255});
    DrawTextEx(game->assets.mainFont, revText, revPos, 18, 1,
               revHovering
                   ? BLACK
                   : (game->reverseMode ? GOLD : (Color){230, 250, 240, 255}));

    // Speed Button
    const char *speedText = TextFormat("SPEED: %d FPS", game->speedFPS);
    Vector2 speedTextSize =
        MeasureTextEx(game->assets.mainFont, speedText, 18, 1);
    Vector2 speedPos = {WINDOW_WIDTH / 2.0f - speedTextSize.x / 2.0f,
                        WINDOW_HEIGHT / 2.0f + 100};
    Rectangle speedRect = {speedPos.x - 20, speedPos.y - 10,
                           speedTextSize.x + 40, speedTextSize.y + 20};
    bool speedHovering = CheckCollisionPointRec(mousePos, speedRect);
    DrawRectangleRounded(speedRect, 0.3f, 10,
                         speedHovering ? (Color){230, 250, 240, 50} : BLANK);
    if (speedHovering)
      DrawRectangleRoundedLines(speedRect, 0.3f, 10, 2,
                                (Color){230, 250, 240, 255});
    DrawTextEx(game->assets.mainFont, speedText, speedPos, 18, 1,
               speedHovering ? BLACK : (Color){230, 250, 240, 255});

    // Scoreboard Button
    Vector2 sbTextSize =
        MeasureTextEx(game->assets.mainFont, "SCOREBOARD", 18, 1);
    Vector2 sbHintPos = {WINDOW_WIDTH / 2.0f - sbTextSize.x / 2.0f,
                         WINDOW_HEIGHT / 2.0f + 130};
    Rectangle sbRect = {sbHintPos.x - 20, sbHintPos.y - 10, sbTextSize.x + 40,
                        sbTextSize.y + 20};
    bool sbHovering = CheckCollisionPointRec(mousePos, sbRect);
    DrawRectangleRounded(sbRect, 0.3f, 10,
                         sbHovering ? (Color){230, 250, 240, 50} : BLANK);
    if (sbHovering)
      DrawRectangleRoundedLines(sbRect, 0.3f, 10, 2, ORANGE);
    DrawTextEx(game->assets.mainFont, "SCOREBOARD", sbHintPos, 18, 1,
               sbHovering ? ORANGE : (Color){230, 250, 240, 255});

    // Hidden Dev Mode UI
    if (game->devModeActive) {
      const char *devText =
          TextFormat("DEV MODE ACTIVE - LEVEL: %d", game->devLevelSelect);
      Vector2 devPos = {
          WINDOW_WIDTH / 2.0f -
              MeasureTextEx(game->assets.mainFont, devText, 16, 1).x / 2.0f,
          WINDOW_HEIGHT / 2.0f + 160};
      DrawTextEx(game->assets.mainFont, devText, devPos, 16, 1, YELLOW);

      const char *promptText = "< LEFT / RIGHT TO CHANGE >";
      Vector2 promptPos = {
          WINDOW_WIDTH / 2.0f -
              MeasureTextEx(game->assets.mainFont, promptText, 12, 1).x / 2.0f,
          WINDOW_HEIGHT / 2.0f + 185};
      DrawTextEx(game->assets.mainFont, promptText, promptPos, 12, 1, GOLD);
    }
  } else if (game->state == STATE_PLAYING) {
    DrawTextEx(game->assets.mainFont, TextFormat("Score: %d", game->score),
               (Vector2){10, 10}, 20, 1, (Color){230, 250, 240, 255});
    DrawTextEx(game->assets.mainFont,
               TextFormat("Best: %d", game->highScores[0].score),
               (Vector2){10, 35}, 16, 1, (Color){230, 250, 240, 255});

    Vector2 mousePos = GetMousePosition();

    const char *pauseStr = game->isPaused ? "[ RESUME ]" : "[ PAUSE ]";
    Vector2 pauseTextSize =
        MeasureTextEx(game->assets.mainFont, pauseStr, 20, 1);
    Vector2 pausePos = {WINDOW_WIDTH / 2.0f - pauseTextSize.x / 2.0f,
                        TOOLBAR_HEIGHT / 2.0f - pauseTextSize.y / 2.0f};
    Rectangle pauseRect = {pausePos.x - 10, pausePos.y - 5,
                           pauseTextSize.x + 20, pauseTextSize.y + 10};
    bool pauseHovering = CheckCollisionPointRec(mousePos, pauseRect);
    DrawRectangleRounded(pauseRect, 0.3f, 10,
                         pauseHovering ? (Color){255, 255, 255, 50} : BLANK);
    if (pauseHovering)
      DrawRectangleRoundedLines(pauseRect, 0.3f, 10, 2,
                                (Color){230, 250, 240, 255});
    DrawTextEx(game->assets.mainFont, pauseStr, pausePos, 20, 1,
               pauseHovering ? WHITE : (Color){230, 250, 240, 255});

    Vector2 menuTextSize =
        MeasureTextEx(game->assets.mainFont, "[ MENU ]", 20, 1);
    Vector2 menuPos = {WINDOW_WIDTH - menuTextSize.x - 20,
                       TOOLBAR_HEIGHT / 2.0f - menuTextSize.y / 2.0f};
    Rectangle menuRect = {menuPos.x - 10, menuPos.y - 5, menuTextSize.x + 20,
                          menuTextSize.y + 10};
    bool menuHovering = CheckCollisionPointRec(mousePos, menuRect);
    DrawRectangleRounded(menuRect, 0.3f, 10,
                         menuHovering ? (Color){255, 255, 255, 50} : BLANK);
    if (menuHovering)
      DrawRectangleRoundedLines(menuRect, 0.3f, 10, 2,
                                (Color){230, 250, 240, 255});
    DrawTextEx(game->assets.mainFont, "[ MENU ]", menuPos, 20, 1,
               menuHovering ? WHITE : (Color){230, 250, 240, 255});

    if (game->isPaused) {
      Vector2 pTxt = MeasureTextEx(game->assets.mainFont, "GAME PAUSED", 40, 2);
      DrawTextEx(
          game->assets.mainFont, "GAME PAUSED",
          (Vector2){WINDOW_WIDTH / 2.0f - pTxt.x / 2.0f, WINDOW_HEIGHT / 2.0f},
          40, 2, WHITE);
    }
  } else if (game->state == STATE_GAME_OVER) {
    Vector2 overPos = {
        WINDOW_WIDTH / 2.0f -
            MeasureTextEx(game->assets.mainFont, "GAME OVER!", 50, 2).x / 2.0f,
        WINDOW_HEIGHT / 2.0f - 50};
    DrawTextEx(game->assets.mainFont, "GAME OVER!", overPos, 50, 2, (Color){230, 250, 240, 255});

    Vector2 scorePos = {
        WINDOW_WIDTH / 2.0f -
            MeasureTextEx(game->assets.mainFont,
                          TextFormat("Final Score: %d", game->score), 20, 1)
                    .x /
                2.0f,
        WINDOW_HEIGHT / 2.0f + 10};
    DrawTextEx(game->assets.mainFont,
               TextFormat("Final Score: %d", game->score), scorePos, 20, 1,
               (Color){230, 250, 240, 255});

    Vector2 restartPos = {
        WINDOW_WIDTH / 2.0f -
            MeasureTextEx(game->assets.mainFont, "TRY AGAIN", 20, 1).x / 2.0f,
        WINDOW_HEIGHT / 2.0f + 40};
    DrawTextEx(game->assets.mainFont, "TRY AGAIN", restartPos, 20, 1,
               (Color){230, 250, 240, 255});
  } else if (game->state == STATE_GAME_WIN) {
    Vector2 winPos = {
        WINDOW_WIDTH / 2.0f -
            MeasureTextEx(game->assets.mainFont, "CHAMPION!", 60, 2).x / 2.0f,
        WINDOW_HEIGHT / 2.0f - 50};
    DrawTextEx(game->assets.mainFont, "CHAMPION!", winPos, 60, 2, (Color){230, 250, 240, 255});

    Vector2 subWinPos = {WINDOW_WIDTH / 2.0f -
                             MeasureTextEx(game->assets.mainFont,
                                           "You conquered the game!", 20, 1)
                                     .x /
                                 2.0f,
                         WINDOW_HEIGHT / 2.0f + 10};
    DrawTextEx(game->assets.mainFont, "You conquered the game!", subWinPos, 20,
               1, (Color){230, 250, 240, 255});

    Vector2 restartPos = {
        WINDOW_WIDTH / 2.0f -
            MeasureTextEx(game->assets.mainFont, "PLAY AGAIN", 20, 1).x / 2.0f,
        WINDOW_HEIGHT / 2.0f + 50};
    DrawTextEx(game->assets.mainFont, "PLAY AGAIN", restartPos, 20, 1,
               (Color){230, 250, 240, 255});
  } else if (game->state == STATE_NAME_INPUT) {
    Vector2 titlePos = {
        WINDOW_WIDTH / 2.0f -
            MeasureTextEx(game->assets.mainFont, "NEW HIGH SCORE!", 40, 2).x /
                2.0f,
        WINDOW_HEIGHT / 2.0f - 80};
    DrawTextEx(game->assets.mainFont, "NEW HIGH SCORE!", titlePos, 40, 2, (Color){230, 250, 240, 255});

    Vector2 promptPos = {
        WINDOW_WIDTH / 2.0f -
            MeasureTextEx(game->assets.mainFont, "Enter your name:", 20, 1).x /
                2.0f,
        WINDOW_HEIGHT / 2.0f - 20};
    DrawTextEx(game->assets.mainFont, "Enter your name:", promptPos, 20, 1,
               (Color){230, 250, 240, 255});

    char displayName[32];
    snprintf(displayName, 32, "%s_", game->currentName);
    Vector2 namePos = {
        WINDOW_WIDTH / 2.0f -
            MeasureTextEx(game->assets.mainFont, displayName, 30, 2).x / 2.0f,
        WINDOW_HEIGHT / 2.0f + 20};
    DrawTextEx(game->assets.mainFont, displayName, namePos, 30, 2, (Color){230, 250, 240, 255});

    Vector2 subPos = {
        WINDOW_WIDTH / 2.0f -
            MeasureTextEx(game->assets.mainFont, "SUBMIT", 15, 1).x / 2.0f,
        WINDOW_HEIGHT / 2.0f + 80};
    DrawTextEx(game->assets.mainFont, "SUBMIT", subPos, 15, 1,
               (Color){200, 220, 210, 255});
  }

  EndDrawing();
}
