#include "mainwindow.hpp"
#include "./ui_mainwindow.h"
#include <iostream>
#include <fstream>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->graphicsView->setScene(new QGraphicsScene(this));
    ui->graphicsView->scene()->addItem(&pixmap);

    // Load JSON Data
    loadSettingsJSON();

    // Set some default data
    currentPlayer = 0;
    setDataToFields(0);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setDataToFields(int p) {
    ui->gameField_x->setValue(player.at(p).gameField_x);
    ui->gameField_y->setValue(player.at(p).gameField_y);
    ui->gameField_width->setValue(player.at(p).gameField_width);
    ui->gameField_height->setValue(player.at(p).gameField_height);
    ui->nextWindow_x->setValue(player.at(p).nextWindow_x);
    ui->nextWindow_y->setValue(player.at(p).nextWindow_y);
    ui->nextWindow_width->setValue(player.at(p).nextWindow_width);
    ui->nextWindow_height->setValue(player.at(p).nextWindow_height);
    ui->score_x->setValue(player.at(p).score_x);
    ui->score_y->setValue(player.at(p).score_y);
    ui->score_width->setValue(player.at(p).score_width);
    ui->score_height->setValue(player.at(p).score_height);
    return;
}

void MainWindow::drawBoundingBoxes(cv::Mat &frame) {
    for (int p = 0; p < 2; p++) {
        cv::Scalar color { p == 0 ? cv::Scalar(255, 255, 0) : cv::Scalar(0, 255, 255) };
        int thickness { 3 };

        cv::rectangle(frame,
                      cv::Rect(
                          player.at(p).gameField_x,
                          player.at(p).gameField_y,
                          player.at(p).gameField_width,
                          player.at(p).gameField_height
                      ),
                      color,
                      thickness);
        cv::rectangle(frame,
                      cv::Rect(
                          player.at(p).nextWindow_x,
                          player.at(p).nextWindow_y,
                          player.at(p).nextWindow_width,
                          player.at(p).nextWindow_height
                      ),
                      color,
                      thickness);
        cv::rectangle(frame,
                      cv::Rect(
                          player.at(p).score_x,
                          player.at(p).score_y,
                          player.at(p).score_width,
                          player.at(p).score_height
                      ),
                      color,
                      thickness);
    }
}

void MainWindow::loadSettingsJSON() {
    std::ifstream stream("detectionsettings.json", std::ifstream::binary);
    Json::Value json;
    stream >> json;
    for (int p = 0; p < 2; p++) {
        std::cout << p;
        player.at(p).gameField_x = json[p]["gameField_x"].asInt();
        player.at(p).gameField_y = json[p]["gameField_y"].asInt();
        player.at(p).gameField_width = json[p]["gameField_width"].asInt();
        player.at(p).gameField_height = json[p]["gameField_height"].asInt();
        player.at(p).nextWindow_x = json[p]["nextWindow_x"].asInt();
        player.at(p).nextWindow_y = json[p]["nextWindow_y"].asInt();
        player.at(p).nextWindow_width = json[p]["nextWindow_width"].asInt();
        player.at(p).nextWindow_height = json[p]["nextWindow_height"].asInt();
        player.at(p).score_x = json[p]["score_x"].asInt();
        player.at(p).score_y = json[p]["score_y"].asInt();
        player.at(p).score_width = json[p]["score_width"].asInt();
        player.at(p).score_height = json[p]["score_height"].asInt();
    }
    stream.close();
    return;
}

void MainWindow::saveSettingsJSON() {
    Json::Value output(Json::arrayValue);
    for (int p = 0; p < 2; p++) {
        Json::Value data;
        data["gameField_x"] = player.at(p).gameField_x;
        data["gameField_y"] = player.at(p).gameField_y;
        data["gameField_width"] = player.at(p).gameField_width;
        data["gameField_height"] = player.at(p).gameField_height;
        data["nextWindow_x"] = player.at(p).nextWindow_x;
        data["nextWindow_y"] = player.at(p).nextWindow_y;
        data["nextWindow_width"] = player.at(p).nextWindow_width;
        data["nextWindow_height"] = player.at(p).nextWindow_height;
        data["score_x"] = player.at(p).score_x;
        data["score_y"] = player.at(p).score_y;
        data["score_width"] = player.at(p).score_width;
        data["score_height"] = player.at(p).score_height;
        output.append(data);
    }

    //https://stackoverflow.com/questions/4289986/jsoncpp-writing-to-files
    Json::StreamWriterBuilder builder;
    builder["commentStyle"] = "None";
    builder["indentation"] = "    ";
    std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
    std::ofstream outputFileStream("detectionsettings.json");
    writer->write(output, &outputFileStream);
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
//        bool openSuccessful = video.open(cameraIndex, cv::CAP_DSHOW);
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
//        cv::resize(frame, frame, cv::Size(1920, 1080), 0, 0, cv::INTER_LINEAR);
        drawBoundingBoxes(frame);
        cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB);
        if (!frame.empty()) {
            // std::cout << "Got frame of type: " << frame.type() << ", " << CV_8UC3 << std::endl;
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
void MainWindow::on_gameField_x_valueChanged(int arg1)
{
    player.at(currentPlayer).gameField_x = arg1;
}
void MainWindow::on_gameField_y_valueChanged(int arg1)
{
    player.at(currentPlayer).gameField_y = arg1;
}
void MainWindow::on_gameField_width_valueChanged(int arg1)
{
    player.at(currentPlayer).gameField_width = arg1;
}
void MainWindow::on_gameField_height_valueChanged(int arg1)
{
    player.at(currentPlayer).gameField_height = arg1;
}
void MainWindow::on_nextWindow_x_valueChanged(int arg1)
{
    player.at(currentPlayer).nextWindow_x = arg1;
}
void MainWindow::on_nextWindow_y_valueChanged(int arg1)
{
    player.at(currentPlayer).nextWindow_y = arg1;
}
void MainWindow::on_nextWindow_width_valueChanged(int arg1)
{
    player.at(currentPlayer).nextWindow_width = arg1;
}
void MainWindow::on_nextWindow_height_valueChanged(int arg1)
{
    player.at(currentPlayer).nextWindow_height = arg1;
}
void MainWindow::on_score_x_valueChanged(int arg1)
{
    player.at(currentPlayer).score_x = arg1;
}
void MainWindow::on_score_y_valueChanged(int arg1)
{
    player.at(currentPlayer).score_y = arg1;
}
void MainWindow::on_score_width_valueChanged(int arg1)
{
    player.at(currentPlayer).score_width = arg1;
}
void MainWindow::on_score_height_valueChanged(int arg1)
{
    player.at(currentPlayer).score_height = arg1;
}

void MainWindow::on_currentPlayer_currentIndexChanged(int index)
{
//    std::cout << "Dropdown box index: " << index << std::endl;
    currentPlayer = index;
    setDataToFields(index);
}
