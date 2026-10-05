#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QCoreApplication>
#include <QTimer>
#include <thread>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    unsigned int maxCores = std::thread::hardware_concurrency();
    if (maxCores == 0) maxCores = 4;
    ui->sliderThreads->setRange(1, maxCores);
    ui->sliderThreads->setValue(maxCores);
    ui->lblThreadsValue->setText(QString::number(maxCores));

    ui->spinBoxLimit->setRange(1, 2000000000);
    ui->spinBoxLimit->setValue(10000000);

    // timer to wait for completion of the async task without blocking the UI
    timer = new QTimer(this);
    timer->setInterval(50); // check every 50ms
    connect(timer, &QTimer::timeout, [this]()
    {
        if (calculationFuture.valid() && calculationFuture.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready) {
            timer->stop();
            FinalResult res = calculationFuture.get();
            onCalculationFinished(res);
        }
    });

    // connect UI signals
    connect(ui->sliderThreads, &QSlider::valueChanged, this, [this](int value)
    {
        ui->lblThreadsValue->setText(QString::number(value));
    });

    connect(ui->btnStart, &QPushButton::clicked, this, &MainWindow::onStartClicked);
    connect(ui->btnStop, &QPushButton::clicked, this, &MainWindow::onStopClicked);
    connect(ui->btnExit, &QPushButton::clicked, qApp, &QCoreApplication::quit);
}

MainWindow::~MainWindow()
{
    if (solver)
    {
        solver->requestStop();
    }
    if (calculationFuture.valid())
    {
        calculationFuture.wait();
    }
    delete solver;
    delete ui;
}

void MainWindow::setRunningState(bool running)
{
    ui->btnStart->setEnabled(!running);
    ui->btnStop->setEnabled(running);
    ui->spinBoxLimit->setEnabled(!running);
    ui->sliderThreads->setEnabled(!running);
}

void MainWindow::onStartClicked()
{
    uint64_t maxLimit = ui->spinBoxLimit->value();
    unsigned int numThreads = ui->sliderThreads->value();

    ui->textOutput->append("Calculation started");
    ui->textOutput->append(QString("Limit: %1, Threads: %2").arg(maxLimit).arg(numThreads));

    setRunningState(true);

    if (solver)
    {
        delete solver;
    }
    solver = new CollatzSolver();

    calculationFuture = std::async(std::launch::async, [this, maxLimit, numThreads]()
    {
        return solver->solve(maxLimit, numThreads);
    });

    timer->start();
}

void MainWindow::onStopClicked()
{
    if (solver)
    {
        solver->requestStop();
        ui->textOutput->append("Stop request sent...");
    }
    ui->btnStop->setEnabled(false);
}

void MainWindow::onCalculationFinished(const FinalResult& result)
{
    if (result.stoppedByUser)
    {
        ui->textOutput->append("Calculation stopped by user.");
    }
    else if (result.overflowOccured)
    {
        ui->textOutput->append("Error: Overflow occurred. Exceeded x64 addressable memory limits.");
    }
    else
    {
        ui->textOutput->append("Result successfully calculated:");
        ui->textOutput->append(QString("  Number with longest chain: %1").arg(result.maxNumber));
        ui->textOutput->append(QString("  Chain length: %1").arg(result.maxLength));
        ui->textOutput->append(QString("  Calculation time: %1 ms").arg(result.durationMs));
    }
    ui->textOutput->append("----------\n");

    delete solver;
    solver = nullptr;

    setRunningState(false);
}