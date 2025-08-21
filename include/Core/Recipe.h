#ifndef CORE_RECIPE_
#define CORE_RECIPE_

#include <vector>
#include <unordered_map>
#include <string>
#include "Core/Item.h"

enum class RecipeID {
  None = 0,
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
  std::string name;
  std::string description;
  std::vector<RecipeIngredient> ingredients;
  ItemID outputItem;
  int outputAmount;
  float craftingTime; // in seconds
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

  bool exists(RecipeID id) const {
    return recipes.count(id) > 0;
  }

private:
  std::unordered_map<RecipeID, RecipeData> recipes;
  
  RecipeDatabase() {
    // Add some basic recipes for testing
    recipes[RecipeID::IronGear] = {
      RecipeID::IronGear,
      "철 기어",
      "철판 2개로 만드는 기본 기어",
      {{ItemID::IronPlate, 2}},
      ItemID::IronPlate, // For now, output iron plate (we'll add gear item later)
      1,
      2.0f
    };

    recipes[RecipeID::CopperWire] = {
      RecipeID::CopperWire,
      "구리 전선",
      "구리판 1개로 만드는 전선 2개",
      {{ItemID::CopperPlate, 1}},
      ItemID::CopperPlate, // For now, output copper plate (we'll add wire item later)
      2,
      0.5f
    };

    recipes[RecipeID::ElectronicCircuit] = {
      RecipeID::ElectronicCircuit,
      "전자 회로",
      "철판과 구리판으로 만드는 전자 회로",
      {{ItemID::IronPlate, 1}, {ItemID::CopperPlate, 3}},
      ItemID::IronPlate, // For now, output iron plate (we'll add circuit item later)
      1,
      4.0f
    };
  }
};

#endif /* CORE_RECIPE_ */