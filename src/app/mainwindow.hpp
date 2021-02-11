#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QImage>
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

private:
    Ui::MainWindow *ui;

    QGraphicsPixmapItem pixmap;
    cv::VideoCapture video;
    cv::Mat m_qtPreview;
    ThreadStatus m_threadStatus;

    void drawBoundingBoxes(cv::Mat &frame);
    void setDataToFields(int p);
    void saveFieldsToData();

    void loadSettingsJSON();
    void saveSettingsJSON();
};
#endif // MAINWINDOW_H
