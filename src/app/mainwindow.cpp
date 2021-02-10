#include "mainwindow.hpp"
#include "./ui_mainwindow.h"
#include <iostream>
#include <fstream>
#include <QMessageBox>
#include "detector/Detector.hpp"
#include <thread>
#include <atomic>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->graphicsView->setScene(new QGraphicsScene(this));
    ui->graphicsView->scene()->addItem(&pixmap);

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

void MainWindow::on_startPreview_clicked()
{
    // If the video capture is already open, then the user
    // is clicking the button to stop it.
    if (video.isOpened()) {
        ui->startPreview->setText("Start");
        video.release();
        return;
    }

    // Check if the user gave an ID integer, or a path string.
    bool isCameraID;
    int cameraIndex = ui->cameraValue->text().toInt(&isCameraID);

    if(isCameraID) {
        // Use the cameraIndex to open the video feed,
        // and also record whether it was successful or not.
        bool openSuccessful = video.open(cameraIndex, cv::CAP_ANY);
        if (!openSuccessful) {
            QMessageBox::critical(this, "Capture Error", "Enter the correct index <br> or make sure the video capture isn't being used by another program.");
            return;
        }
    }

    // Set video to default to 1920x1080
    video.set(cv::CAP_PROP_FRAME_HEIGHT, 1080);
    video.set(cv::CAP_PROP_FRAME_WIDTH, 1920);

    ui->startPreview->setText("Stop");

    cv::Mat frame;
    while(video.isOpened()) {
        video >> frame;
        cv::resize(frame, frame, cv::Size(480, 270), 0, 0, cv::INTER_LINEAR);
//        drawBoundingBoxes(frame);
        cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB);
        if (!frame.empty()) {
            QImage qimg(frame.data,
                        frame.cols,
                        frame.rows,
                        frame.step,
                        QImage::Format_RGB888);
            pixmap.setPixmap(QPixmap::fromImage(qimg));
            ui->graphicsView->fitInView(&pixmap, Qt::KeepAspectRatio);

        }
        qApp->processEvents();
    }

    video.release();
    ui->startPreview->setText("Start");
}

// There's probably a more efficient way to map all these events but
// I'm too tired to figure it out right now lol.
void MainWindow::on_initializeDetector_clicked()
{
    // http://www.cplusplus.com/forum/general/190480/
    int deviceID = ui->cameraValue->text().toInt();
    int modeID = static_cast<int>(cv::CAP_ANY);
    std::atomic<bool> running { true };
    std::thread detectThread(ChainDetector::Detector, std::ref(running), deviceID, modeID);

    while (running)
    {
        qApp->processEvents();
    }

    detectThread.join();
    std::cout << "Finished detection thread.\n";
}
