#pragma once

#include <vector>

#include "../config.h"
#include "PublicView.h"
#include "../interaction/Decision.h"

class RulesValidator {
public:
    RulesValidator(const Config& config);
    Decision validate(const Decision& decision, const PublicView& view) const;

private:
    Config config_;
}
