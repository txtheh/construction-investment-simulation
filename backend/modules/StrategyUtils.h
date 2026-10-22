#pragma once

#include <vector>

#include "../interaction/Decision.h"
#include "../view/PublicView.h"
#include "../config.h"

namespace strategy_utils {

struct AdBudgets {
    double housing = 0.0;
    double supermarket = 0.0;
};

double monthlyPayment(const Config& config,
                      BuildingType buildingType, HouseType houseType);
double buildingCost(
    const Config& config,
    BuildingType buildingType,
    HouseType houseType);
double remainingConstructionCost(
    const PublicView& view,
    const Config& config);
int availableApartments(const PublicView& view, HouseType type);
int totalAvailableApartments(const PublicView& view);
int ownBuildingCount(const PublicView& view, BuildingType type);
bool hasBuiltSupermarket(const PublicView& view);

double nextConstructionPayments(const PublicView& view, const Config& config);

AdBudgets fitAdBudgets(const PublicView& view, const Config& config,
                       double housingBudget, double supermarketBudget);

int affordableNewBuildingCount( //опр затраты на буд строительство и сколько новых объктов можно
    const PublicView& view, const Config& config, // ещё построить
    BuildingType buildingType, HouseType houseType,
    const AdBudgets& ads, double plannedConstructionCost = 0.0);

double fairPricePerSqm(const Config& config, HouseType type);

std::vector<HousingOfferDecision> makeHousingOffers(
    const PublicView& view, const Config& config, double priceFactor);

}
