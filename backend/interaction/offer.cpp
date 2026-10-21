#include "offer.h"

Offer::Offer(int playerIndex, HouseType houseType, int apartments, double pricePerSqm, double advertisingBonus) :
    playerIndex_(playerIndex), houseType_(houseType), apartments_(apartments), 
    pricePerSqm_(pricePerSqm), advertisingBonus_(advertisingBonus) {}


int Offer::getPlayerIndex() const
{
    return playerIndex_;
}
HouseType Offer::getHouseType() const
{
    return houseType_;
}
int Offer::getApartments() const
{
    return apartments_;
}
double Offer::getPricePerSqm() const
{
    return pricePerSqm_;
}
double Offer::getAdvertisingBonus() const
{
    return advertisingBonus_;
}
