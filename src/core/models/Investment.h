#pragma once

#include "Common.h"

#include <string>

using namespace std;

namespace finsight::core::models {

// Identifies the type of investment asset.
enum class InvestmentType {
    Gold,
    Silver,
    Currency,
    Stock,
    Other
};

// Stores one investment position owned by the user.
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

// Summarizes the current value and profit/loss of an investment.
struct InvestmentSnapshot {
    Investment investment;
    double investedValue {0.0};
    double currentValue {0.0};
    double profitLoss {0.0};
};

}  // namespace finsight::core::models
