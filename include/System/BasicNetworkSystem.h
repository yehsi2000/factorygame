#ifndef SYSTEM_BASICNETWORKSYSTEM_
#define SYSTEM_BASICNETWORKSYSTEM_

#include "Core/SystemContext.h"

class BasicNetworkSystem {
  /**
   * //TODO : UDP networking prototype
   * simple things, like sending player position to server, 
   * or broadcasting entity, chunk creation to client whatever
   */
  BasicNetworkSystem(const SystemContext& context);
  ~BasicNetworkSystem();
};

#endif /* SYSTEM_BASICNETWORKSYSTEM_ */
