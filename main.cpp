/**
* Author: [ Eric Lin ]
* Assignment: Lunar Lander
* Date due: [Jun 14]
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/


#include "CS3113/Entity.h"

struct GameState
{
    Entity *rocket;
    Entity *tiles;
    Entity *blocks;
    Entity *universe;
};

// Global Constants
constexpr int SCREEN_WIDTH  = 1000,
              SCREEN_HEIGHT = 600,
              FPS           = 120;

constexpr char    BG_COLOUR[]      = "#C0897E";
constexpr Vector2 ORIGIN           = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 },
                  ATLAS_DIMENSIONS = { 6, 8 };

constexpr int   NUMBER_OF_TILES  = 18,
                NUMBER_OF_BLOCKS = 4;
constexpr float TILE_DIMENSION          = 50.0f,
                ACCELERATION_OF_GRAVITY = 5.0f,
                FIXED_TIMESTEP          = 1.0f / 60.0f,
                END_GAME_THRESHOLD      = 800.0f;
float gFrame;
// Global Variables
AppStatus gAppStatus   = RUNNING;
float gPreviousTicks   = 0.0f,
      gTimeAccumulator = 0.0f;

GameState gState;

bool missionAccomplished = false;
bool missionFailed = false;
int fuel = 999;




// Function Declarations
void initialise();
void processInput();
void update();
void render();
void shutdown();

void initialise()
{
    gFrame = 0.0f;
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Do Not Land On Lava");
    /*
        ----------- Universe -----------
    */
    gState.universe = new Entity(
        ORIGIN, // position
        {SCREEN_WIDTH, SCREEN_HEIGHT}, // scale
        "assets/game/universe.png", // texture filepath
        NONE // entity type
    );




    /*
        ----------- ROCKET -----------
    */
    gState.rocket = new Entity(
        {ORIGIN.x - 300.0f, ORIGIN.y - 200.0f}, // position
        {30.0f , 30.0f},            // scale
        "assets/game/lander.png", // texture filepath
        PLAYER 
    );


    gState.rocket->setJumpingPower(30.0f);
    gState.rocket->setColliderDimensions({
        gState.rocket->getScale().x,
        gState.rocket->getScale().y 
    });

    gState.rocket->setAcceleration({0.0f, ACCELERATION_OF_GRAVITY});

    /*
        ----------- TILES -----------
    */
    gState.tiles = new Entity[NUMBER_OF_TILES];

    float leftMostX = ORIGIN.x - (NUMBER_OF_TILES * TILE_DIMENSION) / 2.0f;

    for (int i = 0; i < NUMBER_OF_TILES; i++)
    {
        // @see https://kenney.nl/assets/pixel-platformer-industrial-expansion
        gState.tiles[i].setTexture("assets/game/tile_0000.png");
        gState.tiles[i].setEntityType(PLATFORM);
        gState.tiles[i].setScale({TILE_DIMENSION, TILE_DIMENSION});
        gState.tiles[i].setColliderDimensions({TILE_DIMENSION, TILE_DIMENSION});
        gState.tiles[i].setPosition({
            leftMostX + i * TILE_DIMENSION,
            SCREEN_HEIGHT - TILE_DIMENSION
        });
    }

    /*
        ----------- BLOCKS -----------
    */
    gState.blocks = new Entity[NUMBER_OF_BLOCKS];

    for (int i = 0; i < NUMBER_OF_BLOCKS; i++)
    {
        // @see https://kenney.nl/assets/pixel-platformer-industrial-expansion
        gState.blocks[i].setTexture("assets/game/lava.png");
        gState.blocks[i].setEntityType(BLOCK);
        gState.blocks[i].setScale({TILE_DIMENSION, TILE_DIMENSION});
        gState.blocks[i].setColliderDimensions({TILE_DIMENSION, TILE_DIMENSION});
        gState.blocks[i].setPosition(
            {ORIGIN.x - TILE_DIMENSION * i * 3, 
            SCREEN_HEIGHT - TILE_DIMENSION * 2.5f
        });
    }







    SetTargetFPS(FPS);
}

