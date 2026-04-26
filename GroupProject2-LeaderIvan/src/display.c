#include "display.h"
#include "food.h"
#include "game.h"
#include "raylib.h"
#include "snake.h"


void DrawGame(GameData *game, Snake *snake, Food *food) {
  BeginDrawing();
  Color arenaColor = (Color){160, 220, 160, 255}; 
  ClearBackground(arenaColor);

  Camera2D cam = {0};
  cam.offset = (Vector2){0, TOOLBAR_HEIGHT};
  cam.zoom = 1.0f;

  if (game->state == STATE_START || game->state == STATE_SCOREBOARD || game->state == STATE_NAME_INPUT) {
    
    if (game->assets.bgTex.id != 0) {
      DrawTexturePro(game->assets.bgTex,
                     (Rectangle){0, 0, (float)game->assets.bgTex.width,
                                 (float)game->assets.bgTex.height},
                     (Rectangle){0, 0, WINDOW_WIDTH, WINDOW_HEIGHT},
                     (Vector2){0, 0}, 0.0f, WHITE);
    } else {
      ClearBackground(arenaColor);
    }
  } else {
    
    ClearBackground(arenaColor);
  }

  if (game->state == STATE_PLAYING || game->state == STATE_GAME_OVER ||
      game->state == STATE_GAME_WIN) {
    BeginMode2D(cam);
    
    if (game->assets.bgTex.id != 0) {
      DrawTexturePro(game->assets.bgTex,
                     (Rectangle){0, 0, (float)game->assets.bgTex.width,
                                 (float)game->assets.bgTex.height},
                     (Rectangle){0, 0, (float)GRID_COUNT_X * GRID_SIZE,
                                 (float)GRID_COUNT_Y * GRID_SIZE},
                     (Vector2){0, 0}, 0.0f, WHITE);
    }
  }

  
  Color gridColor = (Color){
      255, 255, 255, 100}; 
  for (int i = 0; i <= GRID_COUNT_X; i++) {
    DrawLine(i * GRID_SIZE, 0, i * GRID_SIZE, GRID_COUNT_Y * GRID_SIZE,
             gridColor);
  }
  for (int i = 0; i <= GRID_COUNT_Y; i++) {
    DrawLine(0, i * GRID_SIZE, GRID_COUNT_X * GRID_SIZE, i * GRID_SIZE,
             gridColor);
  }

  
  if (game->state == STATE_PLAYING || game->state == STATE_GAME_OVER ||
      game->state == STATE_GAME_WIN) {
    
    for (int x = 0; x < GRID_COUNT_X; x++) {
      for (int y = 0; y < GRID_COUNT_Y; y++) {
        if (game->map[x][y] == TILE_WALL) {
          if (game->assets.wallTex.id != 0) {
            float wallSize = GRID_SIZE * 1.2f;
            float offset = (wallSize - GRID_SIZE) / 2.0f;
            DrawTexturePro(game->assets.wallTex,
                           (Rectangle){0, 0, (float)game->assets.wallTex.width,
                                       (float)game->assets.wallTex.height},
                           (Rectangle){(float)x * GRID_SIZE - offset,
                                       (float)y * GRID_SIZE - offset, wallSize,
                                       wallSize},
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

    
    DrawRectangle(0, 0, WINDOW_WIDTH, TOOLBAR_HEIGHT, (Color){30, 30, 30, 255});
    DrawLine(0, TOOLBAR_HEIGHT, WINDOW_WIDTH, TOOLBAR_HEIGHT, WHITE);
  }

  
  if (game->state == STATE_SCOREBOARD) {
    Vector2 titlePos = {
        WINDOW_WIDTH / 2.0f -
            MeasureTextEx(game->assets.mainFont, "TOP 10 SCOREBOARD", 40, 2).x /
                2.0f,
        50};
    DrawTextEx(game->assets.mainFont, "TOP 10 SCOREBOARD", titlePos, 40, 2,
               (Color){230, 250, 240, 255});

    for (int i = 0; i < 10; i++) {
      const char *entryText = TextFormat("%2d. %-15s %5d", i + 1, game->highScores[i].name,
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
    Rectangle backRect = {WINDOW_WIDTH / 2.0f - backTextSize.x / 2.0f - 20,
                          WINDOW_HEIGHT - 70, backTextSize.x + 40, 40};
    bool isHovering = CheckCollisionPointRec(GetMousePosition(), backRect);

    DrawRectangleRounded(backRect, 0.3f, 10,
                         isHovering ? (Color){230, 250, 240, 50} : BLANK);
    if (isHovering)
      DrawRectangleRoundedLines(backRect, 0.3f, 10, 2,
                                (Color){230, 250, 240, 255});
    DrawTextEx(game->assets.mainFont, "RETURN",
               (Vector2){backRect.x + 20, backRect.y + 10}, 20, 1,
               isHovering ? (Color){230, 250, 240, 255}
                          : (Color){200, 220, 210, 255});

  } else if (game->state == STATE_START) {
    Vector2 titlePos = {
        WINDOW_WIDTH / 2.0f -
            MeasureTextEx(game->assets.mainFont, "SAUSAGE DOG GAME", 56, 2).x / 2.0f,
        WINDOW_HEIGHT / 3.0f - 35};
    DrawTextEx(game->assets.mainFont, "SAUSAGE DOG GAME", titlePos, 56, 2,
               LIME);

    Vector2 mPos = GetMousePosition();

    
    Vector2 startTextSize =
        MeasureTextEx(game->assets.mainFont, "START GAME", 20, 1);
    Rectangle startRect = {WINDOW_WIDTH / 2.0f - startTextSize.x / 2.0f - 20,
                           WINDOW_HEIGHT / 2.0f - 10, startTextSize.x + 40,
                           startTextSize.y + 20};
    bool startHover = CheckCollisionPointRec(mPos, startRect);
    DrawRectangleRounded(startRect, 0.3f, 10,
                         startHover ? (Color){230, 250, 240, 60}
                                    : (Color){230, 250, 240, 20});
    if (startHover)
      DrawRectangleRoundedLines(startRect, 0.3f, 10, 2, WHITE);
    DrawTextEx(game->assets.mainFont, "START GAME",
               (Vector2){startRect.x + 20, startRect.y + 10}, 20, 1,
               startHover ? WHITE : LIME);

    
    const char *modeTexts[] = {"MODE: CLASSIC", "MODE: STAGES", "MODE: CHAOS"};
    Color modeColors[] = {(Color){200, 220, 210, 255}, LIME, MAGENTA};
    Vector2 modeTextSize =
        MeasureTextEx(game->assets.mainFont, modeTexts[game->mode], 18, 1);
    Rectangle modeRect = {WINDOW_WIDTH / 2.0f - modeTextSize.x / 2.0f - 20,
                          WINDOW_HEIGHT / 2.0f + 30, modeTextSize.x + 40,
                          modeTextSize.y + 20};
    bool modeHover = CheckCollisionPointRec(mPos, modeRect);
    DrawRectangleRounded(modeRect, 0.3f, 10,
                         modeHover ? (Color){230, 250, 240, 50} : BLANK);
    if (modeHover)
      DrawRectangleRoundedLines(modeRect, 0.3f, 10, 2, modeColors[game->mode]);
    DrawTextEx(game->assets.mainFont, modeTexts[game->mode],
               (Vector2){modeRect.x + 20, modeRect.y + 10}, 18, 1,
               modeHover ? modeColors[game->mode] : modeColors[game->mode]);

    
    const char *revText =
        game->reverseMode ? "REVERSE MODE: ON" : "REVERSE MODE: OFF";
    Vector2 revTextSize = MeasureTextEx(game->assets.mainFont, revText, 18, 1);
    Rectangle revRect = {WINDOW_WIDTH / 2.0f - revTextSize.x / 2.0f - 20,
                         WINDOW_HEIGHT / 2.0f + 60, revTextSize.x + 40,
                         revTextSize.y + 20};
    bool revHover = CheckCollisionPointRec(mPos, revRect);
    DrawRectangleRounded(revRect, 0.3f, 10,
                         revHover ? (Color){230, 250, 240, 50} : BLANK);
    if (revHover)
      DrawRectangleRoundedLines(revRect, 0.3f, 10, 2, GOLD);
    DrawTextEx(game->assets.mainFont, revText,
               (Vector2){revRect.x + 20, revRect.y + 10}, 18, 1,
               revHover
                   ? GOLD
                   : (game->reverseMode ? GOLD : (Color){230, 250, 240, 255}));

    
    const char *speedText = TextFormat("SPEED: %d FPS", game->speedFPS);
    Vector2 speedTextSize =
        MeasureTextEx(game->assets.mainFont, speedText, 18, 1);
    Rectangle speedRect = {WINDOW_WIDTH / 2.0f - speedTextSize.x / 2.0f - 20,
                           WINDOW_HEIGHT / 2.0f + 90, speedTextSize.x + 40,
                           speedTextSize.y + 20};
    bool speedHover = CheckCollisionPointRec(mPos, speedRect);
    DrawRectangleRounded(speedRect, 0.3f, 10,
                         speedHover ? (Color){230, 250, 240, 50} : BLANK);
    if (speedHover)
      DrawRectangleRoundedLines(speedRect, 0.3f, 10, 2, WHITE);
    DrawTextEx(game->assets.mainFont, speedText,
               (Vector2){speedRect.x + 20, speedRect.y + 10}, 18, 1,
               speedHover ? WHITE : (Color){230, 250, 240, 255});

    
    Vector2 sbTextSize =
        MeasureTextEx(game->assets.mainFont, "SCOREBOARD", 18, 1);
    Rectangle sbRect = {WINDOW_WIDTH / 2.0f - sbTextSize.x / 2.0f - 20,
                        WINDOW_HEIGHT / 2.0f + 120, sbTextSize.x + 40,
                        sbTextSize.y + 20};
    bool sbHover = CheckCollisionPointRec(mPos, sbRect);
    DrawRectangleRounded(sbRect, 0.3f, 10,
                         sbHover ? (Color){230, 250, 240, 50} : BLANK);
    if (sbHover)
      DrawRectangleRoundedLines(sbRect, 0.3f, 10, 2, ORANGE);
    DrawTextEx(game->assets.mainFont, "SCOREBOARD",
               (Vector2){sbRect.x + 20, sbRect.y + 10}, 18, 1,
               sbHover ? ORANGE : (Color){230, 250, 240, 255});

    if (game->devModeActive) {
      const char *devText =
          TextFormat("DEV MODE ACTIVE - LEVEL: %d", game->devLevelSelect);
      DrawTextEx(
          game->assets.mainFont, devText,
          (Vector2){WINDOW_WIDTH / 2.0f -
                        MeasureTextEx(game->assets.mainFont, devText, 16, 1).x /
                            2.0f,
                    WINDOW_HEIGHT / 2.0f + 160},
          16, 1, YELLOW);
    }
  } else if (game->state == STATE_PLAYING) {
    DrawTextEx(game->assets.mainFont, TextFormat("Score: %d", game->score),
               (Vector2){10, 10}, 20, 1, (Color){230, 250, 240, 255});
    DrawTextEx(game->assets.mainFont,
               TextFormat("Best: %d", game->highScores[0].score),
               (Vector2){10, 35}, 16, 1, (Color){230, 250, 240, 255});

    if (game->isPaused) {
      DrawRectangle(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, (Color){0, 0, 0, 150});
      Vector2 mPos = GetMousePosition();
      const char *pTitle = "GAME PAUSED";
      DrawTextEx(
          game->assets.mainFont, pTitle,
          (Vector2){WINDOW_WIDTH / 2.0f -
                        MeasureTextEx(game->assets.mainFont, pTitle, 50, 2).x /
                            2.0f,
                    WINDOW_HEIGHT / 2.0f - 120},
          50, 2, WHITE);

      Rectangle resumeRect = {WINDOW_WIDTH / 2.0f - 150,
                              WINDOW_HEIGHT / 2.0f - 60, 300, 80};
      bool resumeHover = CheckCollisionPointRec(mPos, resumeRect);
      DrawRectangleRounded(resumeRect, 0.3f, 10,
                           resumeHover ? (Color){230, 250, 240, 100}
                                       : (Color){230, 250, 240, 40});
      DrawRectangleRoundedLines(resumeRect, 0.3f, 10, 2, WHITE);
      DrawTextEx(
          game->assets.mainFont, "RESUME",
          (Vector2){
              resumeRect.x +
                  (resumeRect.width -
                   MeasureTextEx(game->assets.mainFont, "RESUME", 30, 1).x) /
                      2.0f,
              resumeRect.y + 25},
          30, 1, WHITE);

      Rectangle backRect = {WINDOW_WIDTH / 2.0f - 225,
                            WINDOW_HEIGHT / 2.0f + 40, 450, 80};
      bool backHover = CheckCollisionPointRec(mPos, backRect);
      DrawRectangleRounded(backRect, 0.3f, 10,
                           backHover ? (Color){230, 250, 240, 100}
                                     : (Color){230, 250, 240, 40});
      DrawRectangleRoundedLines(backRect, 0.3f, 10, 2, WHITE);
      DrawTextEx(game->assets.mainFont, "BACK TO MENU",
                 (Vector2){backRect.x + (backRect.width -
                                         MeasureTextEx(game->assets.mainFont,
                                                       "BACK TO MENU", 30, 1)
                                             .x) /
                                            2.0f,
                           backRect.y + 25},
                 30, 1, WHITE);
    }
  } else if (game->state == STATE_GAME_OVER || game->state == STATE_GAME_WIN) {
    const char *status =
        (game->state == STATE_GAME_OVER) ? "GAME OVER!" : "CHAMPION!";
    DrawTextEx(
        game->assets.mainFont, status,
        (Vector2){WINDOW_WIDTH / 2.0f -
                      MeasureTextEx(game->assets.mainFont, status, 50, 2).x /
                          2.0f,
                  WINDOW_HEIGHT / 2.0f - 50},
        50, 2, (Color){230, 250, 240, 255});
    DrawTextEx(
        game->assets.mainFont, TextFormat("Final Score: %d", game->score),
        (Vector2){WINDOW_WIDTH / 2.0f -
                      MeasureTextEx(game->assets.mainFont,
                                    TextFormat("Final Score: %d", game->score),
                                    20, 1)
                              .x /
                          2.0f,
                  WINDOW_HEIGHT / 2.0f + 10},
        20, 1, (Color){230, 250, 240, 255});

    float reY = (game->state == STATE_GAME_OVER) ? (WINDOW_HEIGHT / 2.0f + 30)
                                                 : (WINDOW_HEIGHT / 2.0f + 40);
    Rectangle reRect = {WINDOW_WIDTH / 2.0f - 80, reY, 160, 40};
    bool reHover = CheckCollisionPointRec(GetMousePosition(), reRect);
    DrawRectangleRounded(reRect, 0.3f, 10,
                         reHover ? (Color){230, 250, 240, 60}
                                 : (Color){230, 250, 240, 20});
    DrawRectangleRoundedLines(reRect, 0.3f, 10, 2, WHITE);
    const char *btnTxt =
        (game->state == STATE_GAME_OVER) ? "TRY AGAIN" : "PLAY AGAIN";
    DrawTextEx(
        game->assets.mainFont, btnTxt,
        (Vector2){reRect.x +
                      (reRect.width -
                       MeasureTextEx(game->assets.mainFont, btnTxt, 20, 1).x) /
                          2.0f,
                  reRect.y + 10},
        20, 1, WHITE);

  } else if (game->state == STATE_NAME_INPUT) {
    DrawTextEx(
        game->assets.mainFont, "NEW HIGH SCORE!",
        (Vector2){WINDOW_WIDTH / 2.0f - MeasureTextEx(game->assets.mainFont,
                                                      "NEW HIGH SCORE!", 40, 2)
                                                .x /
                                            2.0f,
                  WINDOW_HEIGHT / 2.0f - 80},
        40, 2, (Color){230, 250, 240, 255});
    DrawTextEx(
        game->assets.mainFont, "Enter your name:",
        (Vector2){WINDOW_WIDTH / 2.0f - MeasureTextEx(game->assets.mainFont,
                                                      "Enter your name:", 20, 1)
                                                .x /
                                            2.0f,
                  WINDOW_HEIGHT / 2.0f - 20},
        20, 1, (Color){230, 250, 240, 255});
    const char *displayName = TextFormat("%s_", game->currentName);
    DrawTextEx(
        game->assets.mainFont, displayName,
        (Vector2){
            WINDOW_WIDTH / 2.0f -
                MeasureTextEx(game->assets.mainFont, displayName, 30, 2).x /
                    2.0f,
            WINDOW_HEIGHT / 2.0f + 20},
        30, 2, (Color){230, 250, 240, 255});
  }

  EndDrawing();
}
