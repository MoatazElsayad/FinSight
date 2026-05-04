#include "network/tcp/BackendMessageHandler.h"
#include "network/protocol/JsonSerializer.h"
#include <sstream>

using finsight::network::protocol::JsonSerializer;
namespace models = finsight::core::models;
BackendMessageHandler::BackendMessageHandler(
    finsight::core::managers::FinanceTrackerBackend& backend)
    : backend_(backend) {}

std::string BackendMessageHandler::handle(const std::string& json) {
    if (json.empty()) return err("Empty message");
    std::string cmd = getStr(json, "command");

    if (cmd == "ping")
        return "{\"status\":\"ok\",\"message\":\"pong\"}";

    if (cmd == "register") {
        try {
            auto user = backend_.auth().registerUser(
                getStr(json, "full_name"),
                getStr(json, "email"),
                "", "",
                getStr(json, "password"),
                models::Date{2026, 5, 1});
            return "{\"status\":\"ok\",\"user_id\":\"" + user.id + "\"}";
        } catch (const std::exception& e) { return err(e.what()); }
    }

    if (cmd == "login") {
        auto user = backend_.auth().login(
            getStr(json, "email"), getStr(json, "password"));
        if (user)
            return "{\"status\":\"ok\",\"user_id\":\"" + user->id + "\"}";
        return err("Invalid email or password");
    }

    if (cmd == "list_transactions") {
        auto txns = backend_.transactions().listTransactions(getStr(json, "user_id"));
        std::ostringstream out;
        out << "{\"status\":\"ok\",\"count\":" << txns.size() << "}";
        return out.str();
    }

    if (cmd == "dashboard_summary") {
        std::string uid = getStr(json, "user_id");
        models::YearMonth period{2026, 5};
        double income   = backend_.transactions().sumTransactions(
                              uid, models::TransactionType::Income,  period);
        double expenses = backend_.transactions().sumTransactions(
                              uid, models::TransactionType::Expense, period);
        std::ostringstream out;
        out << "{\"status\":\"ok\""
            << ",\"income\":"   << income
            << ",\"expenses\":" << expenses
            << ",\"net\":"      << (income - expenses) << "}";
        return out.str();
    }

    return err("Unknown command: " + cmd);
}

std::string BackendMessageHandler::getStr(const std::string& json,
                                          const std::string& key) {
    return JsonSerializer::getString(json, key);
}

std::string BackendMessageHandler::err(const std::string& msg) {
    return "{\"status\":\"error\",\"message\":\"" + msg + "\"}";
}
