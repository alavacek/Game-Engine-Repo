#include <iostream>
#include <string>

#include "Engine.h"
#include "EngineUtils.h"
#include "Renderer.h"
#include "SDL2/SDL.h"
#include "SDLHelper.h"


int main(int argc, char* argv[])
{
	Engine engine;
	engine.GameLoop();

	return 0;
}