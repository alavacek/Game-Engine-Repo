#include "Input.h"

std::unordered_map<SDL_Scancode, INPUT_STATE> Input::keyboardStates;
std::vector<SDL_Scancode> Input::justBecameDownScancodes;
std::vector<SDL_Scancode> Input::justBecameUpScancodes;

glm::vec2 Input::mousePosition;
std::unordered_map<uint8_t, INPUT_STATE> Input::mouseButtonStates;
std::vector<uint8_t> Input::justBecameDownButtons;
std::vector<uint8_t> Input::justBecameUpButtons;

float Input::mouseScrollThisFrame = 0;

const std::unordered_map<std::string, SDL_Scancode> keycodeToScancode =
{
	// Directional (arrow) Keys
	{"up", SDL_SCANCODE_UP},
	{"down", SDL_SCANCODE_DOWN},
	{"right", SDL_SCANCODE_RIGHT},
	{"left", SDL_SCANCODE_LEFT},

	// Misc Keys
	{"escape", SDL_SCANCODE_ESCAPE},

	// Modifier Keys
	{"lshift", SDL_SCANCODE_LSHIFT},
	{"rshift", SDL_SCANCODE_RSHIFT},
	{"lctrl", SDL_SCANCODE_LCTRL},
	{"rctrl", SDL_SCANCODE_RCTRL},
	{"lalt", SDL_SCANCODE_LALT},
	{"ralt", SDL_SCANCODE_RALT},

	// Editing Keys
	{"tab", SDL_SCANCODE_TAB},
	{"return", SDL_SCANCODE_RETURN},
	{"enter", SDL_SCANCODE_RETURN},
	{"backspace", SDL_SCANCODE_BACKSPACE},
	{"delete", SDL_SCANCODE_DELETE},
	{"insert", SDL_SCANCODE_INSERT},

	// Character Keys
	{"space", SDL_SCANCODE_SPACE},
	{"a", SDL_SCANCODE_A},
	{"b", SDL_SCANCODE_B},
	{"c", SDL_SCANCODE_C},
	{"d", SDL_SCANCODE_D},
	{"e", SDL_SCANCODE_E},
	{"f", SDL_SCANCODE_F},
	{"g", SDL_SCANCODE_G},
	{"h", SDL_SCANCODE_H},
	{"i", SDL_SCANCODE_I},
	{"j", SDL_SCANCODE_J},
	{"k", SDL_SCANCODE_K},
	{"l", SDL_SCANCODE_L},
	{"m", SDL_SCANCODE_M},
	{"n", SDL_SCANCODE_N},
	{"o", SDL_SCANCODE_O},
	{"p", SDL_SCANCODE_P},
	{"q", SDL_SCANCODE_Q},
	{"r", SDL_SCANCODE_R},
	{"s", SDL_SCANCODE_S},
	{"t", SDL_SCANCODE_T},
	{"u", SDL_SCANCODE_U},
	{"v", SDL_SCANCODE_V},
	{"w", SDL_SCANCODE_W},
	{"x", SDL_SCANCODE_X},
	{"y", SDL_SCANCODE_Y},
	{"z", SDL_SCANCODE_Z},
	{"0", SDL_SCANCODE_0},
	{"1", SDL_SCANCODE_1},
	{"2", SDL_SCANCODE_2},
	{"3", SDL_SCANCODE_3},
	{"4", SDL_SCANCODE_4},
	{"5", SDL_SCANCODE_5},
	{"6", SDL_SCANCODE_6},
	{"7", SDL_SCANCODE_7},
	{"8", SDL_SCANCODE_8},
	{"9", SDL_SCANCODE_9},
	{"/", SDL_SCANCODE_SLASH},
	{";", SDL_SCANCODE_SEMICOLON},
	{"=", SDL_SCANCODE_EQUALS},
	{"-", SDL_SCANCODE_MINUS},
	{".", SDL_SCANCODE_PERIOD},
	{",", SDL_SCANCODE_COMMA},
	{"[", SDL_SCANCODE_LEFTBRACKET},
	{"]", SDL_SCANCODE_RIGHTBRACKET},
	{"\\", SDL_SCANCODE_BACKSLASH},
	{"'", SDL_SCANCODE_APOSTROPHE}
};

