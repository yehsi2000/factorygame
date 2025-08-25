#ifndef CORE_ITEM_
#define CORE_ITEM_

#include "Core/Entity.h"
#include <string>
#include <unordered_map>

enum class ItemID {
  None = 0,

  IronOre,
  CopperOre,
  // Stone,
  // Wood,

  IronPlate,
  CopperPlate,

  MiningDrill,
  AssemblingMachine,
  // ConveyorBelt,
  // Smelter,
  MaxItemID
};

enum class ItemCategory { Ore, Ingot, Buildable, Invalid };

enum class OreType {
  Iron = 0,
  Copper,
  // Coal,
  // Stone,
  MaxOreType
};

struct ItemData {
  ItemCategory category;
  std::u8string name;
  std::u8string description;
  std::string icon;

  int maxStackSize = 50;
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
  OreToItemMapper(OreToItemMapper&&) = delete;
  OreToItemMapper& operator=(OreToItemMapper&&) = delete;

 private:
  OreToItemMapper() {
    mapping[OreType::Iron] = ItemID::IronOre;
    mapping[OreType::Copper] = ItemID::CopperOre;
  }
  std::unordered_map<OreType, ItemID> mapping;
};

class ItemDatabase {
 public:
  static const ItemDatabase& instance() {
    static ItemDatabase db;
    return db;
  }

  ItemDatabase(const ItemDatabase&) = delete;
  ItemDatabase& operator=(const ItemDatabase&) = delete;
  ItemDatabase(ItemDatabase&&) = delete;
  ItemDatabase& operator=(ItemDatabase&&) = delete;

  const ItemData& get(ItemID id) const { return db.at(id); }

  bool IsOfCategory(ItemID id, ItemCategory category) const {
    if (db.count(id) == 0) return false;
    return db.at(id).category == category;
  }

 private:
  std::unordered_map<ItemID, ItemData> db;
  ItemDatabase() {
    db[ItemID::IronOre] = {ItemCategory::Ore, u8"철광석",
                           u8"제련하여 철 주괴로 만들 수 있습니다.",
                           "assets/img/icon/iron-ore.png", 100};

    db[ItemID::IronPlate] = {
        ItemCategory::Ingot, u8"철판",
        u8"철을 판 형태로 가공한 물건. 다른 철강 제품을 만드는데 사용된다.",
        "assets/img/icon/iron-plate.png", 100};

    db[ItemID::CopperOre] = {ItemCategory::Ore, u8"구리광석",
                             u8"제련하여 구리 주괴로 만들 수 있습니다.",
                             "assets/img/icon/copper-ore.png", 100};

    db[ItemID::CopperPlate] = {
        ItemCategory::Ingot, u8"구리판",
        u8"구리를 판 형태로 가공한 물건. 다른 구리 제품을 만드는데 사용된다.",
        "assets/img/icon/copper-plate.png", 100};

    db[ItemID::MiningDrill] = {ItemCategory::Buildable, u8"채굴기",
                               u8"현재 타일 아래의 광물을 채굴합니다.",
                               "assets/img/icon/mining-drill.png", 10};

    db[ItemID::AssemblingMachine] = {ItemCategory::Buildable, u8"조립기",
                                     u8"다양한 아이템을 조립하여 새로운 제품을 만듭니다.",
                                     "assets/img/icon/assembling-machine.png", 5};
  }
};

struct ItemPayload {
  int itemIdx;
  EntityID owner;
  ItemID id;
  int amount;
};

#endif /* CORE_ITEM_ */
