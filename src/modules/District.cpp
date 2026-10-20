#include "District.h"
#include "../building/buildings.h"

District::District(const std::vector<std::unique_ptr<Player>>& players,
                   double underConstructionWeight)
    : underConstructionWeight_(underConstructionWeight)
{
    for (const auto& player : players) {
        if (!player) {
            continue;
        }
        builtHouseCount_ += static_cast<int>(player->getBuiltHouses().size());
        builtSupermarketCount_ += static_cast<int>(player->getBuiltSupermarkets().size());
        for (const auto& building : player->getBuildingQueue()) {
            if (!building || building->isBuilt()) {
                continue;
            }
            if (dynamic_cast<const House*>(building.get())) {
                ++underConstructionHouseCount_;
            } else if (dynamic_cast<const Supermarket*>(building.get())) {
                ++underConstructionSupermarketCount_;
            }
        }
    }
}

int District::getBuiltHouseCount() const
{
    return builtHouseCount_;
}

int District::getUnderConstructionHouseCount() const
{
    return underConstructionHouseCount_;
}

int District::getBuiltSupermarketCount() const
{
    return builtSupermarketCount_;
}

int District::getUnderConstructionSupermarketCount() const
{
    return underConstructionSupermarketCount_;
}

double District::getWeightedHouseCount() const
{
    return builtHouseCount_ + underConstructionWeight_ * underConstructionHouseCount_;
}

double District::getWeightedSupermarketCount() const
{
    return builtSupermarketCount_
        + underConstructionWeight_ * underConstructionSupermarketCount_;
}
