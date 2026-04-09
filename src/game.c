#include "game.h"
#include "snake.h"
#include "food.h"
#include <stdio.h>

const char* HIGHSCORE_FILE = "highscore.txt";

int LoadHighScore(void) {
    FILE* file = fopen(HIGHSCORE_FILE, "r");
    if (!file) return 0;
    int high = 0;
    fscanf(file, "%d", &high);
    fclose(file);
    return high;
}

void SaveHighScore(int score) {
    FILE* file = fopen(HIGHSCORE_FILE, "w");
    if (file) {
        fprintf(file, "%d", score);
        fclose(file);
    }
}

void InitGameData(GameData* game) {
    game->state = STATE_START;
    game->score = 0;
    game->highScore = LoadHighScore();
    // Assets are initialized in main.c
}

void RestartGame(GameData* game, Snake* snake, Food* food) {
    game->score = 0;
    InitSnake(snake);
    InitFood(food, snake);
}

void UpdateGame(GameData* game, Snake* snake, Food* food) {
    if (game->state == STATE_START) {
        if (IsKeyPressed(KEY_ENTER)) {
            game->state = STATE_PLAYING;
            RestartGame(game, snake, food);
        }
    } else if (game->state == STATE_PLAYING) {
        HandleSnakeInput(snake);
        
        snake->moveTimer += GetFrameTime();
        if (snake->moveTimer >= snake->timePerMove) {
            snake->moveTimer = 0.0f;
            UpdateSnakeLogic(snake);
            
            // Check collisions
            if (CheckCollisionWithSelfOrWall(snake)) {
                game->state = STATE_GAME_OVER;
                if (game->score > game->highScore) {
                    game->highScore = game->score;
                    SaveHighScore(game->highScore);
                }
            }
            
            // Check food eating
            if (snake->body[0].x == food->position.x && snake->body[0].y == food->position.y) {
                snake->justAteFood = true;
                game->score += 10;
                SpawnFood(food, snake);
            }
        }
    } else if (game->state == STATE_GAME_OVER) {
        if (IsKeyPressed(KEY_ENTER)) {
            game->state = STATE_PLAYING;
            RestartGame(game, snake, food);
        }
    }
}

void DrawGame(GameData* game, Snake* snake, Food* food) {
    BeginDrawing();
    ClearBackground(RAYWHITE);
    
    // Draw Background if loaded
    if (game->assets.bgTex.id != 0) {
        DrawTexturePro(game->assets.bgTex,
            (Rectangle){0, 0, (float)game->assets.bgTex.width, (float)game->assets.bgTex.height},
            (Rectangle){0, 0, WINDOW_WIDTH, WINDOW_HEIGHT}, (Vector2){0, 0}, 0.0f, WHITE);
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
        DrawFood(food, game->assets.fruitTex);
        DrawSnake(snake, game->assets.headTex, game->assets.bodyTex, game->assets.tailTex);
    }

    // Draw UI
    if (game->state == STATE_START) {
        DrawText("SNAKE GAME", WINDOW_WIDTH/2 - MeasureText("SNAKE GAME", 50)/2, WINDOW_HEIGHT/2 - 50, 50, GREEN);
        DrawText("Press ENTER to start", WINDOW_WIDTH/2 - MeasureText("Press ENTER to start", 20)/2, WINDOW_HEIGHT/2 + 20, 20, WHITE);
    } else if (game->state == STATE_PLAYING) {
        DrawText(TextFormat("Score: %d", game->score), 10, 10, 20, WHITE);
        DrawText(TextFormat("Best: %d", game->highScore), 10, 35, 20, LIGHTGRAY);
    } else if (game->state == STATE_GAME_OVER) {
        DrawText("GAME OVER!", WINDOW_WIDTH/2 - MeasureText("GAME OVER!", 50)/2, WINDOW_HEIGHT/2 - 50, 50, RED);
        DrawText(TextFormat("Final Score: %d", game->score), WINDOW_WIDTH/2 - MeasureText(TextFormat("Final Score: %d", game->score), 20)/2, WINDOW_HEIGHT/2 + 10, 20, WHITE);
        DrawText("Press ENTER to try again", WINDOW_WIDTH/2 - MeasureText("Press ENTER to try again", 20)/2, WINDOW_HEIGHT/2 + 40, 20, LIGHTGRAY);
    }

    EndDrawing();
}
