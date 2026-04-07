#include "gui/dashboard/DashboardWindow.h"
#include <QVBoxLayout>
#include <QLabel>

DashboardWindow::DashboardWindow(QWidget *parent) : QWidget(parent) {
    auto *layout = new QVBoxLayout(this);
    layout->addWidget(new QLabel("Dashboard Page"));
}
