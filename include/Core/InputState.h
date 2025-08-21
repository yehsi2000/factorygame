#ifndef CORE_INPUTSTATE_
#define CORE_INPUTSTATE_

#include "Core/Type.h"

struct InputState {
  float xAxis = 0.f;
  float yAxis = 0.f;
  Vec2f axis = {0.f,0.f};

  // Mouse input for camera
  bool rightMouseDown = false;
  bool rightMousePressed = false;   // True only on the frame when pressed
  bool rightMouseReleased = false;  // True only on the frame when released
  Vec2 mousepos = {0,0};
  Vec2 mousedelta = {0,0};
  int mouseX = 0;
  int mouseY = 0;
  int mouseDeltaX = 0;
  int mouseDeltaY = 0;
};

#endif /* CORE_INPUTSTATE_ */
