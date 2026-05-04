#ifndef TRANSACTIONDIALOG_H
#define TRANSACTIONDIALOG_H

#include "core/managers/FinanceTrackerBackend.h"
#include "core/models/Category.h"
#include "core/models/Transaction.h"

#include <QDialog>
#include <QString>
#include <vector>

class QLineEdit;
class QComboBox;
class QDateEdit;
class QDoubleSpinBox;
class QTextEdit;
class QPushButton;

class TransactionDialog : public QDialog {
    Q_OBJECT

public:
    explicit TransactionDialog(finsight::core::managers::FinanceTrackerBackend& backend,
                               const std::string& userId,
                               QWidget *parent = nullptr);

    QString title() const;
    QString type() const;
    QString category() const;
    QString merchant() const;
    QString date() const;
    double amount() const;
    QString description() const;
    std::string categoryId() const;
    finsight::core::models::TransactionType transactionType() const;
    bool categoriesChanged() const;

    void setAvailableCategories(const std::vector<finsight::core::models::Category>& categories);

    void setDialogTitle(const QString &text);
    void setTitle(const QString &value);
    void setType(const QString &value);
    void setCategory(const QString &value);
    void setCategoryId(const std::string& value);
    void setMerchant(const QString &value);
    void setDate(const QString &value);
    void setAmount(double value);
    void setDescription(const QString &value);

protected:
    void accept() override;

private:
    finsight::core::managers::FinanceTrackerBackend& backend_;
    std::string userId_;
    QLineEdit *titleEdit;
    QLineEdit *merchantEdit;
    QComboBox *typeCombo;
    QComboBox *categoryCombo;
    QPushButton *addCategoryButton;
    QDateEdit *dateEdit;
    QDoubleSpinBox *amountSpin;
    QTextEdit *descriptionEdit;
    std::vector<finsight::core::models::Category> categories_;
    bool categoriesChanged_ {false};

    void setupUi();
    void refreshCategoryChoices();
    void addCategory();
};

#endif
