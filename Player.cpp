#include "Player.h"
#include <algorithm>
#include <cmath>
#include <limits>
Player::Player(int id, std::string name, const Config& config)
    : id_(id), name_(name), config_(config),
      capital_(config.initialCapital), totalCapital_(config.initialCapital) {}
int Player::getId() const {
    return id_;
}
const std::string& Player::getName() const {
    return name_;
}
double Player::getCapital() const {
    return capital_;
}
double Player::getTotalCapital() const {
    return totalCapital_;
}
bool Player::isBankrupt() const {
    return capital_ < 0.0;
}
const std::vector<std::unique_ptr<Building>>& Player::getBuildingQueue() const {
    return buildingQueue_;
}
const std::vector<House>& Player::getBuiltHouses() const {
    return builtHouses_;
}
const std::vector<Supermarket>& Player::getBuiltSupermarkets() const {
    return builtSupermarkets_;
}
double Player::getHousingAdBudget() const {
    return housingAdBudget_;
}
double Player::getSupermarketAdBudget() const {
    return supermarketAdBudget_;
}
const Config& Player::getConfig() const {
    return config_;
}
void Player::addCapital(double delta) {
    capital_ += delta;
}
void Player::addExpenses(double amount) {
    capital_ -= amount;
}
void Player::applyDecision(const Decision& decision) {
    const double housingAdBudget = decision.getHousingAdBudget();
    const double supermarketAdBudget = decision.getSupermarketAdBudget();
    addExpenses(housingAdBudget * 1000.0);
    addExpenses(supermarketAdBudget * 1000.0);
    const std::string& type = decision.getTypeDecision();
    if (type == "panel") {
        buildingQueue_.push_back(std::make_unique<House>(
            HouseType::Panel, getHouseConfig(config_, HouseType::Panel)));
    } else if (type == "monolith") {
        buildingQueue_.push_back(std::make_unique<House>(
            HouseType::Monolithic, getHouseConfig(config_, HouseType::Monolithic)));
    } else if (type == "brick") {
        buildingQueue_.push_back(std::make_unique<House>(
            HouseType::Brick, getHouseConfig(config_, HouseType::Brick)));
    } else if (type == "supermarket") {
        buildingQueue_.push_back(std::make_unique<Supermarket>(config_.supermarket));
    }
    housingAdBudget_ = housingAdBudget;
    supermarketAdBudget_ = supermarketAdBudget;
}
void Player::advanceConstruction() {
    auto it = buildingQueue_.begin();
    while (it != buildingQueue_.end()) {
        Building* building = it->get();
        const double payment = building->nextPayment();
        const double tolerance = 8.0 * std::numeric_limits<double>::epsilon()
            * std::max({1.0, std::abs(capital_), std::abs(payment)});
        if (capital_ + tolerance < payment) {
            ++it;
            continue;
        }
        capital_ = std::max(0.0, capital_ - payment);
        building->advance();
        if (building->isBuilt()) {
            if (auto* house = dynamic_cast<House*>(building)) {
                builtHouses_.push_back(*house);
            } else if (auto* supermarket = dynamic_cast<Supermarket*>(building)) {
                builtSupermarkets_.push_back(*supermarket);
            }
            it = buildingQueue_.erase(it);
        } else {
            ++it;
        }
    }
}
void Player::recalculateTotalCapital() {
    totalCapital_ = capital_;
    for (const auto& building : buildingQueue_) {
        if (const auto* house = dynamic_cast<const House*>(building.get())) {
            totalCapital_ += house->constructionValue();
        } else if (const auto* supermarket =
                       dynamic_cast<const Supermarket*>(building.get())) {
            totalCapital_ += supermarket->constructionValue();
        }
    }
    for (const House& house : builtHouses_) {
        totalCapital_ += house.housingValue();
    }
    for (const Supermarket& supermarket : builtSupermarkets_) {
        totalCapital_ += supermarket.marketValue(config_.supermarketCapitalMultiplier);
    }
}
