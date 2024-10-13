#include "Input.h"

std::unordered_map<SDL_Scancode, INPUT_STATE> Input::keyboardStates;
std::vector<SDL_Scancode> Input::justBecameDownScancodes;
std::vector<SDL_Scancode> Input::justBecameUpScancodes;

void Input::Init() // Call before main loop begins.
{

}

void Input::ProcessEvent(const SDL_Event& e) // Call every frame at start of event loop.
{
	if (e.type == SDL_KEYDOWN)
	{
		keyboardStates[e.key.keysym.scancode] = INPUT_STATE_JUST_BECAME_DOWN;
		justBecameDownScancodes.push_back(e.key.keysym.scancode);
	}
	else if (e.type == SDL_KEYUP)
	{
		keyboardStates[e.key.keysym.scancode] = INPUT_STATE_JUST_BECAME_UP;
		justBecameUpScancodes.push_back(e.key.keysym.scancode);
	}
}

void Input::LateUpdate()
{
	for (const SDL_Scancode& code : justBecameDownScancodes)
	{
		keyboardStates[code] = INPUT_STATE_DOWN;
	}
	justBecameDownScancodes.clear();

	for (const SDL_Scancode& code : justBecameUpScancodes)
	{
		keyboardStates[code] = INPUT_STATE_UP;
	}
	justBecameUpScancodes.clear();
}

bool Input::GetKey(SDL_Scancode keycode)
{
	return keyboardStates[keycode] == INPUT_STATE_DOWN || keyboardStates[keycode] == INPUT_STATE_JUST_BECAME_DOWN;
}

bool Input::GetKeyDown(SDL_Scancode keycode)
{
	for (const SDL_Scancode& code : justBecameDownScancodes)
	{
		if (code == keycode)
		{
			return true;
		}
	}

	return false;
}

bool Input::GetKeyUp(SDL_Scancode keycode)
{
	for (const SDL_Scancode& code : justBecameUpScancodes)
	{
		if (code == keycode)
		{
			return true;
		}
	}

	return false;
}