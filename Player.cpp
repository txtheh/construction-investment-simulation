#include "Player.h"
#include "config.h"
// #include "GameState.h"

Player::Player(int id, std::string name) :
    id_(id), name_(name),
    capital_(config::START_CAPITAL), totalCapital_(config::START_CAPITAL) {}

// геттеры

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


// геттеры потсроек игрока 

const std::vector<std::unique_ptr<Building>>& Player::getBuildingQueue() const {
    return buildingQueue_;
}

const std::vector<House>& Player::getBuiltHouses() const {
    return builtHouses_;
}

const std::vector<Supermarket>& Player::getBuiltSupermarkets() const {
    return builtSupermarkets_;
}


// работа с капиталом

void Player::addCapital(double delta) {
    capital_ += delta;
}

void Player::addExpenses(double amount) {
    capital_ -= amount;
}


// применение решения игрока (постройка и реклама)

void Player::applyDecision(const Decision& decision) {
    addExpenses(decision.getHousingAdBudget());
    addExpenses(decision.getSupermarketAdBudget());

    const std::string& type = decision.getTypeDecision();

    if (type == "panel") {
        buildingQueue_.push_back(
        std::make_unique<House>("panel", config::COST_PANEL, 
            config::MONTHS_PANEL));

    } else if (type == "monolith") {
        buildingQueue_.push_back(
        std::make_unique<House>("monolith", config::COST_MONOLITH, 
            config::MONTHS_MONOLITH));

    } else if (type == "brick") {
        buildingQueue_.push_back(
        std::make_unique<House>("brick", config::COST_BRICK, 
            config::MONTHS_BRICK));

    } else if (type == "supermarket") {
        buildingQueue_.push_back(
        std::make_unique<Supermarket>(config::COST_SUPERMARKET,
            config::MONTHS_SUPERMARKET));
    }
    
    housingAdBudget_ = decision.getHousingAdBudget();
    supermarketAdBudget_ = decision.getSupermarketAdBudget();
}

// сдвигаем очередь построек, проверяя есть ли построенные здания

void Player::advanceConstruction() {
    auto it = buildingQueue_.begin();

    while (it != buildingQueue_.end()) {
        Building* b = it->get();

        addExpenses(b->getMonthlyPayment());
        b->payMonth();

        if (b->isBuilt()) {

            if (auto* h = dynamic_cast<House*>(b)) {
                builtHouses_.push_back(*h);

            } else if (auto* s = dynamic_cast<Supermarket*>(b)) {
                builtSupermarkets_.push_back(*s);
            }

            it = buildingQueue_.erase(it);

        } else {
            ++it;
        }
    }
}

// пересчет бюджета игрока

void Player::recalculateTotalCapital() {
    totalCapital_ = capital_;

    for (const auto& b : buildingQueue_) {
        totalCapital_ += b->getInvested();
    }

    for (const auto& h : builtHouses_) {
        if (!h.isSold()) {
            totalCapital_ += h.getInvested();
        }
    }

    for (const auto& s : builtSupermarkets_) {
        totalCapital_ += s.getInvested() * config::SUPERMARKET_VALUE_RATIO;
    }
}
