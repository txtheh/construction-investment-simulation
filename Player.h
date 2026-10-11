#pragma once
#include <string>
#include <vector>
#include <memory>

#include "Building.h"
#include "Decision.h"

#include "config.h"

class GameState;

class Player {
public:
    Player(int id, std::string name /*, double startCapital*/);
    virtual ~Player() = default;

    virtual Decision askDecision(const GameState& state) = 0;

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

    void addCapital(double delta);
    void addExpenses(double amount);

private:
    int id_;
    std::string name_;
    double capital_;
    double totalCapital_;
    
    double housingAdBudget_ = 0.0;
    double supermarketAdBudget_ = 0.0;

    std::vector<std::unique_ptr<Building>> buildingQueue_; // ждем реализацию building
                                          // чтобы понять нужны ли указатели
    std::vector<House> builtHouses_;
    std::vector<Supermarket> builtSupermarkets_;
};
