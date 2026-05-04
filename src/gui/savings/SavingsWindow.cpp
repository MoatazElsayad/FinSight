#include "gui/savings/SavingsWindow.h"

#include "gui/FinSightUi.h"

#include <QDate>
#include <QDateEdit>
#include <QDoubleSpinBox>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QProgressBar>
#include <QPushButton>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>

#include <algorithm>
#include <exception>

using namespace finsight::core::models;

namespace {

Date toDate(const QDate& date) {
    return Date {date.year(), date.month(), date.day()};
}

QDate toQDate(const Date& date) {
    return QDate(date.year, date.month, date.day);
}

QString money(double value) {
    return finsight::gui::ui::formatMoney(value);
}

QLabel* makeMetricCaption(const QString& text) {
    auto* label = new QLabel(text);
    label->setObjectName("metricCaption");
    return label;
}

QLabel* makeMetricValue(const QString& text) {
    auto* label = new QLabel(text);
    label->setObjectName("metricValue");
    return label;
}

QFrame* makeMetricCard(const QString& caption, QLabel*& valueLabel) {
    auto* card = new QFrame();
    card->setObjectName("finCard");
    auto* layout = new QVBoxLayout(card);
    layout->setContentsMargins(18, 16, 18, 16);
    valueLabel = makeMetricValue(QStringLiteral("EGP 0.00"));
    layout->addWidget(makeMetricCaption(caption));
    layout->addWidget(valueLabel);
    layout->addStretch();
    return card;
}

QLabel* formLabel(const QString& text) {
    auto* label = new QLabel(text);
    label->setStyleSheet(finsight::gui::ui::labelStyle());
    return label;
}

}  // namespace

SavingsWindow::SavingsWindow(finsight::core::managers::FinanceTrackerBackend& backend,
                             const std::string& userId,
                             QWidget* parent)
    : QWidget(parent),
      backend_(backend),
      userId_(userId) {
    setupUi();
    refreshData();
}

void SavingsWindow::setUserId(const std::string& userId) {
    userId_ = userId;
    refreshData();
}

