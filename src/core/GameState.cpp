#include "GameState.h"
#include <stdexcept>
#include <utility>

BuildingView GameState::makeBuildingView(const Building& building, bool detailed) const
{
    BuildingView view;
    view.isBuilt = building.isBuilt();
    view.monthsBuilt = building.monthsBuilt();
    view.constructionMonths = building.constructionMonths();
    if (const auto* house = dynamic_cast<const House*>(&building)) {
        view.buildingType = BuildingType::House;
        view.hasHouseType = true;
        view.houseType = house->type();
        if (detailed) {
            view.apartments = house->apartments();
            view.soldApartments = house->sold();
            view.apartmentArea = house->apartmentArea();
        }
    } else if (dynamic_cast<const Supermarket*>(&building)) {
        view.buildingType = BuildingType::Supermarket;
    }
    return view;
}

GameState::GameState(const Config& config) : config_(config) {}

const Config& GameState::getConfig() const
{
    return config_;
}

int GameState::getCurrentMonth() const
{
    return currentMonth_;
}

int GameState::getCalendarMonth() const
{
    if (currentMonth_ == 0) {
        return config_.startMonth;
    }
    return ((config_.startMonth - 1 + currentMonth_ - 1) % 12) + 1;
}

void GameState::setCurrentMonth(int month)
{
    if (month < 0 || month > config_.gameMonths) {
        throw std::out_of_range("Game month is outside the configured game");
    }
    currentMonth_ = month;
}

const std::vector<std::unique_ptr<Player>>& GameState::getPlayers() const
{
    return players_;
}

std::vector<std::unique_ptr<Player>>& GameState::getPlayers()
{
    return players_;
}

void GameState::addPlayer(std::unique_ptr<Player> player)
{
    if (!player) {
        throw std::invalid_argument("Player must not be null");
    }
    for (const auto& existing : players_) {
        if (existing && existing->getId() == player->getId()) {
            throw std::invalid_argument("Player IDs must be unique");
        }
    }
    players_.push_back(std::move(player));
}

const std::vector<SaleRecord>& GameState::getSalesHistory() const
{
    return salesHistory_;
}

const std::vector<DemandRecord>& GameState::getDemandHistory() const
{
    return demandHistory_;
}

void GameState::recordSale(const SaleRecord& sale)
{
    salesHistory_.push_back(sale);
}

void GameState::recordDemand(const DemandRecord& demand)
{
    demandHistory_.push_back(demand);
}

District GameState::getDistrict() const
{
    return District(players_, config_.underConstructionWeight);
}

PublicView GameState::createPublicView(int playerId) const
{
    const Player* selectedPlayer = nullptr;
    for (const auto& player : players_) {
        if (player && player->getId() == playerId) {
            selectedPlayer = player.get();
            break;
        }
    }
    if (!selectedPlayer) {
        throw std::out_of_range("Player ID was not found in the game state");
    }
    PublicView view;
    view.setContext(playerId, currentMonth_, getCalendarMonth(),
                    selectedPlayer->getCapital(),
                    selectedPlayer->getHousingAdBudget(),
                    selectedPlayer->getSupermarketAdBudget());
    view.setHistory(salesHistory_, demandHistory_);
    for (const auto& building : selectedPlayer->getBuildingQueue()) {
        if (building) {
            view.addOwnBuilding(makeBuildingView(*building, true));
        }
    }
    for (const House& house : selectedPlayer->getBuiltHouses()) {
        view.addOwnBuilding(makeBuildingView(house, true));
    }
    for (const Supermarket& supermarket : selectedPlayer->getBuiltSupermarkets()) {
        view.addOwnBuilding(makeBuildingView(supermarket, true));
    }
    for (const auto& player : players_) {
        if (!player || player->getId() == playerId) {
            continue;
        }
        OpponentView opponent;
        opponent.playerId = player->getId();
        opponent.name = player->getName();
        for (const auto& building : player->getBuildingQueue()) {
            if (building) {
                opponent.buildings.push_back(makeBuildingView(*building, false));
            }
        }
        for (const House& house : player->getBuiltHouses()) {
            opponent.buildings.push_back(makeBuildingView(house, false));
        }
        for (const Supermarket& supermarket : player->getBuiltSupermarkets()) {
            opponent.buildings.push_back(makeBuildingView(supermarket, false));
        }
        view.addOpponent(opponent);
    }
    return view;
}
