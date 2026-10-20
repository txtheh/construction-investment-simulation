#pragma once
#include "Player.h"

class MarketPlayer : public Player {
public:
    MarketPlayer(int id, std::string name, const Config& config);
    Decision askDecision(const PublicView& view) override;
};
