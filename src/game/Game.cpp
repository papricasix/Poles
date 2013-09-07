#include "Game.h"
#include <math.h>
#include <Artemis/SystemManager.h>
#include <Artemis/EntityManager.h>
#include <Artemis/Entity.h>
#include "../core/ResourceManager.h"
#include "GameStateManager.h"

/* STATIC VARIABLES */
artemis::World Game::world;
artemis::SystemManager * Game::systemManager = world.getSystemManager();
artemis::EntityManager * Game::entityManager = world.getEntityManager();

RenderingSystem * Game::renderingSystem = NULL;

SDL_Renderer * Game::renderer = NULL;
bool Game::run;
/*------------------*/


Game::Game() {
    run = false;
    
    this->timeLastFrame = 0;
    this->timePerFrame = (Uint32) (((double)1 / (double)FPS_MAX) * 1000);
    this->frameSkip = 0;
    this->fps = 0;
    this->timeLastFPSRecord = SDL_GetTicks();
    
    this->showFPS = false;
    
    /* ARTEMIS */
    this->movementSystem = (MovementSystem *)systemManager->setSystem(new MovementSystem());
    renderingSystem = (RenderingSystem *)systemManager->setSystem(new RenderingSystem());
    
    systemManager->initializeAll();
}

Game::~Game() {
    // Free any SDL resource used
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(this->wnd);
    SDL_Quit();
}

void Game::start() {
    run = true;
    this->initialize();
    this->mainLoop();
}

void Game::exit() {
    run = false;
}

void Game::initialize() {
    int error;
    
    error = SDL_Init(SDL_INIT_EVERYTHING);
    
    if (error != 0) {
        std::cout << SDL_GetError() << std::endl;
    }
    
    SDL_DisplayMode mode;
    error = SDL_GetCurrentDisplayMode(0, &mode);
    
    if (error != 0) {
        std::cout << SDL_GetError() << std::endl;
    }
    
    /* Testing only */
    int w = mode.w / 2; // Remove / 2 for release
    int h = mode.h / 2;
    
    this->wnd = SDL_CreateWindow(GAME_NAME,
                                 SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                 w,h,
                                 SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(wnd, -1, 0);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // Clear Color
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    
    loadResources();
}

void Game::mainLoop() {    
    GameStateManager::setGameState(GAMESTATE_INTRO);
    while (run) {
        world.loopStart();
        world.setDelta(0.0016f);    // Why this value?
        /* ARTEMIS */        
        this->movementSystem->process();
        /*---------*/
        
        this->handleEvents();
        this->update();
        
        if (this->frameSkip == 0) {
            this->render();
            this->manageFPS();
        }
        this->countFSP();
    }
}

/**
 * 
 */
void Game::update() {
    GameStateManager::onLoop();
}

/**
 * 
 */
void Game::render() {
    SDL_RenderClear(renderer);

    renderingSystem->process();
    
    GameStateManager::onRender();

    SDL_RenderPresent(renderer);
}

/**
 * 
 */
void Game::handleEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        GameStateManager::onEvent(&event);
    }
}

void Game::manageFPS() {
    Uint32 currentTime = SDL_GetTicks();    // Total time since the start of the game
    this->timeLastFrame = currentTime;
    
    Uint32 timeElapsed = currentTime - this->timeLastFrame; // Time elapsed since the last frame
    
    if (timeElapsed > this->timePerFrame) {
        // We are running slowly, so we will skip rendering some frames
        this->frameSkip = floor(timeElapsed / this->timePerFrame);
    } else {
        // We are running too fast so we need to make the game wait a bit
        Uint32 timeToWait = this->timePerFrame - timeElapsed;
        SDL_Delay(timeToWait);
    }
}

void Game::countFSP() {
    Uint32 currentTime = SDL_GetTicks();
    Uint32 timeElapsed = currentTime - this->timeLastFPSRecord;
    
    if (timeElapsed >= 1000) {
        // Reset FPS counting
        if (this->showFPS) {
            std::cout << "FPS: " << (int)this->fps << std::endl;
        }
        this->fps = 0;
        this->timeLastFPSRecord = currentTime;
    } else {
        this->fps++;
    }
}

/**
 * Creates a new Game Object to be used in the game.
 * 
 * It is initialized inside the Artemis-Cpp's entity systems and it is ready
 * to attach components as needed. To destroy it use Game::destroyGameObject(GameObject * object).
 * @return 
 */
GameObject * Game::createGameObject() {
    artemis::Entity & objectEntity = world.createEntity();
    
    GameObject * object = new GameObject(objectEntity);
    
    return object;
}

void Game::destroyGameObject(GameObject * object) {
    world.deleteEntity(object->entity);
    
    delete object;
    object = NULL;
}

SDL_Renderer * Game::currentRenderer() {
    return renderer;
}

RenderingSystem * Game::getRenderingSystem() {
    return renderingSystem;
}

/**
 * 
 */
void Game::loadResources() {
    Sprite * sprite;
    unsigned int polesAnim[1] = {2};
    sprite = ResourceManager::loadImage("poles_dude", "images/poles_dude.png", 1, polesAnim);
    sprite->bindAnimation("NONE", 0);
    
    unsigned int testAnim[1] = {1};
    sprite = ResourceManager::loadImage("test_image", "images/test_image.png", 1, testAnim);
    sprite->bindAnimation("NONE", 0);
}
