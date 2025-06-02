#include <raylib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <stdlib.h> 

#define MAX_STARS 1000  // Define the maximum number of stars as a constant

// Enumeration for game screens
typedef enum {
    menu,
    game,
    death
} gamescreen;

// Character struct
typedef struct Character {
    float x;
    float y;
    float width;
    float height;
    float velocity;
} Character;

// Platform struct
typedef struct Platform {
    float x;
    float y;
    float width;
    float height;
    bool scoregiven;  // Flag to track if score has been given for this platform
} Platform;

typedef struct death_plat { //invisible line at the bottom of the screen(moves up with the screen) to kill the character
    float x;
    float y;
    float width;
    float height;
} death_plat;

float rand_number() {
    return (float)((float)rand() / (float)RAND_MAX);
}

// Check if the character is on the platform
int character_on_platform(Character Character, Platform Platforms[], int count, int* score, bool isAlive) {
    for (int i = 0; i < count; i++) {
        if (!isAlive) {
            return -1;  // Skip collision checks if the character is dead
        }
        Platform* Platform = &Platforms[i];
        // Character rectangle
        Rectangle characterRec = { Character.x + 30, Character.y + (Character.height * 0.8), Character.width - 60, Character.height * 0.2 };
        // Platform rectangle
        Rectangle platformRec = { Platforms[i].x, Platforms[i].y, Platforms[i].width, Platforms[i].height };
        // Check if the character is on the platform
        if (CheckCollisionRecs(characterRec, platformRec)) {
            if (!Platform->scoregiven) {
                (*score)++;  // Increment the score
                Platform->scoregiven = true;  // Mark the platform as scored
            }
            return i;
        }
    }
    return -1;
}

