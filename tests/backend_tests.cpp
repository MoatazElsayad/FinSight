#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "core/services/AuthService.h"
#include "core/services/SessionService.h"
#include "core/services/TransactionService.h"
#include "core/services/BudgetService.h"
#include "core/services/SavingsService.h"
#include "core/services/GoalService.h"
#include "core/services/ReportService.h"
#include "core/managers/FinanceTrackerBackend.h"
#include "data/storage/BackendStore.h"
#include "network/tcp/IMessageHandler.h"

#include <filesystem>
#include <string>

using namespace finsight::core::services;
using namespace finsight::core::models;

// i use this date in most tests
Date makeDate() {
    Date d;
    d.year = 2026;
    d.month = 5;
    d.day = 1;
    return d;
}

// helper to find a category id by name
// i loop through all categories and return the one that matches
std::string getCategoryId(TransactionService& txns, std::string name) {
    std::string result = "";
    for (int i = 0; i < txns.getCategories().size(); i++) {
        if (txns.getCategories()[i].name == name) {
            result = txns.getCategories()[i].id;
        }
    }
    return result;
}


// -------------------------------------------------------
// AUTH TESTS
// -------------------------------------------------------

// test 1 - register a new user and check the info is saved
TEST(AuthTest, RegisterUser) {
    AuthService auth;

    User newUser = auth.registerUser("Maya", "maya@aucegypt.edu", "", "", "password123", makeDate());

    EXPECT_EQ(newUser.fullName, "Maya");
    EXPECT_EQ(newUser.email, "maya@aucegypt.edu");
    EXPECT_FALSE(newUser.id.empty());
}

// test 2 - try to register with the same email twice, should throw an error
TEST(AuthTest, DuplicateEmailRejected) {
    AuthService auth;

    auth.registerUser("Maya", "maya@aucegypt.edu", "", "", "password123", makeDate());

    // this second register should fail because email is taken
    EXPECT_THROW(
        auth.registerUser("Maya Copy", "maya@aucegypt.edu", "", "", "pass456", makeDate()),
        std::invalid_argument
    );
}

// test 3 - login with correct email and password should work
TEST(AuthTest, LoginSucceeds) {
    AuthService auth;

    auth.registerUser("Ahmed", "ahmed@aucegypt.edu", "", "", "mypassword", makeDate());

    auto result = auth.login("ahmed@aucegypt.edu", "mypassword");

    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(result->email, "ahmed@aucegypt.edu");
}

// test 4 - login with wrong password should fail
TEST(AuthTest, LoginWrongPassword) {
    AuthService auth;

    auth.registerUser("Mustafa", "mustafa@aucegypt.edu", "", "", "rightpassword", makeDate());

    auto result = auth.login("mustafa@aucegypt.edu", "wrongpassword");

    EXPECT_FALSE(result.has_value());
}


// -------------------------------------------------------
// SESSION TESTS
// -------------------------------------------------------

// test 5 - starting a session should make it active
TEST(SessionTest, SessionIsActiveAfterStart) {
    SessionService sessions;

    Session s = sessions.startSession("usr-1", makeDate());

    EXPECT_TRUE(sessions.isSessionActive(s.token));
}

// test 6 - ending a session should make it inactive
TEST(SessionTest, SessionInactiveAfterEnd) {
    SessionService sessions;

    Session s = sessions.startSession("usr-1", makeDate());
    sessions.endSession(s.token);

    EXPECT_FALSE(sessions.isSessionActive(s.token));
}


// -------------------------------------------------------
// TRANSACTION TESTS
// -------------------------------------------------------