void SavingsWindow::setupUi() {
    setStyleSheet(finsight::gui::ui::pageStyle(QStringLiteral("SavingsWindow")));

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 24, 24, 24);
    mainLayout->setSpacing(20);

    auto* title = new QLabel(QStringLiteral("Savings"));
    title->setStyleSheet(finsight::gui::ui::titleStyle());
    auto* subtitle = new QLabel(QStringLiteral("Track deposits, withdrawals, and progress toward savings targets."));
    subtitle->setStyleSheet(finsight::gui::ui::subtitleStyle());
    subtitle->setWordWrap(true);
    mainLayout->addWidget(title);
    mainLayout->addWidget(subtitle);

    auto* metricRow = new QHBoxLayout();
    metricRow->setSpacing(16);
    metricRow->addWidget(makeMetricCard(QStringLiteral("Current balance"), balanceValue), 1);
    metricRow->addWidget(makeMetricCard(QStringLiteral("Saved this month"), monthlySavedValue), 1);
    metricRow->addWidget(makeMetricCard(QStringLiteral("Monthly target"), monthlyTargetValue), 1);
    mainLayout->addLayout(metricRow);

    auto* progressCard = new QFrame();
    progressCard->setObjectName("finCard");
    auto* progressLayout = new QVBoxLayout(progressCard);
    progressLayout->setContentsMargins(20, 18, 20, 18);
    auto* progressTitle = new QLabel(QStringLiteral("Target progress"));
    progressTitle->setStyleSheet(finsight::gui::ui::cardTitleStyle());
    longTermTargetValue = makeMetricValue(QStringLiteral("Long-term target: EGP 0.00"));
    longTermTargetValue->setStyleSheet(QStringLiteral("font-size: 16px; font-weight: 700; color: #e5e9f4;"));
    targetDateValue = new QLabel(QStringLiteral("Target date: not set"));
    targetDateValue->setStyleSheet(QStringLiteral("color: #9ca6bf; font-size: 12px;"));
    monthlyProgress = new QProgressBar();
    monthlyProgress->setRange(0, 100);
    monthlyProgress->setValue(0);
    progressLayout->addWidget(progressTitle);
    progressLayout->addWidget(longTermTargetValue);
    progressLayout->addWidget(targetDateValue);
    progressLayout->addWidget(monthlyProgress);
    mainLayout->addWidget(progressCard);

    auto* entryCard = new QFrame();
    entryCard->setObjectName("finCard");
    auto* entryLayout = new QVBoxLayout(entryCard);
    entryLayout->setContentsMargins(20, 18, 20, 18);
    auto* entryTitle = new QLabel(QStringLiteral("Record savings activity"));
    entryTitle->setStyleSheet(finsight::gui::ui::cardTitleStyle());
    entryLayout->addWidget(entryTitle);

    auto* entryGrid = new QGridLayout();
    entryGrid->setHorizontalSpacing(16);
    entryGrid->setVerticalSpacing(12);
    amountSpin = new QDoubleSpinBox();
    amountSpin->setRange(0.01, 1000000000.0);
    amountSpin->setDecimals(2);
    amountSpin->setSingleStep(100.0);
    noteEdit = new QLineEdit();
    noteEdit->setPlaceholderText(QStringLiteral("Optional note"));
    entryDateEdit = new QDateEdit(QDate::currentDate());
    entryDateEdit->setCalendarPopup(true);

    auto* depositButton = new QPushButton(QStringLiteral("Add deposit"));
    depositButton->setStyleSheet(finsight::gui::ui::primaryButtonStyle());
    auto* withdrawButton = new QPushButton(QStringLiteral("Add withdrawal"));
    withdrawButton->setStyleSheet(finsight::gui::ui::secondaryButtonStyle());

    entryGrid->addWidget(formLabel(QStringLiteral("Amount")), 0, 0);
    entryGrid->addWidget(amountSpin, 0, 1);
    entryGrid->addWidget(formLabel(QStringLiteral("Date")), 0, 2);
    entryGrid->addWidget(entryDateEdit, 0, 3);
    entryGrid->addWidget(formLabel(QStringLiteral("Note")), 1, 0);
    entryGrid->addWidget(noteEdit, 1, 1, 1, 3);
    entryGrid->addWidget(depositButton, 2, 2);
    entryGrid->addWidget(withdrawButton, 2, 3);
    entryLayout->addLayout(entryGrid);
    mainLayout->addWidget(entryCard);

    auto* targetCard = new QFrame();
    targetCard->setObjectName("finCard");
    auto* targetLayout = new QVBoxLayout(targetCard);
    targetLayout->setContentsMargins(20, 18, 20, 18);
    auto* targetTitle = new QLabel(QStringLiteral("Savings targets"));
    targetTitle->setStyleSheet(finsight::gui::ui::cardTitleStyle());
    targetLayout->addWidget(targetTitle);

    auto* targetGrid = new QGridLayout();
    targetGrid->setHorizontalSpacing(16);
    targetGrid->setVerticalSpacing(12);
    monthlyTargetSpin = new QDoubleSpinBox();
    monthlyTargetSpin->setRange(0.0, 1000000000.0);
    monthlyTargetSpin->setDecimals(2);
    longTermTargetSpin = new QDoubleSpinBox();
    longTermTargetSpin->setRange(0.0, 1000000000.0);
    longTermTargetSpin->setDecimals(2);
    targetDateEdit = new QDateEdit(QDate::currentDate().addYears(1));
    targetDateEdit->setCalendarPopup(true);
    auto* saveTargetButton = new QPushButton(QStringLiteral("Save targets"));
    saveTargetButton->setStyleSheet(finsight::gui::ui::primaryButtonStyle());

    targetGrid->addWidget(formLabel(QStringLiteral("Monthly target")), 0, 0);
    targetGrid->addWidget(monthlyTargetSpin, 0, 1);
    targetGrid->addWidget(formLabel(QStringLiteral("Long-term target")), 0, 2);
    targetGrid->addWidget(longTermTargetSpin, 0, 3);
    targetGrid->addWidget(formLabel(QStringLiteral("Target date")), 1, 0);
    targetGrid->addWidget(targetDateEdit, 1, 1);
    targetGrid->addWidget(saveTargetButton, 1, 3);
    targetLayout->addLayout(targetGrid);
    mainLayout->addWidget(targetCard);

    auto* tableCard = new QFrame();
    tableCard->setObjectName("finCard");
    auto* tableLayout = new QVBoxLayout(tableCard);
    tableLayout->setContentsMargins(16, 16, 16, 16);
    auto* tableTitle = new QLabel(QStringLiteral("Savings history"));
    tableTitle->setStyleSheet(finsight::gui::ui::cardTitleStyle());
    entriesTable = new QTableWidget();
    entriesTable->setColumnCount(5);
    entriesTable->setHorizontalHeaderLabels({
        QStringLiteral("Date"),
        QStringLiteral("Type"),
        QStringLiteral("Amount"),
        QStringLiteral("Note"),
        QStringLiteral("Id")
    });
    finsight::gui::ui::prepareTable(entriesTable);
    entriesTable->setColumnHidden(4, true);
    deleteEntryButton = new QPushButton(QStringLiteral("Delete selected"));
    deleteEntryButton->setStyleSheet(finsight::gui::ui::dangerButtonStyle());
    deleteEntryButton->setEnabled(false);

    tableLayout->addWidget(tableTitle);
    tableLayout->addWidget(entriesTable, 1);
    auto* tableButtons = new QHBoxLayout();
    tableButtons->addStretch();
    tableButtons->addWidget(deleteEntryButton);
    tableLayout->addLayout(tableButtons);
    mainLayout->addWidget(tableCard, 1);

    connect(depositButton, &QPushButton::clicked, this, &SavingsWindow::addDeposit);
    connect(withdrawButton, &QPushButton::clicked, this, &SavingsWindow::addWithdrawal);
    connect(saveTargetButton, &QPushButton::clicked, this, &SavingsWindow::saveTargets);
    connect(deleteEntryButton, &QPushButton::clicked, this, &SavingsWindow::deleteSelectedEntry);
    connect(entriesTable, &QTableWidget::itemSelectionChanged, this, [this]() {
        deleteEntryButton->setEnabled(!selectedEntryId().empty());
    });
}

