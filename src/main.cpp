#include <SDL3/SDL.h>
#include <fstream>

#include "Chip8.h"
#include "UpdateTimer.h"
#include "config.h"

SDL_Window* gWindow = NULL;
SDL_Renderer* gRenderer = NULL;
SDL_FRect** gRenderGrid = NULL;

bool InitialiseSDL()
{
	//Initialization flag
	bool success = true;
	srand(time(NULL));

	//Initialize SDL
	if (!SDL_Init(SDL_INIT_VIDEO))
	{
		SDL_Log("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
		success = false;
	}
	else
	{
		//Create window and renderer
		if (!SDL_CreateWindowAndRenderer("Chip8", SCREEN_WIDTH, SCREEN_HEIGHT, 0, &gWindow, &gRenderer))
		{
			SDL_Log("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
			success = false;
		}
		else
		{
			//Initialize renderer color
			SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
		}
	}

	return success;
}

bool LoadRom(Chip8* chip, char* filepath) {
	uint16_t writeIndex = 0x200;
	char buffer[4096];

	std::ifstream fs;

	fs.open(filepath, std::ios_base::binary);

	fs.seekg(0, std::ios::end);
	uint32_t len = fs.tellg();
	fs.seekg(0, std::ios::beg);

	if (!fs.is_open()) {
		return false;
	}

	if (len >= (0xFFF - 0x200)) {
		printf("%s is too big to load into memory: %d bytes", filepath, len);
		return false;
	}

	if (!fs.read(buffer, len)) {
		return false;
	}

	fs.close();

	chip->LoadRom(buffer, len);

	return true;
}

void Close()
{
	SDL_DestroyRenderer(gRenderer);
	SDL_DestroyWindow(gWindow);
	gWindow = NULL;
	gRenderer = NULL;

	SDL_Quit();
}

void InitialiseGrid() {
	gRenderGrid = new SDL_FRect * [GRID_HEIGHT];
	for (size_t i = 0; i < GRID_HEIGHT; i++)
	{
		gRenderGrid[i] = new SDL_FRect[GRID_WIDTH];
	}

	float cell_width = SCREEN_WIDTH / (float)GRID_WIDTH;
	float cell_height = SCREEN_HEIGHT / (float)GRID_HEIGHT;

	for (size_t i = 0; i < GRID_HEIGHT; i++)
	{
		for (size_t j = 0; j < GRID_WIDTH; j++)
		{
			gRenderGrid[i][j] = {
				(float)j * cell_width,
				(float)i * cell_height,
				cell_width,
				cell_height
			};
		}
	}
}

void DestroyGrid() {
	for (size_t i = 0; i < GRID_HEIGHT; i++)
	{
		delete[] gRenderGrid[i];
		gRenderGrid[i] = NULL;
	}
	delete[] gRenderGrid;
	gRenderGrid = NULL;
}

void RenderGrid(Chip8* chip) {
	bool const *const *pg = chip->GetPixelGrid();
	for (size_t i = 0; i < GRID_HEIGHT; i++)
	{
		for (size_t j = 0; j < GRID_WIDTH; j++)
		{
			uint8_t pixelOn = pg[i][j];

			// a bit of noise for a flickering CRT effect
			uint8_t noise = (rand() % 20) * CRT_RENDER_NOISE;

			SDL_SetRenderDrawColor(gRenderer, 
				pixelOn * 235 + noise, 
				pixelOn * 235 + noise,
				pixelOn * 235 + noise,
				0xFF);
			SDL_RenderFillRect(gRenderer, &(gRenderGrid[i][j]));
		}
	}

	chip->ResetDisplayChanged();
	SDL_RenderPresent(gRenderer);
}

void HandleEvents(SDL_Event* e, Chip8* chip, bool* quit) {
	while (SDL_PollEvent(e) != 0)
	{
		if (e->type == SDL_EVENT_QUIT)
		{
			*quit = true;
		}

		/*  Key layout (QWERTY)
			1 2 3 4 -> 1 2 3 C
			q w e r -> 4 5 6 D
			a s d f -> 7 8 9 E
			z x c v -> A 0 B F
		*/
		if (e->type == SDL_EVENT_KEY_DOWN) {
			switch (e->key.key) {
			case SDLK_1:
				chip->SetInput(1, 1);
				break;
			case SDLK_2:
				chip->SetInput(2, 1);
				break;
			case SDLK_3:
				chip->SetInput(3, 1);
				break;
			case SDLK_4:
				chip->SetInput(12, 1);
				break;

			case SDLK_Q:
				chip->SetInput(4, 1);
				break;
			case SDLK_W:
				chip->SetInput(5, 1);
				break;
			case SDLK_E:
				chip->SetInput(6, 1);
				break;
			case SDLK_R:
				chip->SetInput(13, 1);
				break;

			case SDLK_A:
				chip->SetInput(7, 1);
				break;
			case SDLK_S:
				chip->SetInput(8, 1);
				break;
			case SDLK_D:
				chip->SetInput(9, 1);
				break;
			case SDLK_F:
				chip->SetInput(14, 1);
				break;

			case SDLK_Z:
				chip->SetInput(10, 1);
				break;
			case SDLK_X:
				chip->SetInput(0, 1);
				break;
			case SDLK_C:
				chip->SetInput(11, 1);
				break;
			case SDLK_V:
				chip->SetInput(15, 1);
				break;

			default:
				break;
			}
		}
		
		if (e->type == SDL_EVENT_KEY_UP) {
			switch (e->key.key) {
			
			case SDLK_1:
				chip->SetInput(1, 0);
				break;
			case SDLK_2:
				chip->SetInput(2, 0);
				break;
			case SDLK_3:
				chip->SetInput(3, 0);
				break;
			case SDLK_4:
				chip->SetInput(12, 0);
				break;

			case SDLK_Q:
				chip->SetInput(4, 0);
				break;
			case SDLK_W:
				chip->SetInput(5, 0);
				break;
			case SDLK_E:
				chip->SetInput(6, 0);
				break;
			case SDLK_R:
				chip->SetInput(13, 0);
				break;

			case SDLK_A:
				chip->SetInput(7, 0);
				break;
			case SDLK_S:
				chip->SetInput(8, 0);
				break;
			case SDLK_D:
				chip->SetInput(9, 0);
				break;
			case SDLK_F:
				chip->SetInput(14, 0);
				break;

			case SDLK_Z:
				chip->SetInput(10, 0);
				break;
			case SDLK_X:
				chip->SetInput(0, 0);
				break;
			case SDLK_C:
				chip->SetInput(11, 0);
				break;
			case SDLK_V:
				chip->SetInput(15, 0);
				break;

			default:
				break;
			}
		}
	}
}

int main(int argc, char* args[])
{
	if (argc < 2) {
		SDL_Log("Usage: chip8 <rom path>");
		return -1;
	}

	if (!InitialiseSDL())
	{
		SDL_Log("Failed to initialize!\n");
		return -1;
	}

	Chip8 chip(GRID_HEIGHT, GRID_WIDTH);
	InitialiseGrid();

	if (!LoadRom(&chip, args[1])) {
		SDL_Log("Failed to load ROM!\n");
		return -1;
	}

	bool quit = false;
	SDL_Event e;

	UpdateTimer processorClock(PROCESSOR_CLOCK_SPEED);
	UpdateTimer renderClock(FRAMERATE);

	while (!quit)
	{
		HandleEvents(&e, &chip, &quit);

		if (processorClock.Tick()) 
		{
			chip.ProcessorTick();
		}

		if (renderClock.Tick()) 
		{
			chip.DisplayTick();
			RenderGrid(&chip);
		}
	}

	DestroyGrid();
	Close();

	return 0;
}
