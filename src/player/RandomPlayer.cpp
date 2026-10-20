#include "RandomPlayer.h"
#include "../view/PublicView.h"
#include "../modules/StrategyUtils.h"

#include <algorithm>
#include <utility>
#include <vector>

RandomPlayer::RandomPlayer(int id, std::string name, const Config& config)
    : Player(id, std::move(name), config), generator_(std::random_device{}()) {}

Decision RandomPlayer::askDecision(const PublicView& view)
{
    const Config& config = getConfig();
    const std::vector<double> adChoices = {0.0, 0.5, 1.0, 1.5, 2.0};
    std::uniform_int_distribution<std::size_t> adIndex(0, adChoices.size() - 1);
    const double requestedHousingAd =
        strategy_utils::totalAvailableApartments(view) > 0
            ? adChoices[adIndex(generator_)] : 0.0;
    const double requestedSupermarketAd =
        strategy_utils::hasBuiltSupermarket(view)
            ? adChoices[adIndex(generator_)] : 0.0;
    const auto ads = strategy_utils::fitAdBudgets(
        view, config, requestedHousingAd, requestedSupermarketAd);
    std::vector<ConstructionOrder> orders;
    const std::vector<std::pair<BuildingType, HouseType>> choices = {
        {BuildingType::House, HouseType::Panel},
        {BuildingType::House, HouseType::Monolithic},
        {BuildingType::House, HouseType::Brick},
        {BuildingType::Supermarket, HouseType::Panel}
    };
    std::uniform_int_distribution<int> buildCount(0, 2);
    const int desiredBuilds = buildCount(generator_);
    double plannedMonthlyPayments = 0.0;
    for (int i = 0; i < desiredBuilds; ++i) {
        std::vector<std::pair<BuildingType, HouseType>> affordable;
        for (const auto& choice : choices) {
            if (strategy_utils::affordableNewBuildingCount(
                    view, config, choice.first, choice.second, ads,
                    plannedMonthlyPayments) > 0) {
                affordable.push_back(choice);
            }
        }
        if (affordable.empty()) {
            break;
        }
        std::uniform_int_distribution<std::size_t> eligibleIndex(
            0, affordable.size() - 1);
        const auto choice = affordable[eligibleIndex(generator_)];
        auto existing = std::find_if(
            orders.begin(), orders.end(),
            [&choice](const ConstructionOrder& order) {
                return order.buildingType == choice.first
                    && order.houseType == choice.second;
            });
        if (existing == orders.end()) {
            orders.push_back({choice.first, choice.second, 1});
        } else {
            ++existing->count;
        }
        plannedMonthlyPayments += strategy_utils::monthlyPayment(
            config, choice.first, choice.second);
    }
    std::vector<HousingOfferDecision> offers;
    const std::vector<HouseType> houseTypes = {
        HouseType::Panel, HouseType::Monolithic, HouseType::Brick
    };
    for (HouseType type : houseTypes) {
        const int apartments = strategy_utils::availableApartments(view, type);
        if (apartments <= 0) {
            continue;
        }
        const double maxFactor = config.maxPriceMultiplier;
        const double minFactor = std::min(1.0, maxFactor);
        std::uniform_real_distribution<double> priceFactor(minFactor, maxFactor);
        const double price =
            strategy_utils::fairPricePerSqm(config, type) * priceFactor(generator_);
        if (price > 0.0) {
            offers.push_back({type, apartments, price});
        }
    }
    return Decision(getId(), view.getCurrentMonth(), std::move(orders),
                    std::move(offers), ads.housing, ads.supermarket);
}
