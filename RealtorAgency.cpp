#include "RealtorAgency.h"
#include "District.h"
#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>
namespace {
struct Candidate {
    const Offer* offer;
    int baseSold;
};
struct FractionalShare {
    std::size_t index;
    double fraction;
};
int demandToApartments(double demand) {
    if (!std::isfinite(demand) || demand <= 0.0) {
        return 0;
    }
    return static_cast<int>(std::min(std::floor(demand + 1e-9),
                                     static_cast<double>(std::numeric_limits<int>::max())));
}
int houseTypeIndex(HouseType type) {
    switch (type) {
    case HouseType::Panel:
        return 0;
    case HouseType::Monolithic:
        return 1;
    case HouseType::Brick:
        return 2;
    }
    return -1;
}
}
RealtorAgency::RealtorAgency(const Config& config) : config_(config) {}
std::vector<HousingDemand> RealtorAgency::calculateDemands(int calendarMonth, const District& district) const {
    if (calendarMonth < 1 || calendarMonth > 12) {
        throw std::out_of_range("Calendar month must be between 1 and 12");
    }
    const double demand = config_.baseHousingDemand
        * config_.housingSeason[static_cast<std::size_t>(calendarMonth - 1)]
        * (1.0 + config_.supermarketDemandBonus * district.getWeightedSupermarketCount());
    const double safeDemand = std::isfinite(demand) ? std::max(0.0, demand) : 0.0;
    return {
        {HouseType::Panel, safeDemand},
        {HouseType::Monolithic, safeDemand},
        {HouseType::Brick, safeDemand}
    };
}
std::vector<HousingSale> RealtorAgency::distributeSales(const std::vector<HousingDemand>& demands, const std::vector<Offer>& offers) const {
    std::vector<HousingSale> sales;
    bool processedTypes[3] = {false, false, false};
    for (const HousingDemand& demand : demands) {
        const int index = houseTypeIndex(demand.houseType);
        if (index < 0 || processedTypes[index]) {
            continue;
        }
        processedTypes[index] = true;
        std::vector<HousingSale> typeSales = distributeType(demand.houseType, demand.apartments, offers);
        sales.insert(sales.end(), typeSales.begin(), typeSales.end());
    }
    return sales;
}
std::vector<HousingSale> RealtorAgency::distributeType(HouseType type, double demand, const std::vector<Offer>& offers) const {
    std::vector<Candidate> candidates;
    for (const Offer& offer : offers) {
        const double price = offer.getPricePerSqm();
        if (offer.getHouseType() != type || offer.getApartments() <= 0
            || !std::isfinite(price) || price <= 0.0) {
            continue;
        }
        candidates.push_back({&offer, 0});
    }
    std::stable_sort(candidates.begin(), candidates.end(), [](const Candidate& left, const Candidate& right) {
        return left.offer->getPricePerSqm() < right.offer->getPricePerSqm();
    });
    int remainingDemand = demandToApartments(demand);
    std::size_t groupStart = 0;
    while (groupStart < candidates.size() && remainingDemand > 0) {
        std::size_t groupEnd = groupStart + 1;
        const double price = candidates[groupStart].offer->getPricePerSqm();
        while (groupEnd < candidates.size()
               && candidates[groupEnd].offer->getPricePerSqm() == price) {
            ++groupEnd;
        }
        long long groupSupply = 0;
        for (std::size_t i = groupStart; i < groupEnd; ++i) {
            groupSupply += candidates[i].offer->getApartments();
        }
        if (groupSupply <= remainingDemand) {
            for (std::size_t i = groupStart; i < groupEnd; ++i) {
                candidates[i].baseSold = candidates[i].offer->getApartments();
            }
            remainingDemand -= static_cast<int>(groupSupply);
        } else {
            const int groupDemand = remainingDemand;
            int allocated = 0;
            std::vector<FractionalShare> fractions;
            fractions.reserve(groupEnd - groupStart);
            for (std::size_t i = groupStart; i < groupEnd; ++i) {
                const double exactShare = static_cast<double>(groupDemand)
                    * candidates[i].offer->getApartments() / static_cast<double>(groupSupply);
                const int wholeShare = static_cast<int>(std::floor(exactShare));
                candidates[i].baseSold = wholeShare;
                allocated += wholeShare;
                fractions.push_back({i, exactShare - wholeShare});
            }
            std::stable_sort(fractions.begin(), fractions.end(), [](const FractionalShare& left, const FractionalShare& right) {
                return left.fraction > right.fraction;
            });
            int leftover = groupDemand - allocated;
            for (const FractionalShare& share : fractions) {
                if (leftover <= 0) {
                    break;
                }
                if (candidates[share.index].baseSold < candidates[share.index].offer->getApartments()) {
                    ++candidates[share.index].baseSold;
                    --leftover;
                }
            }
            remainingDemand = 0;
        }
        groupStart = groupEnd;
    }
    std::vector<HousingSale> sales;
    for (const Candidate& candidate : candidates) {
        const Offer& offer = *candidate.offer;
        double bonus = offer.getAdvertisingBonus();
        if (!std::isfinite(bonus) || bonus < 0.0) {
            bonus = 0.0;
        }
        const double adjustedSales = static_cast<double>(candidate.baseSold) * (1.0 + bonus);
        const int sold = adjustedSales >= offer.getApartments()
            ? offer.getApartments()
            : static_cast<int>(std::floor(adjustedSales + 1e-9));
        if (sold > 0) {
            sales.push_back({offer.getPlayerIndex(), type, sold, offer.getPricePerSqm()});
        }
    }
    return sales;
}
