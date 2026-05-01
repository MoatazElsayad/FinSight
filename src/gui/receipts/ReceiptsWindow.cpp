#include "gui/receipts/ReceiptsWindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QTextEdit>
#include <QTableWidget>
#include <QHeaderView>
#include <QPushButton>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QDateEdit>
#include <QMessageBox>
using namespace finsight::core::models;
ReceiptsWindow::ReceiptsWindow(finsight::core::managers::FinanceTrackerBackend& backend,const std::string& userId,QWidget* parent):QWidget(parent),backend_(backend),userId_(userId){setupUi();refreshData();}
void ReceiptsWindow::setUserId(const std::string& userId){userId_=userId;}
void ReceiptsWindow::setupUi(){auto*l=new QVBoxLayout(this); fileEdit=new QLineEdit("receipt.txt"); rawText=new QTextEdit(); auto*upload=new QPushButton("Upload + Parse"); l->addWidget(fileEdit); l->addWidget(rawText); l->addWidget(upload); table=new QTableWidget(); table->setColumnCount(4); table->setHorizontalHeaderLabels({"Receipt Id","Merchant","Amount","Date"}); table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch); l->addWidget(table); auto*c=new QHBoxLayout(); titleEdit=new QLineEdit(); categoryCombo=new QComboBox(); for(const auto& cat: backend_.transactions().getCategoriesForUser(userId_)){ categoryCombo->addItem(QString::fromStdString(cat.name),QString::fromStdString(cat.id)); } amountSpin=new QDoubleSpinBox(); dateEdit=new QDateEdit(QDate::currentDate()); auto*confirm=new QPushButton("Confirm as Transaction"); c->addWidget(titleEdit); c->addWidget(categoryCombo); c->addWidget(amountSpin); c->addWidget(dateEdit); c->addWidget(confirm); l->addLayout(c);
connect(upload,&QPushButton::clicked,this,[this](){ try{ auto r=backend_.receipts().uploadReceipt(userId_,fileEdit->text().toStdString(),rawText->toPlainText().toStdString(),Date::fromString(QDate::currentDate().toString("yyyy-MM-dd").toStdString())); auto p=backend_.receipts().parseReceipt(userId_,r.id,backend_.transactions()); parsedReceiptId_=p.receiptId; refreshData(); amountSpin->setValue(p.amount.value_or(0.0)); if(p.transactionDate.has_value()) dateEdit->setDate(QDate(p.transactionDate->year,p.transactionDate->month,p.transactionDate->day)); titleEdit->setText(QString::fromStdString(p.merchant)); int idx=categoryCombo->findData(QString::fromStdString(p.suggestedCategoryId)); if(idx>=0) categoryCombo->setCurrentIndex(idx); }catch(const std::exception&e){QMessageBox::warning(this,"Error",e.what());}});
connect(confirm,&QPushButton::clicked,this,[this](){ if(parsedReceiptId_.empty()){QMessageBox::information(this,"No Parse","Upload and parse first.");return;} try{ ReceiptConfirmation c{.receiptId=parsedReceiptId_,.title=titleEdit->text().toStdString(),.merchant=titleEdit->text().toStdString(),.categoryId=categoryCombo->currentData().toString().toStdString(),.amount=amountSpin->value(),.date=Date::fromString(dateEdit->date().toString("yyyy-MM-dd").toStdString())}; backend_.receipts().confirmReceiptAsTransaction(userId_,c,backend_.transactions()); QMessageBox::information(this,"Saved","Transaction created from receipt."); emit dataChanged(); }catch(const std::exception&e){QMessageBox::warning(this,"Error",e.what());}});
}
void ReceiptsWindow::refreshData(){table->setRowCount(0); if(userId_.empty()) return; for(const auto&r: backend_.receipts().listReceipts(userId_)){ int i=table->rowCount(); table->insertRow(i); table->setItem(i,0,new QTableWidgetItem(QString::fromStdString(r.id))); table->setItem(i,1,new QTableWidgetItem(QString::fromStdString(r.fileName))); table->setItem(i,2,new QTableWidgetItem(QString::fromStdString(r.rawText).left(24))); table->setItem(i,3,new QTableWidgetItem(QString::fromStdString(r.uploadedAt.toString()))); }}
