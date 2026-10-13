#include "StrategyUtils.h"
#include <algorithm>
#include <array>
#include <cmath>
namespace strategy_utils {
const HouseConfig& houseConfig(const Config& config, HouseType type) {
    switch (type) {
    case HouseType::Panel:
        return config.panelHouse;
    case HouseType::Monolithic:
        return config.monolithicHouse;
    case HouseType::Brick:
        return config.brickHouse;
    }
    return config.panelHouse;
}
double monthlyPayment(const Config& config, BuildingType buildingType, HouseType houseType) {
    double cost = 0.0;
    int months = 0;
    if (buildingType == BuildingType::Supermarket) {
        cost = config.supermarket.cost;
        months = config.supermarket.constructionMonths;
    } else {
        const HouseConfig& house = houseConfig(config, houseType);
        cost = house.cost;
        months = house.constructionMonths;
    }
    return months > 0 ? cost / static_cast<double>(months) : 0.0;
}
int availableApartments(const PublicView& view, HouseType type) {
    int total = 0;
    for (const BuildingView& building : view.getOwnBuildings()) {
        if (building.buildingType != BuildingType::House || !building.hasHouseType || building.houseType != type) {
            continue;
        }
        int ready = building.apartments;
        if (!building.isBuilt) {
            ready = building.constructionMonths > 0
                ? static_cast<int>(static_cast<long long>(building.apartments) * building.monthsBuilt / building.constructionMonths)
                : 0;
        }
        total += std::max(0, ready - building.soldApartments);
    }
    return total;
}
int totalAvailableApartments(const PublicView& view) {
    return availableApartments(view, HouseType::Panel)
        + availableApartments(view, HouseType::Monolithic)
        + availableApartments(view, HouseType::Brick);
}
int ownBuildingCount(const PublicView& view, BuildingType type) {
    int count = 0;
    for (const BuildingView& building : view.getOwnBuildings()) {
        if (building.buildingType == type) {
            ++count;
        }
    }
    return count;
}
bool hasBuiltSupermarket(const PublicView& view) {
    for (const BuildingView& building : view.getOwnBuildings()) {
        if (building.buildingType == BuildingType::Supermarket && building.isBuilt) {
            return true;
        }
    }
    return false;
}
double nextConstructionPayments(const PublicView& view, const Config& config) {
    double total = 0.0;
    for (const BuildingView& building : view.getOwnBuildings()) {
        if (building.isBuilt || building.constructionMonths <= building.monthsBuilt) {
            continue;
        }
        total += monthlyPayment(config, building.buildingType, building.houseType);
    }
    return total;
}
AdBudgets fitAdBudgets(const PublicView& view, const Config& config, double housingBudget, double supermarketBudget) {
    housingBudget = std::max(0.0, housingBudget);
    supermarketBudget = std::max(0.0, supermarketBudget);
    if (totalAvailableApartments(view) == 0) {
        housingBudget = 0.0;
    }
    if (!hasBuiltSupermarket(view) || (view.getCurrentMonth() > 0 && view.getCurrentMonth() >= config.gameMonths)) {
        supermarketBudget = 0.0;
    }
    const double cashAfterReserve = std::max(0.0, view.getCash() - nextConstructionPayments(view, config));
    const double requestedCost = (housingBudget + supermarketBudget) * 1000.0;
    if (requestedCost > cashAfterReserve && requestedCost > 0.0) {
        const double scale = cashAfterReserve / requestedCost;
        housingBudget *= scale;
        supermarketBudget *= scale;
    }
    return {housingBudget, supermarketBudget};
}
int affordableNewBuildingCount(const PublicView& view, const Config& config, BuildingType buildingType, HouseType houseType, const AdBudgets& ads, double plannedMonthlyPayments) {
    if (view.getCurrentMonth() > 0 && view.getCurrentMonth() >= config.gameMonths) {
        return 0;
    }
    const double payment = monthlyPayment(config, buildingType, houseType);
    if (payment <= 0.0) {
        return 0;
    }
    const double availableCash = view.getCash()
        - (ads.housing + ads.supermarket) * 1000.0
        - nextConstructionPayments(view, config)
        - plannedMonthlyPayments;
    if (availableCash <= 0.0) {
        return 0;
    }
    return static_cast<int>(std::floor((availableCash + 1e-8) / payment));
}
double fairPricePerSqm(const Config& config, HouseType type) {
    const HouseConfig& house = houseConfig(config, type);
    const double area = static_cast<double>(house.apartments) * house.apartmentArea;
    return area > 0.0 ? house.cost / area * config.fairPriceMultiplier : 0.0;
}
std::vector<HousingOfferDecision> makeHousingOffers(const PublicView& view, const Config& config, double priceFactor) {
    std::vector<HousingOfferDecision> offers;
    const std::array<HouseType, 3> types = {HouseType::Panel, HouseType::Monolithic, HouseType::Brick};
    for (HouseType type : types) {
        const int apartments = availableApartments(view, type);
        if (apartments <= 0) {
            continue;
        }
        const double factor = std::min(std::max(0.01, priceFactor), config.maxPriceMultiplier);
        const double price = fairPricePerSqm(config, type) * factor;
        if (price > 0.0) {
            offers.push_back({type, apartments, price});
        }
    }
    return offers;
}
}
