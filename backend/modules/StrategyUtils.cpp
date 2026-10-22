#include "StrategyUtils.h"

#include <algorithm>
#include <cmath>

namespace strategy_utils {

double monthlyPayment(const Config& config, BuildingType buildingType, HouseType houseType)
{
    double cost = 0.0;
    int months = 0;

    if (buildingType == BuildingType::Supermarket) {

        cost = config.supermarket.cost;
        months = config.supermarket.constructionMonths;
    } else {

        const HouseConfig& house = getHouseConfig(config, houseType);
        cost = house.cost;
        months = house.constructionMonths;
    }

    return months > 0 ? cost / static_cast<double>(months) : 0.0;
}

double buildingCost( //короче, считает стоимость нового объекта, а не только платеж за месяц
    const Config& config,
    BuildingType buildingType,
    HouseType houseType)
{
    if (buildingType == BuildingType::Supermarket) {
        return config.supermarket.cost;
    }

    return getHouseConfig(config, houseType).cost;
}

double remainingConstructionCost( //учитываем затраты для заверш начатых домов
    const PublicView& view, //чтобы не тратить деньги на новые проекты
    const Config& config)
{
    double total = 0.0;

    for (const BuildingView& building : view.getOwnBuildings()) {
        if (building.isBuilt
            || building.constructionMonths <= building.monthsBuilt) {
            continue;
        }

        const double cost = buildingCost(
            config, building.buildingType, building.houseType);

        const int monthsLeft =
            building.constructionMonths - building.monthsBuilt;

        total += cost * static_cast<double>(monthsLeft)
                 / building.constructionMonths;
    }

    return total;
}

int availableApartments(const PublicView& view, HouseType type)
{
    int total = 0;

    for (const BuildingView& building : view.getOwnBuildings()) {

        if (building.buildingType != BuildingType::House
            || !building.hasHouseType || building.houseType != type) {
            continue;
        }

        int ready = building.apartments;

        if (!building.isBuilt) {

            ready = building.constructionMonths > 0
                ? static_cast<int>(static_cast<long long>(building.apartments)
                                   * building.monthsBuilt / building.constructionMonths) : 0;
        }
        total += std::max(0, ready - building.soldApartments);
    }

    return total;
}

int totalAvailableApartments(const PublicView& view)
{
    return availableApartments(view, HouseType::Panel)
        + availableApartments(view, HouseType::Monolithic)
        + availableApartments(view, HouseType::Brick);
}

int ownBuildingCount(const PublicView& view, BuildingType type)
{
    int count = 0;

    for (const BuildingView& building : view.getOwnBuildings()) {
        if (building.buildingType == type) {
            ++count;
        }
    }

    return count;
}

bool hasBuiltSupermarket(const PublicView& view)
{
    for (const BuildingView& building : view.getOwnBuildings()) {
        if (building.buildingType == BuildingType::Supermarket && building.isBuilt) {
            return true;
        }
    }

    return false;
}

double nextConstructionPayments(const PublicView& view, const Config& config)
{
    double total = 0.0;

    for (const BuildingView& building : view.getOwnBuildings()) {
        if (building.isBuilt || building.constructionMonths <= building.monthsBuilt) {
            continue;
        }

        total += monthlyPayment(config, building.buildingType, building.houseType);
    }

    return total;
}

AdBudgets fitAdBudgets(const PublicView& view, const Config& config,
                       double housingBudget, double supermarketBudget)
{
    housingBudget = std::max(0.0, housingBudget);
    supermarketBudget = std::max(0.0, supermarketBudget);

    if (totalAvailableApartments(view) == 0) {
        housingBudget = 0.0;
    }

    if (!hasBuiltSupermarket(view)
        || (view.getCurrentMonth() > 0 && view.getCurrentMonth() >= config.gameMonths)) {
        supermarketBudget = 0.0;
    }

    const double cashAfterReserve = std::max( //сначала деньги на бюджет, потом на рекламу
        0.0, view.getCash() - remainingConstructionCost(view, config));

    const double requestedCost = (housingBudget + supermarketBudget) * 1000.0;

    if (requestedCost > cashAfterReserve && requestedCost > 0.0) {
        const double scale = cashAfterReserve / requestedCost;
        housingBudget *= scale;
        supermarketBudget *= scale;
    }

    return {housingBudget, supermarketBudget};
}

int affordableNewBuildingCount(
    const PublicView& view,
    const Config& config,
    BuildingType buildingType,
    HouseType houseType,
    const AdBudgets& ads,
    double plannedConstructionCost)
{
    if (view.getCurrentMonth() >= config.gameMonths) {
        return 0;
    }

    const double cost = buildingCost(
        config, buildingType, houseType);

    if (cost <= 0.0 || !std::isfinite(cost)) {
        return 0;
    }

    const double availableCash =
        view.getCash()
        - (ads.housing + ads.supermarket) * 1000.0
        - remainingConstructionCost(view, config)
        - plannedConstructionCost;

    if (availableCash < cost) {
        return 0;
    }

    return static_cast<int>(
        std::floor((availableCash + 1e-8) / cost));
}

double fairPricePerSqm(const Config& config, HouseType type)
{
    const HouseConfig house = getHouseConfig(config, type);
    const double area = static_cast<double>(house.apartments) * house.apartmentArea;

    return area > 0.0 ? house.cost / area * config.fairPriceMultiplier : 0.0;
}

std::vector<HousingOfferDecision> makeHousingOffers(const PublicView& view,
                                                    const Config& config,
                                                    double priceFactor)
{
    std::vector<HousingOfferDecision> offers;
    const std::vector<HouseType> types = {
        HouseType::Panel, HouseType::Monolithic, HouseType::Brick
    };

    for (HouseType type : types) {

        const int apartments = availableApartments(view, type);

        if (apartments <= 0) {
            continue;
        }

        const double factor = std::min(std::max(0.01, priceFactor),
                                       config.maxPriceMultiplier);
        const double price = fairPricePerSqm(config, type) * factor;

        if (price > 0.0) {
            offers.push_back({type, apartments, price});
        }
    }

    return offers;
}
}
