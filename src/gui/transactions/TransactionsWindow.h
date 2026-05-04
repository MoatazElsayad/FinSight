#ifndef TRANSACTIONSWINDOW_H
#define TRANSACTIONSWINDOW_H

#include "core/managers/FinanceTrackerBackend.h"

#include <QWidget>
#include <exception>
#include <optional>

class QTableWidget;
class QLineEdit;
class QComboBox;
class QDateEdit;
class QPushButton;
class QString;

class TransactionsWindow : public QWidget {
    Q_OBJECT

public:
    explicit TransactionsWindow(finsight::core::managers::FinanceTrackerBackend& backend,
                                const std::string& userId,
                                QWidget *parent = nullptr);

    void setUserId(const std::string& userId);
    void refreshData();

signals:
    void dataChanged();

private:
    finsight::core::managers::FinanceTrackerBackend& backend_;
    std::string userId_;

    QLineEdit *searchEdit;
    QComboBox *typeFilter;
    QComboBox *categoryFilter;
    QDateEdit *fromDateEdit;
    QDateEdit *toDateEdit;

    QTableWidget *transactionsTable;

    QPushButton *addButton;
    QPushButton *editButton;
    QPushButton *deleteButton;
    QPushButton *clearFiltersButton;

    void setupUi();
    void populateCategoryFilter();
    void onAddTransaction();
    void onEditTransaction();
    void onDeleteTransaction();
    std::optional<finsight::core::models::Transaction> selectedTransaction();
    bool ensureSignedIn(const QString& actionTitle);
    void showOperationError(const QString& actionTitle, const std::exception& error);
};

#endif
