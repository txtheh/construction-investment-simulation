#include "config.h"

#include <algorithm>
#include <cmath>
#include <limits>

HouseConfig getHouseConfig(const Config& config, HouseType type)
{
    switch (type) {
    case HouseType::Panel:
        return config.panelHouse;
    case HouseType::Monolithic:
        return config.monolithicHouse;
    case HouseType::Brick:
        return config.brickHouse;
    }

    return HouseConfig{0.0, 0, 0, 0.0};
}

bool isHouseConfigValid(const HouseConfig& houseConfig)
{
    return std::isfinite(houseConfig.cost) && houseConfig.cost > 0.0
        && houseConfig.constructionMonths > 0
        && houseConfig.apartments > 0
        && std::isfinite(houseConfig.apartmentArea)
        && houseConfig.apartmentArea > 0.0;
}

bool isSupermarketConfigValid(const SupermarketConfig& supermarketConfig)
{
    return std::isfinite(supermarketConfig.cost) && supermarketConfig.cost > 0.0
        && supermarketConfig.constructionMonths > 0;
}

bool isConfigValid(const Config& config)
{
    if (config.playerCount < 2 || config.playerCount > 5
        || config.gameMonths < 6 || config.gameMonths > 24
        || !std::isfinite(config.initialCapital) || config.initialCapital <= 0.0
        || config.startMonth < 1 || config.startMonth > 12)
{
        return false;
    }

    if (!isHouseConfigValid(config.panelHouse)
        || !isHouseConfigValid(config.monolithicHouse)
        || !isHouseConfigValid(config.brickHouse)
        || !isSupermarketConfigValid(config.supermarket))
{
        return false;
    }

    if (!std::isfinite(config.baseHousingDemand) || config.baseHousingDemand < 0.0
        || !std::isfinite(config.baseSupermarketProfit) || config.baseSupermarketProfit < 0.0
        || !std::isfinite(config.supermarketDemandBonus) || config.supermarketDemandBonus < 0.0
        || !std::isfinite(config.houseProfitBonus) || config.houseProfitBonus < 0.0
        || !std::isfinite(config.underConstructionWeight)
        || config.underConstructionWeight < 0.0 || config.underConstructionWeight > 1.0
        || !std::isfinite(config.fairPriceMultiplier) || config.fairPriceMultiplier <= 0.0
        || !std::isfinite(config.maxPriceMultiplier) || config.maxPriceMultiplier <= 0.0
        || !std::isfinite(config.housingAdBonusPer1000) || config.housingAdBonusPer1000 < 0.0
        || !std::isfinite(config.supermarketAdBonusPer500) || config.supermarketAdBonusPer500 < 0.0
        || !std::isfinite(config.supermarketCapitalMultiplier)
        || config.supermarketCapitalMultiplier <= 0.0)
{
        return false;
    }

    if (config.housingSeason.size() != 12
        || config.supermarketSeason.size() != 12)
{
        return false;
    }

    for (int month = 0; month < 12; ++month) {
        if (!std::isfinite(config.housingSeason[month])
            || config.housingSeason[month] < 0.0
            || !std::isfinite(config.supermarketSeason[month])
            || config.supermarketSeason[month] < 0.0) {
            return false;
        }
    }

    return true;
}
