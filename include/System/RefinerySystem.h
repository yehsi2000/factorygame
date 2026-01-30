#pragma once

#include "Core/SystemContext.h"

class RefineryComponent;

/**
 * @brief Current does nothing. Will be used for handling resource refinement
 *
 */
class RefinerySystem {
 public:
  explicit RefinerySystem(const SystemContext& context);
  ~RefinerySystem();
  void Update();
};