void SavingsWindow::addDeposit() {
    try {
        backend_.savings().addDeposit(userId_,
                                      amountSpin->value(),
                                      toDate(entryDateEdit->date()),
                                      noteEdit->text().trimmed().toStdString());
        clearEntryForm();
        refreshData();
        emit dataChanged();
    } catch (const std::exception& error) {
        QMessageBox::warning(this, QStringLiteral("Savings Error"), error.what());
    }
}

void SavingsWindow::addWithdrawal() {
    try {
        backend_.savings().addWithdrawal(userId_,
                                         amountSpin->value(),
                                         toDate(entryDateEdit->date()),
                                         noteEdit->text().trimmed().toStdString());
        clearEntryForm();
        refreshData();
        emit dataChanged();
    } catch (const std::exception& error) {
        QMessageBox::warning(this, QStringLiteral("Savings Error"), error.what());
    }
}

void SavingsWindow::saveTargets() {
    try {
        backend_.savings().setGoal(userId_,
                                   monthlyTargetSpin->value(),
                                   longTermTargetSpin->value(),
                                   toDate(targetDateEdit->date()));
        refreshData();
        emit dataChanged();
        QMessageBox::information(this, QStringLiteral("Targets Saved"), QStringLiteral("Savings targets updated."));
    } catch (const std::exception& error) {
        QMessageBox::warning(this, QStringLiteral("Savings Error"), error.what());
    }
}

