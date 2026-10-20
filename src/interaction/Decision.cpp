#include "Decision.h"
#include <utility>

Decision::Decision(int id, int month, std::string decision,
                   double housingAdBudget, double supermarketAdBudget)
    : idPlayer_(id), month_(month), type_decision_(std::move(decision)),
      housingAdBudget_(housingAdBudget), supermarketAdBudget_(supermarketAdBudget) {}

Decision::Decision(int id, int month,
                   std::vector<ConstructionOrder> constructionOrders,
                   std::vector<HousingOfferDecision> housingOffers,
                   double housingAdBudget, double supermarketAdBudget)
    : idPlayer_(id), month_(month),
      constructionOrders_(std::move(constructionOrders)),
      housingOffers_(std::move(housingOffers)),
      housingAdBudget_(housingAdBudget),
      supermarketAdBudget_(supermarketAdBudget) {}

Decision::~Decision() = default;

// геттеры
int Decision::getIdPlayer() const
{
    return idPlayer_;
}

int Decision::getMonth() const
{
    return month_;
}

std::string Decision::getTypeDecision() const
{
    return type_decision_;
}

const std::vector<ConstructionOrder>& Decision::getConstructionOrders() const
{
    return constructionOrders_;
}

const std::vector<HousingOfferDecision>& Decision::getHousingOffers() const
{
    return housingOffers_;
}

double Decision::getHousingAdBudget() const
{
    return housingAdBudget_;
}

double Decision::getSupermarketAdBudget() const
{
    return supermarketAdBudget_;
}

// сеттеры
void Decision::setIdPlayer(int id)
{
    idPlayer_ = id;
}

void Decision::setMonth(int month)
{
    month_ = month;
}

void Decision::setTypeDecision(std::string decision)
{
    type_decision_ = std::move(decision);
}

void Decision::setConstructionOrders(std::vector<ConstructionOrder> constructionOrders)
{
    constructionOrders_ = std::move(constructionOrders);
}

void Decision::setHousingOffers(std::vector<HousingOfferDecision> housingOffers)
{
    housingOffers_ = std::move(housingOffers);
}
