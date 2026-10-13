#pragma once
#include "Player.h"
class BuilderPlayer : public Player {
public:
    BuilderPlayer(int id, std::string name, const Config& config);
    Decision askDecision(const PublicView& view) override;
};