// test 7 - add a transaction and check it was saved correctly
TEST(TransactionTest, AddTransaction) {
    TransactionService txns;

    std::string catId = getCategoryId(txns, "Salary");

    Transaction t;
    t.userId = "usr-1";
    t.title = "May Salary";
    t.categoryId = catId;
    t.type = TransactionType::Income;
    t.amount = 5000.0;
    t.date = makeDate();

    Transaction saved = txns.addTransaction(t);

    EXPECT_EQ(saved.title, "May Salary");
    EXPECT_EQ(saved.amount, 5000.0);
    EXPECT_FALSE(saved.id.empty());
}

// test 8 - update a transaction and check the new values
TEST(TransactionTest, UpdateTransaction) {
    TransactionService txns;

    std::string catId = getCategoryId(txns, "Food");

    Transaction t;
    t.userId = "usr-1";
    t.title = "Old Title";
    t.categoryId = catId;
    t.type = TransactionType::Expense;
    t.amount = 50.0;
    t.date = makeDate();

    Transaction saved = txns.addTransaction(t);

    // change the title and amount
    saved.title = "New Title";
    saved.amount = 99.0;

    Transaction updated = txns.updateTransaction("usr-1", saved.id, saved);

    EXPECT_EQ(updated.title, "New Title");
    EXPECT_EQ(updated.amount, 99.0);
}

// test 9 - delete a transaction and check it is gone
TEST(TransactionTest, DeleteTransaction) {
    TransactionService txns;

    std::string catId = getCategoryId(txns, "Food");

    Transaction t;
    t.userId = "usr-1";
    t.title = "Buy stuff";
    t.categoryId = catId;
    t.type = TransactionType::Expense;
    t.amount = 30.0;
    t.date = makeDate();

    Transaction saved = txns.addTransaction(t);
    txns.deleteTransaction("usr-1", saved.id);

    // list should be empty now
    std::vector<Transaction> list = txns.listTransactions("usr-1");
    EXPECT_EQ(list.size(), 0);
}

// test 10 - add two transactions in different categories
// check that only the right one shows up when i filter
TEST(TransactionTest, FilterByCategory) {
    TransactionService txns;

    std::string foodId = getCategoryId(txns, "Food");
    std::string transportId = getCategoryId(txns, "Transport");

    Transaction t1;
    t1.userId = "usr-1";
    t1.title = "Lunch";
    t1.categoryId = foodId;
    t1.type = TransactionType::Expense;
    t1.amount = 50.0;
    t1.date = makeDate();
    txns.addTransaction(t1);

    Transaction t2;
    t2.userId = "usr-1";
    t2.title = "Uber";
    t2.categoryId = transportId;
    t2.type = TransactionType::Expense;
    t2.amount = 30.0;
    t2.date = makeDate();
    txns.addTransaction(t2);

    // manually count food transactions
    std::vector<Transaction> all = txns.listTransactions("usr-1");
    int foodCount = 0;
    for (int i = 0; i < all.size(); i++) {
        if (all[i].categoryId == foodId) {
            foodCount++;
        }
    }

    EXPECT_EQ(foodCount, 1);
}


// -------------------------------------------------------
// BUDGET TESTS
// -------------------------------------------------------

// test 11 - create a budget and check the limit is saved
TEST(BudgetTest, CreateBudget) {
    TransactionService txns;
    BudgetService budgets;

    std::string foodId = getCategoryId(txns, "Food");

    YearMonth period;
    period.year = 2026;
    period.month = 5;

    Budget b = budgets.createBudget("usr-1", foodId, period, 1000.0);

    EXPECT_EQ(b.limit, 1000.0);
    EXPECT_EQ(b.period.month, 5);
}

// test 12 - spend more than the budget limit
// check that it shows as overspent
TEST(BudgetTest, OverspendingDetected) {
    TransactionService txns;
    BudgetService budgets;

    std::string foodId = getCategoryId(txns, "Food");

    YearMonth period;
    period.year = 2026;
    period.month = 5;

    budgets.createBudget("usr-1", foodId, period, 100.0);

    // spend 150 which is over the 100 limit
    Transaction t;
    t.userId = "usr-1";
    t.title = "Big dinner";
    t.categoryId = foodId;
    t.type = TransactionType::Expense;
    t.amount = 150.0;
    t.date = makeDate();
    txns.addTransaction(t);

    std::vector<BudgetStatus> statuses = budgets.summarizeBudgets("usr-1", period, txns);

    EXPECT_EQ(statuses.size(), 1);
    EXPECT_TRUE(statuses[0].overspent);
    EXPECT_EQ(statuses[0].spent, 150.0);
}


