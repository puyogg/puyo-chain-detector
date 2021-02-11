#include "mainwindow.hpp"
#include "./ui_mainwindow.h"
#include <iostream>
#include <fstream>
#include <QMessageBox>
#include "detector/Detector.hpp"
#include <thread>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Set up preview window
    ui->graphicsView->setScene(new QGraphicsScene(this));
    ui->graphicsView->scene()->addItem(&pixmap);

    // Thread Status
    m_threadStatus = ThreadStatus{ false, false, true, true };

    // Load JSON Data
    loadSettingsJSON();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::loadSettingsJSON() {
//    std::ifstream stream("detectionsettings.json", std::ifstream::binary);
//    Json::Value json;
//    stream >> json;
//    for (int p = 0; p < 2; p++) {
//        std::cout << p;
//        player.at(p).gameField_x = json[p]["gameField_x"].asInt();
//        player.at(p).gameField_y = json[p]["gameField_y"].asInt();
//        player.at(p).gameField_width = json[p]["gameField_width"].asInt();
//        player.at(p).gameField_height = json[p]["gameField_height"].asInt();
//        player.at(p).nextWindow_x = json[p]["nextWindow_x"].asInt();
//        player.at(p).nextWindow_y = json[p]["nextWindow_y"].asInt();
//        player.at(p).nextWindow_width = json[p]["nextWindow_width"].asInt();
//        player.at(p).nextWindow_height = json[p]["nextWindow_height"].asInt();
//        player.at(p).score_x = json[p]["score_x"].asInt();
//        player.at(p).score_y = json[p]["score_y"].asInt();
//        player.at(p).score_width = json[p]["score_width"].asInt();
//        player.at(p).score_height = json[p]["score_height"].asInt();
//    }
//    stream.close();
    return;
}

void MainWindow::saveSettingsJSON() {
//    Json::Value output(Json::arrayValue);
//    for (int p = 0; p < 2; p++) {
//        Json::Value data;
//        data["gameField_x"] = player.at(p).gameField_x;
//        data["gameField_y"] = player.at(p).gameField_y;
//        data["gameField_width"] = player.at(p).gameField_width;
//        data["gameField_height"] = player.at(p).gameField_height;
//        data["nextWindow_x"] = player.at(p).nextWindow_x;
//        data["nextWindow_y"] = player.at(p).nextWindow_y;
//        data["nextWindow_width"] = player.at(p).nextWindow_width;
//        data["nextWindow_height"] = player.at(p).nextWindow_height;
//        data["score_x"] = player.at(p).score_x;
//        data["score_y"] = player.at(p).score_y;
//        data["score_width"] = player.at(p).score_width;
//        data["score_height"] = player.at(p).score_height;
//        output.append(data);
//    }

//    //https://stackoverflow.com/questions/4289986/jsoncpp-writing-to-files
//    Json::StreamWriterBuilder builder;
//    builder["commentStyle"] = "None";
//    builder["indentation"] = "    ";
//    std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
//    std::ofstream outputFileStream("detectionsettings.json");
//    writer->write(output, &outputFileStream);
    return;
}

void MainWindow::on_saveSettings_clicked()
{
    saveSettingsJSON();
    QMessageBox::information(this, "Saved JSON", "Saved data to detectionsettings.json");
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    m_threadStatus.runDetector = false;
    m_threadStatus.runWebsocket = false;

    // Block until the detector closes
    while(!m_threadStatus.detectorClosed)
    {
    }

    event->accept();
}

void MainWindow::on_startCapture_clicked()
{
    // If the detection and websocket threads are running, close them.
    if (m_threadStatus.runDetector || m_threadStatus.runWebsocket)
    {
        m_threadStatus.runDetector = false;
        m_threadStatus.runWebsocket = false;
        // Block until detector is closed
        while(!m_threadStatus.detectorClosed)
        {
        }
        ui->startCapture->setText("Start Capture");
        return;
    }

    // Otherwise, pass data to chain detector method and start a new thread.
    m_threadStatus.runDetector = true;
    m_threadStatus.runWebsocket = true;
    ui->startCapture->setText("Stop Capture");

    int deviceID = ui->cameraValue->text().toInt();
    int modeID = static_cast<int>(cv::CAP_DSHOW);
    std::thread detectThread(ChainDetector::Detector, std::ref(m_threadStatus), std::ref(m_qtPreview), deviceID, modeID);
    detectThread.detach();

    cv::Mat qtPreviewRGB;
    while(m_threadStatus.runDetector) {
        if (!m_qtPreview.empty()) {
            cv::cvtColor(m_qtPreview, qtPreviewRGB, cv::COLOR_BGR2RGB);
            QImage qimg(qtPreviewRGB.data,
                        qtPreviewRGB.cols,
                        qtPreviewRGB.rows,
                        qtPreviewRGB.step,
                        QImage::Format_RGB888);
            pixmap.setPixmap(QPixmap::fromImage(qimg));
            ui->graphicsView->fitInView(&pixmap, Qt::KeepAspectRatio);
        }
        qApp->processEvents();
    }
    m_threadStatus.runDetector = false;
    m_threadStatus.runWebsocket = false;
    // Block until detector is closed
    while(!m_threadStatus.detectorClosed)
    {
    }
    ui->startCapture->setText("Start Capture");
}
