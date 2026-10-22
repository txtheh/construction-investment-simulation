#pragma once

#include "../config.h"

// dto объект
class Offer {
public:
    Offer(int playerIndex, HouseType houseType, int apartments,
          double pricePerSqm, double advertisingBonus);

    // геттеры
    int getPlayerIndex() const;
    HouseType getHouseType() const;
    int getApartments() const;
    double getPricePerSqm() const;
    double getAdvertisingBonus() const;

private:
    int playerIndex_;
    HouseType houseType_;
    int apartments_;
    double pricePerSqm_;
    double advertisingBonus_;
};
