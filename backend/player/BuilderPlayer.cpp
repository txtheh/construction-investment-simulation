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
    if (view.getCurrentMonth() == 0) { //эта проверка, чтобы заказы можно было сделать на след месяц
        return Decision(
            getId(), 0,
            std::vector<ConstructionOrder>{},
            std::vector<HousingOfferDecision>{},
            0.0, 0.0
            );
    }
    const Config& config = getConfig();
    const auto ads = strategy_utils::fitAdBudgets(view, config, 1.0, 0.0);

    std::vector<ConstructionOrder> orders;

    const std::vector<HouseType> choices = {
        HouseType::Panel, HouseType::Monolithic, HouseType::Brick
    };

    for (HouseType type : choices) {
        const int affordable = strategy_utils::affordableNewBuildingCount
            (view, config, BuildingType::House, type, ads);

        if (affordable > 0) {
            orders.push_back({BuildingType::House, type, affordable});
            break;
        }
    }

    auto offers = strategy_utils::makeHousingOffers(view, config, 0.98);

    return Decision(getId(), view.getCurrentMonth(), std::move(orders),
                    std::move(offers), ads.housing, ads.supermarket);
}
