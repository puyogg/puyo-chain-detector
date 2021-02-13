#include "mainwindow.hpp"
#include "./ui_mainwindow.h"
#include <iostream>
#include <fstream>
#include <QMessageBox>
#include "detector/Detector.hpp"
#include <thread>
#include <QTimer>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Set up preview window
    ui->graphicsView->setScene(new QGraphicsScene(this));
    ui->graphicsView->scene()->addItem(&pixmap);

    // Thread Status
    m_threadStatus = ThreadStatus{ false, false, true, true, false, false };

    // Load JSON Data
    loadSettingsJSON();

    m_qtPreview = cv::Mat(270, 480, CV_8UC3);

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::updatePixmap);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setCaptureMode(int mode)
{
    switch(mode)
    {
        case cv::CAP_DSHOW: //700
            ui->captureMethod->setCurrentIndex(1);
            m_captureSettings.mode = cv::CAP_DSHOW;
            break;
        default: // cv::CAP_ANY, 0
            ui->captureMethod->setCurrentIndex(0);
            m_captureSettings.mode = cv::CAP_ANY;
    }
}

void MainWindow::setDevice(int deviceID)
{
    std::string idString = std::to_string(deviceID);
    ui->cameraValue->setText(QString::fromStdString(idString));
    m_captureSettings.deviceID = deviceID;
}

void MainWindow::setPresetName(std::string shortName)
{
    if (shortName == "PPT2")
    {
        ui->gamePreset->setCurrentIndex(0);
        m_captureSettings.preset = "PPT2";
    }
    else if (shortName == "PPe")
    {
        ui->gamePreset->setCurrentIndex(1);
        m_captureSettings.preset = "PPe";
    }
}

void MainWindow::setPresetName(int presetIdx)
{
    switch(presetIdx)
    {
    case 0:
        m_captureSettings.preset = "PPT2";
        break;
    case 1:
        m_captureSettings.preset = "PPe";
        break;
    default:
        m_captureSettings.preset = "PPT2";
    }
}

void MainWindow::loadSettingsJSON() {
    std::ifstream stream("settings.json", std::ifstream::binary);
    Json::Value json;
    stream >> json;

    // Call setup methods
    setDevice(json["device"].asInt());
    setCaptureMode(json["mode"].asInt());
    setPresetName(json["preset"].asString());

    // Get the other settings
    std::string& preset = m_captureSettings.preset;
    m_captureSettings.bonusThreshold = json["region"][preset]["bonus_thresh"].asDouble();
    bool enablePreview = json["preview"].asBool();
    m_captureSettings.enablePreview = enablePreview;
    Qt::CheckState previewState = enablePreview ? Qt::CheckState::Checked : Qt::CheckState::Unchecked;
    ui->enablePreview->setCheckState(previewState);

    for (int p = 0; p < 2; p++) {
        ChainDetector::PlayerRects& playerRect = m_captureSettings.player.at(p);
        playerRect.fieldRect.x = json["region"][preset]["player"][p]["field"]["x"].asInt();
        playerRect.fieldRect.y = json["region"][preset]["player"][p]["field"]["y"].asInt();
        playerRect.fieldRect.width = json["region"][preset]["player"][p]["field"]["w"].asInt();
        playerRect.fieldRect.height = json["region"][preset]["player"][p]["field"]["h"].asInt();
        playerRect.scoreRect.x = json["region"][preset]["player"][p]["score"]["x"].asInt();
        playerRect.scoreRect.y = json["region"][preset]["player"][p]["score"]["y"].asInt();
        playerRect.scoreRect.width = json["region"][preset]["player"][p]["score"]["w"].asInt();
        playerRect.scoreRect.height = json["region"][preset]["player"][p]["score"]["h"].asInt();
        playerRect.bonusRect.x = json["region"][preset]["player"][p]["bonus"]["x"].asInt();
        playerRect.bonusRect.y = json["region"][preset]["player"][p]["bonus"]["y"].asInt();
        playerRect.bonusRect.width = json["region"][preset]["player"][p]["bonus"]["w"].asInt();
        playerRect.bonusRect.height = json["region"][preset]["player"][p]["bonus"]["h"].asInt();
        for (int i = 0; i < 4; i++)
        {
            playerRect.nextRects.at(i).x = json["region"][preset]["player"][p]["next"][i]["x"].asInt();
            playerRect.nextRects.at(i).y = json["region"][preset]["player"][p]["next"][i]["y"].asInt();
            playerRect.nextRects.at(i).width = json["region"][preset]["player"][p]["next"][i]["w"].asInt();
            playerRect.nextRects.at(i).height = json["region"][preset]["player"][p]["next"][i]["h"].asInt();
        }
    }

    stream.close();
    return;
}

