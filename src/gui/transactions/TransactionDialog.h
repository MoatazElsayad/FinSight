#ifndef TRANSACTIONDIALOG_H
#define TRANSACTIONDIALOG_H

#include <QDialog>
#include <QString>

class QLineEdit;
class QComboBox;
class QDateEdit;
class QDoubleSpinBox;
class QTextEdit;

class TransactionDialog : public QDialog {
    Q_OBJECT

public:
    explicit TransactionDialog(QWidget *parent = nullptr);

    QString title() const;
    QString type() const;
    QString category() const;
    QString date() const;
    double amount() const;
    QString description() const;

    void setDialogTitle(const QString &text);
    void setTitle(const QString &value);
    void setType(const QString &value);
    void setCategory(const QString &value);
    void setDate(const QString &value);
    void setAmount(double value);
    void setDescription(const QString &value);

protected:
    void accept() override;

private:
    QLineEdit *titleEdit;
    QComboBox *typeCombo;
    QComboBox *categoryCombo;
    QDateEdit *dateEdit;
    QDoubleSpinBox *amountSpin;
    QTextEdit *descriptionEdit;

    void setupUi();
};

#endif
