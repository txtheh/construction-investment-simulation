#include "Decision.h"

Decision::Decision(int id, int month, std::string decision, double budget) :
    idPlayer_(id), month_(month), type_decision_(decision), budget_(budget) {}


Decision::~Decision() {
}

int Decision::getIdPlayer() const {
    return idPlayer_;
}

int Decision::getMonth() const {
    return month_;
}

std::string Decision::getTypeDecision() const {
    return type_decision_;
}

double Decision::getBudget() const {
    return budget_;
}

// Возврат порядка строительства
// std::vector<Building> getBuildOrder();

void Decision::setIdPlayer(int id) {
    idPlayer_ = id;
}

void Decision::setMonth(int month) {
    month_ = month;
}

void Decision::setTypeDecision(std::string decision) {
    type_decision_ = decision;
}

void Decision::setBudget(double budget) {
    budget_ = budget;
}

