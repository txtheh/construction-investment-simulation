#pragma once

#include <string>
#include <vector>

#include "../config.h"

// взгляд на дом
struct BuildingView {
    BuildingType buildingType = BuildingType::House;
    bool hasHouseType = false;
    HouseType houseType = HouseType::Panel;
    bool isBuilt = false;
    int monthsBuilt = 0;
    int constructionMonths = 0;
    int apartments = 0;
    int soldApartments = 0;
    double apartmentArea = 0.0;
};

// взгляд на игрока
struct OpponentView {
    int playerId = -1;
    std::string name;
    std::vector<BuildingView> buildings;
};

// взгляд на сделку в истории
struct SaleRecord {
    int month = 0;
    int playerId = -1;
    HouseType houseType = HouseType::Panel;
    int apartmentsSold = 0;
    double pricePerSqm = 0.0;
};

// спрос на квартиры в месяце
struct DemandRecord {
    int month = 0;
    HouseType houseType = HouseType::Panel;
    double demand = 0.0;
};

class PublicView {
public:
    void setContext(int playerId, int currentMonth, int calendarMonth,
                    double cash, double previousHousingAdBudget,
                    double previousSupermarketAdBudget);

    void addOwnBuilding(const BuildingView& building);
    void addOpponent(const OpponentView& opponent);

    void setHistory(const std::vector<SaleRecord>& salesHistory,
                    const std::vector<DemandRecord>& demandHistory);

    // геттеры
    int getPlayerId() const;
    int getCurrentMonth() const;
    int getCalendarMonth() const;

    double getCash() const;
    double getPreviousHousingAdBudget() const;
    double getPreviousSupermarketAdBudget() const;

    const std::vector<BuildingView>& getOwnBuildings() const;
    const std::vector<OpponentView>& getOpponents() const;
    const std::vector<SaleRecord>& getSalesHistory() const;
    const std::vector<DemandRecord>& getDemandHistory() const;

private:
    int playerId_ = -1;
    int currentMonth_ = 0;
    int calendarMonth_ = 3;
    double cash_ = 0.0;
    double previousHousingAdBudget_ = 0.0;
    double previousSupermarketAdBudget_ = 0.0;

    std::vector<BuildingView> ownBuildings_;
    std::vector<OpponentView> opponents_;
    std::vector<SaleRecord> salesHistory_;
    std::vector<DemandRecord> demandHistory_;
};
