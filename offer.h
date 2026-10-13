#pragma once

#include "config.h"

class Offer {
public:
    Offer(int playerIndex, HouseType houseType, int apartments,
          double pricePerSqm, double advertisingBonus);

    int getPlayerIndex() const;
    HouseType getHouseType() const;
    int getApartments() const;
    double getPricePerSqm() const;
    double getAdvertisingBonus() const;

private:
    int playerIndex;
    HouseType houseType;
    int apartments;
    double pricePerSqm;
    double advertisingBonus;
};
