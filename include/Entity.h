// Entity.h
#pragma once
#include <vector>
#include <memory>
#include <stdexcept>
#include "Component.h"

using EntityID = unsigned long long;

class Entity {
  EntityID id;
  std::vector<std::unique_ptr<Component>> Components;

public:
  Entity();
  
  template <class T, class... Args>
  T* AddComponent(Args&&... args) {
    auto ptr = std::make_unique<T>(std::forward<Args>(args)...);
    T* raw = ptr.get();
    Components.push_back(std::move(ptr));
    return raw;
  }

  void update() {
    for (auto& c : Components) c->Update();
  }

  EntityID getID() const { return id; }
};
