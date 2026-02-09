#pragma once
#include "DataStruct/Type.h"  // For Vec2

enum class MouseButton { LEFT, RIGHT, MIDDLE };

/**
 * @brief A snapshot of the state of all input devices at a single point in
 * time.
 * @details This struct holds the current state of the mouse buttons, position,
 * and movement, as well as keyboard-derived axis inputs. It is used by the
 * InputManager to track input changes between frames.
 */
struct InputState {
  // Mouse button state
  bool isLeftMouseDown = false;
  bool isRightMouseDown = false;
  bool isLeftMousePressed = false;    // True for one frame on press
  bool isRightMousePressed = false;   // True for one frame on press
  bool isLeftMouseReleased = false;   // True for one frame on release
  bool isRightMouseReleased = false;  // True for one frame on release
  bool isDraggedFromUI = false;

  Vec2 mousePos = {0, 0};
  Vec2 mouseDelta = {0, 0};
  Vec2 mouseWheel = {0, 0};

  Vec2 axis = {0, 0};

  bool isQuit = false;
};
