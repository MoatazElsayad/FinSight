#ifndef TRANSACTIONSWINDOW_H
#define TRANSACTIONSWINDOW_H

#include <QWidget>

class QTableWidget;
class QLineEdit;
class QComboBox;
class QDateEdit;
class QPushButton;

class TransactionsWindow : public QWidget {
    Q_OBJECT

public:
    explicit TransactionsWindow(QWidget *parent = nullptr);

private:
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
    void loadDummyData();
    void onAddTransaction();
    void onEditTransaction();
    void addTransactionRow(const QString &date,
                           const QString &title,
                           const QString &type,
                           const QString &category,
                           const QString &amountText);
};

#endif
