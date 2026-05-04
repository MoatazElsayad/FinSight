#include "gui/goals/GoalsWindow.h"

#include "core/services/GoalService.h"
#include "gui/FinSightUi.h"

#include <QDate>
#include <QDateEdit>
#include <QDoubleSpinBox>
#include <QBrush>
#include <QColor>
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
#include <QTextEdit>
#include <QVBoxLayout>

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

QLabel* formLabel(const QString& text) {
    auto* label = new QLabel(text);
    label->setStyleSheet(finsight::gui::ui::labelStyle());
    return label;
}

QFrame* metricCard(const QString& caption, QLabel*& value) {
    auto* card = new QFrame();
    card->setObjectName("finCard");
    auto* layout = new QVBoxLayout(card);
    layout->setContentsMargins(18, 16, 18, 16);
    auto* captionLabel = new QLabel(caption);
    captionLabel->setObjectName("metricCaption");
    value = new QLabel(QStringLiteral("0"));
    value->setObjectName("metricValue");
    layout->addWidget(captionLabel);
    layout->addWidget(value);
    layout->addStretch();
    return card;
}

}  // namespace

GoalsWindow::GoalsWindow(finsight::core::managers::FinanceTrackerBackend& backend,
                         const std::string& userId,
                         QWidget* parent)
    : QWidget(parent),
      backend_(backend),
      userId_(userId) {
    setupUi();
    refreshData();
}

void GoalsWindow::setUserId(const std::string& userId) {
    userId_ = userId;
    refreshData();
}

void GoalsWindow::setupUi() {
    setStyleSheet(finsight::gui::ui::pageStyle(QStringLiteral("GoalsWindow")));

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 24, 24, 24);
    mainLayout->setSpacing(20);

    auto* title = new QLabel(QStringLiteral("Goals"));
    title->setStyleSheet(finsight::gui::ui::titleStyle());
    auto* subtitle = new QLabel(QStringLiteral("Create financial goals and keep progress visible."));
    subtitle->setStyleSheet(finsight::gui::ui::subtitleStyle());
    subtitle->setWordWrap(true);
    mainLayout->addWidget(title);
    mainLayout->addWidget(subtitle);

    auto* metrics = new QHBoxLayout();
    metrics->setSpacing(16);
    metrics->addWidget(metricCard(QStringLiteral("Active goals"), activeCountLabel), 1);
    metrics->addWidget(metricCard(QStringLiteral("Completed goals"), completedCountLabel), 1);
    metrics->addWidget(metricCard(QStringLiteral("Total target"), totalTargetLabel), 1);
    mainLayout->addLayout(metrics);

    auto* formCard = new QFrame();
    formCard->setObjectName("finCard");
    auto* formLayout = new QVBoxLayout(formCard);
    formLayout->setContentsMargins(20, 18, 20, 18);
    auto* formTitle = new QLabel(QStringLiteral("Goal details"));
    formTitle->setStyleSheet(finsight::gui::ui::cardTitleStyle());
    formLayout->addWidget(formTitle);

    auto* grid = new QGridLayout();
    grid->setHorizontalSpacing(16);
    grid->setVerticalSpacing(12);
    titleEdit = new QLineEdit();
    titleEdit->setPlaceholderText(QStringLiteral("Goal title"));
    descriptionEdit = new QTextEdit();
    descriptionEdit->setPlaceholderText(QStringLiteral("Optional description"));
    descriptionEdit->setFixedHeight(72);
    targetSpin = new QDoubleSpinBox();
    targetSpin->setRange(1.0, 1000000000.0);
    targetSpin->setDecimals(2);
    currentSpin = new QDoubleSpinBox();
    currentSpin->setRange(0.0, 1000000000.0);
    currentSpin->setDecimals(2);
    dueDateEdit = new QDateEdit(QDate::currentDate().addMonths(6));
    dueDateEdit->setCalendarPopup(true);

    grid->addWidget(formLabel(QStringLiteral("Title")), 0, 0);
    grid->addWidget(titleEdit, 0, 1, 1, 3);
    grid->addWidget(formLabel(QStringLiteral("Target amount")), 1, 0);
    grid->addWidget(targetSpin, 1, 1);
    grid->addWidget(formLabel(QStringLiteral("Current amount")), 1, 2);
    grid->addWidget(currentSpin, 1, 3);
    grid->addWidget(formLabel(QStringLiteral("Due date")), 2, 0);
    grid->addWidget(dueDateEdit, 2, 1);
    grid->addWidget(formLabel(QStringLiteral("Description")), 3, 0);
    grid->addWidget(descriptionEdit, 3, 1, 1, 3);
    formLayout->addLayout(grid);

    auto* buttons = new QHBoxLayout();
    auto* addButton = new QPushButton(QStringLiteral("Add goal"));
    addButton->setStyleSheet(finsight::gui::ui::primaryButtonStyle());
    updateButton = new QPushButton(QStringLiteral("Save selected"));
    updateButton->setStyleSheet(finsight::gui::ui::secondaryButtonStyle());
    progressButton = new QPushButton(QStringLiteral("Update progress"));
    progressButton->setStyleSheet(finsight::gui::ui::secondaryButtonStyle());
    deleteButton = new QPushButton(QStringLiteral("Delete"));
    deleteButton->setStyleSheet(finsight::gui::ui::dangerButtonStyle());
    updateButton->setEnabled(false);
    progressButton->setEnabled(false);
    deleteButton->setEnabled(false);
    buttons->addStretch();
    buttons->addWidget(addButton);
    buttons->addWidget(updateButton);
    buttons->addWidget(progressButton);
    buttons->addWidget(deleteButton);
    formLayout->addLayout(buttons);
    mainLayout->addWidget(formCard);

    auto* tableCard = new QFrame();
    tableCard->setObjectName("finCard");
    auto* tableLayout = new QVBoxLayout(tableCard);
    tableLayout->setContentsMargins(16, 16, 16, 16);
    auto* tableTitle = new QLabel(QStringLiteral("Goal list"));
    tableTitle->setStyleSheet(finsight::gui::ui::cardTitleStyle());
    goalsTable = new QTableWidget();
    goalsTable->setColumnCount(7);
    goalsTable->setHorizontalHeaderLabels({
        QStringLiteral("Title"),
        QStringLiteral("Target"),
        QStringLiteral("Current"),
        QStringLiteral("Progress"),
        QStringLiteral("Due"),
        QStringLiteral("Status"),
        QStringLiteral("Id")
    });
    finsight::gui::ui::prepareTable(goalsTable);
    goalsTable->setColumnHidden(6, true);
    tableLayout->addWidget(tableTitle);
    tableLayout->addWidget(goalsTable, 1);
    mainLayout->addWidget(tableCard, 1);

    connect(addButton, &QPushButton::clicked, this, &GoalsWindow::addGoal);
    connect(updateButton, &QPushButton::clicked, this, &GoalsWindow::updateSelectedGoal);
    connect(progressButton, &QPushButton::clicked, this, &GoalsWindow::updateSelectedProgress);
    connect(deleteButton, &QPushButton::clicked, this, &GoalsWindow::deleteSelectedGoal);
    connect(goalsTable, &QTableWidget::itemSelectionChanged, this, &GoalsWindow::fillFormFromSelection);
}

