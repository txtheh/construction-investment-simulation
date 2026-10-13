#include "PublicView.h"

void PublicView::setContext(int playerId, int currentMonth, int calendarMonth,
                            double cash, double previousHousingAdBudget,
                            double previousSupermarketAdBudget) {
    playerId_ = playerId;
    currentMonth_ = currentMonth;
    calendarMonth_ = calendarMonth;
    cash_ = cash;
    previousHousingAdBudget_ = previousHousingAdBudget;
    previousSupermarketAdBudget_ = previousSupermarketAdBudget;
}

void PublicView::addOwnBuilding(const BuildingView& building) {
    ownBuildings_.push_back(building);
}

void PublicView::addOpponent(const OpponentView& opponent) {
    opponents_.push_back(opponent);
}

void PublicView::setHistory(const std::vector<SaleRecord>& salesHistory,
                            const std::vector<DemandRecord>& demandHistory) {
    salesHistory_ = salesHistory;
    demandHistory_ = demandHistory;
}

int PublicView::getPlayerId() const {
    return playerId_;
}

int PublicView::getCurrentMonth() const {
    return currentMonth_;
}

int PublicView::getCalendarMonth() const {
    return calendarMonth_;
}

double PublicView::getCash() const {
    return cash_;
}

double PublicView::getPreviousHousingAdBudget() const {
    return previousHousingAdBudget_;
}

double PublicView::getPreviousSupermarketAdBudget() const {
    return previousSupermarketAdBudget_;
}

const std::vector<BuildingView>& PublicView::getOwnBuildings() const {
    return ownBuildings_;
}

const std::vector<OpponentView>& PublicView::getOpponents() const {
    return opponents_;
}

const std::vector<SaleRecord>& PublicView::getSalesHistory() const {
    return salesHistory_;
}

const std::vector<DemandRecord>& PublicView::getDemandHistory() const {
    return demandHistory_;
}
