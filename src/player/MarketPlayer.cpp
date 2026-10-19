#include "MarketPlayer.h"
#include "PublicView.h"
#include "StrategyUtils.h"
#include <utility>
#include <vector>

MarketPlayer::MarketPlayer(int id, std::string name, const Config& config)
    : Player(id, std::move(name), config) {}

Decision MarketPlayer::askDecision(const PublicView& view)
{
    const Config& config = getConfig();
    const auto ads = strategy_utils::fitAdBudgets(view, config, 0.0, 0.5);
    std::vector<ConstructionOrder> orders;
    if (strategy_utils::affordableNewBuildingCount(
            view, config, BuildingType::Supermarket, HouseType::Panel, ads) > 0)
{
        orders.push_back({BuildingType::Supermarket, HouseType::Panel, 1});
    }
    auto offers = strategy_utils::makeHousingOffers(view, config, 1.08);
    return Decision(getId(), view.getCurrentMonth(), std::move(orders),
                    std::move(offers), ads.housing, ads.supermarket);
}
