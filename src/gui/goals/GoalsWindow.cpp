#include "gui/goals/GoalsWindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTableWidget>
#include <QHeaderView>
#include <QLineEdit>
#include <QDoubleSpinBox>
#include <QDateEdit>
#include <QPushButton>
#include <QMessageBox>
using namespace finsight::core::models;
GoalsWindow::GoalsWindow(finsight::core::managers::FinanceTrackerBackend& backend,const std::string& userId,QWidget* parent):QWidget(parent),backend_(backend),userId_(userId){setupUi();refreshData();}
void GoalsWindow::setUserId(const std::string& userId){userId_=userId;}
void GoalsWindow::setupUi(){auto*l=new QVBoxLayout(this); auto*f=new QHBoxLayout(); titleEdit=new QLineEdit(); targetSpin=new QDoubleSpinBox(); currentSpin=new QDoubleSpinBox(); dueEdit=new QDateEdit(QDate::currentDate().addMonths(6)); auto*add=new QPushButton("Add Goal"); auto*update=new QPushButton("Update Progress"); auto*del=new QPushButton("Delete"); f->addWidget(titleEdit); f->addWidget(targetSpin); f->addWidget(currentSpin); f->addWidget(dueEdit); f->addWidget(add); f->addWidget(update); f->addWidget(del); l->addLayout(f); table=new QTableWidget(); table->setColumnCount(6); table->setHorizontalHeaderLabels({"Title","Target","Current","Progress","Due","Id"}); table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch); l->addWidget(table);
connect(add,&QPushButton::clicked,this,[this](){ try{ backend_.goals().createGoal(Goal{.userId=userId_,.title=titleEdit->text().toStdString(),.targetAmount=targetSpin->value(),.currentAmount=currentSpin->value(),.targetDate=Date::fromString(dueEdit->date().toString("yyyy-MM-dd").toStdString())}); refreshData(); emit dataChanged(); }catch(const std::exception&e){QMessageBox::warning(this,"Error",e.what());}});
connect(update,&QPushButton::clicked,this,[this](){int r=table->currentRow(); if(r<0)return; try{ backend_.goals().updateProgress(userId_,table->item(r,5)->text().toStdString(),currentSpin->value()); refreshData(); emit dataChanged(); }catch(const std::exception&e){QMessageBox::warning(this,"Error",e.what());}});
connect(del,&QPushButton::clicked,this,[this](){int r=table->currentRow(); if(r<0)return; try{ backend_.goals().deleteGoal(userId_,table->item(r,5)->text().toStdString()); refreshData(); emit dataChanged(); }catch(const std::exception&e){QMessageBox::warning(this,"Error",e.what());}});
}
void GoalsWindow::refreshData(){table->setRowCount(0); if(userId_.empty()) return; for(const auto& g: backend_.goals().listGoals(userId_)){ int r=table->rowCount(); table->insertRow(r); table->setItem(r,0,new QTableWidgetItem(QString::fromStdString(g.title))); table->setItem(r,1,new QTableWidgetItem(QString::number(g.targetAmount,'f',2))); table->setItem(r,2,new QTableWidgetItem(QString::number(g.currentAmount,'f',2))); double prog=g.targetAmount<=0?0:(g.currentAmount/g.targetAmount*100.0); table->setItem(r,3,new QTableWidgetItem(QString::number(prog,'f',1)+"%")); table->setItem(r,4,new QTableWidgetItem(QString::fromStdString(g.targetDate.toString()))); table->setItem(r,5,new QTableWidgetItem(QString::fromStdString(g.id))); }}
