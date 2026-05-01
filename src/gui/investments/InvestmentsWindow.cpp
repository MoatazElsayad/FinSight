#include "gui/investments/InvestmentsWindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTableWidget>
#include <QHeaderView>
#include <QLineEdit>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QDateEdit>
#include <QPushButton>
#include <QMessageBox>
using namespace finsight::core::models;
InvestmentsWindow::InvestmentsWindow(finsight::core::managers::FinanceTrackerBackend& backend,const std::string& userId,QWidget* parent):QWidget(parent),backend_(backend),userId_(userId){setupUi();refreshData();}
void InvestmentsWindow::setUserId(const std::string& userId){userId_=userId;}
void InvestmentsWindow::setupUi(){auto*l=new QVBoxLayout(this); auto*f=new QHBoxLayout(); assetEdit=new QLineEdit(); symbolEdit=new QLineEdit(); typeCombo=new QComboBox(); typeCombo->addItems({"Stock","Gold","Silver","Currency","Other"}); qtySpin=new QDoubleSpinBox(); buySpin=new QDoubleSpinBox(); curSpin=new QDoubleSpinBox(); dateEdit=new QDateEdit(QDate::currentDate()); auto*add=new QPushButton("Add"); auto*del=new QPushButton("Delete Selected"); f->addWidget(assetEdit); f->addWidget(symbolEdit); f->addWidget(typeCombo); f->addWidget(qtySpin); f->addWidget(buySpin); f->addWidget(curSpin); f->addWidget(dateEdit); f->addWidget(add); f->addWidget(del); l->addLayout(f); table=new QTableWidget(); table->setColumnCount(7); table->setHorizontalHeaderLabels({"Asset","Symbol","Qty","Buy","Current","P/L","Id"}); table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch); l->addWidget(table);
connect(add,&QPushButton::clicked,this,[this](){ InvestmentType t=InvestmentType::Other; if(typeCombo->currentText()=="Stock")t=InvestmentType::Stock; else if(typeCombo->currentText()=="Gold")t=InvestmentType::Gold; else if(typeCombo->currentText()=="Silver")t=InvestmentType::Silver; else if(typeCombo->currentText()=="Currency")t=InvestmentType::Currency; try{ backend_.savings().addInvestment(Investment{.userId=userId_,.assetName=assetEdit->text().toStdString(),.symbol=symbolEdit->text().toStdString(),.type=t,.quantity=qtySpin->value(),.buyRate=buySpin->value(),.currentRate=curSpin->value(),.purchaseDate=Date::fromString(dateEdit->date().toString("yyyy-MM-dd").toStdString())}); refreshData(); emit dataChanged(); }catch(const std::exception& e){QMessageBox::warning(this,"Error",e.what());}});
connect(del,&QPushButton::clicked,this,[this](){int r=table->currentRow(); if(r<0)return; try{ backend_.savings().deleteInvestment(userId_,table->item(r,6)->text().toStdString()); refreshData(); emit dataChanged(); }catch(const std::exception& e){QMessageBox::warning(this,"Error",e.what());}});
}
void InvestmentsWindow::refreshData(){table->setRowCount(0); if(userId_.empty()) return; for(const auto&s: backend_.savings().investmentSnapshots(userId_)){ int r=table->rowCount(); table->insertRow(r); table->setItem(r,0,new QTableWidgetItem(QString::fromStdString(s.investment.assetName))); table->setItem(r,1,new QTableWidgetItem(QString::fromStdString(s.investment.symbol))); table->setItem(r,2,new QTableWidgetItem(QString::number(s.investment.quantity,'f',2))); table->setItem(r,3,new QTableWidgetItem(QString::number(s.investment.buyRate,'f',2))); table->setItem(r,4,new QTableWidgetItem(QString::number(s.investment.currentRate,'f',2))); table->setItem(r,5,new QTableWidgetItem(QString::number(s.profitLoss,'f',2))); table->setItem(r,6,new QTableWidgetItem(QString::fromStdString(s.investment.id))); }}
