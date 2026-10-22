#include "RulesValidator.h"
#include "StrategyUtils.h"

#include <vector>
#include <limits>
#include <cmath>
#include <algorithm>

RulesValidator::RulesValidator(const Config& config) : config_(config) {}

Decision RulesValidator::validate(const Decision& decision, 
        const PublicView& view) const
{
    if (decision.getIdPlayer() != view.getPlayerId()
        || decision.getMonth() != view.getCurrentMonth()) {
        return Decision(view.getPlayerId(), view.getCurrentMonth(),
                std::vector<ConstructionOrder>{},
                std::vector<HousingOfferDecision>{}, 0.0, 0.0);
    }

    double housingAds = decision.getHousingAdBudget();
    double marketAds = decision.getSupermarketAdBudget();

    if (!std::isfinite(housingAds) || housingAds < 0.0) housingAds = 0.0;
    if (!std::isfinite(marketAds) || marketAds < 0.0) marketAds = 0.0;

    const double existingPayments = // резерв денег на сущ строеки, чтобы новые заказы и реклама
        strategy_utils::remainingConstructionCost(view, config_); //не повысились
    const double maxAdsCost = std::max(0.0, view.getCash() - existingPayments);
    const double requestedAdsCost = (housingAds + marketAds) * 1000.0;

    if (requestedAdsCost > maxAdsCost && requestedAdsCost > 0.0) {
        const double scale = maxAdsCost / requestedAdsCost;
        housingAds *= scale;
        marketAds *= scale;
    }

    const double actualAdsCost = (housingAds + marketAds) * 1000.0;
    double availableForNew = view.getCash() - actualAdsCost - existingPayments;

    std::vector<ConstructionOrder> validOrders;

    for (const ConstructionOrder& order : decision.getConstructionOrders()) {
        if (order.count <= 0) continue;

        const double payment =
            strategy_utils::buildingCost(
                config_, order.buildingType, order.houseType);

        if (payment <= 0.0) continue;

        const double maxCountD = std::floor((availableForNew + 1e-8) / payment);

        if (maxCountD < 1.0) continue;

        const int maxCount = maxCountD >= std::numeric_limits<int>::max()
            ? std::numeric_limits<int>::max()
            : static_cast<int>(maxCountD);

        const int count = std::min(order.count, maxCount);

        validOrders.push_back({order.buildingType, order.houseType, count});
        availableForNew -= payment * count;
    }

    std::vector<HousingOfferDecision> validOffers;

    for (const HousingOfferDecision& offer : decision.getHousingOffers()) {
        if (!std::isfinite(offer.pricePerSqm) || offer.pricePerSqm <= 0.0) continue;
        if (offer.apartments <= 0) continue;

        const double fairPrice =
            strategy_utils::fairPricePerSqm(config_, offer.houseType);

        if (offer.pricePerSqm > fairPrice * config_.maxPriceMultiplier) continue;

        const int available =
            strategy_utils::availableApartments(view, offer.houseType);

        if (offer.apartments > available) continue;

        validOffers.push_back(offer);
    }

    return Decision(view.getPlayerId(), view.getCurrentMonth(),
                    std::move(validOrders), std::move(validOffers),
                    housingAds, marketAds);
}
