#include "snake.h"
#include "game.h"
#include <math.h>

void InitSnake(Snake* snake) {
    snake->length = 3;
    // Start somewhere in the middle
    int startX = GRID_COUNT_X / 2;
    int startY = GRID_COUNT_Y / 2;
    
    // Initialize body horizontally at CENTER of grid cells (0.5 offset)
    for (int i = 0; i < snake->length; i++) {
        snake->body[i] = (Vector2){(float)startX + 0.5f, (float)startY + i + 0.5f};
    }
    
    snake->direction = (Vector2){0, -1}; // Moving UP initially
    snake->nextDirection = snake->direction;
    snake->inputQueueLength = 0;
    snake->isAlive = true;
    snake->justAteFood = false;
    snake->invulnerabilityTimer = 0.0f;
    
    // Pre-fill trail with initial body positions to ensure smooth segments from frame 1
    // We space points out between the segments
    snake->trailHead = 0;
    snake->trailCount = 0;
    
    // Fill from tail to head
    for (int i = snake->length - 1; i >= 0; i--) {
        snake->trail[snake->trailHead] = snake->body[i];
        snake->trailHead = (snake->trailHead + 1) % TRAIL_SIZE;
        snake->trailCount++;
    }
    // trailHead currently points to the next empty slot; back it up to the head
    snake->trailHead = (snake->trailHead - 1 + TRAIL_SIZE) % TRAIL_SIZE;
    
    snake->trailTimer = 0.0f;
}

void HandleSnakeInput(Snake* snake) {
    Vector2 nextDir = {0};
    bool keyHit = false;

    if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) { nextDir = (Vector2){0, -1}; keyHit = true; }
    else if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) { nextDir = (Vector2){0, 1}; keyHit = true; }
    else if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A)) { nextDir = (Vector2){-1, 0}; keyHit = true; }
    else if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) { nextDir = (Vector2){1, 0}; keyHit = true; }

    if (keyHit) {
        // Find current effective direction (last queued or current)
        Vector2 currentDir = snake->direction;
        if (snake->inputQueueLength > 0) {
            currentDir = snake->inputQueue[snake->inputQueueLength - 1];
        }

        // Prevent reversing direction
        if ((nextDir.x != 0 && nextDir.x != -currentDir.x) ||
            (nextDir.y != 0 && nextDir.y != -currentDir.y)) {
            
            // Add to queue
            if (snake->inputQueueLength < 2) {
                snake->inputQueue[snake->inputQueueLength] = nextDir;
                snake->inputQueueLength++;
            }
        }
    }
}

