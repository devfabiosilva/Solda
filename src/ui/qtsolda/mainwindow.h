#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    void on_loadDatabase_clicked_a();
    ~MainWindow() override;

private slots:

    //void on_loadDatabase_clicked();

    void on_loadDatabase_clicked();

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
