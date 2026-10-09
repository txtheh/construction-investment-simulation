#pragma once
#include <array>

enum class BuildingType {
    House,
    Supermarket
};

enum class HouseType {
    Panel,
    Monolithic,
    Brick
};

struct HouseConfig {
    double cost;
    int constructionMonths;
    int apartments;
    double apartmentArea;
};

struct SupermarketConfig {
    double cost;
    int constructionMonths;
};
enum class TieRule {
    Proportional
};
struct Config {
    int playerCount;
    int gameMonths;
    double initialCapital = 37000000;
    int startMonth = 3;
    HouseConfig panelHouse{8000000, 7, 60, 50};
    HouseConfig monolithicHouse{10000000, 9, 60, 55};
    HouseConfig brickHouse{12000000, 10, 60, 60};
    SupermarketConfig supermarket{2500000, 5};
    double baseHousingDemand = 25;
    std::array<double, 12> housingSeason = {0.6, 0.6, 0.7, 0.8, 0.9, 1.0,1.1, 1.2, 1.4, 1.5, 1.3, 0.8};
    double baseSupermarketProfit = 80000;
    std::array<double, 12> supermarketSeason = {1.5, 1.3, 1.1, 0.9, 0.8, 0.8,0.8, 0.9, 1.0, 1.1, 1.2, 1.4};
    double supermarketDemandBonus = 0.04;
    double houseProfitBonus = 0.02;
    double underConstructionWeight = 0.5;
    double fairPriceMultiplier = 1.4;
    double maxPriceMultiplier = 1.5;
    double housingAdBonusPer1000 = 0.005;
    double supermarketAdBonusPer500 = 0.03;
    double supermarketCapitalMultiplier = 1.6;
};
