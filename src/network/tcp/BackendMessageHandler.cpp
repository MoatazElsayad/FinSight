#include "network/tcp/BackendMessageHandler.h"
#include "network/protocol/JsonSerializer.h"
#include <iostream>
#include <string>

using namespace finsight::core::models;
using finsight::network::protocol::JsonSerializer;

BackendMessageHandler::BackendMessageHandler(
    finsight::core::managers::FinanceTrackerBackend& backend)
    : backend_(backend) {}

// this function gets called every time the server receives a message
// it reads the command and calls the right backend function
std::string BackendMessageHandler::handle(std::string json) {

    // check if the message is empty
    if (json.empty()) {
        return err("empty message");
    }

    // check if it looks like json (should start with {)
    if (json[0] != '{') {
        return err("message should be JSON like {\"command\":\"ping\"}");
    }

    // get the command field from the json
    std::string command = getStr(json, "command");

    if (command.empty()) {
        return err("missing command field");
    }

    std::cout << "[server] got command: " << command << std::endl;

    // loz - just check if server is alive
    if (command == "loz") {return "{\"status\":\"ok\",\"message\":\"goz\"}";
    }

// help - returns a list of all available commands
    if (command == "help") {
        std::string response = "{\"status\":\"ok\",\"commands\":[";
        response += "\"ping\",";
        response += "\"help\",";
        response += "\"register\",";
        response += "\"login\",";
        response += "\"add_transaction\",";
        response += "\"list_transactions\",";
        response += "\"dashboard_summary\",";
        response += "\"generate_report\"";
        response += "]}";
        return response;
    }

    // register - create a new user account
    if (command == "register") {
        std::string name = getStr(json, "full_name");
        std::string email = getStr(json, "email");
        std::string password = getStr(json, "password");

        if (email.empty() || password.empty()) {
            return err("register needs email and password");
        }

        try {
            Date today;
            today.year = 2026;
            today.month = 5;
            today.day = 1;

            User newUser = backend_.auth().registerUser(
                name, email, "", "", password, today);

            return "{\"status\":\"ok\",\"user_id\":\"" + newUser.id + "\"}";

        } catch (std::exception& e) {
            return err(e.what());
        }
    }

    // login - check email and password
    if (command == "login") {
        std::string email = getStr(json, "email");
        std::string password = getStr(json, "password");

        if (email.empty() || password.empty()) {
            return err("login needs email and password");
        }

        auto result = backend_.auth().login(email, password);

        if (result.has_value()) {
            return "{\"status\":\"ok\",\"user_id\":\"" + result->id + "\"}";
        } else {
            return err("wrong email or password");
        }
    }

    // add_transaction - add a new income or expense
    if (command == "add_transaction") {
        std::string userId = getStr(json, "user_id");
        std::string title = getStr(json, "title");
        std::string categoryId = getStr(json, "category_id");
        std::string typeStr = getStr(json, "type");
        std::string amountStr = getStr(json, "amount");

        if (userId.empty() || title.empty() || categoryId.empty() || amountStr.empty()) {
            return err("add_transaction needs user_id, title, category_id, type, amount");
        }

        double amount = 0.0;
        try {
            amount = std::stod(amountStr);
        } catch (...) {
            return err("amount must be a number like 50.0");
        }

        Transaction t;
        t.userId = userId;
        t.title = title;
        t.categoryId = categoryId;
        t.amount = amount;
        t.date = Date{2026, 5, 1};

        if (typeStr == "income") {
            t.type = TransactionType::Income;
        } else {
            t.type = TransactionType::Expense;
        }

        try {
            Transaction saved = backend_.transactions().addTransaction(t);
            return "{\"status\":\"ok\",\"transaction_id\":\"" + saved.id + "\"}";
        } catch (std::exception& e) {
            return err(e.what());
        }
    }

    // list_transactions - get all transactions for a user
    if (command == "list_transactions") {
        std::string userId = getStr(json, "user_id");

        if (userId.empty()) {
            return err("list_transactions needs user_id");
        }

        std::vector<Transaction> list = backend_.transactions().listTransactions(userId);

        // build the response manually
        std::string response = "{\"status\":\"ok\",\"count\":";
        response += std::to_string(list.size());
        response += ",\"transactions\":[";

        for (int i = 0; i < list.size(); i++) {
            if (i > 0) {
                response += ",";
            }
            response += "{";
            response += "\"id\":\"" + list[i].id + "\",";
            response += "\"title\":\"" + list[i].title + "\",";
            response += "\"amount\":" + std::to_string(list[i].amount);
            response += "}";
        }

        response += "]}";
        return response;
    }

    // dashboard_summary - get income, expenses, net for the month
    if (command == "dashboard_summary") {
        std::string userId = getStr(json, "user_id");

        if (userId.empty()) {
            return err("dashboard_summary needs user_id");
        }

        YearMonth period;
        period.year = 2026;
        period.month = 5;

        double income = backend_.transactions().sumTransactions(
            userId, TransactionType::Income, period);

        double expenses = backend_.transactions().sumTransactions(
            userId, TransactionType::Expense, period);

        double net = income - expenses;

        std::string response = "{\"status\":\"ok\"";
        response += ",\"income\":" + std::to_string(income);
        response += ",\"expenses\":" + std::to_string(expenses);
        response += ",\"net\":" + std::to_string(net);
        response += "}";

        return response;
    }

    // generate_report - full report for the month
    if (command == "generate_report") {
        std::string userId = getStr(json, "user_id");

        if (userId.empty()) {
            return err("generate_report needs user_id");
        }

        ReportRequest req;
        req.userId = userId;
        req.from = Date{2026, 5, 1};
        req.to = Date{2026, 5, 31};

        try {
            FinancialReport report = backend_.reports().generateReport(
                req, backend_.transactions(), backend_.budgets());

            std::string response = "{\"status\":\"ok\"";
            response += ",\"total_income\":" + std::to_string(report.totalIncome);
            response += ",\"total_expenses\":" + std::to_string(report.totalExpenses);
            response += ",\"net\":" + std::to_string(report.net);
            response += ",\"transaction_count\":" + std::to_string(report.transactions.size());
            response += "}";

            return response;

        } catch (std::exception& e) {
            return err(e.what());
        }
    }

    // if we get here the command was not recognized
    return err("unknown command: " + command);
}

// helper - get a string value from a simple json message
std::string BackendMessageHandler::getStr(std::string json, std::string key) {
    return JsonSerializer::getString(json, key);
}

// helper - build an error response
std::string BackendMessageHandler::err(std::string message) {
    return "{\"status\":\"error\",\"message\":\"" + message + "\"}";
}
