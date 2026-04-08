#ifndef REGISTERDIALOG_H
#define REGISTERDIALOG_H

#include <QDialog>

class QLineEdit;
class QComboBox;

class RegisterDialog : public QDialog {
    Q_OBJECT

public:
    explicit RegisterDialog(QWidget *parent = nullptr);

    QString fullName() const;
    QString email() const;
    QString phone() const;
    QString gender() const;
    QString password() const;

protected:
    void accept() override;

private:
    QLineEdit *fullNameEdit;
    QLineEdit *emailEdit;
    QLineEdit *phoneEdit;
    QComboBox *genderCombo;
    QLineEdit *passwordEdit;

    void setupUi();
};

#endif
