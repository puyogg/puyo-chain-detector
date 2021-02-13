#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QImage>
#include <QTimer>
#include <QPixmap>
#include <QCloseEvent>
#include <QMessageBox>
#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <json/json.h>
#include <array>
#include <fstream>
#include <atomic>

#include "threading.hpp"
#include "detector/Settings.hpp"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent* event);

private slots:
    void on_startCapture_clicked();
    void on_saveSettings_clicked();

    void on_gamePreset_currentIndexChanged(int index);

    void on_enablePreview_stateChanged(int arg1);

    void on_cameraValue_textEdited(const QString &arg1);

    void on_captureMethod_currentIndexChanged(int index);

    void updatePixmap();

private:
    Ui::MainWindow *ui;

    std::thread m_detectorThread;

    QGraphicsPixmapItem pixmap;
    cv::VideoCapture video;
    cv::Mat m_qtPreview;
    ThreadStatus m_threadStatus;

    void drawBoundingBoxes(cv::Mat &frame);
    void setDataToFields(int p);
    void saveFieldsToData();

    void loadSettingsJSON();
    void saveSettingsJSON();

    void setDevice(int deviceID);
    void setCaptureMode(int mode);
    void setPresetName(std::string shortName);
    void setPresetName(int presetIdx);

    ChainDetector::CaptureSettings m_captureSettings;

    QTimer* timer;
};
#endif // MAINWINDOW_H
