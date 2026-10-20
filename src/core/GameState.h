#pragma once
#include <memory>
#include <vector>

#include "../modules/District.h"
#include "../player/Player.h"
#include "../view/PublicView.h"
#include "../config.h"

class GameState {
public:
    explicit GameState(const Config& config);
    const Config& getConfig() const;
    int getCurrentMonth() const;
    int getCalendarMonth() const;
    void setCurrentMonth(int month);
    const std::vector<std::unique_ptr<Player>>& getPlayers() const;
    std::vector<std::unique_ptr<Player>>& getPlayers();
    void addPlayer(std::unique_ptr<Player> player);
    const std::vector<SaleRecord>& getSalesHistory() const;
    const std::vector<DemandRecord>& getDemandHistory() const;
    void recordSale(const SaleRecord& sale);
    void recordDemand(const DemandRecord& demand);
    District getDistrict() const;
    PublicView createPublicView(int playerId) const;

private:
    BuildingView makeBuildingView(const Building& building, bool detailed) const;
    Config config_;
    int currentMonth_ = 0;
    std::vector<std::unique_ptr<Player>> players_;
    std::vector<SaleRecord> salesHistory_;
    std::vector<DemandRecord> demandHistory_;
};
