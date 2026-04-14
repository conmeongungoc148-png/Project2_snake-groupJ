#include "food.h"
#include "game.h"

void InitFood(Food* food, Snake* snake) {
    SpawnFood(food, snake);
}

void SpawnFood(Food* food, Snake* snake) {
    bool valid = false;
    while (!valid) {
        // Generate random coordinate between 0 and GRID_COUNT - 1
        food->position.x = (float)GetRandomValue(0, GRID_COUNT_X - 1);
        food->position.y = (float)GetRandomValue(0, GRID_COUNT_Y - 1);
        valid = true;
        
        // Ensure food doesn't spawn inside snake body
        for (int i = 0; i < snake->length; i++) {
            if (food->position.x == snake->body[i].x && food->position.y == snake->body[i].y) {
                valid = false;
                break; // Try again
            }
        }
    }
}

void DrawFood(Food* food, Texture2D fruitTex) {
    Vector2 center = { 
        food->position.x * GRID_SIZE + (GRID_SIZE / 2.0f), 
        food->position.y * GRID_SIZE + (GRID_SIZE / 2.0f) 
    };
    
    // Radius slightly smaller than the grid cell
    float radius = GRID_SIZE / 2.0f * 0.8f;
    
    if (fruitTex.id != 0) {
        // Draw the user provided fruit.png
        float diam = radius * 2.0f;
        float scale = diam / (float)fruitTex.width;
        Rectangle source = {0, 0, (float)fruitTex.width, (float)fruitTex.height};
        Rectangle dest = {center.x, center.y, diam, diam};
        Vector2 origin = {diam / 2.0f, diam / 2.0f};
        DrawTexturePro(fruitTex, source, dest, origin, 0.0f, WHITE);
    } else {
        // Fallback orange circle
        DrawCircle(center.x, center.y, radius, ORANGE);
        DrawCircle(center.x - radius*0.3f, center.y - radius*0.3f, radius*0.2f, WHITE);
    }
}