void GoalsWindow::addGoal() {
    try {
        backend_.goals().createGoal(Goal {
            .userId = userId_,
            .title = titleEdit->text().trimmed().toStdString(),
            .description = descriptionEdit->toPlainText().trimmed().toStdString(),
            .targetAmount = targetSpin->value(),
            .currentAmount = currentSpin->value(),
            .targetDate = toDate(dueDateEdit->date()),
        });
        clearForm();
        refreshData();
        emit dataChanged();
    } catch (const std::exception& error) {
        QMessageBox::warning(this, QStringLiteral("Goal Error"), error.what());
    }
}

void GoalsWindow::updateSelectedGoal() {
    const auto id = selectedGoalId();
    if (id.empty()) {
        QMessageBox::information(this, QStringLiteral("Update Goal"), QStringLiteral("Please select a goal first."));
        return;
    }

    try {
        backend_.goals().updateGoal(userId_, id, Goal {
            .userId = userId_,
            .title = titleEdit->text().trimmed().toStdString(),
            .description = descriptionEdit->toPlainText().trimmed().toStdString(),
            .targetAmount = targetSpin->value(),
            .currentAmount = currentSpin->value(),
            .targetDate = toDate(dueDateEdit->date()),
        });
        refreshData();
        emit dataChanged();
    } catch (const std::exception& error) {
        QMessageBox::warning(this, QStringLiteral("Goal Error"), error.what());
    }
}

void GoalsWindow::updateSelectedProgress() {
    const auto id = selectedGoalId();
    if (id.empty()) {
        QMessageBox::information(this, QStringLiteral("Update Progress"), QStringLiteral("Please select a goal first."));
        return;
    }

    try {
        backend_.goals().updateProgress(userId_, id, currentSpin->value());
        refreshData();
        emit dataChanged();
    } catch (const std::exception& error) {
        QMessageBox::warning(this, QStringLiteral("Goal Error"), error.what());
    }
}

