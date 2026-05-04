#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "core/services/AuthService.h"
#include "core/services/SessionService.h"
#include "core/services/TransactionService.h"
#include "core/services/BudgetService.h"
#include "core/services/SavingsService.h"
#include "network/tcp/IMessageHandler.h"

using namespace finsight::core::services;
using namespace finsight::core::models;
static Date today() { return Date{2026, 5, 1}; }
static std::string findCatId(const TransactionService& txns,
                              const std::string& name) {
    for (const auto& c : txns.getCategories())
        if (c.name == name) return c.id;
    return {};
}

// Register user 
TEST(AuthTest, RegisterUser) {
    AuthService auth;
    auto user = auth.registerUser("Moataz", "moataz@aucegypt.edu",
                                  "", "", "password123", today());
    EXPECT_EQ(user.fullName, "Moataz");
    EXPECT_EQ(user.email,    "moataz@aucegypt.edu");
    EXPECT_FALSE(user.id.empty());
}

// Duplicate email 
TEST(AuthTest, DuplicateEmailRejected) {
    AuthService auth;
    auth.registerUser("Moataz", "moataz@aucegypt.edu", "", "", "password123", today());
    EXPECT_THROW(
        auth.registerUser("Moataz1", "moataz@aucegypt.edu", "", "", "pass12345", today()),
        std::invalid_argument
    );
}

// Login successful
TEST(AuthTest, LoginSucceeds) {
    AuthService auth;
    auth.registerUser("ahmed", "ahmed@aucegypt.edu", "", "", "ahmed2", today());
    auto result = auth.login("ahmed@aucegypt.edu", "ahmed2");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->email, "ahmed@aucegypt.edu");
}

// Login fails with wrong password
TEST(AuthTest, LoginFailsWrongPassword) {
    AuthService auth;
    auth.registerUser("Maya", "maya@aucegypt.edu", "", "", "correct", today());
    auto result = auth.login("maya@aucegypt.edu", "wrongpassword");
    EXPECT_FALSE(result.has_value());
}

// Session starts 
TEST(SessionTest, SessionStartsActive) {
    SessionService sessions;
    auto session = sessions.startSession("user-maya", today());
    EXPECT_TRUE(sessions.isSessionActive(session.token));
    EXPECT_FALSE(session.token.empty());
}

// Session ends after logout 
TEST(SessionTest, SessionEndsAfterLogout) {
    SessionService sessions;
    auto session = sessions.startSession("usr-maya", today());
    sessions.endSession(session.token);
    EXPECT_FALSE(sessions.isSessionActive(session.token));
}

// Add transaction 
TEST(TransactionTest, AddTransaction) {
    TransactionService txns;
    std::string catId = findCatId(txns, "Salary");

    Transaction t;
    t.userId     = "usr-maya";
    t.title      = "Monthly Salary";
    t.categoryId = catId;
    t.type       = TransactionType::Income;
    t.amount     = 5000.0;
    t.date       = today();

    auto saved = txns.addTransaction(t);
    EXPECT_FALSE(saved.id.empty());
    EXPECT_DOUBLE_EQ(saved.amount, 5000.0);
    EXPECT_EQ(saved.title, "Monthly Salary");
}

// Update transaction 
TEST(TransactionTest, UpdateTransaction) {
    TransactionService txns;
    std::string catId = findCatId(txns, "Food");

    Transaction t;
    t.userId     = "usr-maya";
    t.title      = "Old Title";
    t.categoryId = catId;
    t.type       = TransactionType::Expense;
    t.amount     = 50.0;
    t.date       = today();

    auto saved   = txns.addTransaction(t);
    Transaction updated = saved;
    updated.title  = "New Title";
    updated.amount = 75.0;

    auto result = txns.updateTransaction("usr-maya", saved.id, updated);
    EXPECT_EQ(result.title, "New Title");
    EXPECT_DOUBLE_EQ(result.amount, 75.0);
}

// Delete transaction 
TEST(TransactionTest, DeleteTransaction) {
    TransactionService txns;
    std::string catId = findCatId(txns, "Food");

    Transaction t;
    t.userId     = "usr-maya";
    t.title      = "To Delete";
    t.categoryId = catId;
    t.type       = TransactionType::Expense;
    t.amount     = 20.0;
    t.date       = today();

    auto saved = txns.addTransaction(t);
    txns.deleteTransaction("usr-maya", saved.id);
    EXPECT_TRUE(txns.listTransactions("usr-maya").empty());
}

