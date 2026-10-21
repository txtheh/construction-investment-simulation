#pragma once

#include <vector>

#include "../config.h"
#include "../interaction/offer.h"

class District;

struct HousingDemand {
    HouseType houseType;
    double apartments;
};

struct HousingSale {
    int playerIndex;
    HouseType houseType;
    int apartmentsSold;
    double pricePerSqm;
};

class RealtorAgency {
public:
    explicit RealtorAgency(const Config& config);

    std::vector<HousingDemand> calculateDemands(
        int calendarMonth, const District& district) const;

    std::vector<HousingSale> distributeSales(
        const std::vector<HousingDemand>& demands,
        const std::vector<Offer>& offers) const;

private:
    std::vector<HousingSale> distributeType(
        HouseType type, double demand,
        const std::vector<Offer>& offers) const;

    Config config_;
};
