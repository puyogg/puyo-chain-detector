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

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_startPreview_clicked();

    void on_gameField_x_valueChanged(int arg1);
    void on_gameField_y_valueChanged(int arg1);
    void on_gameField_width_valueChanged(int arg1);
    void on_gameField_height_valueChanged(int arg1);
    void on_nextWindow_x_valueChanged(int arg1);
    void on_nextWindow_y_valueChanged(int arg1);
    void on_nextWindow_width_valueChanged(int arg1);
    void on_nextWindow_height_valueChanged(int arg1);
    void on_score_x_valueChanged(int arg1);
    void on_score_y_valueChanged(int arg1);
    void on_score_width_valueChanged(int arg1);
    void on_score_height_valueChanged(int arg1);

    void on_currentPlayer_currentIndexChanged(int index);

    void on_saveSettings_clicked();

private:
    Ui::MainWindow *ui;

    QGraphicsPixmapItem pixmap;
    cv::VideoCapture video;

    struct PlayerData {
        int gameField_x;
        int gameField_y;
        int gameField_width;
        int gameField_height;
        int nextWindow_x;
        int nextWindow_y;
        int nextWindow_width;
        int nextWindow_height;
        int score_x;
        int score_y;
        int score_width;
        int score_height;
    };
    std::array<PlayerData, 2> player;

    int currentPlayer;

    void drawBoundingBoxes(cv::Mat &frame);
    void setDataToFields(int p);
    void saveFieldsToData();

    void loadSettingsJSON();
    void saveSettingsJSON();
};
#endif // MAINWINDOW_H
