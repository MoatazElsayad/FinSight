#include "gui/savings/SavingsWindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QTableWidget>
#include <QHeaderView>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QDateEdit>
#include <QPushButton>
#include <QMessageBox>
using namespace finsight::core::models;
SavingsWindow::SavingsWindow(finsight::core::managers::FinanceTrackerBackend& backend,const std::string& userId,QWidget* parent):QWidget(parent),backend_(backend),userId_(userId){setupUi();refreshData();}
void SavingsWindow::setUserId(const std::string& userId){userId_=userId;}
void SavingsWindow::setupUi(){auto*l=new QVBoxLayout(this); balanceLabel=new QLabel("Balance: 0"); monthlyLabel=new QLabel("Monthly saved: 0"); l->addWidget(balanceLabel); l->addWidget(monthlyLabel); auto*f=new QHBoxLayout(); amountSpin=new QDoubleSpinBox(); amountSpin->setRange(0.01,1e9); noteEdit=new QLineEdit(); dateEdit=new QDateEdit(QDate::currentDate()); dateEdit->setCalendarPopup(true); auto*dep=new QPushButton("Deposit"); auto*wdr=new QPushButton("Withdraw"); f->addWidget(amountSpin); f->addWidget(noteEdit); f->addWidget(dateEdit); f->addWidget(dep); f->addWidget(wdr); l->addLayout(f); auto*g=new QHBoxLayout(); monthlyTargetSpin=new QDoubleSpinBox(); longTargetSpin=new QDoubleSpinBox(); targetDateEdit=new QDateEdit(QDate::currentDate().addYears(1)); auto*saveGoal=new QPushButton("Save Targets"); g->addWidget(monthlyTargetSpin); g->addWidget(longTargetSpin); g->addWidget(targetDateEdit); g->addWidget(saveGoal); l->addLayout(g); table=new QTableWidget(); table->setColumnCount(4); table->setHorizontalHeaderLabels({"Date","Type","Amount","Note"}); table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch); l->addWidget(table);
connect(dep,&QPushButton::clicked,this,[this](){ try{ backend_.savings().addEntry(SavingsEntry{.userId=userId_,.type=SavingsEntryType::Deposit,.amount=amountSpin->value(),.date=Date::fromString(dateEdit->date().toString("yyyy-MM-dd").toStdString()),.note=noteEdit->text().toStdString()}); refreshData(); emit dataChanged(); }catch(const std::exception&e){QMessageBox::warning(this,"Error",e.what());}});
connect(wdr,&QPushButton::clicked,this,[this](){ try{ backend_.savings().addEntry(SavingsEntry{.userId=userId_,.type=SavingsEntryType::Withdrawal,.amount=amountSpin->value(),.date=Date::fromString(dateEdit->date().toString("yyyy-MM-dd").toStdString()),.note=noteEdit->text().toStdString()}); refreshData(); emit dataChanged(); }catch(const std::exception&e){QMessageBox::warning(this,"Error",e.what());}});
connect(saveGoal,&QPushButton::clicked,this,[this](){ try{ backend_.savings().setGoal(userId_,monthlyTargetSpin->value(),longTargetSpin->value(),Date::fromString(targetDateEdit->date().toString("yyyy-MM-dd").toStdString())); QMessageBox::information(this,"Saved","Savings targets updated."); refreshData(); }catch(const std::exception&e){QMessageBox::warning(this,"Error",e.what());}});
}
void SavingsWindow::refreshData(){ table->setRowCount(0); if(userId_.empty()) return; YearMonth p{QDate::currentDate().year(),QDate::currentDate().month()}; auto o=backend_.savings().summarize(userId_,p); balanceLabel->setText("Balance: "+QString::number(o.currentBalance,'f',2)); monthlyLabel->setText("Monthly saved: "+QString::number(o.monthlySaved,'f',2)); for(const auto&e: backend_.savings().listEntries(userId_)){ int r=table->rowCount(); table->insertRow(r); table->setItem(r,0,new QTableWidgetItem(QString::fromStdString(e.date.toString()))); table->setItem(r,1,new QTableWidgetItem(e.type==SavingsEntryType::Deposit?"Deposit":"Withdrawal")); table->setItem(r,2,new QTableWidgetItem(QString::number(e.amount,'f',2))); table->setItem(r,3,new QTableWidgetItem(QString::fromStdString(e.note))); }}
