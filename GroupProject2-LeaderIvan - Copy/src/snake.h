#ifndef SNAKE_H
#define SNAKE_H

#include "raylib.h"
#include <stdbool.h>
#include "game.h"

#define MAX_SNAKE_LENGTH (GRID_COUNT_X * GRID_COUNT_Y)

typedef struct Snake {
    Vector2 body[MAX_SNAKE_LENGTH];
    int length;
    Vector2 direction;
    
    // Input queue for smooth turning
    Vector2 inputQueue[2];
    int inputQueueLength;
    
    // Fixed timestep
    float moveTimer;
    float timePerMove; // seconds per movement tick
    
    bool isAlive;
    bool justAteFood;
    float invincibilityTimer;
} Snake;

void InitSnake(Snake* snake);
void HandleSnakeInput(Snake* snake);
void UpdateSnakeLogic(Snake* snake, GameData* game);
void DrawSnake(Snake* snake, Texture2D headTex, Texture2D bodyTex, Texture2D tailTex);
bool CheckCollisionWithSelfOrWall(Snake* snake, GameData* game);
void ReverseSnake(Snake* snake);

#endif // SNAKE_H
