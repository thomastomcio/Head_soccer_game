#include "game.h"
#include "resource_manager.h"
#include "sprite_renderer.h"
#include "game_object.h"

// Game-related State data
SpriteRenderer  *Renderer;

// Represents the four possible (collision) directions
enum Direction {
    UP,
    RIGHT,
    DOWN,
    LEFT
};
// Defines a Collision typedef that represents collision data
typedef std::tuple<bool, Direction, glm::vec2> Collision; // <collision?, what direction?, difference vector center - closest point>

// Initial velocity of the Ball
const glm::vec2 INITIAL_BALL_VELOCITY(100.0f, -350.0f);
// Radius of the ball object
const float BALL_RADIUS = 25.0f;
BallObject     *Ball; 

// Initial velocity of the players
const float PLAYER_VELOCITY(500.0f);
// players size
const glm::vec2 PLAYER_SIZE(100.0f, 200.0f);
GameObject *Player1;
GameObject *Player2;

Game::Game(unsigned int width, unsigned int height) 
    : State(GAME_ACTIVE), Keys(), Width(width), Height(height)
{ 

}

Game::~Game()
{
    delete Renderer;
    delete Player1;
    // delete Player2;
    delete Ball;
}

void Game::Init()
{
       // load shaders
    ResourceManager::LoadShader("shaders/sprite.vs", "shaders/sprite.frag", nullptr, "sprite");
    // configure shaders
    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(this->Width), 
        static_cast<float>(this->Height), 0.0f, -1.0f, 1.0f);
    ResourceManager::GetShader("sprite").Use().SetInteger("image", 0);
    ResourceManager::GetShader("sprite").SetMatrix4("projection", projection);
    Renderer = new SpriteRenderer(ResourceManager::GetShader("sprite"));
    // set render-specific controls
    // load textures
    ResourceManager::LoadTexture("resources/textures/background_wisla.jpg", false, "background");
    ResourceManager::LoadTexture("resources/textures/bramka_3.png", false, "bramka");
    ResourceManager::LoadTexture("resources/textures/bramka4.png", false, "bramka_mirror");

    ResourceManager::LoadTexture("resources/textures/ball.png", true, "ball");
    ResourceManager::LoadTexture("resources/textures/ronaldo.png", true, "player1");
    ResourceManager::LoadTexture("resources/textures/rooney.png", true, "player2");

    glm::vec2 ballPos = glm::vec2(this->Width/2.0f - BALL_RADIUS, this->Height/2.0f - BALL_RADIUS * 2.0f);
    Ball = new BallObject(ballPos, BALL_RADIUS, INITIAL_BALL_VELOCITY, ResourceManager::GetTexture("ball"));

    glm::vec2  Player1_Initial_Pos =  glm::vec2(this->Width/5.0f - PLAYER_SIZE.x, this->Height/1.2f - PLAYER_SIZE.y);
    glm::vec2  Player2_Initial_Pos =  glm::vec2(this->Width/2.0f - PLAYER_SIZE.x, this->Height/1.2f - PLAYER_SIZE.y);
    Player1 = new GameObject(Player1_Initial_Pos, PLAYER_SIZE, ResourceManager::GetTexture("player1"));
    Player2 = new GameObject(Player2_Initial_Pos, PLAYER_SIZE, ResourceManager::GetTexture("player2"));

}

void Game::Update(float dt)
{
    Ball->Move(dt, this->Width, this->Height);
    // check for collisions
    this->DoCollisions();
}

void Game::ProcessInput(float dt)
{
     float velocity = PLAYER_VELOCITY * dt;
        // move playerboard

        // player1
        if (this->Keys[GLFW_KEY_A])
        {
            if (Player1->Position.x >= 0.0f)
            {
                Player1->Position.x -= velocity;
            }
        }
        if (this->Keys[GLFW_KEY_D])
        {
            if (Player1->Position.x <= this->Width - Player1->Size.x)
            {
                Player1->Position.x += velocity;
            }
        }

        // player2
        if (this->Keys[GLFW_KEY_LEFT])
        {
            if (Player2->Position.x >= 0.0f)
            {
                Player2->Position.x -= velocity;
            }
        }
        if (this->Keys[GLFW_KEY_RIGHT])
        {
            if (Player2->Position.x <= this->Width - Player2->Size.x)
            {
                Player2->Position.x += velocity;
            }
        }
        
    if (this->Keys[GLFW_KEY_SPACE]) Ball->Stuck = false;
}

void Game::Render()
{
    // draw background
        Renderer->DrawSprite(ResourceManager::GetTexture("background"), glm::vec2(0.0f, 0.0f), glm::vec2(this->Width, this->Height), 0.0f);
    // draw bramka
        // Renderer->DrawSprite(ResourceManager::GetTexture("bramka"), glm::vec2(30.0f, 300.0f), glm::vec2(150.0f, 150.0f), 0.0f);
        // Renderer->DrawSprite(ResourceManager::GetTexture("bramka_mirror"), glm::vec2(this->Width-200.0f, this->Height-300.0f), glm::vec2(150.0f, 150.0f), 0.0f);
    // draw ball
        Ball->Draw(*Renderer);
    // draw players
        Player1->Draw(*Renderer);
        Player2->Draw(*Renderer);

    // draw face
    //Renderer->DrawSprite(ResourceManager::GetTexture("face"), glm::vec2(200.0f, 200.0f), glm::vec2(300.0f, 400.0f), 45.0f, glm::vec3(0.0f, 1.0f, 0.0f));
}