// -------------------------------------------------------
// SAVINGS TESTS
// -------------------------------------------------------

// test 13 - add a deposit and check it was saved
TEST(SavingsTest, AddDeposit) {
    SavingsService savings;

    SavingsEntry entry = savings.addDeposit("usr-1", 500.0, makeDate(), "first deposit");

    EXPECT_EQ(entry.amount, 500.0);
    EXPECT_EQ(entry.type, SavingsEntryType::Deposit);
}

// test 14 - add a withdrawal and check it was saved
TEST(SavingsTest, AddWithdrawal) {
    SavingsService savings;

    savings.addDeposit("usr-1", 500.0, makeDate(), "deposit");
    SavingsEntry w = savings.addWithdrawal("usr-1", 200.0, makeDate(), "rent");

    EXPECT_EQ(w.amount, 200.0);
    EXPECT_EQ(w.type, SavingsEntryType::Withdrawal);
}

// test 15 - deposit 1000, withdraw 300, deposit 200
// balance should be 900
TEST(SavingsTest, BalanceIsCorrect) {
    SavingsService savings;

    savings.addDeposit("usr-1", 1000.0, makeDate(), "salary");
    savings.addWithdrawal("usr-1", 300.0, makeDate(), "groceries");
    savings.addDeposit("usr-1", 200.0, makeDate(), "side job");

    YearMonth period;
    period.year = 2026;
    period.month = 5;

    SavingsOverview overview = savings.summarize("usr-1", period);

    EXPECT_EQ(overview.currentBalance, 900.0);
}


// -------------------------------------------------------
// GOAL TESTS
// -------------------------------------------------------

// test 16 - create a goal and check info is saved
TEST(GoalTest, CreateGoal) {
    GoalService goals;

    Goal g;
    g.userId = "usr-1";
    g.title = "Buy a laptop";
    g.targetAmount = 2000.0;
    g.currentAmount = 0.0;
    g.targetDate = makeDate();

    Goal saved = goals.createGoal(g);

    EXPECT_EQ(saved.title, "Buy a laptop");
    EXPECT_EQ(saved.targetAmount, 2000.0);
    EXPECT_FALSE(saved.completed);
}

// test 17 - update how much i saved toward the goal
TEST(GoalTest, UpdateProgress) {
    GoalService goals;

    Goal g;
    g.userId = "usr-1";
    g.title = "Vacation fund";
    g.targetAmount = 3000.0;
    g.currentAmount = 0.0;
    g.targetDate = makeDate();

    Goal saved = goals.createGoal(g);
    Goal updated = goals.updateProgress("usr-1", saved.id, 1000.0);

    EXPECT_EQ(updated.currentAmount, 1000.0);
    EXPECT_FALSE(updated.completed);
}

// test 18 - when current amount reaches target, goal should be completed
TEST(GoalTest, GoalCompletedWhenTargetReached) {
    GoalService goals;

    Goal g;
    g.userId = "usr-1";
    g.title = "Small goal";
    g.targetAmount = 500.0;
    g.currentAmount = 0.0;
    g.targetDate = makeDate();

    Goal saved = goals.createGoal(g);
    Goal updated = goals.updateProgress("usr-1", saved.id, 500.0);

    EXPECT_TRUE(updated.completed);
}


// -------------------------------------------------------
// REPORT TEST
// -------------------------------------------------------

