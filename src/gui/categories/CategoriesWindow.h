#ifndef CATEGORIESWINDOW_H
#define CATEGORIESWINDOW_H
#include "core/managers/FinanceTrackerBackend.h"
#include <QWidget>
class QTableWidget; class QLineEdit; class QComboBox; class QPushButton;
class CategoriesWindow: public QWidget {
    Q_OBJECT
public: explicit CategoriesWindow(finsight::core::managers::FinanceTrackerBackend& backend,const std::string& userId,QWidget* parent=nullptr);
void setUserId(const std::string& userId); void refreshData();
signals: void dataChanged();
private: finsight::core::managers::FinanceTrackerBackend& backend_; std::string userId_; QTableWidget* table; QLineEdit* nameEdit; QComboBox* kindCombo; QLineEdit* iconEdit; QPushButton* addButton; QPushButton* deleteButton; void setupUi(); };
#endif
