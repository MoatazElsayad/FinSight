#ifndef RECEIPTSWINDOW_H
#define RECEIPTSWINDOW_H
#include "core/managers/FinanceTrackerBackend.h"
#include <QWidget>
class QLineEdit; class QTextEdit; class QTableWidget; class QComboBox; class QDoubleSpinBox; class QDateEdit;
class ReceiptsWindow: public QWidget{
    Q_OBJECT
public: explicit ReceiptsWindow(finsight::core::managers::FinanceTrackerBackend& backend,const std::string& userId,QWidget* parent=nullptr); void setUserId(const std::string& userId); void refreshData(); signals: void dataChanged(); private: finsight::core::managers::FinanceTrackerBackend& backend_; std::string userId_; QLineEdit* fileEdit; QTextEdit* rawText; QTableWidget* table; QComboBox* categoryCombo; QDoubleSpinBox* amountSpin; QDateEdit* dateEdit; QLineEdit* titleEdit; std::string parsedReceiptId_; void setupUi();};
#endif