void UpdateSnakeLogic(Snake* snake, GameData* game) {
    if (!snake->isAlive) return;
    
    float dt = GetFrameTime();
    
    if (snake->invulnerabilityTimer > 0) {
        snake->invulnerabilityTimer -= dt;
        if (snake->invulnerabilityTimer < 0) snake->invulnerabilityTimer = 0;
    }

    // Handle growth
    if (snake->justAteFood) {
        if (snake->length < MAX_SNAKE_LENGTH) {
            snake->length++;
        }
        snake->justAteFood = false;
    }

    // --- TURNING LOGIC (Grid Alignment) ---
    // We only allow turning when the head is near the center of a grid cell
    // This maintains the "Snake" logic where paths are on the grid.
    float centerX = floorf(snake->body[0].x) + 0.5f;
    float centerY = floorf(snake->body[0].y) + 0.5f;
    float distToCenter = sqrtf(powf(snake->body[0].x - centerX, 2) + powf(snake->body[0].y - centerY, 2));
    
    // If we are close to center and have a turn queued
    if (distToCenter < 0.2f && snake->inputQueueLength > 0) {
        Vector2 requestedDir = snake->inputQueue[0];
        // Ensure not reversing
        if (!((requestedDir.x != 0 && requestedDir.x == -snake->direction.x) ||
              (requestedDir.y != 0 && requestedDir.y == -snake->direction.y))) {
            
            // Snap to center when turning for perfect alignment
            snake->body[0].x = centerX;
            snake->body[0].y = centerY;
            snake->direction = requestedDir;
            
            // Pop queue
            snake->inputQueue[0] = snake->inputQueue[1];
            snake->inputQueueLength--;
        }
    }

    // --- MOVEMENT FORMULA ---
    // New Position = Old Position + (Direction * Speed * dt)
    Vector2 oldPos = snake->body[0];
    snake->body[0].x += snake->direction.x * snake->speed * dt;
    snake->body[0].y += snake->direction.y * snake->speed * dt;

    // --- TRAIL AND BODY FOLLOWING ---
    // Record head position in history
    snake->trailHead = (snake->trailHead + 1) % TRAIL_SIZE;
    snake->trail[snake->trailHead] = snake->body[0];
    if (snake->trailCount < TRAIL_SIZE) snake->trailCount++;

    // Position each segment by looking back through the trail for 1.0 distance intervals
    Vector2 currentPoint = snake->body[0];
    float targetDist = 1.0f;
    float accumulatedDist = 0.0f;
    int trailIdx = snake->trailHead;
    int segmentsFound = 1;

    for (int i = 0; i < snake->trailCount && segmentsFound < snake->length; i++) {
        int prevIdx = (trailIdx - 1 + TRAIL_SIZE) % TRAIL_SIZE;
        Vector2 p1 = snake->trail[trailIdx];
        Vector2 p2 = snake->trail[prevIdx];
        
        float d = sqrtf(powf(p1.x - p2.x, 2) + powf(p1.y - p2.y, 2));
        
        while (accumulatedDist + d >= targetDist && segmentsFound < snake->length) {
            // Found a segment position! Interpolate between p1 and p2
            float needed = targetDist - accumulatedDist;
            float t = (d > 0) ? needed / d : 0;
            
            snake->body[segmentsFound].x = p1.x + (p2.x - p1.x) * t;
            snake->body[segmentsFound].y = p1.y + (p2.y - p1.y) * t;
            
            segmentsFound++;
            targetDist += 1.0f; // Next segment is 1.0 unit further
        }
        
        accumulatedDist += d;
        trailIdx = prevIdx;
    }
    
    // Portal Check (Lookahead for snappier interaction)
    int headGridX = (int)(snake->body[0].x + snake->direction.x * 0.4f);
    int headGridY = (int)(snake->body[0].y + snake->direction.y * 0.4f);

    if (headGridX >= 0 && headGridX < GRID_COUNT_X && 
        headGridY >= 0 && headGridY < GRID_COUNT_Y) {
        
        if (game->map[headGridX][headGridY] == TILE_PORTAL) {
            // Find which portal we hit
            int pIdx = (headGridX == (int)game->portals[0].x && headGridY == (int)game->portals[0].y) ? 0 : 1;
            Vector2 exit = game->portals[1 - pIdx];
            
            // Warp head
            snake->body[0] = (Vector2){exit.x + 0.5f, exit.y + 0.5f};
            // Clear trail to prevent body "stretching" over the map
            for(int i=0; i<TRAIL_SIZE; i++) snake->trail[i] = snake->body[0];
            snake->trailCount = 1;
        }
    }
}

bool CheckCollisionWithSelfOrWall(Snake* snake, GameData* game) {
    Vector2 head = snake->body[0];
    
    // Bounds check
    if (head.x < 0 || head.x >= GRID_COUNT_X || head.y < 0 || head.y >= GRID_COUNT_Y) {
        snake->isAlive = false;
        return true;
    }
    
    // Map check (Obstacles)
    int headX = (int)(head.x + (snake->direction.x * 0.4f));
    int headY = (int)(head.y + (snake->direction.y * 0.4f));
    
    // Boundary clamp for index access
    if (headX < 0) headX = 0; if (headX >= GRID_COUNT_X) headX = GRID_COUNT_X - 1;
    if (headY < 0) headY = 0; if (headY >= GRID_COUNT_Y) headY = GRID_COUNT_Y - 1;

    if (game->map[headX][headY] == TILE_WALL) {
        // Skip wall collision if in Ghost Mode
        if (snake->invulnerabilityTimer <= 0) {
            snake->isAlive = false;
            return true;
        }
    }
    
    // Self check (Skip segments very close to head)
    for (int i = 2; i < snake->length; i++) {
        float d = sqrtf(powf(head.x - snake->body[i].x, 2) + powf(head.y - snake->body[i].y, 2));
        if (d < 0.6f) { // Collision radius
            snake->isAlive = false;
            return true;
        }
    }
    
    return false;
}

// Helper to draw rotated texture
void DrawTextureCenteredEx(Texture2D tex, Vector2 pos, float rotation, float scale, Color tint) {
    if (tex.id == 0) return;
    Rectangle source = {0, 0, (float)tex.width, (float)tex.height};
    Rectangle dest = {pos.x, pos.y, tex.width * scale, tex.height * scale};
    Vector2 origin = {(tex.width * scale) / 2.0f, (tex.height * scale) / 2.0f};
    
    DrawTexturePro(tex, source, dest, origin, rotation, tint);
}