void processInput()
{
    if (IsKeyPressed(KEY_Q) || WindowShouldClose()) {
        gAppStatus = TERMINATED;
    }
    gState.rocket->setAcceleration({0.0f, 0.0f}); //clean acceleration and then calculate for this frame.
    gState.rocket->setEntityType(PLAYER);

    if (fuel > 0) {
        bool usingFuel = false;
        if      (IsKeyDown(KEY_W) && IsKeyDown(KEY_A)) { gState.rocket->accelerateUpLeft();     usingFuel=true;}    
        else if (IsKeyDown(KEY_W) && IsKeyDown(KEY_D)) { gState.rocket->accelerateUpRight();    usingFuel=true;}    
        else if (IsKeyDown(KEY_S) && IsKeyDown(KEY_A)) { gState.rocket->accelerateDownLeft();   usingFuel=true;}    
        else if (IsKeyDown(KEY_S) && IsKeyDown(KEY_D)) { gState.rocket->accelerateDownRight();  usingFuel=true;}    
        else if (IsKeyDown(KEY_A))                     { gState.rocket->accelerateLeft();       usingFuel=true;}
        else if (IsKeyDown(KEY_D))                     { gState.rocket->accelerateRight();      usingFuel=true;}
        else if (IsKeyDown(KEY_W))                     { gState.rocket->accelerateUp();         usingFuel=true;}
        else if (IsKeyDown(KEY_S))                     { gState.rocket->accelerateDown();       usingFuel=true;}
        if (usingFuel) {
            fuel--;
            gState.rocket->setEntityType(PROPEL);
        }
    }
    //add gravity
    gState.rocket->setAcceleration({
        gState.rocket->getAcceleration().x,
        gState.rocket->getAcceleration().y + ACCELERATION_OF_GRAVITY
    }); 

}

void update()
{
    float ticks    = (float) GetTime();
    float deltaTime = ticks - gPreviousTicks;
    gPreviousTicks  = ticks;

    deltaTime += gTimeAccumulator;

    if (deltaTime < FIXED_TIMESTEP)
    {
        gTimeAccumulator = deltaTime;
        return;
    }

    while (deltaTime >= FIXED_TIMESTEP)
    {
        gFrame++;
        if (gState.rocket->getPosition().y > SCREEN_HEIGHT)
            missionFailed = true;
        if (gState.rocket->getPosition().y < 0)
            missionFailed = true;
        if (gState.rocket->getPosition().x > SCREEN_WIDTH)
            missionFailed = true;
        if (gState.rocket->getPosition().x < 0)
            missionFailed = true;
        for (int i = 0; i < NUMBER_OF_BLOCKS; i++) {
            missionFailed = gState.rocket->isColliding(&gState.blocks[i]);
            if (missionFailed) {
                return;
            }
        }
        for (int i = 0; i < NUMBER_OF_TILES; i++)
        {
            missionAccomplished = gState.rocket->isColliding(&gState.tiles[i]);
            if (missionAccomplished)
            {
                return;
            }
            
        }





        gState.blocks[0].setPosition({
            ORIGIN.x + 100.0f * cos(gFrame / 60.0f),
            gState.blocks[0].getPosition().y
        });
        gState.rocket->update(FIXED_TIMESTEP, nullptr, gState.tiles,
            NUMBER_OF_TILES, gState.blocks, NUMBER_OF_BLOCKS);

        deltaTime -= FIXED_TIMESTEP;
    }

    if (gState.rocket->getPosition().y > END_GAME_THRESHOLD)
        gAppStatus = TERMINATED;
}

void render()
{
    BeginDrawing();
    ClearBackground(ColorFromHex(BG_COLOUR));
    
    if (missionAccomplished)
    {
        DrawText("Mission Accomplished!", 350, 250, 40, GREEN);
        EndDrawing();
        return;
    }
    if (missionFailed)
    {
        DrawText("Mission Failed!", 350, 250, 40, RED);
        EndDrawing();
        return;
    }

    gState.universe->render();
    gState.rocket->render();
    //gState.rocket->displayCollider();


    // Display fuel
    char fuelText[20];
    snprintf(fuelText, 20, "Fuel: %d", fuel);
    DrawText(fuelText, 20, 20, 30, RED);

    for (int i = 0; i < NUMBER_OF_TILES;  i++) gState.tiles[i].render();
    for (int i = 0; i < NUMBER_OF_BLOCKS; i++) gState.blocks[i].render();

    EndDrawing();
}

void shutdown()
{
    delete   gState.rocket;
    delete[] gState.tiles;
    delete[] gState.blocks;
    delete   gState.universe;

    CloseWindow();
}

int main(void)
{
    initialise();

    while (gAppStatus == RUNNING)
    {
        processInput();
        update();
        render();
    }

    shutdown();

    return 0;
}
