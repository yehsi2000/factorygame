#include "Inventory.h"

bool Inventory::consume(Item it, int n) {
  if (counts.find(it) != counts.end() && counts[it] >= n) {
    counts[it] -= n;
    if (counts[it] == 0) counts.erase(it);
    return true;
  }
  return false;
}

void Inventory::add(Item it, int n) {
  counts[it] += n;
}

int Inventory::get(Item it) const {
  auto pos = counts.find(it);
  return (pos != counts.end()) ? pos->second : 0;
}
