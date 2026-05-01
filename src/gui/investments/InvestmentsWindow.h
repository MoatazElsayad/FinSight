#ifndef INVESTMENTSWINDOW_H
#define INVESTMENTSWINDOW_H
#include "core/managers/FinanceTrackerBackend.h"
#include <QWidget>
class QTableWidget; class QLineEdit; class QComboBox; class QDoubleSpinBox; class QDateEdit;
class InvestmentsWindow: public QWidget{
    Q_OBJECT
public: explicit InvestmentsWindow(finsight::core::managers::FinanceTrackerBackend& backend,const std::string& userId,QWidget* parent=nullptr); void setUserId(const std::string& userId); void refreshData(); signals: void dataChanged(); private: finsight::core::managers::FinanceTrackerBackend& backend_; std::string userId_; QTableWidget* table; QLineEdit* assetEdit; QLineEdit* symbolEdit; QComboBox* typeCombo; QDoubleSpinBox* qtySpin; QDoubleSpinBox* buySpin; QDoubleSpinBox* curSpin; QDateEdit* dateEdit; void setupUi();};
#endif
