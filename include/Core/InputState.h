#pragma once
#include "Type.h" // For Vec2

enum class MouseButton {
    LEFT,
    RIGHT,
    MIDDLE
};

/**
 * @brief A snapshot of the state of all input devices at a single point in
 * time.
 * @details This struct holds the current state of the mouse buttons, position,
 * and movement, as well as keyboard-derived axis inputs. It is used by the
 * InputManager to track input changes between frames.
 */
struct InputState {
    // Mouse button state
    bool bIsLeftMouseDown = false;
    bool bIsRightMouseDown = false;
    bool bIsLeftMousePressed = false; // True for one frame on press
    bool bIsRightMousePressed = false; // True for one frame on press
    bool bIsLeftMouseReleased = false; // True for one frame on release
    bool bIsRightMouseReleased = false; // True for one frame on release
    bool bIsDraggedFromUI = false;

    Vec2 mousePos = {0, 0};
    Vec2 mouseDelta = {0, 0};
    Vec2 mouseWheel = {0, 0};

    Vec2 axis = {0, 0};

    bool bIsQuit = false;
};
