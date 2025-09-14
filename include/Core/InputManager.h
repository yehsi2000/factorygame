#pragma once

#include <vector>
#include "SDL.h"
#include "Core/InputState.h"

struct SDL_Window;
struct ImGuiIO;

/**
 * @brief Manages all user input from keyboard, mouse, and other devices.
 * @details This class abstracts the underlying SDL input events into a clean
 * query-based interface. It tracks the current and previous state of keys and
 * mouse buttons, allowing for the detection of presses, releases, and held
 * states. It also provides access to mouse position, movement delta, and axis
 * inputs.
 */
class InputManager {
public:
    InputManager(SDL_Window* window);
    ~InputManager();

    void PrepareForNewFrame();

    void ProcessEvent(const SDL_Event& event);

    void UpdateContinuousState();

    // Keyboard queries
    bool IsKeyDown(SDL_Scancode key) const;
    bool IsKeyUp(SDL_Scancode key) const;
    bool WasKeyPressedThisFrame(SDL_Scancode key) const;
    bool WasKeyReleasedThisFrame(SDL_Scancode key) const;

    bool IsMouseButtonDown(MouseButton button) const;
    bool IsMouseButtonUp(MouseButton button) const;
    bool WasMouseButtonPressed(MouseButton button) const;
    bool WasMouseButtonReleased(MouseButton button) const;

    Vec2 GetMousePosition() const;
    Vec2 GetMouseDelta() const;
    int GetMouseWheelScroll() const;

    Vec2 GetAxis() const;
    int GetXAxis() const;
    int GetYAxis() const;
    Vec2 GetScreenSize();

    bool IsQuit() const;

private:
    InputState state;
    SDL_Window* window;
    int numKeys;
    const ImGuiIO& io;
    const Uint8* currentKeyState = nullptr;
    std::vector<Uint8> prevKeyState;
    Vec2 screenSize;
};