void MainWindow::saveSettingsJSON() {
    // Load the json file again to get its structure
    std::ifstream stream("settings.json", std::ifstream::binary);
    Json::Value json;
    stream >> json;

    json["mode"] = m_captureSettings.mode;
    json["device"] = m_captureSettings.deviceID;
    json["preset"] = m_captureSettings.preset;
    json["preview"] = m_captureSettings.enablePreview;

    //https://stackoverflow.com/questions/4289986/jsoncpp-writing-to-files
    Json::StreamWriterBuilder builder;
    builder["commentStyle"] = "None";
    builder["indentation"] = "    ";
    std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
    std::ofstream outputFileStream("settings.json");
    writer->write(json, &outputFileStream);
    return;
}

void MainWindow::on_saveSettings_clicked()
{
    saveSettingsJSON();
    QMessageBox::information(this, "Saved JSON", "Saved data to settings.json");
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    m_threadStatus.runDetector = false;
    m_threadStatus.runWebsocket = false;

    // Block until the detector closes
    if (m_detectorThread.joinable())
    {
        m_detectorThread.join();
    }

    event->accept();
}

void MainWindow::updatePixmap()
{
    cv::Mat qtPreviewRGB;
    if (!m_qtPreview.empty() && m_captureSettings.enablePreview)
    {
        cv::cvtColor(m_qtPreview, qtPreviewRGB, cv::COLOR_BGR2RGB);
        QImage qimg(qtPreviewRGB.data,
                    qtPreviewRGB.cols,
                    qtPreviewRGB.rows,
                    qtPreviewRGB.step,
                    QImage::Format_RGB888);
        pixmap.setPixmap(QPixmap::fromImage(qimg));
        ui->graphicsView->fitInView(&pixmap, Qt::KeepAspectRatio);
    }

    if (m_threadStatus.captureFailed)
    {
        m_threadStatus.captureFailed = false; // reset
        QMessageBox::warning(this, "Video Error", "The chain detector couldn't open your capture device.\nTry a different device id.");
        timer->stop();
        m_threadStatus.runDetector = false;
        m_threadStatus.runWebsocket = false;
        ui->cameraValue->setEnabled(true);
        ui->captureMethod->setEnabled(true);
        ui->startCapture->setText("Start Chain Detector");
        return;
    }

    if (!m_captureSettings.enablePreview)
    {
        timer->stop();
        return;
    }
}

void MainWindow::on_startCapture_clicked()
{
    std::cout << "Capture button clicked." << std::endl;

    // If the detection and websocket threads are running, close them.
    if (m_threadStatus.runDetector || m_threadStatus.runWebsocket)
    {
        m_threadStatus.runDetector = false;
        m_threadStatus.runWebsocket = false;
        // Block until detector is closed
        if (m_detectorThread.joinable()){
            m_detectorThread.join();
        }
        ui->startCapture->setText("Start Chain Detector");
        ui->cameraValue->setEnabled(true);
        ui->captureMethod->setEnabled(true);
        return;
    }

    // Otherwise, pass data to chain detector method and start a new thread.
    m_threadStatus.runDetector = true;
    m_threadStatus.runWebsocket = true;
    ui->startCapture->setText("Stop Chain Detector");

    // Update settings object
    int deviceID = ui->cameraValue->text().toInt();
    setDevice(deviceID);
    int modeID = static_cast<int>(ui->captureMethod->currentIndex()) == 0 ? cv::CAP_ANY : cv::CAP_DSHOW;
    setCaptureMode(modeID);
    int presetIdx = ui->gamePreset->currentIndex();
    setPresetName(presetIdx);

    // Disable the device and mode widgets
    ui->cameraValue->setEnabled(false);
    ui->captureMethod->setEnabled(false);

    std::thread detectThread(ChainDetector::Detector, std::ref(m_threadStatus), std::ref(m_qtPreview), std::ref(m_captureSettings));
    m_detectorThread = std::move(detectThread);
    m_detectorThread.detach();
    timer->start(33);
    return;
}

