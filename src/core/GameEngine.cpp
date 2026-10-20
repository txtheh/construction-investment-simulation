#include "GameEngine.h"

#include <algorithm>
#include <stdexcept>
#include <utility>

#include "../building/buildings.h"
#include "../interaction/offer.h"

GameEngine::GameEngine(const Config& config)
    : config_(config),
      state_(config),
      realtorAgency_(config)
{
}

void GameEngine::addPlayer(std::unique_ptr<Player> player)
{
    if (started_) {
        throw std::logic_error("Players cannot be added after the game has started");
    }

    state_.addPlayer(std::move(player));
}

void GameEngine::startGame()
{
    if (started_) {
        throw std::logic_error("The game has already started");
    }

    if (!isConfigValid(config_)) {
        throw std::invalid_argument("The game configuration is invalid");
    }

    const auto& players = state_.getPlayers();

    if (static_cast<int>(players.size()) != config_.playerCount) {
        throw std::invalid_argument("The number of players does not match the configuration");
    }

    pendingHousingRevenue_.assign(players.size(), 0.0);
    pendingSupermarketProfit_.assign(players.size(), 0.0);

    // Collect every starting decision before applying any of them.
    std::vector<PublicView> views;

    for (const auto& player : players) {
        views.push_back(state_.createPublicView(player->getId()));
    }

    std::vector<Decision> decisions;

    for (std::size_t i = 0; i < players.size(); ++i) {
        decisions.push_back(players[i]->askDecision(views[i]));
    }

    // The starting phase accepts construction orders only.
    // RulesValidator will be connected here when that module is implemented.
    for (std::size_t i = 0; i < players.size(); ++i) {
        const Decision& decision = decisions[i];
        const auto& orders = decision.getConstructionOrders();

        if (!orders.empty()) {
            Decision startingDecision(
                players[i]->getId(), 0, orders,
                std::vector<HousingOfferDecision>(), 0.0, 0.0
            );
            players[i]->applyDecision(startingDecision);
        } else if (!decision.getTypeDecision().empty()) {
            Decision startingDecision(
                players[i]->getId(), 0,
                decision.getTypeDecision(), 0.0, 0.0
            );
            players[i]->applyDecision(startingDecision);
        } else {
            Decision startingDecision(
                players[i]->getId(), 0,
                std::vector<ConstructionOrder>(),
                std::vector<HousingOfferDecision>(), 0.0, 0.0
            );
            players[i]->applyDecision(startingDecision);
        }
    }

    started_ = true;
    updateTotalCapital();
}

bool GameEngine::advanceMonth()
{
    if (!started_) {
        throw std::logic_error("Start the game before advancing a month");
    }

    if (finished_) {
        return false;
    }

    const int month = state_.getCurrentMonth() + 1;
    state_.setCurrentMonth(month);

    auto& players = state_.getPlayers();

    // 1. Pay construction costs. A building pauses if its owner cannot pay.
    for (const auto& player : players) {
        player->advanceConstruction();
    }

    // 2-3. Credit income earned during the previous month.
    payPendingIncome();

    // Calculate the district before applying this month's decisions.
    // New construction starts progressing next month.
    const District district = state_.getDistrict();
    const std::vector<HousingDemand> demands =
        realtorAgency_.calculateDemands(state_.getCalendarMonth(), district);

    // Every player sees the same state and submits a decision independently.
    std::vector<PublicView> views;

    for (const auto& player : players) {
        views.push_back(state_.createPublicView(player->getId()));
    }

    std::vector<Decision> decisions;

    for (std::size_t i = 0; i < players.size(); ++i) {
        decisions.push_back(players[i]->askDecision(views[i]));
    }

    // TODO: Send decisions through RulesValidator when its interface is ready.
    // Apply them only after every player has submitted a decision.
    for (std::size_t i = 0; i < players.size(); ++i) {
        players[i]->applyDecision(decisions[i]);
    }

    std::vector<Offer> offers;

    for (std::size_t i = 0; i < decisions.size(); ++i) {
        const Decision& decision = decisions[i];
        const double currentAdBudget = decision.getHousingAdBudget();
        const double previousAdBudget = views[i].getPreviousHousingAdBudget();
        const double advertisingBonus = config_.housingAdBonusPer1000
            * (currentAdBudget + previousAdBudget);

        for (const HousingOfferDecision& housingOffer : decision.getHousingOffers()) {
            offers.push_back(Offer(
                static_cast<int>(i),
                housingOffer.houseType,
                housingOffer.apartments,
                housingOffer.pricePerSqm,
                advertisingBonus
            ));
        }
    }

    // 4-7. Distribute this month's housing demand.
    const std::vector<HousingSale> sales =
        realtorAgency_.distributeSales(demands, offers);

    for (const HousingDemand& demand : demands) {
        state_.recordDemand({
            month,
            demand.houseType,
            demand.apartments
        });
    }

    for (const HousingSale& sale : sales) {
        if (sale.playerIndex < 0
            || sale.playerIndex >= static_cast<int>(players.size())) {
            continue;
        }

        const std::size_t playerIndex = static_cast<std::size_t>(sale.playerIndex);
        Player* player = players[playerIndex].get();
        const int soldApartments =
            player->sellApartments(sale.houseType, sale.apartmentsSold);

        if (soldApartments <= 0) {
            continue;
        }

        const HouseConfig house = getHouseConfig(config_, sale.houseType);
        const double revenue = soldApartments
            * house.apartmentArea * sale.pricePerSqm;

        pendingHousingRevenue_[playerIndex] += revenue;

        state_.recordSale({
            month,
            player->getId(),
            sale.houseType,
            soldApartments,
            sale.pricePerSqm
        });
    }

    // Profit earned this month is credited at the start of the next month.
    calculatePendingSupermarketProfit(district);
    updateTotalCapital();

    if (month == config_.gameMonths) {
        // Sales from the final month must still count toward final cash.
        for (std::size_t i = 0; i < players.size(); ++i) {
            players[i]->addCapital(pendingHousingRevenue_[i]);
            pendingHousingRevenue_[i] = 0.0;
        }

        updateTotalCapital();
        finished_ = true;
    }

    return true;
}

