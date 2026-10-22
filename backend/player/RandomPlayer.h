#pragma once
#include <random>
#include "Player.h"

class RandomPlayer : public Player {
public:
    RandomPlayer(int id, std::string name, const Config& config);
    Decision askDecision(const PublicView& view) override;
private:
    std::mt19937 generator_;
};
