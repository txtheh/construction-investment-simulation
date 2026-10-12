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

} 
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
    if (config.playerCount < 2 || config.playerCount > 5) {
        return false;
    }
    if (config.gameMonths < 6 || config.gameMonths > 24) {
        return false;
    }
    if (!isPositive(config.initialCapital)) {
        return false;
    }
    if (config.startMonth < 1 || config.startMonth > 12) {
        return false;
    }

    if (!isHouseConfigValid(config.panelHouse)
        || !isHouseConfigValid(config.monolithicHouse)
        || !isHouseConfigValid(config.brickHouse)
        || !isSupermarketConfigValid(config.supermarket)) {
        return false;
    }

    if (!isNonNegative(config.baseHousingDemand)
        || !isNonNegative(config.baseSupermarketProfit)) {
        return false;
    }

    for (int month = 0; month < 12; ++month) {
        if (!isNonNegative(config.housingSeason[month])
            || !isNonNegative(config.supermarketSeason[month])) {
            return false;
        }
    }

    if (!isNonNegative(config.supermarketDemandBonus)
        || !isNonNegative(config.houseProfitBonus)) {
        return false;
    }

    if (!isNonNegative(config.underConstructionWeight)
        || config.underConstructionWeight > 1.0) {
        return false;
    }

    if (!isPositive(config.fairPriceMultiplier)
        || !isPositive(config.maxPriceMultiplier)) {
        return false;
    }

    if (!isNonNegative(config.housingAdBonusPer1000)
        || !isNonNegative(config.supermarketAdBonusPer500)
        || !isPositive(config.supermarketCapitalMultiplier)) {
        return false;
    }
    return true;
}
Building::Building(double cost, int constructionMonths)
    : cost_(cost), constructionMonths_(constructionMonths) {
}

bool Building::isBuilt() const {
    return monthsBuilt_ >= constructionMonths_;
}

double Building::monthlyPayment() const {
    return cost_ / constructionMonths_;
}

double Building::nextPayment() const {
    return isBuilt() ? 0.0 : monthlyPayment();
}

void Building::advance() {
    if (isBuilt()) {
        return;
    }
    ++monthsBuilt_;
    if (isBuilt()) {
        spent_ = cost_;
    } else {
        spent_ += monthlyPayment();
    }
}

House::House(HouseType type, const HouseConfig& houseConfig)
    : Building(houseConfig.cost, houseConfig.constructionMonths),
      type_(type),
      apartments_(houseConfig.apartments),
      apartmentArea_(houseConfig.apartmentArea) {
}

double House::costPerSqm() const {
    return cost() / (static_cast<double>(apartments_) * apartmentArea_);
}

int House::availableToSell() const {
    int totalAvailable;
    if (isBuilt()) {
        totalAvailable = apartments_;
    } else {
        const long long readyApartments =
            static_cast<long long>(apartments_) * monthsBuilt()
            / constructionMonths();
        totalAvailable = static_cast<int>(readyApartments);
    }

    return std::max(0, totalAvailable - sold_);
}

int House::sell(int count) {
    if (count <= 0) {
        return 0;
    }
    const int actuallySold = std::min(count, availableToSell());
    sold_ += actuallySold;
    return actuallySold;
}

double House::constructionValue() const {
    if (isBuilt()) {
        return 0.0;
    }
    const double soldApartmentsCost =
        static_cast<double>(sold_) * apartmentArea_ * costPerSqm();

    return std::max(0.0, spent() - soldApartmentsCost);
}

double House::housingValue() const {
    if (!isBuilt()) {
        return 0.0;
    }
    const int unsoldApartments = apartments_ - sold_;
    return static_cast<double>(unsoldApartments) * apartmentArea_ * costPerSqm();
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
    house.setStartOrder(nextStartOrder_++);
    houses_.push_back(house);
    return true;
}

bool Portfolio::addSupermarket(const Config& config) {
    if (!isSupermarketConfigValid(config.supermarket)) {
        return false;
    }
    Supermarket supermarket(config.supermarket);
    supermarket.setStartOrder(nextStartOrder_++);
    markets_.push_back(supermarket);
    return true;
}

double Portfolio::totalMonthlyPayment() const {
    double total = 0.0;
    for (const House& house : houses_) {
        total += house.nextPayment();
    }
    for (const Supermarket& supermarket : markets_) {
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
    while (houseIndex < houses_.size() || marketIndex < markets_.size()) {
        bool takeHouse;
        if (houseIndex >= houses_.size()) {
            takeHouse = false;
        } else if (marketIndex >= markets_.size()) {
            takeHouse = true;
        } else {
            takeHouse = houses_[houseIndex].startOrder()
                < markets_[marketIndex].startOrder();
        }
        bool paid;
        if (takeHouse) {
            paid = payOne(houses_[houseIndex], cash);
            ++houseIndex;
        } else {
            paid = payOne(markets_[marketIndex], cash);
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
    for (const House& house : houses_) {
        if (house.isBuilt()) {
            ++count;
        }
    }
    return count;
}

int Portfolio::buildingHouses() const {
    return static_cast<int>(houses_.size()) - builtHouses();
}

int Portfolio::builtMarkets() const {
    int count = 0;
    for (const Supermarket& supermarket : markets_) {
        if (supermarket.isBuilt()) {
            ++count;
        }
    }
    return count;
}

int Portfolio::buildingMarkets() const {
    return static_cast<int>(markets_.size()) - builtMarkets();
}

int Portfolio::availableApartments(HouseType type) const {
    int count = 0;
    for (const House& house : houses_) {
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
    for (House& house : houses_) {
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

    for (const House& house : houses_) {
        total += house.constructionValue();
    }
    for (const Supermarket& supermarket : markets_) {
        total += supermarket.constructionValue();
    }

    return total;
}

double Portfolio::housingValue() const {
    double total = 0.0;
    for (const House& house : houses_) {
        total += house.housingValue();
    }
    return total;
}

double Portfolio::marketsValue(const Config& config) const {
    double total = 0.0;
    for (const Supermarket& supermarket : markets_) {
        total += supermarket.marketValue(config.supermarketCapitalMultiplier);
    }
    return total;
}

double Portfolio::capitalWithoutCash(const Config& config) const {
    return constructionValue() + housingValue() + marketsValue(config);
}