void GameEngine::runToEnd()
{
    if (!started_) {
        startGame();
    }

    while (!finished_) {
        advanceMonth();
    }
}

bool GameEngine::isStarted() const
{
    return started_;
}

bool GameEngine::isFinished() const
{
    return finished_;
}

int GameEngine::getCurrentMonth() const
{
    return state_.getCurrentMonth();
}

const GameState& GameEngine::getState() const
{
    return state_;
}

std::vector<PlayerResult> GameEngine::getResults() const
{
    std::vector<PlayerResult> results;

    for (const auto& player : state_.getPlayers()) {
        results.push_back({
            player->getId(),
            player->getName(),
            player->getCapital(),
            player->getTotalCapital()
        });
    }

    return results;
}

std::vector<int> GameEngine::getWinnerIds() const
{
    std::vector<int> winners;

    if (!finished_ || state_.getPlayers().empty()) {
        return winners;
    }

    double highestCapital = state_.getPlayers().front()->getTotalCapital();

    for (const auto& player : state_.getPlayers()) {
        highestCapital = std::max(highestCapital, player->getTotalCapital());
    }

    for (const auto& player : state_.getPlayers()) {
        if (player->getTotalCapital() == highestCapital) {
            winners.push_back(player->getId());
        }
    }

    return winners;
}

void GameEngine::payPendingIncome()
{
    auto& players = state_.getPlayers();

    for (std::size_t i = 0; i < players.size(); ++i) {
        players[i]->addCapital(pendingHousingRevenue_[i]);
        players[i]->addCapital(pendingSupermarketProfit_[i]);

        pendingHousingRevenue_[i] = 0.0;
        pendingSupermarketProfit_[i] = 0.0;
    }
}

void GameEngine::calculatePendingSupermarketProfit(const District& district)
{
    const int calendarMonth = state_.getCalendarMonth();
    const double season = config_.supermarketSeason[
        static_cast<std::size_t>(calendarMonth - 1)
    ];

    const double housingBonus = 1.0
        + config_.houseProfitBonus * district.getWeightedHouseCount();

    const auto& players = state_.getPlayers();

    for (std::size_t i = 0; i < players.size(); ++i) {
        const Player& player = *players[i];
        const double advertisingBonus = config_.supermarketAdBonusPer500
            * (player.getSupermarketAdBudget() / 0.5);

        const double profitPerSupermarket = config_.baseSupermarketProfit
            * season
            * housingBonus
            * (1.0 + advertisingBonus);

        pendingSupermarketProfit_[i] =
            static_cast<double>(player.getBuiltSupermarkets().size())
            * profitPerSupermarket;
    }
}

void GameEngine::updateTotalCapital()
{
    for (const auto& player : state_.getPlayers()) {
        player->recalculateTotalCapital();
    }
}
