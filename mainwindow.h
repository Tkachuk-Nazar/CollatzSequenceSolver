#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <future>
#include "CollatzSolver.h"

QT_BEGIN_NAMESPACE
namespace Ui
{
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

private slots:
    void onStartClicked();
    void onStopClicked();
    void onCalculationFinished(const FinalResult& result);

private:
    void setRunningState(bool running);

    Ui::MainWindow* ui;
    CollatzSolver* solver = nullptr;
    std::future<FinalResult> calculationFuture;
    QTimer* timer = nullptr;
};

#endif // MAINWINDOW_H