#include <QApplication>
#include <filesystem>
#include <iostream>
#include <vector>

#include "core/models/AI.h"
#include "core/models/Email.h"
#include "core/utils/EnvLoader.h"
#include "gui/MainWindow.h"
#include "data/storage/BackendStore.h"

using namespace finsight::core::managers;
using namespace finsight::core::models;
using namespace finsight::core::utils;
using namespace finsight::data::storage;
using namespace std;

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    bool envLoaded = false;
    const vector<string> possibleEnvPaths = {
        ".env",
        "./.env",
        "../.env",
        "../../.env",
        "../../../.env",
        "/mnt/c/Users/moael/Programming/FinSight/.env",
        "C:\\Users\\moael\\Programming\\FinSight\\.env"
    };
    for (const auto& path : possibleEnvPaths) {
        if (EnvLoader::loadFile(path)) {
            envLoaded = true;
            cout << "Loaded .env from: " << path << endl;
            break;
        }
    }
    if (!envLoaded) {
        cout << "Trying loadFromNearestFile..." << endl;
        EnvLoader::loadFromNearestFile();
    }

    const string apiKey = EnvLoader::get("FINSIGHT_OPENROUTER_API_KEY", "PASTE_REAL_API_KEY_HERE");
    if (apiKey == "PASTE_REAL_API_KEY_HERE") {
        cout << "Warning: API key not loaded from .env file. Using placeholder." << endl;
    }
    const string apiUrl = EnvLoader::get("FINSIGHT_OPENROUTER_API_URL", "https://openrouter.ai/api/v1/chat/completions");
    const string primaryModel = EnvLoader::get("FINSIGHT_OPENROUTER_MODEL", "mistralai/mistral-small-3.1-24b-instruct:free");
    const bool emailEnabled = EnvLoader::getBool("FINSIGHT_EMAIL_ENABLED", false);
    const string emailApiUrl = EnvLoader::get("FINSIGHT_RESEND_API_URL", "https://api.resend.com/emails");
    const string emailApiKey = EnvLoader::get("FINSIGHT_RESEND_API_KEY", "PASTE_REAL_RESEND_API_KEY_HERE");
    const string emailFromEmail = EnvLoader::get("FINSIGHT_RESEND_FROM_EMAIL");
    const string emailFromName = EnvLoader::get("FINSIGHT_RESEND_FROM_NAME", "FinSight");

    FinanceTrackerBackend backend;
    backend.ai().configure(AIProviderConfig {
        .apiUrl = apiUrl,
        .apiKey = apiKey,
        .model = primaryModel,
        .fallbackModels = {
            "mistralai/mistral-small-3.1-24b-instruct:free",
            "meta-llama/llama-3.3-70b-instruct:free",
            "deepseek/deepseek-r1-0528:free",
            "qwen/qwen3-coder:free",
            "google/gemma-3-27b-it:free",
            "mistralai/mistral-7b-instruct:free",
            "qwen/qwen-2.5-vl-7b-instruct:free",
            "liquid/lfm-2.5-1.2b-instruct:free",
            "cognitivecomputations/dolphin-mistral-24b-venice-edition:free",
            "meta-llama/llama-3.2-3b-instruct:free"
        },
        .appName = "FinSight",
        .appUrl = "https://example.com/finsight",
    });
    backend.email().configure(EmailProviderConfig {
        .enabled = emailEnabled,
        .apiUrl = emailApiUrl,
        .apiKey = emailApiKey,
        .fromEmail = emailFromEmail,
        .fromName = emailFromName,
    });

    BackendStore store;
    const auto exeDir = filesystem::path(QCoreApplication::applicationDirPath().toStdString());
    const auto projectRoot = exeDir.parent_path();
    const filesystem::path preferredDirectory = projectRoot / "runtime_data_terminal_demo";
    const filesystem::path fallbackDirectory = projectRoot / "runtime_data";
    const filesystem::path loadDirectory = filesystem::exists(store.databasePath(preferredDirectory)) ? preferredDirectory : fallbackDirectory;
    cout << "Loading persistence from: " << loadDirectory << endl;
    store.load(backend, loadDirectory);

    MainWindow window(backend);
    window.show();

    QObject::connect(&app, &QApplication::aboutToQuit, [&backend, &store, &preferredDirectory]() {
        store.save(backend, preferredDirectory);
    });

    return app.exec();
}