// Filter transactions by category 

TEST(TransactionTest, FilterByCategory) {
    TransactionService txns;
    std::string foodId      = findCatId(txns, "Food");
    std::string transportId = findCatId(txns, "Transport");

    Transaction t1;
    t1.userId = "usr-maya"; t1.title = "Lunch";
    t1.categoryId = foodId; t1.type = TransactionType::Expense;
    t1.amount = 50.0; t1.date = today();
    txns.addTransaction(t1);

    Transaction t2;
    t2.userId = "usr-maya"; t2.title = "Uber";
    t2.categoryId = transportId; t2.type = TransactionType::Expense;
    t2.amount = 30.0; t2.date = today();
    txns.addTransaction(t2);

    // Verify 
    auto all = txns.listTransactions("usr-maya");
    ASSERT_EQ(all.size(), 2u);

    // counting
    int foodCount = 0;
    std::string foundTitle;
    for (const auto& t : all) {
        if (t.categoryId == foodId) {
            foodCount++;
            foundTitle = t.title;
        }
    }
    EXPECT_EQ(foodCount, 1);
    EXPECT_EQ(foundTitle, "Lunch");
}

// Create budget 
TEST(BudgetTest, CreateBudget) {
    TransactionService txns;
    BudgetService budgets;
    std::string foodId = findCatId(txns, "Food");

    auto b = budgets.createBudget("usr-1", foodId, YearMonth{2026, 5}, 1000.0);
    EXPECT_DOUBLE_EQ(b.limit, 1000.0);
    EXPECT_EQ(b.period.month, 5);
    EXPECT_EQ(b.period.year, 2026);
}

// Overspending detected 
TEST(BudgetTest, OverspendingDetected) {
    TransactionService txns;
    BudgetService budgets;
    std::string foodId = findCatId(txns, "Food");

    budgets.createBudget("usr-1", foodId, YearMonth{2026, 5}, 100.0);

    Transaction t;
    t.userId     = "usr-1";
    t.title      = "Expensive Dinner";
    t.categoryId = foodId;
    t.type       = TransactionType::Expense;
    t.amount     = 150.0;
    t.date       = {2026, 5, 1};
    txns.addTransaction(t);

    auto statuses = budgets.summarizeBudgets("usr-1", YearMonth{2026, 5}, txns);
    ASSERT_EQ(statuses.size(), 1u);
    EXPECT_TRUE(statuses.front().overspent);
    EXPECT_NEAR(statuses.front().spent,     150.0,  0.001);
    EXPECT_NEAR(statuses.front().remaining, -50.0,  0.001);
}

TEST(SavingsTest, RejectsWithdrawalBeyondCurrentBalance) {
    SavingsService savings;

    savings.addDeposit("usr-1", 500.0, today());

    EXPECT_THROW(
        savings.addWithdrawal("usr-1", 600.0, today()),
        std::invalid_argument
    );

    const auto overview = savings.summarize("usr-1", YearMonth {2026, 5});
    EXPECT_DOUBLE_EQ(overview.currentBalance, 500.0);
}

TEST(SavingsTest, AllowsWithdrawalUpToCurrentBalance) {
    SavingsService savings;

    savings.addDeposit("usr-1", 500.0, today());
    savings.addWithdrawal("usr-1", 500.0, today());

    const auto overview = savings.summarize("usr-1", YearMonth {2026, 5});
    EXPECT_DOUBLE_EQ(overview.currentBalance, 0.0);
}

// Mock: handler called with correct input 
class MockMessageHandler : public IMessageHandler {
public:
    MOCK_METHOD(std::string, handle, (const std::string& json), (override));
};

TEST(MockTest, HandlerIsCalledAndReturnsResponse) {
    MockMessageHandler mock;

    EXPECT_CALL(mock, handle("{\"command\":\"ping\"}"))
        .Times(1)
        .WillOnce(testing::Return("{\"status\":\"ok\",\"message\":\"pong\"}"));

    std::string response = mock.handle("{\"command\":\"ping\"}");
    EXPECT_EQ(response, "{\"status\":\"ok\",\"message\":\"pong\"}");
}
