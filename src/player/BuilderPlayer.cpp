#include "BuilderPlayer.h"
#include "../view/PublicView.h"
#include "../modules/StrategyUtils.h"

#include <string>
#include <utility>
#include <vector>

BuilderPlayer::BuilderPlayer(int id, std::string name, const Config& config)
    : Player(id, std::move(name), config) {}

Decision BuilderPlayer::askDecision(const PublicView& view)
{
    const Config& config = getConfig();
    const auto ads = strategy_utils::fitAdBudgets(view, config, 1.0, 0.0);
    std::vector<ConstructionOrder> orders;
    const std::vector<HouseType> choices = {
        HouseType::Panel, HouseType::Monolithic, HouseType::Brick
    };
    for (HouseType type : choices) {
        if (strategy_utils::affordableNewBuildingCount(
                view, config, BuildingType::House, type, ads) > 0) {
            orders.push_back({BuildingType::House, type, 1});
            break;
        }
    }
    auto offers = strategy_utils::makeHousingOffers(view, config, 0.98);
    return Decision(getId(), view.getCurrentMonth(), std::move(orders),
                    std::move(offers), ads.housing, ads.supermarket);
}
