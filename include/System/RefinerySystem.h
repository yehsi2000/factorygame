#ifndef SYSTEM_REFINERYSYSTEM_
#define SYSTEM_REFINERYSYSTEM_

#include "Core/SystemContext.h"

class RefineryComponent;

/**
 * @brief Current does nothing. Will be used for handling resource refinement
 *
 */
class RefinerySystem {
 public:
  RefinerySystem(const SystemContext& context);
  ~RefinerySystem();
  void Update();
};

#endif /* SYSTEM_REFINERYSYSTEM_ */
