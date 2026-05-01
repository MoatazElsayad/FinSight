#ifndef REPORTSWINDOW_H
#define REPORTSWINDOW_H
#include "core/managers/FinanceTrackerBackend.h"
#include <QWidget>
class QDateEdit; class QTextEdit; class QLabel;
class ReportsWindow: public QWidget{
    Q_OBJECT
public: explicit ReportsWindow(finsight::core::managers::FinanceTrackerBackend& backend,const std::string& userId,QWidget* parent=nullptr); void setUserId(const std::string& userId); void refreshData(); private: finsight::core::managers::FinanceTrackerBackend& backend_; std::string userId_; QDateEdit* fromDate; QDateEdit* toDate; QLabel* summary; QTextEdit* details; std::string exported_; void setupUi();};
#endif
