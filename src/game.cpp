#include "game.h"
#include "resource_manager.h"
#include "sprite_renderer.h"
#include "game_object.h"

// Game-related State data
SpriteRenderer  *Renderer;

Game::Game(unsigned int width, unsigned int height) 
    : State(GAME_ACTIVE), Keys(), Width(width), Height(height)
{ 

}

Game::~Game()
{
    delete Renderer;
}


// Initial velocity of the Ball
const glm::vec2 INITIAL_BALL_VELOCITY(100.0f, -350.0f);
// Radius of the ball object
const float BALL_RADIUS = 25.0f;
  
BallObject     *Ball; 

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
    ResourceManager::LoadTexture("resources/textures/bramka2.bmp", false, "bramka");
    ResourceManager::LoadTexture("resources/textures/bramka2_mirror.bmp", false, "bramka_mirror");

    ResourceManager::LoadTexture("resources/textures/ball.png", true, "face");


    glm::vec2 ballPos = glm::vec2(this->Width/2.0f - BALL_RADIUS, this->Height/2.0f - BALL_RADIUS * 2.0f);
    Ball = new BallObject(ballPos, BALL_RADIUS, INITIAL_BALL_VELOCITY, ResourceManager::GetTexture("face"));
}

void Game::Update(float dt)
{
    Ball->Move(dt, this->Width, this->Height);
}

void Game::ProcessInput(float dt)
{
    if (this->Keys[GLFW_KEY_SPACE]) Ball->Stuck = false;
}

void Game::Render()
{
    // draw background
        Renderer->DrawSprite(ResourceManager::GetTexture("background"), glm::vec2(0.0f, 0.0f), glm::vec2(this->Width, this->Height), 0.0f);
    // draw bramka
        Renderer->DrawSprite(ResourceManager::GetTexture("bramka"), glm::vec2(30.0f, 300.0f), glm::vec2(100.0f, 200.0f), 0.0f);
        Renderer->DrawSprite(ResourceManager::GetTexture("bramka_mirror"), glm::vec2(this->Width-130.0f, this->Height-300.0f), glm::vec2(100.0f, 200.0f), 0.0f);
    // draw ball
        Ball->Draw(*Renderer);

    // draw face
    //Renderer->DrawSprite(ResourceManager::GetTexture("face"), glm::vec2(200.0f, 200.0f), glm::vec2(300.0f, 400.0f), 45.0f, glm::vec3(0.0f, 1.0f, 0.0f));
}