#pragma once
#include <memory>
#include <string>
#include <vector>
#include "buildings.h"
#include "Decision.h"
#include "config.h"
class PublicView;
class Player {
public:
    Player(int id, std::string name, const Config& config);
    virtual ~Player() = default;
    virtual Decision askDecision(const PublicView& view) = 0;
    void applyDecision(const Decision& decision);
    void advanceConstruction();
    void recalculateTotalCapital();
    int getId() const;
    const std::string& getName() const;
    double getCapital() const;
    double getTotalCapital() const;
    bool isBankrupt() const;
    const std::vector<std::unique_ptr<Building>>& getBuildingQueue() const;
    const std::vector<House>& getBuiltHouses() const;
    const std::vector<Supermarket>& getBuiltSupermarkets() const;
    double getHousingAdBudget() const;
    double getSupermarketAdBudget() const;
    void addCapital(double delta);
    void addExpenses(double amount);
protected:
    const Config& getConfig() const;
private:
    int id_;
    std::string name_;
    Config config_;
    double capital_;
    double totalCapital_;
    double housingAdBudget_ = 0.0;
    double supermarketAdBudget_ = 0.0;
    std::vector<std::unique_ptr<Building>> buildingQueue_;
    std::vector<House> builtHouses_;
    std::vector<Supermarket> builtSupermarkets_;
};
