#ifndef FOOD_H
#define FOOD_H

#include "raylib.h"
#include "snake.h"

typedef struct Food {
    Vector2 position;
} Food;

void InitFood(Food* food, Snake* snake, GameData* game);
void SpawnFood(Food* food, Snake* snake, GameData* game);
void DrawFood(Food* food, Texture2D fruitTex);

#endif 