// test 19 - add income and expenses then generate a report
// check that totals and transaction count are correct
TEST(ReportTest, GenerateReport) {
    TransactionService txns;
    BudgetService budgets;
    ReportService reports;

    std::string salaryId = getCategoryId(txns, "Salary");
    std::string foodId = getCategoryId(txns, "Food");

    // add one income
    Transaction income;
    income.userId = "usr-1";
    income.title = "Salary";
    income.categoryId = salaryId;
    income.type = TransactionType::Income;
    income.amount = 5000.0;
    income.date = makeDate();
    txns.addTransaction(income);

    // add one expense
    Transaction expense;
    expense.userId = "usr-1";
    expense.title = "Groceries";
    expense.categoryId = foodId;
    expense.type = TransactionType::Expense;
    expense.amount = 800.0;
    expense.date = makeDate();
    txns.addTransaction(expense);

    ReportRequest req;
    req.userId = "usr-1";
    req.from = makeDate();
    req.to = makeDate();

    FinancialReport report = reports.generateReport(req, txns, budgets);

    EXPECT_EQ(report.totalIncome, 5000.0);
    EXPECT_EQ(report.totalExpenses, 800.0);
    EXPECT_EQ(report.net, 4200.0);
    EXPECT_EQ(report.transactions.size(), 2);
}


// -------------------------------------------------------
// PERSISTENCE TEST
// -------------------------------------------------------

// test 20 - save data to a file, load it back, check it is still there
TEST(PersistenceTest, SaveAndLoad) {
    std::filesystem::path testFolder = "/tmp/finsight_test_save";
    std::filesystem::remove_all(testFolder);
    std::filesystem::create_directories(testFolder);

    finsight::data::storage::BackendStore store;

    // save step - register a user and add a transaction
    {
        finsight::core::managers::FinanceTrackerBackend backend;

        User u = backend.auth().registerUser(
            "Moataz", "moataz@aucegypt.edu", "", "", "pass1234", makeDate());

        std::string foodId = "";
        for (int i = 0; i < backend.transactions().getCategories().size(); i++) {
            if (backend.transactions().getCategories()[i].name == "Food") {
                foodId = backend.transactions().getCategories()[i].id;
            }
        }

        Transaction t;
        t.userId = u.id;
        t.title = "Lunch";
        t.categoryId = foodId;
        t.type = TransactionType::Expense;
        t.amount = 50.0;
        t.date = makeDate();
        backend.transactions().addTransaction(t);

        store.save(backend, testFolder);
    }

    // load step - create a fresh backend and load the saved data
    {
        finsight::core::managers::FinanceTrackerBackend backend;
        store.load(backend, testFolder);

        std::vector<User> users = backend.auth().listUsers();
        EXPECT_EQ(users.size(), 1);
        EXPECT_EQ(users[0].email, "moataz@aucegypt.edu");

        std::vector<Transaction> txns = backend.transactions().listTransactions(users[0].id);
        EXPECT_EQ(txns.size(), 1);
        EXPECT_EQ(txns[0].title, "Lunch");
        EXPECT_EQ(txns[0].amount, 50.0);
    }

    std::filesystem::remove_all(testFolder);
}


// -------------------------------------------------------
// MOCK TEST
// -------------------------------------------------------

// this tests the message handler interface using a fake/mock version
class MockMessageHandler : public IMessageHandler {
public:
    MOCK_METHOD(std::string, handle, (const std::string& json), (override));
};

TEST(MockTest, PingReturnsCorrectResponse) {
    MockMessageHandler mock;

    // tell the mock what to expect and what to return
    EXPECT_CALL(mock, handle("{\"command\":\"ping\"}"))
        .Times(1)
        .WillOnce(testing::Return("{\"status\":\"ok\",\"message\":\"pong\"}"));

    std::string response = mock.handle("{\"command\":\"ping\"}");

    EXPECT_EQ(response, "{\"status\":\"ok\",\"message\":\"pong\"}");
}
