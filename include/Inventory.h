#pragma once
#include <unordered_map>

enum class Item { Ore, Ingot };

class Inventory {
  std::unordered_map<Item, int> counts;

 public:
  bool consume(Item it, int n);
  void add(Item it, int n);
  int get(Item it) const;
};
