#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>

class QLineEdit;

class LoginDialog : public QDialog {
    Q_OBJECT

public:
    explicit LoginDialog(QWidget *parent = nullptr);

    QString email() const;
    QString password() const;
    bool wantsRegister() const;

private:
    QLineEdit *emailEdit;
    QLineEdit *passwordEdit;
    bool wantsRegister_ {false};

    void setupUi();
};

#endif
