#include "Entity.h"

EntityID nextID = 1;

Entity::Entity() {
  if (nextID <= (1 << 30)) {
    id = nextID++;
  } else {
    throw std::out_of_range("사용가능한 index가 없습니다");
  }
}