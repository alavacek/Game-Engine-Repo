#include "Input.h"

std::unordered_map<SDL_Scancode, INPUT_STATE> Input::keyboard_states;
std::vector<SDL_Scancode> Input::just_became_down_scancodes;
std::vector<SDL_Scancode> Input::just_became_up_scancodes;

void Input::Init() // Call before main loop begins.
{

}

void Input::ProcessEvent(const SDL_Event& e) // Call every frame at start of event loop.
{

}

void Input::LateUpdate()
{

}

bool Input::GetKey(SDL_Scancode keycode)
{

}

bool Input::GetKeyDown(SDL_Scancode keycode)
{

}

bool Input::GetKeyUp(SDL_Scancode keycode)
{

}