void Input::Init() // Call before main loop begins.
{

}

void Input::ProcessEvent(const SDL_Event& e) // Call every frame at start of event loop.
{
	// Keys
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

	// Mouse movement
	if (e.type == SDL_MOUSEMOTION)
	{
		mousePosition = glm::vec2(e.motion.x, e.motion.y);
	}

	// Mouse buttons
	if (e.type == SDL_MOUSEBUTTONDOWN)
	{
		mouseButtonStates[e.button.button] = INPUT_STATE_JUST_BECAME_DOWN;
		justBecameDownButtons.push_back(e.button.button);
	}
	else if (e.type == SDL_MOUSEBUTTONUP)
	{
		mouseButtonStates[e.button.button] = INPUT_STATE_JUST_BECAME_UP;
		justBecameUpButtons.push_back(e.button.button);
	}

	// Mouse Scroll
	if (e.type == SDL_MOUSEWHEEL)
	{
		mouseScrollThisFrame = e.wheel.preciseY;
	}
	else
	{
		mouseScrollThisFrame = 0;
	}
}

void Input::LateUpdate()
{
	// Keycodes
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

	// Buttons
	for (uint8_t buttons : justBecameDownButtons)
	{
		mouseButtonStates[buttons] = INPUT_STATE_DOWN;
	}
	justBecameDownButtons.clear();

	for (uint8_t buttons : justBecameUpButtons)
	{
		mouseButtonStates[buttons] = INPUT_STATE_UP;
	}
	justBecameUpButtons.clear();

}

bool Input::GetKey(const std::string& keycode)
{
	auto scancodeToKeycode = keycodeToScancode.find(keycode);
	if (scancodeToKeycode != keycodeToScancode.end())
	{
		SDL_Scancode scancode = scancodeToKeycode->second;
		return keyboardStates[scancode] == INPUT_STATE_DOWN || keyboardStates[scancode] == INPUT_STATE_JUST_BECAME_DOWN;
	}
	else
	{
		// KeycodeKey does not map to a Scancode
		return false;
	}
}

bool Input::GetKeyDown(const std::string& keycode)
{
	auto scancodeToKeycode = keycodeToScancode.find(keycode);
	if (scancodeToKeycode != keycodeToScancode.end())
	{
		SDL_Scancode scancode = scancodeToKeycode->second;

		return (keyboardStates.find(scancode) != keyboardStates.end()) && (keyboardStates[scancode] == INPUT_STATE_JUST_BECAME_DOWN);
	}
	else
	{
		// KeycodeKey does not map to a Scancode
		return false;
	}
}

bool Input::GetKeyUp(const std::string& keycode)
{
	auto scancodeToKeycode = keycodeToScancode.find(keycode);
	if (scancodeToKeycode != keycodeToScancode.end())
	{
		SDL_Scancode scancode = scancodeToKeycode->second;
		for (const SDL_Scancode& code : justBecameUpScancodes)
		{
			if (code == scancode)
			{
 				return true;
			}
		}

		return false;;
	}
	else
	{
		// KeycodeKey does not map to a Scancode
		return false;
	}

}

glm::vec2 Input::GetMousePosition()
{
	return mousePosition;
}

bool Input::GetMouseButton(int button)
{
	// TODO FIX
	return mouseButtonStates[button] == INPUT_STATE_DOWN || mouseButtonStates[button] == INPUT_STATE_JUST_BECAME_DOWN;
}

bool Input::GetMouseButtonDown(int button)
{
	return (mouseButtonStates.find(button) != mouseButtonStates.end()) &&  (mouseButtonStates[button] == INPUT_STATE_JUST_BECAME_DOWN);
}

bool Input::GetMouseButtonUp(int button)
{
	for (int upButton : justBecameUpButtons)
	{
		if (upButton == button)
		{
			return true;
		}
	}
	
	return false;
}

float Input::GetMouseScrollDelta()
{
	return mouseScrollThisFrame;
}