#include "Game.h"
#include <stdio.h>
#include <SDL_image.h>
#include <bass.h>



Game::Game(int width, int height, int fps)
{
	this->SCREEN_WIDTH = width;
	this->SCREEN_HEIGHT = height;
	this->SCREEN_FPS = fps;
}

Game::Game()
{
	this->SCREEN_WIDTH = 1280;
	this->SCREEN_HEIGHT = 720;
	this->SCREEN_FPS = 60;
}
Game::~Game()
{
}

bool Game::init()
{
	this->gRenderer = NULL;
	this->gWindow = NULL;
	bool success = true;

	msPerFrame = 1000 / SCREEN_FPS;
	prevFrameTime = SDL_GetTicks();

	//Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
		success = false;
	}

	//Set texture filtering to linear
	if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1"))
	{
		printf("Warning: Linear texture filtering not enabled!");
	}

	//Create window
	gWindow = SDL_CreateWindow("Rhythm Game", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
	if (gWindow == NULL)
	{
		printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
		success = false;
	}

	//Create renderer for window
	gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED);
	if (gRenderer == NULL)
	{
		printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
		success = false;
	}

	//Initialize renderer color
	SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);

	//Initialize PNG loading
	int imgFlags = IMG_INIT_PNG;
	if (!(IMG_Init(imgFlags) & imgFlags))
	{
		printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
		success = false;
	}

	//Initialize BASS module
	if (!BASS_Init(-1, 44100, BASS_DEVICE_STEREO, 0, NULL)) {
		printf("BASS initialization failed.\n");
	}

	//Load Game States
	states[enums::STARTING_SCREEN] = std::unique_ptr<GameState>(new StartingScreen(gRenderer));
	states[enums::MAIN_MENU] = std::unique_ptr<GameState>(new MainMenu(gRenderer));
	states[enums::MAIN_GAME] = std::unique_ptr<GameState>(new MainGame(gRenderer,SCREEN_HEIGHT,SCREEN_WIDTH));



	currentState = enums::STARTING_SCREEN;
	return success;
}

void Game::uninit()
{
	//Don't forgget to free loaded images!
	//SDL_DestroyTexture(gTexture);
	//gTexture = NULL;

	//Destroy window	
	SDL_DestroyRenderer(this->gRenderer);
	SDL_DestroyWindow(this->gWindow);
	this->gWindow = NULL;
	this->gRenderer = NULL;

	uninitializeAllStates();

	BASS_Free();

	//Quit SDL subsystems
	IMG_Quit();
	SDL_Quit();
}

void Game::uninitializeAllStates()
{
	for (int i = 0; i < enums::TOTAL_STATES; i++)
	{
		states[i]->uninit();
	}
}

bool Game::newFrame(SDL_Event e)
{
	SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
	SDL_RenderClear(gRenderer);

	//Process and get instructions
	nextInstruction = states[currentState]->process(e, previousInstruction);
	currentState = nextInstruction.nextState;
	previousInstruction = nextInstruction;

	//Update screen
	SDL_RenderPresent(gRenderer);

	currentFrameTime = SDL_GetTicks();
	if (currentFrameTime - prevFrameTime < msPerFrame)
	{
		SDL_Delay(msPerFrame - (currentFrameTime - prevFrameTime));
	}
	return nextInstruction.quit;
}