void SavingsWindow::deleteSelectedEntry() {
    const auto entryId = selectedEntryId();
    if (entryId.empty()) {
        QMessageBox::information(this, QStringLiteral("Delete Entry"), QStringLiteral("Please select an entry first."));
        return;
    }

    if (QMessageBox::question(this,
                              QStringLiteral("Delete Entry"),
                              QStringLiteral("Delete the selected savings entry?")) != QMessageBox::Yes) {
        return;
    }

    try {
        backend_.savings().deleteEntry(userId_, entryId);
        refreshData();
        emit dataChanged();
    } catch (const std::exception& error) {
        QMessageBox::warning(this, QStringLiteral("Savings Error"), error.what());
    }
}

void SavingsWindow::clearEntryForm() {
    amountSpin->setValue(0.01);
    noteEdit->clear();
    entryDateEdit->setDate(QDate::currentDate());
}

std::string SavingsWindow::selectedEntryId() const {
    const int row = entriesTable->currentRow();
    if (row < 0 || entriesTable->item(row, 4) == nullptr) {
        return {};
    }
    return entriesTable->item(row, 4)->text().toStdString();
}

void SavingsWindow::refreshData() {
    entriesTable->setRowCount(0);
    deleteEntryButton->setEnabled(false);

    if (userId_.empty()) {
        balanceValue->setText(money(0.0));
        monthlySavedValue->setText(money(0.0));
        monthlyTargetValue->setText(money(0.0));
        longTermTargetValue->setText(QStringLiteral("Long-term target: EGP 0.00"));
        targetDateValue->setText(QStringLiteral("Target date: not set"));
        monthlyProgress->setValue(0);
        return;
    }

    const YearMonth period {QDate::currentDate().year(), QDate::currentDate().month()};
    const auto overview = backend_.savings().summarize(userId_, period);
    balanceValue->setText(money(overview.currentBalance));
    monthlySavedValue->setText(money(overview.monthlySaved));
    monthlyTargetValue->setText(money(overview.monthlyTarget));
    longTermTargetValue->setText(QStringLiteral("Long-term target: ") + money(overview.longTermTarget));

    const int progress = std::clamp(static_cast<int>(overview.progressToMonthlyTarget * 100.0), 0, 100);
    monthlyProgress->setValue(progress);
    monthlyProgress->setFormat(QStringLiteral("%1% monthly target").arg(progress));

    if (const auto goal = backend_.savings().getGoal(userId_)) {
        monthlyTargetSpin->setValue(goal->monthlyTarget);
        longTermTargetSpin->setValue(goal->longTermTarget);
        targetDateEdit->setDate(toQDate(goal->targetDate));
        targetDateValue->setText(QStringLiteral("Target date: ") + QString::fromStdString(goal->targetDate.toString()));
    } else {
        targetDateValue->setText(QStringLiteral("Target date: not set"));
    }

    for (const auto& entry : backend_.savings().listEntries(userId_)) {
        const int row = entriesTable->rowCount();
        entriesTable->insertRow(row);
        auto* dateItem = new QTableWidgetItem(QString::fromStdString(entry.date.toString()));
        dateItem->setData(Qt::UserRole, QString::fromStdString(entry.id));
        entriesTable->setItem(row, 0, dateItem);
        entriesTable->setItem(row, 1, new QTableWidgetItem(
                                           entry.type == SavingsEntryType::Deposit ? QStringLiteral("Deposit")
                                                                                   : QStringLiteral("Withdrawal")));
        entriesTable->setItem(row, 2, new QTableWidgetItem(money(entry.amount)));
        entriesTable->setItem(row, 3, new QTableWidgetItem(QString::fromStdString(entry.note)));
        entriesTable->setItem(row, 4, new QTableWidgetItem(QString::fromStdString(entry.id)));
    }
}