int main() {

    srand(time(NULL)); // Seed the random number generator

    int time_counter = 0; // Set the time counter
    int jump_flag = 0;
    int score = 0;
    int world_height = 950 * 100;
    int platform_count = 100;
    int isMovingPlatform = 0;
    bool wasMovingRight = false; // last direction of movement
    bool IsOnGround = true;
    bool isJumping = true;
    bool isAlive = true;
    bool isGameOver = false; // Game-over state flag
    bool transition = false; // Variable to handle screen transitions
    bool movingPlatform = false;
    float platformSpeed = 3.0f;
    float transitionAlpha = 0.0f; // Speed of transition
    float camera_speed = 2; // speed of the character
    float gravity = 0.5; // Set the gravity
    float jump_size = 15; // size of the jump
    float screenWidth = 800; // Set the screen width
    float screenHeight = 950; // Set the screen height
    float speed_timer = 0; // Set the timer
    float timer_stopper = 5; // Set the timer stopper
    double startTime = GetTime(); // Starting time
    double timerDuration = 15.0;  // Timer duration in seconds
    double elapsedTime = 0.0;

    Color platform_color = { 255, 50, 50, 255 }; // Set the background color
    Color moving_platform_color = { 50, 50, 255, 255 }; // Set the moving platform color
    Character Character = { 351, 628, 98, 142, 4 }; // Set the character data
    death_plat death_plat = { 0, 950, screenWidth, 1 }; // Set the dead platform data
    gamescreen main = menu; // Initialize the screen to menu

    // Initialize the window
    InitWindow(screenWidth, screenHeight, "A Way Up");
    SetTargetFPS(60); // setting the FPS

    InitAudioDevice();  // Initializes the audio device
    Sound BackgroundSound = LoadSound("C:/Users/ahmed/Downloads/Undertale - Megalovania.mp3"); // Load a sound file

    SetSoundVolume(BackgroundSound, 0.2);
    Sound Jump = LoadSound("C:/Users/ahmed/Downloads/Mario Jump Sound Effect.mp3");
    Sound GameOver = LoadSound("C:/Users/ahmed/Downloads/waa.mp3");
    SetSoundVolume(Jump, 0.09);
    SetSoundVolume(GameOver, 0.1);

    // Load textures for menu and game screens
    Texture2D start_s = LoadTexture("C:/Users/ahmed/Downloads/ufo background 1.png");
    Texture2D game_s = LoadTexture("C:/Users/ahmed/Downloads/ufo background 1.png");
    Texture2D characterTextureLeft = LoadTexture("C:/Users/ahmed/Downloads/50%/right.png");
    Texture2D characterTextureRight = LoadTexture("C:/Users/ahmed/Downloads/50%/left.png");
    Texture2D characterTextureJumping = LoadTexture("C:/Users/ahmed/Downloads/50%/jumping.png");
    Texture2D UFO = LoadTexture("C:/Users/ahmed/Downloads/BigUFO.png");
    Texture2D currentTexture = characterTextureLeft;  // Default texture

    if (start_s.id == 0 || game_s.id == 0) {
        TraceLog(LOG_ERROR, "Failed to load texture.");
        CloseWindow();
        return -1; // Exit the program if the texture loading fails
    }

    // Set up source and destination rectangles for scaling textures
    Rectangle sourceRec = { 0, 0, (float)start_s.width, (float)start_s.height };
    if (start_s.width > screenWidth || start_s.height > screenHeight) {
        float aspectRatio = (float)start_s.width / start_s.height;
        if (aspectRatio > (float)screenWidth / screenHeight) {
            sourceRec.width = sourceRec.width * ((float)screenWidth / screenHeight);
            sourceRec.x = (start_s.width - sourceRec.width) / 2.0f;
        }
        else {
            sourceRec.height = sourceRec.height * ((float)screenHeight / screenWidth);
            sourceRec.y = (start_s.height - sourceRec.height) / 2.0f;
        }
    }

    Rectangle destRec = { 0, 0, (float)screenWidth, (float)screenHeight };
    Vector2 origin = { 0, 0 };
    Vector2 stars[MAX_STARS]; // Define an array of stars

    // Initialize the stars
    for (int i = 0; i < MAX_STARS; i++) {
        stars[i].x = GetRandomValue(0, screenWidth);
        stars[i].y = GetRandomValue(0, screenHeight);
    }

    // Title text for menu screen
    const char* message1 = "A WAY UP";
    int fontSize = 50;
    int textHeight = fontSize;
    int textWidth = MeasureText(message1, textHeight);
    int xPos = (screenWidth - textWidth) / 2;
    int yPos = (screenHeight - textHeight) / 2;

    // assign platforms first data (ground/UFO)
    Platform Platforms[101];
    Platforms[0].x = 277;
    Platforms[0].y = 770;
    Platforms[0].width = 246;
    Platforms[0].height = 30;
    Platforms[0].scoregiven = true;

    // assign random platforms data
    int platform_y = 570;
    for (int i = 1; i < platform_count; i++) {
        Platforms[i].x = fabs((screenWidth - 150) * rand_number()); 
        Platforms[i].y = platform_y;
        Platforms[i].width = 200;
        Platforms[i].height = 30;
        Platforms[i].scoregiven = false;  // Initialize score given to false
        platform_y -= 200;
    }

    // Platform count
    int Platform_count = sizeof(Platforms) / sizeof(Platform); //how many elemts inside the array
    //size of platforms in bytes = array/struct
    // Camera setup
    Camera2D camera = { 0 };
    camera.offset = (Vector2{ screenWidth / 2 , screenHeight - 180 }); //180 is the y of first platform
    camera.target = (Vector2{ Character.x + Character.width / 2, Character.y + Character.height });//camera focused on character
    camera.zoom = 1.0f; // Camera zoom level (normal)

    PlaySound(BackgroundSound);
    while (!WindowShouldClose()) {

        speed_timer += GetFrameTime(); // Update the timer

        // Event Handling
        // horizontal movement
        if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) {
            Character.x -= 5;
            if (IsOnGround) {
                currentTexture = characterTextureLeft;
                wasMovingRight = false;
            }
            else {
                currentTexture = characterTextureJumping;
                IsOnGround = false;
            }
        }
        if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) {
            Character.x += 5;
            if (IsOnGround) {
                currentTexture = characterTextureRight;
                wasMovingRight = true;
            }
            else {
                currentTexture = characterTextureJumping;
                IsOnGround = false;
            }
        }

        // gravity
        Character.y += Character.velocity; //to make the character fall
        Character.velocity += gravity;  // to prevent constant velocity while falling

        // Check if the character is on the platform 
        int current_platform = character_on_platform(Character, Platforms, Platform_count, &score, isAlive);
        if (current_platform != -1) {
            if (Character.velocity > 0) {
                Character.y = Platforms[current_platform].y - Character.height; //make the character stand on the platform
                Character.velocity = 0;
                IsOnGround = true;
            }

            // Jumping when on when on platform only
            if (IsKeyDown(KEY_SPACE) || IsKeyDown(KEY_UP) || IsKeyDown(KEY_W)) {
                if (IsOnGround) { // Only jump if on the ground
                    Character.velocity = -jump_size;
                    IsOnGround = false;
                    currentTexture = characterTextureJumping;
                    isJumping = true; // Set isJumping to true when jumping
                    PlaySound(Jump);
                }
            }

            // return the pose to either left or right when landing
            if (IsOnGround) {
                currentTexture = wasMovingRight ? characterTextureRight : characterTextureLeft;
            }
        }

        // taking last pose when the character is the ground to use it again when landing
        if (!(IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D) || IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A))) {
            if (IsOnGround) {
                currentTexture = wasMovingRight ? characterTextureRight : characterTextureLeft;
            }
        }

        // Wrap around
        if (Character.x > screenWidth) {
            Character.x = 0;
        }
        if (Character.x < 0) {
            Character.x = screenWidth;
        }

        for (int j = 1; j < Platform_count; j++) {
            if (j % 5 == 0 && j != 0) { //moving platform after every 5 static platforms
                movingPlatform = true;
                isMovingPlatform = j;
            }
            else {
                movingPlatform = false;
            }
            if (movingPlatform == true) {
                Platforms[j].x += platformSpeed;
                if (Platforms[j].x > screenWidth) {
                    Platforms[j].x = 0;
                }
                if (Platforms[j].x < 0) {
                    Platforms[j].x = screenWidth;
                }
            }
        }

        // Handle transitions between screens
        if (main == menu && !transition) {
            if (IsKeyPressed(KEY_ENTER)) {
                transition = true; // Start transition to game screen
            }
        }

        if (transition) { //fade in
            transitionAlpha += 0.01f; // Slow transition effect,keeps increasing the opacity of the screen

            if (transitionAlpha >= 1.0f) {
                transitionAlpha = 1.0f;
                transition = false;
                main = game; // Switch to game screen
            }
        }
        else if (transitionAlpha > 0.0f && main == game) { //fade out
            transitionAlpha -= 0.05f; // Slow transition effect while in-game
        }

        else if (main == game && !transition) {
            if (!isAlive) {

                transition = true; // Start transition to game screen
            }
        }

        if (transition) {
            transitionAlpha += 0.01f; // Slow transition effect

            if (transitionAlpha >= 1.0f) {
                transitionAlpha = 1.0f;
                transition = false;
                main = death; // Switch to game screen

            }
        }
        else if (transitionAlpha > 0 && main == death) {
            // Keep fade fully active on the death screen
            transitionAlpha -= 0.5f;
        }

        // Position Updates
        float time = GetTime(); // Get elapsed time
        int red = (int)(sin(time * 2.0f) * 127 + 128);   // Oscillate between 0 and 255(colors are from 0 to 255)
        int green = (int)(sin(time * 2.0f + 2) * 127 + 128);
        int blue = (int)(sin(time * 2.0f + 4) * 127 + 128);

        Color textColor;   // Explicitly assign each color component
        textColor.r = (unsigned char)red;
        textColor.g = (unsigned char)green;
        textColor.b = (unsigned char)blue;
        textColor.a = 255; //opacity

        if (isJumping) {
            for (int i = 0; i < MAX_STARS; i++) {
                stars[i].y += 1;  // Move each star downwards
                if (stars[i].y > screenHeight) {
                    stars[i].y = 0; // Reset star position when off-screen
                    stars[i].x = GetRandomValue(0, screenWidth); // Random X position
                }
            }
        }

        // increase camera speed  5 seconds
        if (speed_timer >= timer_stopper && time_counter <= 5) { // Check if the timer is greater than the stopper and increases only 5 times
            speed_timer = 0; // Reset the timer
            camera_speed += 0.5; // Increase the speed
            timer_stopper += 5; // Increase the stopper
            time_counter++; // Increase the time counter
        }

        // Camera follows the character, with vertical movement (Y-axis) in the beginning of the game
        if (IsKeyDown(KEY_SPACE) || IsKeyDown(KEY_UP) || IsKeyDown(KEY_W)) {
            jump_flag = 1;
        }
        if (jump_flag == 1) {
            if (Character.y < camera.offset.y) {
                camera.offset.y += camera_speed; // Move the camera up
                death_plat.y -= camera_speed; // Move the dead platform up
            }
        }

        // death part
        Rectangle characterRec = { Character.x, Character.y, Character.width, Character.height / 2 }; // character rectangle (Hitbox of character)
        Rectangle death_platRec = { death_plat.x, death_plat.y, death_plat.width, death_plat.height }; // death platform rectangle
        // Check if the character touched death platform
        if (CheckCollisionRecs(characterRec, death_platRec)) {
            isAlive = false;
            main = death;
            StopSound(BackgroundSound);
            PlaySound(GameOver);


            // Reset score given flags for all platforms
            for (int i = 1; i < Platform_count; i++) {
                Platforms[i].scoregiven = false;
            }
        }

        // Restart the game
        if (IsKeyPressed(KEY_R)) {
            score = 0;
            isGameOver = false;   // Exit game-over state
            isAlive = true;       // Reset character's alive status
            Character.x = 351;    // Reset character's position
            Character.y = 628;
            Character.velocity = 0;
            camera.offset = (Vector2{ screenWidth / 2, screenHeight - 180 });
            death_plat.y = 950;
            Platforms[0].scoregiven = true;
            jump_flag = 0;
            main = game;
            camera_speed = 2;
            PlaySound(BackgroundSound);
        }

        // Drawing section
        BeginDrawing();
        ClearBackground(RAYWHITE);

        if (main == menu) {
            // Draw menu background and text
            DrawTexturePro(start_s, sourceRec, destRec, origin, 0.0f, WHITE);
            DrawText("Menu Screen", 320, 200, 20, RED);
            DrawText("Press ENTER to start the game", 240, 250, 20, WHITE);
            DrawText("use left and right arrow for horizontal movement", 155, 500, 20, WHITE);
            DrawText("and use space or up arrow to jump", 230, 520, 20, WHITE);
            DrawText(message1, xPos, yPos, fontSize, textColor);
        }

        else if (main == game) {

            ClearBackground(BLACK);

            for (int i = 0; i < MAX_STARS; i++) {
                DrawCircle(stars[i].x, stars[i].y, 1, WHITE); // Draw each star
            }
            if (jump_flag == 0) {
                DrawText("Game Screen", 320, 200, 20, platform_color);
            }
            BeginMode2D(camera);
            DrawTexture(UFO, Platforms[0].x, Platforms[0].y, WHITE);

            DrawRectangle(death_plat.x, death_plat.y, death_plat.width, death_plat.height, platform_color);

            // draw the platforms
            for (int i = 1; i < Platform_count; i++) {
                // Draw a rectangle with rounded corners
                Rectangle rect = { Platforms[i].x, Platforms[i].y, Platforms[i].width, Platforms[i].height }; // x, y, width, height
                float roundness = 1.0f; // Between 0.0f (sharp corners) and 1.0f (fully rounded)
                int segments = 8;       // Number of segments for corner smoothing
                DrawRectangleRounded(rect, roundness, segments, platform_color);
                for (int j = 1; j <= Platform_count; j++) {
                    if (j % 5 == 0 && j != 0) {
                        movingPlatform = true;
                        isMovingPlatform = j;
                    }
                    else {
                        movingPlatform = false;
                    }
                    if (movingPlatform == true) {
                        Rectangle movingRect = { Platforms[j].x, Platforms[j].y, Platforms[j].width, Platforms[j].height }; // x, y, width, heigh
                        DrawRectangleRounded(movingRect, roundness, segments, moving_platform_color);
                    }
                }
            }
            DrawTexture(currentTexture, Character.x, Character.y, WHITE);
            EndMode2D();
            DrawText(TextFormat("Score: %d", score), 10, 10, 20, YELLOW);

            BeginMode2D(camera);
        }

        else if (main == death)
        {
            ClearBackground(BLACK);
            DrawText("You Died!", 225, 400, 80, RED);
            DrawText("Press R to restart", 260, 765, 30, RED);
            DrawText(TextFormat("Score: %d", score), 335, 690, 30, YELLOW);
        }

        // Draw transition effect overlay
        if (transitionAlpha > 0.0f) {
            DrawRectangle(0, 0, 800, 950, Fade(BLACK, transitionAlpha));
        }

        EndMode2D();
        EndDrawing();
    }

    // Unload textures and close the window
    UnloadTexture(start_s);
    UnloadTexture(game_s);
    UnloadTexture(characterTextureLeft);
    UnloadTexture(characterTextureRight);
    UnloadTexture(UFO);
    UnloadTexture(characterTextureJumping);
    UnloadSound(BackgroundSound);  // Unload the sound from memory
    CloseAudioDevice();      // Close the audio device

    CloseWindow();
    return 0;
}