void Game::ResetPlayers()
{
    // reset player/ball stats
    Player1->Size = PLAYER_SIZE;
    Player1->Position = glm::vec2(this->Width / 2.0f - PLAYER_SIZE.x / 2.0f, this->Height - PLAYER_SIZE.y);

    Player2->Size = PLAYER_SIZE;
    Player2->Position = glm::vec2(this->Width / 2.0f - PLAYER_SIZE.x / 2.0f, this->Height - PLAYER_SIZE.y);

    //Ball->Reset(Player1->Position + glm::vec2(PLAYER_SIZE.x / 2.0f - BALL_RADIUS, -(BALL_RADIUS * 2.0f)), INITIAL_BALL_VELOCITY);
}

// collision detection
bool CheckCollision(GameObject &one, GameObject &two);
Collision CheckCollision(BallObject &one, GameObject &two);
Direction VectorDirection(glm::vec2 closest);

void Game::DoCollisions()
{
    // check collisions for player1
    Collision result1 = CheckCollision(*Ball, *Player1);
    if (!Ball->Stuck && std::get<0>(result1))
    {
        // check where it hit the board, and change velocity based on where it hit the board
        float centerBoard = Player1->Position.x + Player1->Size.x / 2.0f;
        float distance = (Ball->Position.x + Ball->Radius) - centerBoard;
        float percentage = distance / (Player1->Size.x / 2.0f);
        // then move accordingly
        float strength = 2.0f;
        glm::vec2 oldVelocity = Ball->Velocity;
        Ball->Velocity.x = INITIAL_BALL_VELOCITY.x * percentage * strength; 
        //Ball->Velocity.y = -Ball->Velocity.y;
        Ball->Velocity = glm::normalize(Ball->Velocity) * glm::length(oldVelocity); // keep speed consistent over both axes (multiply by length of old velocity, so total strength is not changed)
        // fix sticky paddle
        Ball->Velocity.y = -1.0f * abs(Ball->Velocity.y);
    }

        // check collisions for player2
    Collision result2 = CheckCollision(*Ball, *Player2);
    if (!Ball->Stuck && std::get<0>(result2))
    {
        // check where it hit the board, and change velocity based on where it hit the board
        float centerBoard = Player2->Position.x + Player2->Size.x / 2.0f;
        float distance = (Ball->Position.x + Ball->Radius) - centerBoard;
        float percentage = distance / (Player2->Size.x / 2.0f);
        // then move accordingly
        float strength = 2.0f;
        glm::vec2 oldVelocity = Ball->Velocity;
        Ball->Velocity.x = INITIAL_BALL_VELOCITY.x * percentage * strength; 
        //Ball->Velocity.y = -Ball->Velocity.y;
        Ball->Velocity = glm::normalize(Ball->Velocity) * glm::length(oldVelocity); // keep speed consistent over both axes (multiply by length of old velocity, so total strength is not changed)
        // fix sticky paddle
        Ball->Velocity.y = -1.0f * abs(Ball->Velocity.y);
    }
}

bool CheckCollision(GameObject &one, GameObject &two) // AABB - AABB collision
{
    // collision x-axis?
    bool collisionX = one.Position.x + one.Size.x >= two.Position.x &&
        two.Position.x + two.Size.x >= one.Position.x;
    // collision y-axis?
    bool collisionY = one.Position.y + one.Size.y >= two.Position.y &&
        two.Position.y + two.Size.y >= one.Position.y;
    // collision only if on both axes
    return collisionX && collisionY;
}

Collision CheckCollision(BallObject &one, GameObject &two) // AABB - Circle collision
{
    // get center point circle first 
    glm::vec2 center(one.Position + one.Radius);
    // calculate AABB info (center, half-extents)
    glm::vec2 aabb_half_extents(two.Size.x / 2.0f, two.Size.y / 2.0f);
    glm::vec2 aabb_center(two.Position.x + aabb_half_extents.x, two.Position.y + aabb_half_extents.y);
    // get difference vector between both centers
    glm::vec2 difference = center - aabb_center;
    glm::vec2 clamped = glm::clamp(difference, -aabb_half_extents, aabb_half_extents);
    // now that we know the the clamped values, add this to AABB_center and we get the value of box closest to circle
    glm::vec2 closest = aabb_center + clamped;
    // now retrieve vector between center circle and closest point AABB and check if length < radius
    difference = closest - center;

    if (glm::length(difference) < one.Radius) // not <= since in that case a collision also occurs when object one exactly touches object two, which they are at the end of each collision resolution stage.
        return std::make_tuple(true, VectorDirection(difference), difference);
    else
        return std::make_tuple(false, UP, glm::vec2(0.0f, 0.0f));
}

// calculates which direction a vector is facing (N,E,S or W)
Direction VectorDirection(glm::vec2 target)
{
    glm::vec2 compass[] = {
        glm::vec2(0.0f, 1.0f),	// up
        glm::vec2(1.0f, 0.0f),	// right
        glm::vec2(0.0f, -1.0f),	// down
        glm::vec2(-1.0f, 0.0f)	// left
    };
    float max = 0.0f;
    unsigned int best_match = -1;
    for (unsigned int i = 0; i < 4; i++)
    {
        float dot_product = glm::dot(glm::normalize(target), compass[i]);
        if (dot_product > max)
        {
            max = dot_product;
            best_match = i;
        }
    }
    return (Direction)best_match;
}