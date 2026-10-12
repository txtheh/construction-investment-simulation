#include "buildings.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace {

bool isPositive(double value) {
    return std::isfinite(value) && value > 0.0;
}

bool isNonNegative(double value) {
    return std::isfinite(value) && value >= 0.0;
}

} // namespace

HouseConfig getHouseConfig(const Config& config, HouseType type) {
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

bool isHouseConfigValid(const HouseConfig& houseConfig) {
    return isPositive(houseConfig.cost)
        && houseConfig.constructionMonths > 0
        && houseConfig.apartments > 0
        && isPositive(houseConfig.apartmentArea);
}

bool isSupermarketConfigValid(const SupermarketConfig& supermarketConfig) {
    return isPositive(supermarketConfig.cost)
        && supermarketConfig.constructionMonths > 0;
}

bool isConfigValid(const Config& config) {
    if (config.playerCount < 2 || config.playerCount > 5
        || config.gameMonths < 6 || config.gameMonths > 24
        || !isPositive(config.initialCapital)
        || config.startMonth < 1 || config.startMonth > 12) {
        return false;
    }

    if (!isHouseConfigValid(config.panelHouse)
        || !isHouseConfigValid(config.monolithicHouse)
        || !isHouseConfigValid(config.brickHouse)
        || !isSupermarketConfigValid(config.supermarket)) {
        return false;
    }

    if (!isNonNegative(config.baseHousingDemand)
        || !isNonNegative(config.baseSupermarketProfit)
        || !isNonNegative(config.supermarketDemandBonus)
        || !isNonNegative(config.houseProfitBonus)
        || !isNonNegative(config.underConstructionWeight)
        || config.underConstructionWeight > 1.0
        || !isPositive(config.fairPriceMultiplier)
        || !isPositive(config.maxPriceMultiplier)
        || !isNonNegative(config.housingAdBonusPer1000)
        || !isNonNegative(config.supermarketAdBonusPer500)
        || !isPositive(config.supermarketCapitalMultiplier)) {
        return false;
    }

    for (int month = 0; month < 12; ++month) {
        if (!isNonNegative(config.housingSeason[month])
            || !isNonNegative(config.supermarketSeason[month])) {
            return false;
        }
    }

    return true;
}

Building::Building(double cost, int constructionMonths)
    : costValue(cost),
      constructionMonthsValue(constructionMonths),
      monthsBuiltValue(0),
      spentValue(0.0),
      startOrderValue(0) {
}

double Building::cost() const {
    return costValue;
}

int Building::constructionMonths() const {
    return constructionMonthsValue;
}

int Building::monthsBuilt() const {
    return monthsBuiltValue;
}

double Building::spent() const {
    return spentValue;
}

int Building::startOrder() const {
    return startOrderValue;
}

void Building::setStartOrder(int order) {
    startOrderValue = order;
}

bool Building::isBuilt() const {
    return monthsBuiltValue >= constructionMonthsValue;
}

double Building::monthlyPayment() const {
    if (constructionMonthsValue <= 0) {
        return 0.0;
    }

    return costValue / static_cast<double>(constructionMonthsValue);
}

double Building::nextPayment() const {
    return isBuilt() ? 0.0 : monthlyPayment();
}

void Building::advance() {
    if (isBuilt()) {
        return;
    }

    ++monthsBuiltValue;
    spentValue += monthlyPayment();

    // Ensure the total spent equals the configured cost when construction ends.
    if (isBuilt()) {
        spentValue = costValue;
    }
}

House::House(HouseType type, const HouseConfig& houseConfig)
    : Building(houseConfig.cost, houseConfig.constructionMonths),
      houseType(type),
      apartmentCount(houseConfig.apartments),
      apartmentAreaValue(houseConfig.apartmentArea),
      soldCount(0) {
}

HouseType House::type() const {
    return houseType;
}

int House::apartments() const {
    return apartmentCount;
}

double House::apartmentArea() const {
    return apartmentAreaValue;
}

int House::sold() const {
    return soldCount;
}

double House::costPerSqm() const {
    const double totalArea =
        static_cast<double>(apartmentCount) * apartmentAreaValue;

    if (totalArea <= 0.0) {
        return 0.0;
    }

    return cost() / totalArea;
}

int House::availableToSell() const {
    int readyApartments = apartmentCount;

    if (!isBuilt()) {
        if (constructionMonths() <= 0) {
            return 0;
        }

        const long long ready =
            static_cast<long long>(apartmentCount) * monthsBuilt()
            / constructionMonths();

        readyApartments = static_cast<int>(ready);
    }

    return std::max(0, readyApartments - soldCount);
}

int House::sell(int count) {
    if (count <= 0) {
        return 0;
    }

    const int actuallySold = std::min(count, availableToSell());
    soldCount += actuallySold;
    return actuallySold;
}

double House::constructionValue() const {
    if (isBuilt()) {
        return 0.0;
    }

    const double soldApartmentsCost =
        static_cast<double>(soldCount) * apartmentAreaValue * costPerSqm();

    return std::max(0.0, spent() - soldApartmentsCost);
}

double House::housingValue() const {
    if (!isBuilt()) {
        return 0.0;
    }

    const int unsoldApartments = std::max(0, apartmentCount - soldCount);
    return static_cast<double>(unsoldApartments)
        * apartmentAreaValue * costPerSqm();
}

Supermarket::Supermarket(const SupermarketConfig& supermarketConfig)
    : Building(supermarketConfig.cost, supermarketConfig.constructionMonths) {
}

double Supermarket::constructionValue() const {
    return isBuilt() ? 0.0 : spent();
}

double Supermarket::marketValue(double capitalMultiplier) const {
    return isBuilt() ? spent() * capitalMultiplier : 0.0;
}

bool Portfolio::addHouse(HouseType type, const Config& config) {
    const HouseConfig houseConfig = getHouseConfig(config, type);
    if (!isHouseConfigValid(houseConfig)) {
        return false;
    }

    House house(type, houseConfig);
    house.setStartOrder(nextStartOrder++);
    houses.push_back(house);
    return true;
}

bool Portfolio::addSupermarket(const Config& config) {
    if (!isSupermarketConfigValid(config.supermarket)) {
        return false;
    }

    Supermarket supermarket(config.supermarket);
    supermarket.setStartOrder(nextStartOrder++);
    markets.push_back(supermarket);
    return true;
}

double Portfolio::totalMonthlyPayment() const {
    double total = 0.0;

    for (const House& house : houses) {
        total += house.nextPayment();
    }

    for (const Supermarket& supermarket : markets) {
        total += supermarket.nextPayment();
    }

    return total;
}

bool Portfolio::payOne(Building& building, double& cash) {
    if (building.isBuilt()) {
        return true;
    }

    const double payment = building.nextPayment();
    const double tolerance = 8.0 * std::numeric_limits<double>::epsilon()
        * std::max({1.0, std::abs(cash), std::abs(payment)});

    if (cash + tolerance < payment) {
        return false;
    }

    cash = std::max(0.0, cash - payment);
    building.advance();
    return true;
}

int Portfolio::payConstruction(double& cash) {
    int frozenCount = 0;
    std::size_t houseIndex = 0;
    std::size_t marketIndex = 0;

    // Process houses and supermarkets in the order in which construction began.
    while (houseIndex < houses.size() || marketIndex < markets.size()) {
        bool takeHouse;

        if (houseIndex >= houses.size()) {
            takeHouse = false;
        } else if (marketIndex >= markets.size()) {
            takeHouse = true;
        } else {
            takeHouse = houses[houseIndex].startOrder()
                < markets[marketIndex].startOrder();
        }

        bool paid;
        if (takeHouse) {
            paid = payOne(houses[houseIndex], cash);
            ++houseIndex;
        } else {
            paid = payOne(markets[marketIndex], cash);
            ++marketIndex;
        }

        if (!paid) {
            ++frozenCount;
        }
    }

    return frozenCount;
}

int Portfolio::builtHouses() const {
    int count = 0;

    for (const House& house : houses) {
        if (house.isBuilt()) {
            ++count;
        }
    }

    return count;
}

int Portfolio::buildingHouses() const {
    return static_cast<int>(houses.size()) - builtHouses();
}

int Portfolio::builtMarkets() const {
    int count = 0;

    for (const Supermarket& supermarket : markets) {
        if (supermarket.isBuilt()) {
            ++count;
        }
    }

    return count;
}

int Portfolio::buildingMarkets() const {
    return static_cast<int>(markets.size()) - builtMarkets();
}

int Portfolio::availableApartments(HouseType type) const {
    int count = 0;

    for (const House& house : houses) {
        if (house.type() == type) {
            count += house.availableToSell();
        }
    }

    return count;
}

int Portfolio::sellApartments(HouseType type, int count) {
    if (count <= 0) {
        return 0;
    }

    int soldCount = 0;

    for (House& house : houses) {
        if (count == 0) {
            break;
        }

        if (house.type() == type) {
            const int soldHere = house.sell(count);
            soldCount += soldHere;
            count -= soldHere;
        }
    }

    return soldCount;
}

double Portfolio::constructionValue() const {
    double total = 0.0;

    for (const House& house : houses) {
        total += house.constructionValue();
    }

    for (const Supermarket& supermarket : markets) {
        total += supermarket.constructionValue();
    }

    return total;
}

double Portfolio::housingValue() const {
    double total = 0.0;

    for (const House& house : houses) {
        total += house.housingValue();
    }

    return total;
}

double Portfolio::marketsValue(const Config& config) const {
    double total = 0.0;

    for (const Supermarket& supermarket : markets) {
        total += supermarket.marketValue(config.supermarketCapitalMultiplier);
    }

    return total;
}

double Portfolio::capitalWithoutCash(const Config& config) const {
    return constructionValue() + housingValue() + marketsValue(config);
}