void DrawSnake(Snake* snake, Texture2D headTex, Texture2D bodyTex, Texture2D tailTex) {
    // Distance between nodes is GRID_SIZE. 
    // Reduced from 2.0f to 1.1f so the snake fits inside the grid blocks better.
    float overlapDiameter = GRID_SIZE * 1.1f;
    
    // Ghost mode tint (pulsing transparency)
    Color snakeTint = WHITE;
    if (snake->invulnerabilityTimer > 0) {
        float alpha = (sinf((float)GetTime() * 15.0f) * 0.5f + 0.5f) * 150 + 50; 
        snakeTint = (Color){255, 255, 255, (unsigned char)alpha};
    }

    // HIGHEST Z-INDEX is HEAD. LOWEST Z-INDEX is TAIL.
    // So we loop backwards: Tail -> Body -> Head (so head is rendered last and stays on top)
    for (int i = snake->length - 1; i >= 0; i--) {
        Vector2 gridPos = snake->body[i];
        // The coordinates are already centered (+0.5), so we just multiply by GRID_SIZE
        Vector2 center = { 
            gridPos.x * GRID_SIZE, 
            gridPos.y * GRID_SIZE 
        };

        if (i == 0) { // HEAD
            if (headTex.id != 0) {
                float scale = overlapDiameter / (float)headTex.width;
                float rot = 0.0f;
                // Raylib rotation is in degrees. Adjust if your image points a different default way.
                // Assuming default head sprite points UP
                if (snake->direction.x == 1) rot = 90.0f;
                else if (snake->direction.x == -1) rot = 270.0f;
                else if (snake->direction.y == 1) rot = 180.0f;
                else if (snake->direction.y == -1) rot = 0.0f;
                
                rot += 90.0f; // User requested: lật sang phải 1 lần
                
                DrawTextureCenteredEx(headTex, center, rot, scale, WHITE);
            } else {
                DrawRectangle(gridPos.x * GRID_SIZE, gridPos.y * GRID_SIZE, GRID_SIZE, GRID_SIZE, DARKGREEN);
            }
        } 
        else if (i == snake->length - 1) { // TAIL
            if (tailTex.id != 0) {
                float scale = overlapDiameter / (float)tailTex.width;
                // Rotation based on the segment it is attached to (i-1)
                Vector2 diff = { snake->body[i-1].x - snake->body[i].x, snake->body[i-1].y - snake->body[i].y };
                float rot = 0.0f;
                // Assuming tail points UP towards the body
                if (diff.x == 1) rot = 90.0f;
                else if (diff.x == -1) rot = 270.0f;
                else if (diff.y == 1) rot = 180.0f;
                else if (diff.y == -1) rot = 0.0f;
                
                rot += 90.0f; // User requested: lật sang phải 1 lần
                
                DrawTextureCenteredEx(tailTex, center, rot, scale, WHITE);
            } else {
                DrawRectangle(gridPos.x * GRID_SIZE, gridPos.y * GRID_SIZE, GRID_SIZE, GRID_SIZE, GREEN);
            }
        } 
        else { // BODY
            if (bodyTex.id != 0) {
                float scale = overlapDiameter / (float)bodyTex.width;
                // Body circles don't strictly need rotation
                DrawTextureCenteredEx(bodyTex, center, 0.0f, scale, snakeTint);
            } else {
                Color tint = (i % 2 == 0) ? LIME : GREEN;
                // Fallback circular overlap
                DrawCircle(center.x, center.y, overlapDiameter / 2.0f, tint);
            }
        }
    }
}

void ReverseSnake(Snake* snake) {
    // Reverse the body array up to length
    for (int i = 0; i < snake->length / 2; i++) {
        Vector2 temp = snake->body[i];
        snake->body[i] = snake->body[snake->length - 1 - i];
        snake->body[snake->length - 1 - i] = temp;
    }
    
    // Update direction based on new head and new second segment
    if (snake->length > 1) {
        snake->direction.x = snake->body[0].x - snake->body[1].x;
        snake->direction.y = snake->body[0].y - snake->body[1].y;
    }
    
    // Clear input queue as old directions are no longer valid
    snake->inputQueueLength = 0;
}
