#pragma once
#include "Type.h" // For Vec2

enum class MouseButton {
    LEFT,
    RIGHT,
    MIDDLE
};

struct InputState {
    // Mouse button state
    bool leftMouseDown = false;
    bool rightMouseDown = false;
    bool leftMousePressed = false; // True for one frame on press
    bool rightMousePressed = false; // True for one frame on press
    bool leftMouseReleased = false; // True for one frame on release
    bool rightMouseReleased = false; // True for one frame on release
    bool isDraggedFromUI = false;

    Vec2 mousePos = {0, 0};
    Vec2 mouseDelta = {0, 0};
    Vec2 mouseWheel = {0, 0};

    Vec2 axis = {0, 0};

    bool isQuit = false;
};
