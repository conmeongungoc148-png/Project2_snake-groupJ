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

  
  for (int x = 0; x < GRID_COUNT_X; x++)
    for (int y = 0; y < GRID_COUNT_Y; y++)
      game->map[x][y] = TILE_EMPTY;
}

void RestartGame(GameData *game, Snake *snake, Food *food) {
  game->score = 0;
  game->isPaused = false;
  InitSnake(snake);

  
  if (game->mode == MODE_STAGES) {
    LoadStage(game, 1);
  } else if (game->mode == MODE_CHAOS) {
    LoadChaosStage(game, 1);
  } else {
    
    for (int x = 0; x < GRID_COUNT_X; x++)
      for (int y = 0; y < GRID_COUNT_Y; y++)
        game->map[x][y] = TILE_EMPTY;
  }

  snake->timePerMove = 1.0f / (float)game->speedFPS;
  InitFood(food, snake, game);
}

void UpdateGame(GameData *game, Snake *snake, Food *food) {
  switch (game->state) {
  case STATE_START: {
    bool clickedSb = false;
    bool clickedStart = false;
    bool clickedMode = false;
    bool clickedRev = false;
    bool clickedSpeed = false;

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
      Vector2 mousePos = GetMousePosition();

      
      Vector2 startTextSize =
          MeasureTextEx(game->assets.mainFont, "START GAME", 20, 1);
      Rectangle startRect = {WINDOW_WIDTH / 2.0f - startTextSize.x / 2.0f - 20,
                             WINDOW_HEIGHT / 2.0f - 10, startTextSize.x + 40,
                             startTextSize.y + 20};
      if (CheckCollisionPointRec(mousePos, startRect))
        clickedStart = true;

      
      const char *modeTexts[] = {"MODE: CLASSIC", "MODE: STAGES",
                                 "MODE: CHAOS"};
      Vector2 modeTextSize =
          MeasureTextEx(game->assets.mainFont, modeTexts[game->mode], 18, 1);
      Rectangle modeRect = {WINDOW_WIDTH / 2.0f - modeTextSize.x / 2.0f - 20,
                            WINDOW_HEIGHT / 2.0f + 30, modeTextSize.x + 40,
                            modeTextSize.y + 20};
      if (CheckCollisionPointRec(mousePos, modeRect))
        clickedMode = true;

      
      const char *revText =
          game->reverseMode ? "REVERSE MODE: ON" : "REVERSE MODE: OFF";
      Vector2 revTextSize =
          MeasureTextEx(game->assets.mainFont, revText, 18, 1);
      Rectangle revRect = {WINDOW_WIDTH / 2.0f - revTextSize.x / 2.0f - 20,
                           WINDOW_HEIGHT / 2.0f + 60, revTextSize.x + 40,
                           revTextSize.y + 20};
      if (CheckCollisionPointRec(mousePos, revRect))
        clickedRev = true;

      
      const char *speedText = TextFormat("SPEED: %d FPS", game->speedFPS);
      Vector2 speedTextSize =
          MeasureTextEx(game->assets.mainFont, speedText, 18, 1);
      Rectangle speedRect = {WINDOW_WIDTH / 2.0f - speedTextSize.x / 2.0f - 20,
                             WINDOW_HEIGHT / 2.0f + 90, speedTextSize.x + 40,
                             speedTextSize.y + 20};
      if (CheckCollisionPointRec(mousePos, speedRect))
        clickedSpeed = true;

      
      Vector2 sbTextSize =
          MeasureTextEx(game->assets.mainFont, "SCOREBOARD", 18, 1);
      Rectangle sbRect = {WINDOW_WIDTH / 2.0f - sbTextSize.x / 2.0f - 20,
                          WINDOW_HEIGHT / 2.0f + 120, sbTextSize.x + 40,
                          sbTextSize.y + 20};
      if (CheckCollisionPointRec(mousePos, sbRect))
        clickedSb = true;
    }

    
    if (clickedStart || IsKeyPressed(KEY_ENTER)) {
      game->state = STATE_PLAYING;
      RestartGame(game, snake, food);

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
      if (game->mode != MODE_CLASSIC)
        snake->invincibilityTimer = 2.0f;
    }

    if (clickedMode || IsKeyPressed(KEY_L)) {
      game->mode = (game->mode + 1) % 3;
      game->devLevelSelect = 1;
    }

    if (clickedRev || IsKeyPressed(KEY_M)) {
      game->reverseMode = !game->reverseMode;
    }

    if (clickedSpeed || IsKeyPressed(KEY_K)) {
      game->speedFPS += 4;
      if (game->speedFPS > 20)
        game->speedFPS = 4;
    }

    if (clickedSb || IsKeyPressed(KEY_H)) {
      game->state = STATE_SCOREBOARD;
    }

    if (IsKeyPressed(KEY_D)) {
      game->devModeActive = !game->devModeActive;
    }

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
  } break;

  case STATE_PLAYING: {
    if (IsKeyPressed(KEY_R)) {
      game->isPaused = !game->isPaused;
    }

    if (game->isPaused) {
      Vector2 mPos = GetMousePosition();
      Rectangle resumeRect = {WINDOW_WIDTH / 2.0f - 150,
                              WINDOW_HEIGHT / 2.0f - 60, 300, 80};
      if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) &&
          CheckCollisionPointRec(mPos, resumeRect)) {
        game->isPaused = false;
      }
      Rectangle backRect = {WINDOW_WIDTH / 2.0f - 225,
                            WINDOW_HEIGHT / 2.0f + 40, 450, 80};
      if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) &&
          CheckCollisionPointRec(mPos, backRect)) {
        game->isPaused = false;
        game->state = STATE_START;
      }
    } else {
      HandleSnakeInput(snake);
      if (snake->invincibilityTimer > 0)
        snake->invincibilityTimer -= GetFrameTime();

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
              } else
                game->state = STATE_GAME_WIN;
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
              } else
                game->state = STATE_GAME_WIN;
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
  } break;

  case STATE_NAME_INPUT: {
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
      if (game->nameLength > 0) {
        game->nameLength--;
        game->currentName[game->nameLength] = '\0';
      }
    }
    if (IsKeyPressed(KEY_ENTER)) {
      int insertIndex = 9;
      while (insertIndex >= 0 &&
             game->score > game->highScores[insertIndex].score) {
        insertIndex--;
      }
      insertIndex++;
      for (int i = 9; i > insertIndex; i--)
        game->highScores[i] = game->highScores[i - 1];
      game->highScores[insertIndex].score = game->score;
      snprintf(game->highScores[insertIndex].name, 16, "%s",
               (game->nameLength == 0) ? "Unknown" : game->currentName);
      SaveScoreboard(game);
      game->state = (game->score >= 500 && (game->mode != MODE_CLASSIC))
                        ? STATE_GAME_WIN
                        : STATE_GAME_OVER;
    }
  } break;

  case STATE_GAME_OVER:
  case STATE_GAME_WIN: {
    bool clickedRestart = false;
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
      Vector2 mPos = GetMousePosition();
      float reY = (game->state == STATE_GAME_OVER)
                      ? (WINDOW_HEIGHT / 2.0f + 30)
                      : (WINDOW_HEIGHT / 2.0f + 40);
      Rectangle reRect = {WINDOW_WIDTH / 2.0f - 80, reY, 160, 40};
      if (CheckCollisionPointRec(mPos, reRect))
        clickedRestart = true;
    }
    if (IsKeyPressed(KEY_ENTER) || clickedRestart) {
      game->state = STATE_PLAYING;
      RestartGame(game, snake, food);
    }
  } break;

  case STATE_SCOREBOARD: {
    bool clickedBack = false;
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
      Vector2 mousePos = GetMousePosition();
      Vector2 backTextSize =
          MeasureTextEx(game->assets.mainFont, "RETURN", 20, 1);
      Rectangle backRect = {WINDOW_WIDTH / 2.0f - backTextSize.x / 2.0f - 20,
                            WINDOW_HEIGHT - 70, backTextSize.x + 40, 40};
      if (CheckCollisionPointRec(mousePos, backRect))
        clickedBack = true;
    }
    if (clickedBack || IsKeyPressed(KEY_H) || IsKeyPressed(KEY_ENTER) ||
        IsKeyPressed(KEY_ESCAPE)) {
      game->state = STATE_START;
    }
  } break;
  }
}