void MainWindow::on_gamePreset_currentIndexChanged(int index)
{
    // Get the preset name as a string
    setPresetName(index); // updates m_captureSettings.preset
    std::string& preset = m_captureSettings.preset;

    // Read the json file again
    std::ifstream stream("settings.json", std::ifstream::binary);
    Json::Value json;
    stream >> json;

    // update blackfraction threshold
    m_captureSettings.bonusThreshold = json["region"][preset]["bonus_thresh"].asDouble();

    for (int p = 0; p < 2; p++)
    {
        ChainDetector::PlayerRects& playerRect = m_captureSettings.player.at(p);
        playerRect.fieldRect.x = json["region"][preset]["player"][p]["field"]["x"].asInt();
        playerRect.fieldRect.y = json["region"][preset]["player"][p]["field"]["y"].asInt();
        playerRect.fieldRect.width = json["region"][preset]["player"][p]["field"]["w"].asInt();
        playerRect.fieldRect.height = json["region"][preset]["player"][p]["field"]["h"].asInt();
        playerRect.scoreRect.x = json["region"][preset]["player"][p]["score"]["x"].asInt();
        playerRect.scoreRect.y = json["region"][preset]["player"][p]["score"]["y"].asInt();
        playerRect.scoreRect.width = json["region"][preset]["player"][p]["score"]["w"].asInt();
        playerRect.scoreRect.height = json["region"][preset]["player"][p]["score"]["h"].asInt();
        playerRect.bonusRect.x = json["region"][preset]["player"][p]["bonus"]["x"].asInt();
        playerRect.bonusRect.y = json["region"][preset]["player"][p]["bonus"]["y"].asInt();
        playerRect.bonusRect.width = json["region"][preset]["player"][p]["bonus"]["w"].asInt();
        playerRect.bonusRect.height = json["region"][preset]["player"][p]["bonus"]["h"].asInt();
        for (int i = 0; i < 4; i++)
        {
            playerRect.nextRects.at(i).x = json["region"][preset]["player"][p]["next"][i]["x"].asInt();
            playerRect.nextRects.at(i).y = json["region"][preset]["player"][p]["next"][i]["y"].asInt();
            playerRect.nextRects.at(i).width = json["region"][preset]["player"][p]["next"][i]["w"].asInt();
            playerRect.nextRects.at(i).height = json["region"][preset]["player"][p]["next"][i]["h"].asInt();
        }
    }
}

void MainWindow::on_enablePreview_stateChanged(int arg1)
{
    bool enabled = arg1 == Qt::CheckState::Checked;
    m_captureSettings.enablePreview = enabled;
    if (enabled) timer->start(33);
}

void MainWindow::on_cameraValue_textEdited(const QString &arg1)
{
    bool isInt;
    int newCameraValue = arg1.toInt(&isInt);
    if (isInt)
    {
        m_captureSettings.deviceID = newCameraValue;
    }
    else
    {
        m_captureSettings.deviceID = newCameraValue;
        ui->cameraValue->setText("0");
    }
}

void MainWindow::on_captureMethod_currentIndexChanged(int index)
{
    setCaptureMode(index == 0 ? cv::CAP_ANY : cv::CAP_DSHOW);
}
