#ifndef CORE_RECIPE_
#define CORE_RECIPE_

#include <string>
#include <unordered_map>
#include <vector>

#include "Core/Item.h"

enum class RecipeID {
  None = 0,
  IronPlate,
  CopperPlate,
  IronGear,
  CopperWire,
  ElectronicCircuit,
  MaxRecipeID
};

struct RecipeIngredient {
  ItemID itemId;
  int amount;
};

struct RecipeData {
  RecipeID id;
  std::u8string name;
  std::u8string description;
  std::vector<RecipeIngredient> ingredients;
  ItemID outputItem;
  int outputAmount;
  float craftingTime;  // in seconds
};

class RecipeDatabase {
 public:
  static const RecipeDatabase& instance() {
    static RecipeDatabase db;
    return db;
  }

  RecipeDatabase(const RecipeDatabase&) = delete;
  RecipeDatabase& operator=(const RecipeDatabase&) = delete;
  RecipeDatabase(RecipeDatabase&&) = delete;
  RecipeDatabase& operator=(RecipeDatabase&&) = delete;

  const RecipeData& get(RecipeID id) const { return recipes.at(id); }

  std::vector<RecipeID> getAllRecipes() const {
    std::vector<RecipeID> result;
    for (const auto& [id, data] : recipes) {
      if (id != RecipeID::None) {
        result.push_back(id);
      }
    }
    return result;
  }

  bool exists(RecipeID id) const { return recipes.count(id) > 0; }

 private:
  std::unordered_map<RecipeID, RecipeData> recipes;

  RecipeDatabase() {
    recipes[RecipeID::IronPlate] = {RecipeID::IronPlate,
                                    u8"철판",
                                    u8"철광석 2개로 만드는 철판",
                                    {{ItemID::IronOre, 2}},
                                    ItemID::IronPlate,
                                    1,
                                    1.0f};

    recipes[RecipeID::CopperPlate] = {RecipeID::CopperPlate,
                                      u8"구리판",
                                      u8"구리광석 2개로 만드는 철판",
                                      {{ItemID::CopperOre, 2}},
                                      ItemID::CopperPlate,
                                      1,
                                      1.0f};

    recipes[RecipeID::IronGear] = {RecipeID::IronGear,
                                   u8"철 기어",
                                   u8"철판 2개로 만드는 기본 기어",
                                   {{ItemID::IronPlate, 2}},
                                   ItemID::IronPlate,
                                   1,
                                   2.0f};

    recipes[RecipeID::CopperWire] = {RecipeID::CopperWire,
                                     u8"구리 전선",
                                     u8"구리판 1개로 만드는 전선 2개",
                                     {{ItemID::CopperPlate, 1}},
                                     ItemID::CopperPlate,
                                     2,
                                     0.5f};

    recipes[RecipeID::ElectronicCircuit] = {
        RecipeID::ElectronicCircuit,
        u8"전자 회로",
        u8"철판과 구리판으로 만드는 전자 회로",
        {{ItemID::IronPlate, 1}, {ItemID::CopperPlate, 3}},
        ItemID::IronPlate,
        1,
        4.0f};
  }
};

#endif /* CORE_RECIPE_ */