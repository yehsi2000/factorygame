#ifndef CORE_INPUTPOLLER_
#define CORE_INPUTPOLLER_

#include <vector>

#include "Core/Type.h"
#include "SDL.h"
#include "imgui.h"

class EventDispatcher;
struct SDL_Window;

class InputPoller {

  ImGuiIO& io;
  SDL_Window* window;
  Vec2 screenSize;
  
  int numKeys;
  const Uint8* keyState;
  std::vector<Uint8> prevKeyState;

  struct {
    Vec2f axis;
    Vec2 mousePos;
    Vec2 mouseDelta;
    Vec2 mouseWheel;
    bool leftMouseDown;
    bool leftMousePressed;
    bool leftMouseReleased;
    bool rightMouseDown;
    bool rightMousePressed;
    bool rightMouseReleased;
    bool isDraggingOutside;
    bool isQuit;
  } state;

 public:
  enum class Mouse { LEFT, RIGHT };
  InputPoller(SDL_Window* window);
  ~InputPoller();
  void PollEvents();

  bool IsKeyDown(SDL_Scancode key) const;
  bool IsKeyUp(SDL_Scancode key) const;
  bool WasKeyPressedThisFrame(SDL_Scancode key) const;
  bool WasKeyReleasedThisFrame(SDL_Scancode key) const;
  bool IsMouseButtonDown(Mouse button) const;
  bool IsMouseButtonUp(Mouse button) const;
  bool WasMouseButtonPressed(Mouse button) const;
  bool WasMouseButtonReleased(Mouse button) const;
  bool IsDraggingOutSide() const { return state.isDraggingOutside; }
  bool IsQuit() const { return state.isQuit; }

  int GetScrollAmount();
  Vec2 GetMousePositon() const;
  Vec2 GetScreenSize() const { return screenSize; }
  float GetXAxis() const { return state.axis.x; }
  float GetYAxis() const { return state.axis.y; }

};

#endif/* CORE_INPUTPOLLER_ */
