#pragma once
#include "Player.h"
class BalancedPlayer : public Player {
public:
    BalancedPlayer(int id, std::string name, const Config& config);
    Decision askDecision(const PublicView& view) override;
};
