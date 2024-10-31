#ifndef INPUT_H
#define INPUT_H

#include <unordered_map>
#include <string>
#include <vector>

#include "glm/glm.hpp"
#include "SDL2/SDL.h"


enum INPUT_STATE { INPUT_STATE_UP, INPUT_STATE_JUST_BECAME_DOWN, INPUT_STATE_DOWN, INPUT_STATE_JUST_BECAME_UP };

class Input
{
public:
	static void Init(); // Call before main loop begins.
	static void ProcessEvent(const SDL_Event& e); // Call every frame at start of event loop.
	static void LateUpdate();

	static bool GetKey(const std::string& keycode);
	static bool GetKeyDown(const std::string& keycode);
	static bool GetKeyUp(const std::string& keycode);

	static glm::vec2 GetMousePosition();

	static bool GetMouseButton(int button);
	static bool GetMouseButtonDown(int button);
	static bool GetMouseButtonUp(int button);
	static float GetMouseScrollDelta();

private:
	static std::unordered_map<SDL_Scancode, INPUT_STATE> keyboardStates;
	static std::vector<SDL_Scancode> justBecameDownScancodes;
	static std::vector<SDL_Scancode> justBecameUpScancodes;

	static glm::vec2 mousePosition;
	static std::unordered_map<uint8_t, INPUT_STATE> mouseButtonStates;
	static std::vector<uint8_t> justBecameDownButtons;
	static std::vector<uint8_t> justBecameUpButtons;

	static float mouseScrollThisFrame;
};

#endif