#include "offer.h"

Offer::Offer(int playerIndex, HouseType houseType, int apartments, double pricePerSqm, double advertisingBonus) {
    this->playerIndex = playerIndex;
    this->houseType = houseType;
    this->apartments = apartments;
    this->pricePerSqm = pricePerSqm;
    this->advertisingBonus = advertisingBonus;
}
int Offer::getPlayerIndex() const {
    return playerIndex;
}
HouseType Offer::getHouseType() const {
    return houseType;
}
int Offer::getApartments() const {
    return apartments;
}
double Offer::getPricePerSqm() const {
    return pricePerSqm;
}
double Offer::getAdvertisingBonus() const {
    return advertisingBonus;
}