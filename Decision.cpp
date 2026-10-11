#include "Decision.h"

Decision::Decision(int id, int month, std::string decision,
        double housingAdBudget, double supermarketAdBudget) :
    idPlayer_(id), month_(month), type_decision_(decision),
    housingAdBudget_(housingAdBudget),
    supermarketAdBudget_(supermarketAdBudget) {}


Decision::~Decision() {
}

// геттеры

int Decision::getIdPlayer() const {
    return idPlayer_;
}

int Decision::getMonth() const {
    return month_;
}

std::string Decision::getTypeDecision() const {
    return type_decision_;
}

double Decision::getHousingAdBudget() const {
    return housingAdBudget_;
}

double Decision::getSupermarketAdBudget() const {
    return supermarketAdBudget_;
}

// сеттеры

void Decision::setIdPlayer(int id) {
    idPlayer_ = id;
}

void Decision::setMonth(int month) {
    month_ = month;
}

void Decision::setTypeDecision(std::string decision) {
    type_decision_ = decision;
}

