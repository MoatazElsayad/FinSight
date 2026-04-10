#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include "core/managers/FinanceTrackerBackend.h"
#include <QWidget>

class QLineEdit;
class QString;

class LoginDialog : public QWidget {
    Q_OBJECT

public:
    explicit LoginDialog(finsight::core::managers::FinanceTrackerBackend& backend, QWidget *parent = nullptr);

    QString email() const;
    QString password() const;
    bool wantsRegister() const;

signals:
    void loginSuccessful(const QString& userId);
    void registerRequested();

private:
    finsight::core::managers::FinanceTrackerBackend& backend_;
    QLineEdit *emailEdit;
    QLineEdit *passwordEdit;
    bool wantsRegister_ {false};

    void setupUi();
    void attemptLogin();
};

#endif
