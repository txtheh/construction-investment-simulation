#pragma once

#include <vector>

#include "../config.h"
#include "../view/PublicView.h"
#include "../interaction/Decision.h"

class RulesValidator {
public:
    RulesValidator(const Config& config);
    Decision validate(const Decision& decision, const PublicView& view,
            int alreadyCommittedBuildings = 0) const;

private:
    Config config_;
};
