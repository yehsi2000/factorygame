/**
 * @brief Defines all possible item types in the game.
 */
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

/**
 * @brief Categorizes items for easier filtering and management.
 */
enum class ItemCategory { Ore, Ingot, Buildable, Invalid };

/**
 * @brief Defines the types of ores available for mining.
 */
enum class OreType {
  Iron = 0,
  Copper,
  // Coal,
  // Stone,
  MaxOreType
};