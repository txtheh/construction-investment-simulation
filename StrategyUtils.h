#pragma once

#include <vector>

#include "Decision.h"
#include "PublicView.h"

namespace strategy_utils {

struct AdBudgets {
    double housing = 0.0;
    double supermarket = 0.0;
};

const HouseConfig& houseConfig(const Config& config, HouseType type);

double monthlyPayment(const Config& config,
                      BuildingType buildingType, HouseType houseType);

int availableApartments(const PublicView& view, HouseType type);
int totalAvailableApartments(const PublicView& view);
int ownBuildingCount(const PublicView& view, BuildingType type);
bool hasBuiltSupermarket(const PublicView& view);

double nextConstructionPayments(const PublicView& view, const Config& config);

AdBudgets fitAdBudgets(const PublicView& view, const Config& config,
                       double housingBudget, double supermarketBudget);

int affordableNewBuildingCount(
    const PublicView& view, const Config& config,
    BuildingType buildingType, HouseType houseType,
    const AdBudgets& ads, double plannedMonthlyPayments = 0.0);

double fairPricePerSqm(const Config& config, HouseType type);

std::vector<HousingOfferDecision> makeHousingOffers(
    const PublicView& view, const Config& config, double priceFactor);

}
