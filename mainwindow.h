#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMenuBar>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QCheckBox>
#include <QPushButton>
#include <QListView>
#include <QLabel>
#include <QMutex>
#include <QStandardItemModel>
#include <QListView>
#include <QAction>

#include "opencv2/opencv.hpp"
#include "capture_thread.h"

#include <QCamera>


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();    


private:
   
    void initUI();
    void createActions();
    void populateSavedList();

private slots:
    void showCameraInfo();
    void openCamera();
    void updateFrame(cv::Mat *mat);
    void takePhoto();
    
    void appendSavedPhoto(QString name);
    void updateMasks(int status);

private:
    QMenu *fileMenu;

    QAction *cameraInfoAction;
    QAction *openCameraAction;
    QAction *exitAction;
    // выбор масок
    QCheckBox *mask_checkboxes[CaptureThread::MASK_COUNT];

    QGraphicsScene *imageScene;
    QGraphicsView *imageView;

    QPushButton *shutterButton;

    QListView *saved_list;
    QStatusBar *mainStatusBar;
    QLabel *mainStatusLabel;

    cv::Mat currentFrame;

    //for capture thread
    QMutex *data_lock;
    CaptureThread *capturer;
    // for video saving
    QStandardItemModel *list_model;

};

#endif // MAINWINDOW_H
