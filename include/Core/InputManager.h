#pragma once

#include <vector>
#include "SDL.h"
#include "Core/InputState.h"

struct SDL_Window;
struct ImGuiIO;

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
    
    
    // bool IsUIMouseButtonDown(MouseButton button) const;
    // bool IsUIMouseButtonUp(MouseButton button) const;
    // bool WasUIMouseButtonPressed(MouseButton button) const;
    // bool WasUIMouseButtonReleased(MouseButton button) const;

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
