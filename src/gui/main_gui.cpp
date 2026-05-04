#include <QApplication>
#include <QCoreApplication>
#include <filesystem>
#include <string>

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

    if (!EnvLoader::loadFromNearestFile()) {
        const auto exeDir = filesystem::path(QCoreApplication::applicationDirPath().toStdString());
        EnvLoader::loadFromNearestFile(exeDir);
    }

    const string apiKey = EnvLoader::get("OPENROUTER_API_KEY", "");
    const string apiUrl = EnvLoader::get("OPENROUTER_API_URL", "https://openrouter.ai/api/v1/chat/completions");
    const bool emailEnabled = EnvLoader::getBool("EMAIL_ENABLED", false);
    const string emailApiUrl = EnvLoader::get("EMAIL_API_URL", "https://api.resend.com/emails");
    const string emailApiKey = EnvLoader::get("EMAIL_API_KEY", "");
    const string emailFromEmail = EnvLoader::get("EMAIL_FROM_EMAIL");
    const string emailFromName = EnvLoader::get("EMAIL_FROM_NAME", "FinSight");

    FinanceTrackerBackend backend;
    backend.ai().configure(AIProviderConfig {
        .apiUrl = apiUrl,
        .apiKey = apiKey,
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
    const filesystem::path dataDirectory = projectRoot / "runtime_data";
    store.load(backend, dataDirectory);

    MainWindow window(backend, &store, dataDirectory);
    window.show();

    QObject::connect(&app, &QApplication::aboutToQuit, [&backend, &store, &dataDirectory]() {
        store.save(backend, dataDirectory);
    });

    return app.exec();
}
