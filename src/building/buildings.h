#pragma once
#include <vector>
#include "../config.h"

HouseConfig getHouseConfig(const Config& config, HouseType type);
bool isHouseConfigValid(const HouseConfig& houseConfig);
bool isSupermarketConfigValid(const SupermarketConfig& supermarketConfig);
bool isConfigValid(const Config& config);

class Building {
public:
    Building(double cost, int constructionMonths);
    virtual ~Building() = default;
    double cost() const;
    int constructionMonths() const;
    int monthsBuilt() const;
    double spent() const;
    int startOrder() const;
    void setStartOrder(int order);
    bool isBuilt() const;
    double monthlyPayment() const;
    double nextPayment() const;
    void advance();
private:
    double costValue;
    int constructionMonthsValue;
    int monthsBuiltValue;
    double spentValue;
    int startOrderValue;
};

class House : public Building {
public:
    House(HouseType type, const HouseConfig& houseConfig);
    HouseType type() const;
    int apartments() const;
    double apartmentArea() const;
    int sold() const;
    double costPerSqm() const;
    int availableToSell() const;
    int sell(int count);
    double constructionValue() const;
    double housingValue() const;
private:
    HouseType houseType;
    int apartmentCount;
    double apartmentAreaValue;
    int soldCount;
};

class Supermarket : public Building {
public:
    explicit Supermarket(const SupermarketConfig& supermarketConfig);
    double constructionValue() const;
    double marketValue(double capitalMultiplier) const;
};

class Portfolio {
public:
    std::vector<House> houses;
    std::vector<Supermarket> markets;
    bool addHouse(HouseType type, const Config& config);
    bool addSupermarket(const Config& config);
    double totalMonthlyPayment() const;
    int payConstruction(double& cash);
    int builtHouses() const;
    int buildingHouses() const;
    int builtMarkets() const;
    int buildingMarkets() const;
    int availableApartments(HouseType type) const;
    int sellApartments(HouseType type, int count);
    double constructionValue() const;
    double housingValue() const;
    double marketsValue(const Config& config) const;
    double capitalWithoutCash(const Config& config) const;
private:
    int nextStartOrder = 0;
    bool payOne(Building& building, double& cash);
};

