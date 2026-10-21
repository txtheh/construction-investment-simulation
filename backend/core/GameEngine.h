#pragma once

#include <memory>
#include <string>
#include <vector>

#include "../config.h"
#include "GameState.h"
#include "../player/Player.h"
#include "../modules/RealtorAgency.h"
#include "../modules/RulesValidator.h"

struct PlayerResult {
    int playerId;
    std::string playerName;
    double cash;
    double totalCapital;
};

class GameEngine {
public:
    explicit GameEngine(const Config& config);

    void addPlayer(std::unique_ptr<Player> player);
    void startGame();

    bool advanceMonth();
    void runToEnd();

    bool isStarted() const;
    bool isFinished() const;
    int getCurrentMonth() const;

    const GameState& getState() const;
    std::vector<PlayerResult> getResults() const;
    std::vector<int> getWinnerIds() const;

private:
    void payPendingIncome();
    void calculatePendingSupermarketProfit(const District& district);
    void updateTotalCapital();

    Config config_;
    GameState state_;
    RealtorAgency realtorAgency_;

    std::vector<double> pendingHousingRevenue_;
    std::vector<double> pendingSupermarketProfit_;

    bool started_ = false;
    bool finished_ = false;
};
