#include "gui/profile/ProfileWindow.h"

#include <QComboBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

ProfileWindow::ProfileWindow(finsight::core::managers::FinanceTrackerBackend& backend,
                             const std::string& userId,
                             QWidget *parent)
    : QWidget(parent),
      backend_(backend),
      userId_(userId) {
    setupUi();
    refreshData();
}

void ProfileWindow::setUserId(const std::string& userId) {
    userId_ = userId;
}

void ProfileWindow::refreshData() {
    if (userId_.empty()) {
        fullNameEdit->clear();
        phoneEdit->clear();
        emailValueLabel->setText("No user is signed in.");
        createdAtValueLabel->clear();
        return;
    }

    const auto& user = backend_.auth().getUser(userId_);
    fullNameEdit->setText(QString::fromStdString(user.fullName));
    phoneEdit->setText(QString::fromStdString(user.phone));
    emailValueLabel->setText(QString::fromStdString(user.email));
    createdAtValueLabel->setText(QString::fromStdString(user.createdAt.toString()));
    const int genderIndex = genderCombo->findText(QString::fromStdString(user.gender));
    genderCombo->setCurrentIndex(genderIndex >= 0 ? genderIndex : 0);
}

void ProfileWindow::setupUi() {
    auto *mainLayout = new QVBoxLayout(this);

    auto *titleLabel = new QLabel("Profile");
    titleLabel->setStyleSheet("font-size: 20px; font-weight: bold;");
    mainLayout->addWidget(titleLabel);

    auto *formLayout = new QFormLayout();
    fullNameEdit = new QLineEdit();
    phoneEdit = new QLineEdit();
    genderCombo = new QComboBox();
    genderCombo->addItems({"Prefer not to say", "Male", "Female"});
    emailValueLabel = new QLabel();
    createdAtValueLabel = new QLabel();

    formLayout->addRow("Full name:", fullNameEdit);
    formLayout->addRow("Phone:", phoneEdit);
    formLayout->addRow("Gender:", genderCombo);
    formLayout->addRow("Email:", emailValueLabel);
    formLayout->addRow("Created at:", createdAtValueLabel);
    mainLayout->addLayout(formLayout);

    saveButton = new QPushButton("Save Profile");
    connect(saveButton, &QPushButton::clicked, this, &ProfileWindow::onSaveProfile);
    mainLayout->addWidget(saveButton);
    mainLayout->addStretch();
}

void ProfileWindow::onSaveProfile() {
    if (userId_.empty()) {
        return;
    }

    backend_.auth().updateProfile(
        userId_,
        fullNameEdit->text().trimmed().toStdString(),
        phoneEdit->text().trimmed().toStdString(),
        genderCombo->currentText().toStdString());
    QMessageBox::information(this, "Profile", "Profile updated successfully.");
    emit profileUpdated();
}
