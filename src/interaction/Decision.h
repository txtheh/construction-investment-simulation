#pragma once

#include <string>
#include <vector>

#include "../config.h"

struct ConstructionOrder {
    BuildingType buildingType;
    HouseType houseType;
    int count;
};

struct HousingOfferDecision {
    HouseType houseType;
    int apartments;
    double pricePerSqm;
};

class Decision {
public:
    Decision(int id, int month, std::string decision,
             double housingAdBudget, double supermarketAdBudget);

    Decision(int id, int month,
             std::vector<ConstructionOrder> constructionOrders,
             std::vector<HousingOfferDecision> housingOffers,
             double housingAdBudget, double supermarketAdBudget);

    ~Decision();

    int getIdPlayer() const;
    int getMonth() const;
    std::string getTypeDecision() const;

    const std::vector<ConstructionOrder>& getConstructionOrders() const;
    const std::vector<HousingOfferDecision>& getHousingOffers() const;

    double getHousingAdBudget() const;
    double getSupermarketAdBudget() const;

    void setIdPlayer(int id);
    void setMonth(int month);
    void setTypeDecision(std::string decision);
    void setConstructionOrders(std::vector<ConstructionOrder> constructionOrders);
    void setHousingOffers(std::vector<HousingOfferDecision> housingOffers);

private:
    int idPlayer_;
    int month_;
    std::string type_decision_;
    std::vector<ConstructionOrder> constructionOrders_;
    std::vector<HousingOfferDecision> housingOffers_;
    double housingAdBudget_;
    double supermarketAdBudget_;
};
