#include "BalancedPlayer.h"
#include "PublicView.h"
#include "StrategyUtils.h"
#include <utility>
#include <vector>

BalancedPlayer::BalancedPlayer(int id, std::string name, const Config& config)
    : Player(id, std::move(name), config) {}

Decision BalancedPlayer::askDecision(const PublicView& view)
{
    const Config& config = getConfig();
    const auto ads = strategy_utils::fitAdBudgets(
        view, config,
        strategy_utils::totalAvailableApartments(view) > 0 ? 0.5 : 0.0,
        strategy_utils::hasBuiltSupermarket(view) ? 0.5 : 0.0);
    std::vector<ConstructionOrder> orders;
    const int houses = strategy_utils::ownBuildingCount(view, BuildingType::House);
    const int markets = strategy_utils::ownBuildingCount(view, BuildingType::Supermarket);
    const bool preferHouse = houses <= markets;
    if (preferHouse) {
        const std::vector<HouseType> houseTypes = {
            HouseType::Panel, HouseType::Monolithic, HouseType::Brick
        };
        for (HouseType type : houseTypes) {
            if (strategy_utils::affordableNewBuildingCount(
                    view, config, BuildingType::House, type, ads) > 0) {
                orders.push_back({BuildingType::House, type, 1});
                break;
            }
        }
        if (orders.empty()
            && strategy_utils::affordableNewBuildingCount(
                   view, config, BuildingType::Supermarket, HouseType::Panel, ads) > 0) {
            orders.push_back({BuildingType::Supermarket, HouseType::Panel, 1});
        }
    } else if (strategy_utils::affordableNewBuildingCount(
                   view, config, BuildingType::Supermarket, HouseType::Panel, ads) > 0)
{
        orders.push_back({BuildingType::Supermarket, HouseType::Panel, 1});
    } else {
        const std::vector<HouseType> houseTypes = {
            HouseType::Panel, HouseType::Monolithic, HouseType::Brick
        };
        for (HouseType type : houseTypes) {
            if (strategy_utils::affordableNewBuildingCount(
                    view, config, BuildingType::House, type, ads) > 0) {
                orders.push_back({BuildingType::House, type, 1});
                break;
            }
        }
    }
    auto offers = strategy_utils::makeHousingOffers(view, config, 1.0);
    return Decision(getId(), view.getCurrentMonth(), std::move(orders),
                    std::move(offers), ads.housing, ads.supermarket);
}