void GoalsWindow::deleteSelectedGoal() {
    const auto id = selectedGoalId();
    if (id.empty()) {
        QMessageBox::information(this, QStringLiteral("Delete Goal"), QStringLiteral("Please select a goal first."));
        return;
    }
    if (QMessageBox::question(this,
                              QStringLiteral("Delete Goal"),
                              QStringLiteral("Delete the selected goal?")) != QMessageBox::Yes) {
        return;
    }

    try {
        backend_.goals().deleteGoal(userId_, id);
        clearForm();
        refreshData();
        emit dataChanged();
    } catch (const std::exception& error) {
        QMessageBox::warning(this, QStringLiteral("Goal Error"), error.what());
    }
}

void GoalsWindow::fillFormFromSelection() {
    const auto id = selectedGoalId();
    const bool hasSelection = !id.empty();
    updateButton->setEnabled(hasSelection);
    progressButton->setEnabled(hasSelection);
    deleteButton->setEnabled(hasSelection);

    if (!hasSelection) {
        return;
    }

    for (const auto& goal : backend_.goals().listGoals(userId_)) {
        if (goal.id != id) {
            continue;
        }
        titleEdit->setText(QString::fromStdString(goal.title));
        descriptionEdit->setPlainText(QString::fromStdString(goal.description));
        targetSpin->setValue(goal.targetAmount);
        currentSpin->setValue(goal.currentAmount);
        dueDateEdit->setDate(toQDate(goal.targetDate));
        return;
    }
}

void GoalsWindow::clearForm() {
    titleEdit->clear();
    descriptionEdit->clear();
    targetSpin->setValue(1.0);
    currentSpin->setValue(0.0);
    dueDateEdit->setDate(QDate::currentDate().addMonths(6));
    goalsTable->clearSelection();
    updateButton->setEnabled(false);
    progressButton->setEnabled(false);
    deleteButton->setEnabled(false);
}

std::string GoalsWindow::selectedGoalId() const {
    const int row = goalsTable->currentRow();
    if (row < 0 || goalsTable->item(row, 6) == nullptr) {
        return {};
    }
    return goalsTable->item(row, 6)->text().toStdString();
}

void GoalsWindow::refreshData() {
    goalsTable->setRowCount(0);
    updateButton->setEnabled(false);
    progressButton->setEnabled(false);
    deleteButton->setEnabled(false);

    if (userId_.empty()) {
        activeCountLabel->setText(QStringLiteral("0"));
        completedCountLabel->setText(QStringLiteral("0"));
        totalTargetLabel->setText(money(0.0));
        return;
    }

    const auto goals = backend_.goals().listGoals(userId_);
    int activeCount = 0;
    int completedCount = 0;
    double totalTarget = 0.0;

    for (const auto& goal : goals) {
        goal.completed ? ++completedCount : ++activeCount;
        totalTarget += goal.targetAmount;

        const int row = goalsTable->rowCount();
        goalsTable->insertRow(row);
        goalsTable->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(goal.title)));
        goalsTable->setItem(row, 1, new QTableWidgetItem(money(goal.targetAmount)));
        goalsTable->setItem(row, 2, new QTableWidgetItem(money(goal.currentAmount)));

        auto* progress = new QProgressBar();
        const int progressValue = static_cast<int>(finsight::core::services::GoalService::progressRatio(goal) * 100.0);
        progress->setRange(0, 100);
        progress->setValue(progressValue);
        progress->setFormat(QStringLiteral("%1%").arg(progressValue));
        if (goal.completed) {
            progress->setStyleSheet(QStringLiteral(
                "QProgressBar::chunk { background-color: %1; border-radius: 8px; }")
                .arg(finsight::gui::ui::successColor()));
        }
        goalsTable->setCellWidget(row, 3, progress);

        goalsTable->setItem(row, 4, new QTableWidgetItem(QString::fromStdString(goal.targetDate.toString())));
        auto* statusItem = new QTableWidgetItem(goal.completed ? QStringLiteral("Completed")
                                                               : QStringLiteral("Active"));
        statusItem->setForeground(QBrush(QColor(goal.completed ? finsight::gui::ui::successColor()
                                                               : finsight::gui::ui::accentColor())));
        goalsTable->setItem(row, 5, statusItem);
        goalsTable->setItem(row, 6, new QTableWidgetItem(QString::fromStdString(goal.id)));
    }

    activeCountLabel->setText(QString::number(activeCount));
    completedCountLabel->setText(QString::number(completedCount));
    totalTargetLabel->setText(money(totalTarget));
}
