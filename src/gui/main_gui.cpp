#include <QApplication>
#include <filesystem>

#include "gui/MainWindow.h"
#include "data/storage/BackendStore.h"

using namespace finsight::core::managers;
using namespace finsight::core::models;
using namespace finsight::data::storage;
using namespace std;

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    FinanceTrackerBackend backend;
    BackendStore store;
    const filesystem::path preferredDirectory {"runtime_data_terminal_demo"};
    const filesystem::path fallbackDirectory {"runtime_data"};
    if (filesystem::exists(store.databasePath(preferredDirectory))) {
        store.load(backend, preferredDirectory);
    } else {
        store.load(backend, fallbackDirectory);
    }

    MainWindow window(backend);
    if (!window.promptForAuthentication()) {
        return 0;
    }
    window.show();

    QObject::connect(&app, &QApplication::aboutToQuit, [&backend, &store, &preferredDirectory]() {
        store.save(backend, preferredDirectory);
    });

    return app.exec();
}
