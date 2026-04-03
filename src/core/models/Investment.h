#pragma once

#include "Common.h"

#include <string>

namespace finsight::core::models {

enum class InvestmentType {
    Gold,
    Silver,
    Currency,
    Stock,
    Other
};

struct Investment {
    std::string id;
    std::string userId;
    std::string assetName;
    std::string symbol;
    InvestmentType type {InvestmentType::Other};
    double quantity {0.0};
    double buyRate {0.0};
    double currentRate {0.0};
    Date purchaseDate;
};

struct InvestmentSnapshot {
    Investment investment;
    double investedValue {0.0};
    double currentValue {0.0};
    double profitLoss {0.0};
};

}  // namespace finsight::core::models
