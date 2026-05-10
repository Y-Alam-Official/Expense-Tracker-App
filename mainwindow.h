#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include<QCloseEvent>
#include<QInputDialog>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:

    void on_btnadd_clicked();
    void on_btn_delete_clicked();
    void on_btnSearch_clicked();
    void on_btnShowAll_clicked();
    void updateDayName(const QDate &date);
    void on_btnFilterDay_clicked();
    void on_btnFilterWeek_clicked();
    void on_btnFilterMonth_clicked();
     void on_btnHighestCategory_clicked();
    void on_btnSetLimit_clicked();
 protected:
     void closeEvent(QCloseEvent *event) override;
private:

    Ui::MainWindow *ui;
    void loadExpenses();
    void saveExpenses();
    void calculateTotal();
    double dailyLimit = 2000.0;
};
#endif // MAINWINDOW_H
