#pragma once

#include <QAbstractItemView>
#include <QHeaderView>
#include <QString>
#include <QTableWidget>

namespace finsight::gui::ui {

inline QString formatMoney(double value) {
    return QStringLiteral("EGP ") + QString::number(value, 'f', 2);
}

inline QString successColor() {
    return QStringLiteral("#8cf4b8");
}

inline QString dangerColor() {
    return QStringLiteral("#ff9191");
}

inline QString warningColor() {
    return QStringLiteral("#f5c66b");
}

inline QString accentColor() {
    return QStringLiteral("#5b8cff");
}

inline QString pageStyle(const QString& rootName) {
    return QString(
        "%1, %1 > QWidget {"
        "  background-color: #0b1020;"
        "  color: #e5e9f4;"
        "}"
        "QLabel {"
        "  background-color: transparent;"
        "}"
        "QFrame#finCard {"
        "  background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #1a2135, stop:1 #141a27);"
        "  border: 1px solid #2b3245;"
        "  border-radius: 14px;"
        "}"
        "QLabel#metricCaption {"
        "  background-color: transparent;"
        "  color: #9ca6bf;"
        "  font-size: 12px;"
        "  font-weight: 500;"
        "}"
        "QLabel#metricValue {"
        "  background-color: transparent;"
        "  color: #ffffff;"
        "  font-size: 24px;"
        "  font-weight: 800;"
        "}"
        "QLabel#statusBadge {"
        "  background-color: #10223c;"
        "  border: 1px solid #2a4080;"
        "  border-radius: 9px;"
        "  color: #c9d8ff;"
        "  padding: 4px 8px;"
        "  font-size: 11px;"
        "  font-weight: 700;"
        "}"
        "QLabel#emptyState {"
        "  background-color: transparent;"
        "  color: #8d97ac;"
        "  font-size: 13px;"
        "  padding: 14px;"
        "}"
        "QLineEdit, QComboBox, QDateEdit, QDoubleSpinBox, QTextEdit {"
        "  background-color: #0f1527;"
        "  border: 1px solid #2b3245;"
        "  border-radius: 10px;"
        "  color: #e5e9f4;"
        "  padding: 8px 10px;"
        "  min-height: 22px;"
        "}"
        "QTextEdit {"
        "  padding: 10px;"
        "}"
        "QLineEdit:focus, QComboBox:focus, QDateEdit:focus, QDoubleSpinBox:focus, QTextEdit:focus {"
        "  border-color: #5b8cff;"
        "  background-color: #111a2f;"
        "}"
        "QComboBox::drop-down {"
        "  background-color: transparent;"
        "  border: none;"
        "  padding-right: 8px;"
        "}"
        "QComboBox::down-arrow {"
        "  image: none;"
        "  width: 0px;"
        "}"
        "QAbstractItemView {"
        "  background-color: #0f1527;"
        "  border: 1px solid #2b3245;"
        "  border-radius: 8px;"
        "  color: #e5e9f4;"
        "  selection-background-color: #253355;"
        "  outline: 0;"
        "}"
        "QAbstractItemView::item {"
        "  padding: 8px 10px;"
        "  background-color: #0f1527;"
        "  color: #e5e9f4;"
        "  margin: 0px;"
        "}"
        "QAbstractItemView::item:hover {"
        "  background-color: #1a2742;"
        "}"
        "QAbstractItemView::item:selected {"
        "  background-color: #253355;"
        "  color: #ffffff;"
        "}"
        "QTableWidget {"
        "  background-color: #0f1527;"
        "  border: 1px solid #2a4080;"
        "  border-radius: 10px;"
        "  color: #e5e9f4;"
        "  gridline-color: #1e2436;"
        "  selection-background-color: #253355;"
        "  alternate-background-color: #121d34;"
        "}"
        "QTableWidget::item {"
        "  padding: 8px;"
        "  border-bottom: 1px solid #1e2436;"
        "  background-color: #0f1527;"
        "  color: #e5e9f4;"
        "}"
        "QTableWidget::item:alternate {"
        "  background-color: #121d34;"
        "  color: #e5e9f4;"
        "}"
        "QTableWidget::item:selected {"
        "  background-color: #253355;"
        "  color: #ffffff;"
        "}"
        "QHeaderView::section {"
        "  background-color: #0f1a33;"
        "  color: #5b8cff;"
        "  border: 0;"
        "  padding: 10px 8px;"
        "  font-weight: 700;"
        "  border-bottom: 2px solid #2a4080;"
        "}"
        "QProgressBar {"
        "  background-color: #0f1527;"
        "  border: 1px solid #2b3245;"
        "  border-radius: 8px;"
        "  color: #ffffff;"
        "  text-align: center;"
        "  min-height: 18px;"
        "}"
        "QProgressBar::chunk {"
        "  background-color: #5b8cff;"
        "  border-radius: 8px;"
        "}"
        "QScrollBar:vertical {"
        "  background-color: #0f1527;"
        "  width: 12px;"
        "  border-radius: 6px;"
        "}"
        "QScrollBar::handle:vertical {"
        "  background-color: #2b3245;"
        "  border-radius: 6px;"
        "  min-height: 24px;"
        "}"
        "QScrollBar::handle:vertical:hover {"
        "  background-color: #3a4155;"
        "}"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {"
        "  border: none;"
        "  background: none;"
        "}"
    ).arg(rootName);
}

inline QString dialogStyle(const QString& rootName) {
    return QString(
        "%1 {"
        "  background-color: #0b1020;"
        "  color: #e5e9f4;"
        "}"
        "QFrame#finCard {"
        "  background-color: #141a27;"
        "  border: 1px solid #2b3245;"
        "  border-radius: 14px;"
        "}"
        "QLineEdit, QComboBox, QDateEdit, QDoubleSpinBox, QTextEdit {"
        "  background-color: #0f1527;"
        "  border: 1px solid #2b3245;"
        "  border-radius: 10px;"
        "  color: #e5e9f4;"
        "  padding: 8px 10px;"
        "  min-height: 24px;"
        "}"
        "QLineEdit:focus, QComboBox:focus, QDateEdit:focus, QDoubleSpinBox:focus, QTextEdit:focus {"
        "  border-color: #5b8cff;"
        "}"
        "QComboBox::drop-down {"
        "  background-color: transparent;"
        "  border: none;"
        "  padding-right: 8px;"
        "}"
        "QComboBox::down-arrow {"
        "  image: none;"
        "  width: 0px;"
        "}"
        "QAbstractItemView {"
        "  background-color: #0f1527;"
        "  border: 1px solid #2b3245;"
        "  border-radius: 8px;"
        "  color: #e5e9f4;"
        "  selection-background-color: #253355;"
        "  outline: 0;"
        "}"
        "QAbstractItemView::item {"
        "  padding: 8px 10px;"
        "  background-color: #0f1527;"
        "  color: #e5e9f4;"
        "  margin: 0px;"
        "}"
        "QAbstractItemView::item:hover {"
        "  background-color: #1a2742;"
        "}"
        "QAbstractItemView::item:selected {"
        "  background-color: #253355;"
        "  color: #ffffff;"
        "}"
        "QLabel {"
        "  background-color: transparent;"
        "  color: #9ca6bf;"
        "}"
        "QDialogButtonBox QPushButton {"
        "  min-width: 96px;"
        "}"
    ).arg(rootName);
}

inline QString titleStyle() {
    return QStringLiteral("background-color: transparent; font-size: 30px; font-weight: 800; color: #ffffff;");
}

inline QString subtitleStyle() {
    return QStringLiteral("background-color: transparent; font-size: 13px; color: #8d97ac;");
}

inline QString labelStyle() {
    return QStringLiteral("background-color: transparent; color: #9ca6bf; font-size: 12px; font-weight: 500;");
}

inline QString cardTitleStyle() {
    return QStringLiteral("background-color: transparent; font-size: 14px; font-weight: 600; color: #e5e9f4;");
}

inline QString sectionHeaderStyle() {
    return QStringLiteral("background-color: transparent; font-size: 13px; font-weight: 700; color: #5b8cff;");
}

inline QString mutedTextStyle() {
    return QStringLiteral("background-color: transparent; color: #9ca6bf; font-size: 12px;");
}

inline QString badgeStyle(const QString& color) {
    return QStringLiteral(
        "background-color: #10223c;"
        "border: 1px solid #2a4080;"
        "border-radius: 9px;"
        "color: %1;"
        "padding: 4px 8px;"
        "font-size: 11px;"
        "font-weight: 700;").arg(color);
}

inline QString primaryButtonStyle() {
    return QStringLiteral(
        "QPushButton {"
        "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #5b8cff, stop:1 #4a7ae6);"
        "  color: #ffffff;"
        "  border: none;"
        "  border-radius: 10px;"
        "  padding: 10px 18px;"
        "  font-weight: 600;"
        "}"
        "QPushButton:hover {"
        "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #6b9cff, stop:1 #5a8af6);"
        "}"
        "QPushButton:disabled {"
        "  background-color: #293148;"
        "  color: #6f7890;"
        "}"
    );
}

inline QString secondaryButtonStyle() {
    return QStringLiteral(
        "QPushButton {"
        "  background-color: #0f1527;"
        "  color: #e5e9f4;"
        "  border: 1px solid #3a5490;"
        "  border-radius: 10px;"
        "  padding: 10px 18px;"
        "  font-weight: 600;"
        "}"
        "QPushButton:hover { background-color: #1a2742; }"
        "QPushButton:disabled { color: #5c6578; border-color: #2b3245; }"
    );
}

inline QString dangerButtonStyle() {
    return QStringLiteral(
        "QPushButton {"
        "  background-color: #2a1a24;"
        "  color: #ff9db0;"
        "  border: 1px solid #5a3040;"
        "  border-radius: 10px;"
        "  padding: 10px 18px;"
        "  font-weight: 600;"
        "}"
        "QPushButton:hover { background-color: #3a222e; }"
        "QPushButton:disabled { color: #6b4a55; border-color: #2b3245; }"
    );
}

inline QString ghostButtonStyle() {
    return QStringLiteral(
        "QPushButton {"
        "  background-color: transparent;"
        "  color: #aab2c5;"
        "  border: 1px solid #2b3245;"
        "  border-radius: 10px;"
        "  padding: 10px 18px;"
        "  font-weight: 600;"
        "}"
        "QPushButton:hover { background-color: #1a2135; border-color: #3a4155; }"
        "QPushButton:disabled { color: #5c6578; border-color: #2b3245; }"
    );
}

inline QString navButtonStyle() {
    return QStringLiteral(
        "QPushButton {"
        "  background-color: transparent;"
        "  border: 1px solid transparent;"
        "  border-radius: 10px;"
        "  color: #dce4f8;"
        "  text-align: left;"
        "  padding: 11px 12px;"
        "  font-weight: 650;"
        "  font-size: 13px;"
        "}"
        "QPushButton:hover { background-color: #172443; }"
        "QPushButton:pressed { background-color: #233d72; }"
        "QPushButton:checked {"
        "  background-color: #203b70;"
        "  border-color: #5b8cff;"
        "  color: #ffffff;"
        "}"
    );
}

inline QString filterButtonStyle() {
    return QStringLiteral(
        "QPushButton {"
        "  background-color: #0f1527;"
        "  color: #9ca6bf;"
        "  border: 1px solid #2b3245;"
        "  border-radius: 8px;"
        "  padding: 8px 12px;"
        "  font-weight: 600;"
        "  font-size: 12px;"
        "}"
        "QPushButton:hover { background-color: #1a2135; border-color: #3a4155; color: #dce4f8; }"
        "QPushButton:checked {"
        "  background-color: #203b70;"
        "  border-color: #5b8cff;"
        "  color: #ffffff;"
        "}"
    );
}

inline void prepareTable(QTableWidget* table) {
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->horizontalHeader()->setStretchLastSection(true);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setShowGrid(false);
    table->verticalHeader()->setVisible(false);
    table->setAlternatingRowColors(true);
}

}  // namespace finsight::gui::ui
