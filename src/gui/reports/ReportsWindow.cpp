#include "gui/reports/ReportsWindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDateEdit>
#include <QPushButton>
#include <QTextEdit>
#include <QLabel>
#include <QFileDialog>
#include <QFile>
#include <QMessageBox>
using namespace finsight::core::models;
ReportsWindow::ReportsWindow(finsight::core::managers::FinanceTrackerBackend& backend,const std::string& userId,QWidget* parent):QWidget(parent),backend_(backend),userId_(userId){setupUi();}
void ReportsWindow::setUserId(const std::string& userId){userId_=userId;}
void ReportsWindow::setupUi(){auto*l=new QVBoxLayout(this); auto*r=new QHBoxLayout(); fromDate=new QDateEdit(QDate::currentDate().addMonths(-1)); toDate=new QDateEdit(QDate::currentDate()); auto*gen=new QPushButton("Generate Report"); auto*exp=new QPushButton("Export Text"); r->addWidget(fromDate); r->addWidget(toDate); r->addWidget(gen); r->addWidget(exp); l->addLayout(r); summary=new QLabel("No report"); details=new QTextEdit(); details->setReadOnly(true); l->addWidget(summary); l->addWidget(details); connect(gen,&QPushButton::clicked,this,[this](){ try{ ReportRequest req{.userId=userId_,.from=Date::fromString(fromDate->date().toString("yyyy-MM-dd").toStdString()),.to=Date::fromString(toDate->date().toString("yyyy-MM-dd").toStdString())}; auto rep=backend_.reports().generateReport(req,backend_.transactions(),backend_.budgets()); exported_=rep.exportedText; summary->setText(QString("Income: %1 Expenses: %2 Net: %3 Txn: %4").arg(rep.totalIncome).arg(rep.totalExpenses).arg(rep.net).arg(rep.transactions.size())); details->setPlainText(QString::fromStdString(rep.exportedText)); }catch(const std::exception&e){QMessageBox::warning(this,"Error",e.what());}}); connect(exp,&QPushButton::clicked,this,[this](){ QString path=QFileDialog::getSaveFileName(this,"Save Report","report.txt","Text (*.txt)"); if(path.isEmpty()) return; QFile file(path); if(!file.open(QIODevice::WriteOnly|QIODevice::Text)){QMessageBox::warning(this,"Error","Cannot write file.");return;} file.write(exported_.c_str()); file.close(); QMessageBox::information(this,"Saved","Report exported.");}); }
void ReportsWindow::refreshData(){}
