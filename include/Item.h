#pragma once

#include <map>
#include <unordered_map>
#include <string>
#include <vector>

enum class ItemID {
  None = 0,

  IronOre,
  CopperOre,
  // Stone,
  // Wood,

  IronIngot,
  CopperIngot,

  // ConveyorBelt,
  // Smelter,
  // Assembler
  MaxItemID
};

enum class ItemCategory { Ore, Ingot };

enum class OreType {
  Iron = 0,
  Copper,
  // Coal,
  // Stone,
  MaxOreType
};

struct ItemData {
  ItemID id;
  ItemCategory category;
  std::string name;
  std::string description;
  int maxStackSize = 50;
  // std::string iconPath;
};

class OreToItemMapper {
 public:
  static const OreToItemMapper& instance() {
    static OreToItemMapper mapper;
    return mapper;
  }

  ItemID get(OreType oreType) const {
    auto it = mapping.find(oreType);
    if (it != mapping.end()) {
      return it->second;
    }
    return ItemID::None;
  }
  OreToItemMapper(const OreToItemMapper&) = delete;
  OreToItemMapper& operator=(const OreToItemMapper&) = delete;

 private:
  OreToItemMapper() {
    mapping[OreType::Iron] = ItemID::IronOre;
    mapping[OreType::Copper] = ItemID::CopperOre;
  }
  std::unordered_map<OreType, ItemID> mapping;
};

// 3. 아이템 데이터베이스 클래스
class ItemDatabase {
 public:
  void initialize() {
    db[ItemID::IronOre] = {ItemID::IronOre, ItemCategory::Ore, "철광석",
                           "제련하여 철 주괴로 만들 수 있습니다.", 100};
    db[ItemID::IronIngot] = {
        ItemID::IronIngot, ItemCategory::Ingot, "철주괴",
        "철로 된 주괴. 다른 철강 제품을 만드는데 사용된다.", 100};
    db[ItemID::CopperOre] = {ItemID::CopperOre, ItemCategory::Ore, "구리광석",
                             "제련하여 구리 주괴로 만들 수 있습니다.", 100};
    db[ItemID::CopperIngot] = {
        ItemID::CopperIngot, ItemCategory::Ingot, "구리주괴",
        "구리로 된 주괴. 다른 구리 제품을 만드는데 사용된다.", 100};
  }

  const ItemData& get(ItemID id) const { return db.at(id); }

  bool IsOfCategory(ItemID id, ItemCategory category) const {
    if (db.count(id) == 0) return false;
    return db.at(id).category == category;
  }

 private:
  std::map<ItemID, ItemData> db;
};