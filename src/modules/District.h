#pragma once

#include <memory>
#include <vector>

#include "../player/Player.h"

class District {
public:
    District(const std::vector<std::unique_ptr<Player>>& players,
             double underConstructionWeight);

    int getBuiltHouseCount() const;
    int getUnderConstructionHouseCount() const;
    int getBuiltSupermarketCount() const;
    int getUnderConstructionSupermarketCount() const;
    double getWeightedHouseCount() const;
    double getWeightedSupermarketCount() const;

private:
    int builtHouseCount_ = 0;
    int underConstructionHouseCount_ = 0;
    int builtSupermarketCount_ = 0;
    int underConstructionSupermarketCount_ = 0;
    double underConstructionWeight_ = 0.5;
};
