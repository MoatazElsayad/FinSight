#include "gui/profile/ProfileWindow.h"

#include "gui/FinSightUi.h"

#include <QComboBox>
#include <QFormLayout>
#include <QFrame>
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
    mainLayout->setContentsMargins(24, 24, 24, 24);
    mainLayout->setSpacing(20);

    setStyleSheet(finsight::gui::ui::pageStyle(QStringLiteral("ProfileWindow")) +
                  QStringLiteral(
                      "QLabel#profileReadOnly {"
                      "  background-color: #0f1527;"
                      "  border: 1px solid #2b3245;"
                      "  border-radius: 10px;"
                      "  color: #c8d0e4;"
                      "  padding: 10px 12px;"
                      "}"));

    auto *headerColumn = new QVBoxLayout();
    auto *titleLabel = new QLabel(QStringLiteral("Profile"));
    titleLabel->setStyleSheet(finsight::gui::ui::titleStyle());
    auto *subtitleLabel = new QLabel(
        QStringLiteral("Keep your contact details current. Email and signup date are read-only."));
    subtitleLabel->setStyleSheet(finsight::gui::ui::subtitleStyle());
    subtitleLabel->setWordWrap(true);
    headerColumn->addWidget(titleLabel);
    headerColumn->addWidget(subtitleLabel);
    mainLayout->addLayout(headerColumn);

    auto *card = new QFrame();
    card->setObjectName("finCard");
    auto *cardLay = new QVBoxLayout(card);
    cardLay->setContentsMargins(24, 22, 24, 24);
    auto *cardTitle = new QLabel(QStringLiteral("Account details"));
    cardTitle->setStyleSheet(finsight::gui::ui::cardTitleStyle());
    cardLay->addWidget(cardTitle);
    cardLay->addSpacing(16);

    auto *formLayout = new QFormLayout();
    formLayout->setHorizontalSpacing(20);
    formLayout->setVerticalSpacing(14);
    formLayout->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);

    auto labelStyle = [](const QString& text) {
        auto *l = new QLabel(text);
        l->setStyleSheet(finsight::gui::ui::labelStyle());
        return l;
    };

    fullNameEdit = new QLineEdit();
    phoneEdit = new QLineEdit();
    genderCombo = new QComboBox();
    genderCombo->addItems(
        {QStringLiteral("Prefer not to say"), QStringLiteral("Male"), QStringLiteral("Female")});
    emailValueLabel = new QLabel();
    emailValueLabel->setObjectName(QStringLiteral("profileReadOnly"));
    emailValueLabel->setMinimumHeight(36);
    createdAtValueLabel = new QLabel();
    createdAtValueLabel->setObjectName(QStringLiteral("profileReadOnly"));
    createdAtValueLabel->setMinimumHeight(36);

    formLayout->addRow(labelStyle(QStringLiteral("Full name")), fullNameEdit);
    formLayout->addRow(labelStyle(QStringLiteral("Phone")), phoneEdit);
    formLayout->addRow(labelStyle(QStringLiteral("Gender")), genderCombo);
    formLayout->addRow(labelStyle(QStringLiteral("Email")), emailValueLabel);
    formLayout->addRow(labelStyle(QStringLiteral("Member since")), createdAtValueLabel);
    cardLay->addLayout(formLayout);
    cardLay->addSpacing(20);

    saveButton = new QPushButton(QStringLiteral("Save changes"));
    saveButton->setStyleSheet(finsight::gui::ui::primaryButtonStyle());
    cardLay->addWidget(saveButton, 0, Qt::AlignLeft);

    mainLayout->addWidget(card);
    mainLayout->addStretch();

    connect(saveButton, &QPushButton::clicked, this, &ProfileWindow::onSaveProfile);
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
