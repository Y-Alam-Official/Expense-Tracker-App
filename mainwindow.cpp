#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include<fstream>
#include<QTableWidgetItem>
#include<QProcessEnvironment>
#include<QFile>
#include<QTextStream>
#include<QMessageBox>
#include<QCloseEvent>
using namespace std;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableWidget->horizontalHeader()->setStretchLastSection(true);
    loadExpenses();
 ui->dateSearch->setDate(QDate::currentDate());

    connect(ui->dateInput, &QDateEdit::dateChanged, this, &MainWindow::updateDayName);

    updateDayName(ui->dateInput->date());

    ui->dateInput->setDate(QDate::currentDate());

}

MainWindow::~MainWindow()
{
    delete ui;
}
void MainWindow::on_btnadd_clicked() {
    QString date = ui->dateInput->date().toString("dd/MM/yyyy");
    QString day = ui->txtday->text();
    QString category = ui->comboCategory->currentText();
    QString amount = ui->txtamount->text();
    QString note = ui->txtnote->text();
    // Check for 'Others' category and empty note
    if (category == "Others" && note.isEmpty()) {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, "Description Required",
                                      "You selected 'Others'. Would you like to add a description to remember this expense?",
                                      QMessageBox::Yes | QMessageBox::No);

        if (reply == QMessageBox::Yes) {
            ui->txtnote->setFocus();
            return; // Stops the function here so user can type
        }
    }

    if (amount.isEmpty()) return;

    // --- یہاں سے پیسٹ کریں ---
    double newAmount = amount.toDouble(); // آپ کے کوڈ میں 'amount' اسٹرنگ پہلے سے موجود ہے
    QDate entryDate = ui->dateInput->date(); // آپ کی ایپ میں 'dateInput' استعمال ہوا ہے
    double currentDayTotal = 0;

    for (int i = 0; i < ui->tableWidget->rowCount(); ++i) {
        QDate rowDate = QDate::fromString(ui->tableWidget->item(i, 0)->text(), "dd/MM/yyyy");
        if (rowDate == entryDate) {
            currentDayTotal += ui->tableWidget->item(i, 3)->text().toDouble();
        }
    }

    if ((currentDayTotal + newAmount) > dailyLimit) {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, "Limit Exceeded!",
                                      "This entry exceeds your daily limit. Change limit now?",
                                      QMessageBox::Yes | QMessageBox::No);

        if (reply == QMessageBox::Yes) {
            on_btnSetLimit_clicked();
        }
        return;
    }
    // --- یہاں سے پیسٹ شروع کریں ---

    // 1. ڈسکرپشن حاصل کریں اور کوما کو ہٹائیں
    QString description = ui->txtnote->text();
    description.replace(",", " "); // کوما کو سپیس سے بدل دیا تاکہ فائل فارمیٹ خراب نہ ہو

    // 2. ٹیبل میں نئی رو (Row) شامل کریں
    int row = ui->tableWidget->rowCount();
    ui->tableWidget->insertRow(row);

    // 3. ڈیٹا کو ٹیبل کے کالمز میں سیٹ کریں
    ui->tableWidget->setItem(row, 0, new QTableWidgetItem(date));      // Date
    ui->tableWidget->setItem(row, 1, new QTableWidgetItem(day));       // Day
    ui->tableWidget->setItem(row, 2, new QTableWidgetItem(category));  // Category
    ui->tableWidget->setItem(row, 3, new QTableWidgetItem(amount));    // Amount
    ui->tableWidget->setItem(row, 4, new QTableWidgetItem(description)); // Cleaned Description

    // 4. ڈیٹا محفوظ کریں اور فیلڈز صاف کریں
    saveExpenses();
    ui->tableWidget->scrollToBottom();
    ui->txtamount->clear();
    ui->txtnote->clear();
    calculateTotal();


}
void MainWindow::on_btn_delete_clicked() {

    int currentRow = ui->tableWidget->currentRow();


    if (currentRow < 0 || ui->tableWidget->selectionModel()->selectedRows().isEmpty()) {
        QMessageBox::warning(this, "Selection Error", "Please click on a row first to select the expense you want to delete.");
        return;
    }


    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Confirm Delete", "Are you sure you want to delete this expense?",
                                  QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        ui->tableWidget->removeRow(currentRow);


        ui->tableWidget->clearSelection();
        ui->tableWidget->setCurrentCell(-1, -1);

        saveExpenses();
        calculateTotal();
    }
}
void MainWindow::loadExpenses() {
    QFile file("expenses.txt");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return;

    QTextStream in(&file);
    ui->tableWidget->setRowCount(0);

    while (!in.atEnd()) {
        QString line = in.readLine();
        QStringList fields = line.split(",");

        if (fields.size() == 5) {
            int row = ui->tableWidget->rowCount();
            ui->tableWidget->insertRow(row);
            for (int i = 0; i < 5; ++i) {
                ui->tableWidget->setItem(row, i, new QTableWidgetItem(fields[i]));
            }
        }
    }
    file.close();
    calculateTotal();
}



    void MainWindow::on_btnSearch_clicked() {

        QString searchDate = ui->dateSearch->date().toString("dd/MM/yyyy");
        bool found = false;


        for (int i = 0; i < ui->tableWidget->rowCount(); ++i) {
            QTableWidgetItem *item = ui->tableWidget->item(i, 0);
            if (item) {
                if (item->text() == searchDate) {
                    ui->tableWidget->setRowHidden(i, false);
                    found = true;
                } else {
                    ui->tableWidget->setRowHidden(i, true);
                }
            }
        }


        if (!found) {
            QMessageBox::information(this, "Not Found", "No expense found for: " + searchDate);
            on_btnShowAll_clicked();
        }

        calculateTotal();
    }
    void MainWindow::on_btnShowAll_clicked() {
        for (int i = 0; i < ui->tableWidget->rowCount(); ++i) {
            ui->tableWidget->setRowHidden(i, false);
        }

        // Reset UI to Default
        ui->btnFilterDay->setStyleSheet("");
        ui->btnFilterWeek->setStyleSheet("");
        ui->btnFilterMonth->setStyleSheet("");
        ui->lblTotal->setStyleSheet("");

        calculateTotal(); // اصلی ٹوٹل دوبارہ دکھائیں
    }
    void MainWindow::calculateTotal() {
        double total = 0;
        for (int i = 0; i < ui->tableWidget->rowCount(); ++i) {
            if (!ui->tableWidget->isRowHidden(i)) {
                QTableWidgetItem *item = ui->tableWidget->item(i, 3);
                if (item) {
                    total += item->text().toDouble();
                }
            }
        }
        ui->lblTotal->setText("Total: " + QString::number(total));
    }
    void MainWindow::saveExpenses() {
        QFile file("expenses.txt");
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            for (int i = 0; i < ui->tableWidget->rowCount(); ++i) {
                out << ui->tableWidget->item(i, 0)->text() << "," // Date
                    << ui->tableWidget->item(i, 1)->text() << "," // Day
                    << ui->tableWidget->item(i, 2)->text() << "," // Category
                    << ui->tableWidget->item(i, 3)->text() << "," // Amount
                    << ui->tableWidget->item(i, 4)->text() << "\n"; // Description (نیا کالم)
            }
            file.close();
        }
    }
    void MainWindow::updateDayName(const QDate &date) {
        ui->txtday->setText(date.toString("dddd"));
    }
    void MainWindow::on_btnFilterDay_clicked() {
        QDate selectedDate = ui->dateSearch->date();
        double dayTotal = 0;

        for (int i = 0; i < ui->tableWidget->rowCount(); ++i) {
            QDate rowDate = QDate::fromString(ui->tableWidget->item(i, 0)->text(), "dd/MM/yyyy");
            if (rowDate == selectedDate) {
                ui->tableWidget->setRowHidden(i, false);
                dayTotal += ui->tableWidget->item(i, 3)->text().toDouble();
            } else {
                ui->tableWidget->setRowHidden(i, true);
            }
        }
        // UI Updates
        ui->lblTotal->setText("Day Total: " + QString::number(dayTotal));
        ui->btnFilterDay->setStyleSheet("background-color: #3498db; color: white; font-weight: bold; border-radius: 5px;");
        ui->lblTotal->setStyleSheet("background-color: #3498db; color: white; padding: 5px; border-radius: 5px;");

        // Reset others
        ui->btnFilterWeek->setStyleSheet("");
        ui->btnFilterMonth->setStyleSheet("");
    }
    void MainWindow::on_btnFilterWeek_clicked() {
        QDate startDate = ui->dateSearch->date();
        QDate endDate = startDate.addDays(7);
        double weekTotal = 0;

        for (int i = 0; i < ui->tableWidget->rowCount(); ++i) {
            QDate rowDate = QDate::fromString(ui->tableWidget->item(i, 0)->text(), "dd/MM/yyyy");
            if (rowDate >= startDate && rowDate <= endDate) {
                ui->tableWidget->setRowHidden(i, false);
                weekTotal += ui->tableWidget->item(i, 3)->text().toDouble();
            } else {
                ui->tableWidget->setRowHidden(i, true);
            }
        }
        // UI Updates
        ui->lblTotal->setText("Weekly Total: " + QString::number(weekTotal));
        ui->btnFilterWeek->setStyleSheet("background-color: #2ecc71; color: white; font-weight: bold; border-radius: 5px;");
        ui->lblTotal->setStyleSheet("background-color: #2ecc71; color: white; padding: 5px; border-radius: 5px;");

        // Reset others
        ui->btnFilterDay->setStyleSheet("");
        ui->btnFilterMonth->setStyleSheet("");
    }

    void MainWindow::on_btnFilterMonth_clicked() {
        QDate selectedDate = ui->dateSearch->date();
        int targetMonth = selectedDate.month();
        int targetYear = selectedDate.year();
        double monthTotal = 0;

        for (int i = 0; i < ui->tableWidget->rowCount(); ++i) {
            QDate rowDate = QDate::fromString(ui->tableWidget->item(i, 0)->text(), "dd/MM/yyyy");
            if (rowDate.month() == targetMonth && rowDate.year() == targetYear) {
                ui->tableWidget->setRowHidden(i, false);
                monthTotal += ui->tableWidget->item(i, 3)->text().toDouble();
            } else {
                ui->tableWidget->setRowHidden(i, true);
            }
        }
        // UI Updates
        ui->lblTotal->setText("Monthly Total: " + QString::number(monthTotal));
        ui->btnFilterMonth->setStyleSheet("background-color: #9b59b6; color: white; font-weight: bold; border-radius: 5px;");
        ui->lblTotal->setStyleSheet("background-color: #9b59b6; color: white; padding: 5px; border-radius: 5px;");

        // Reset others
        ui->btnFilterDay->setStyleSheet("");
        ui->btnFilterWeek->setStyleSheet("");
    }
    void MainWindow::on_btnHighestCategory_clicked() {
        QMap<QString, double> highestExpenses;

        for (int i = 0; i < ui->tableWidget->rowCount(); ++i) {
            // کالم انڈیکس 2 استعمال کریں تاکہ کیٹیگری کا نام آئے (Food, Bills وغیرہ)
            QString category = ui->tableWidget->item(i, 2)->text();
            double amount = ui->tableWidget->item(i, 3)->text().toDouble();

            if (!highestExpenses.contains(category) || amount > highestExpenses[category]) {
                highestExpenses[category] = amount;
            }
        }

        QString resultText = "Top Spending in Each Category:\n";
        resultText += "--------------------------------------\n";

        QMapIterator<QString, double> i(highestExpenses);
        while (i.hasNext()) {
            i.next();
            // اب یہاں 'Food', 'Transport' جیسے نام آئیں گے
            resultText += "📌 " + i.key() + ": " + QString::number(i.value(), 'f', 2) + "\n";
        }

        if (highestExpenses.isEmpty()) {
            resultText = "No data available.";
        }

        QMessageBox::information(this, "Highest Expense Insights", resultText);
    }
    void MainWindow::on_btnSetLimit_clicked() {
        bool ok;
        // ان پٹ ڈائیلاگ باکس کے ذریعے یوزر سے لمٹ لینا
        double newLimit = QInputDialog::getDouble(this, "Set Daily Limit",
                                                  "Enter your spending limit:",
                                                  dailyLimit, 0, 1000000, 2, &ok);
        if (ok) {
            dailyLimit = newLimit;
            // UI لیبل پر نئی لمٹ دکھائیں
            ui->lblCurrentLimit->setText("Limit: " + QString::number(dailyLimit));
            QMessageBox::information(this, "Updated", "New limit has been set.");
        }
    }
    void MainWindow::closeEvent(QCloseEvent *event) {
        QMessageBox::StandardButton reply = QMessageBox::question(this, "Exit", "Do you want to exit and save data?",
                                                                  QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::Yes) {
            saveExpenses(); // <--- یہاں سیو فنکشن کال کریں
            event->accept(); // ایپ بند کر دیں
        } else {
            event->ignore(); // بند ہونے سے روک دیں
        }
    }
