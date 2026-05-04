#ifndef RECEIPTSWINDOW_H
#define RECEIPTSWINDOW_H

#include "core/managers/FinanceTrackerBackend.h"

#include <QWidget>
#include <string>

class QComboBox;
class QDateEdit;
class QDoubleSpinBox;
class QLabel;
class QLineEdit;
class QTableWidget;
class QTextEdit;

class ReceiptsWindow : public QWidget {
    Q_OBJECT

public:
    explicit ReceiptsWindow(finsight::core::managers::FinanceTrackerBackend& backend,
                            const std::string& userId,
                            QWidget* parent = nullptr);

    void setUserId(const std::string& userId);
    void refreshData();

signals:
    void dataChanged();

private:
    finsight::core::managers::FinanceTrackerBackend& backend_;
    std::string userId_;

    QLineEdit* fileEdit {nullptr};
    QTextEdit* rawTextEdit {nullptr};
    QLabel* parsedSummaryLabel {nullptr};
    QLabel* parsedMerchantValue {nullptr};
    QLabel* parsedAmountValue {nullptr};
    QLabel* parsedDateValue {nullptr};
    QLabel* parsedNotesValue {nullptr};
    QLineEdit* titleEdit {nullptr};
    QLineEdit* merchantEdit {nullptr};
    QComboBox* categoryCombo {nullptr};
    QDoubleSpinBox* amountSpin {nullptr};
    QDateEdit* dateEdit {nullptr};
    QTableWidget* receiptsTable {nullptr};
    std::string parsedReceiptId_;

    void setupUi();
    void browseReceiptFile();
    void uploadAndParse();
    void confirmTransaction();
    void refreshCategoryCombo();
    void selectCategory(const std::string& categoryId);
};

#endif
