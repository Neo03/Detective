#include "mainwindow.h"
#include <QGridLayout>
#include <QApplication>
#include <QCameraInfo>
#include <QMessageBox>
#include <QFileInfoList>
#include <QIcon>
#include <QDir>
#include <QStatusBar>

#include "capture_thread.h"
#include "utilities.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    fileMenu(nullptr),
    capturer(nullptr)
    
{    
    initUI();
    data_lock = new QMutex();
}

MainWindow::~MainWindow()
{
    delete capturer;
}

void MainWindow::initUI(){
    this->resize(1000, 800);

    //setup menuBar
    fileMenu = menuBar()->addMenu("&File");
    //main area
    QGridLayout *main_layout = new QGridLayout();

    imageScene = new QGraphicsScene(this);
    imageView = new QGraphicsView(imageScene);
    main_layout->addWidget(imageView, 0, 0, 12, 1);

    QGridLayout *tools_layout = new QGridLayout();
    main_layout->addLayout(tools_layout, 12, 0, 1, 1);    


    shutterButton = new QPushButton(this);
    shutterButton->setText("Get a photo");
    tools_layout->addWidget(shutterButton, 0, 0, Qt::AlignHCenter);
    connect(shutterButton, SIGNAL(clicked(bool)), this, SLOT(takePhoto()));
    
    QGridLayout *mask_layout = new QGridLayout();

    main_layout->addLayout(mask_layout, 13, 0, 1, 1);
    mask_layout->addWidget(new QLabel("Select Mask", this));
    for(int i = 0; i < CaptureThread::MASK_COUNT; i++){
        mask_checkboxes[i] = new QCheckBox(this);
        mask_layout->addWidget(mask_checkboxes[i], 0, i + 1);
        connect(mask_checkboxes[i], SIGNAL(stateChanged(int)), this, SLOT(updateMasks(int)));
    }

    mask_checkboxes[0]->setText("Rectangle");
    mask_checkboxes[1]->setText("Landmarks");
    mask_checkboxes[2]->setText("Glasses");
    mask_checkboxes[3]->setText("Mustache");
    mask_checkboxes[4]->setText("Mouse nose");

    // list of saved videos
    saved_list = new QListView(this);
    saved_list->setViewMode(QListView::IconMode);
    saved_list->setResizeMode(QListView::Adjust);
    saved_list->setSpacing(5);
    saved_list->setWrapping(false);
    list_model = new QStandardItemModel(this);
    saved_list->setModel(list_model);
    main_layout->addWidget(saved_list, 14, 0, 4, 1);

    QWidget *widget = new QWidget();
    widget->setLayout(main_layout);
    setCentralWidget(widget);

    //setup Status bar
    mainStatusBar = statusBar();
    mainStatusLabel = new QLabel(mainStatusBar);
    mainStatusBar->addPermanentWidget(mainStatusLabel);
    mainStatusLabel->setText("Facetious is ready");

    createActions();

}

void MainWindow::createActions(){
    cameraInfoAction = new QAction("Camera &Information", this);
    fileMenu->addAction(cameraInfoAction);
    openCameraAction = new QAction("Open &Camera", this);
    fileMenu->addAction(openCameraAction);    
    exitAction = new QAction("&Exit");
    fileMenu->addAction(exitAction);

    connect(exitAction, SIGNAL(triggered(bool)), QApplication::instance(), SLOT(quit()));
    connect(cameraInfoAction, SIGNAL(triggered(bool)), this, SLOT(showCameraInfo()));
    connect(openCameraAction, SIGNAL(triggered(bool)), this, SLOT(openCamera()));
    

}

void MainWindow::showCameraInfo(){
    QList<QCameraInfo> cameras = QCameraInfo::availableCameras();
    QString info = QString("Avalable Cameras: \n");

    foreach(const QCameraInfo &cameraInfo, cameras){
        info += " - " + cameraInfo.deviceName() + ": ";
        info += cameraInfo.description() + "\n";
    }
    QMessageBox::information(this, "Cameras:", info);
}

void MainWindow::openCamera(){
    if(capturer != nullptr){
        // if a thread is already running, stop it
        capturer->setRunning(false);
        disconnect(capturer, &CaptureThread::frameCaptured, this, &MainWindow::updateFrame);
        disconnect(capturer, &CaptureThread::photoTaken, this, &MainWindow::appendSavedPhoto);
        connect(capturer, &CaptureThread::finished, capturer, &CaptureThread::deleteLater);
    }
    int camID = 0;
    capturer = new CaptureThread(camID, data_lock);
  
    connect(capturer, &CaptureThread::frameCaptured, this, &MainWindow::updateFrame);
    connect(capturer, &CaptureThread::photoTaken, this, &MainWindow::appendSavedPhoto);
    capturer->start();
    mainStatusLabel->setText(QString("Capturing Camera %1").arg(camID));
    //mainStatusLabel->setText(QString("Capturing Camera %1").arg(pathVideo));    

}


void MainWindow::updateFrame(cv::Mat *mat){
    data_lock->lock();
    currentFrame = *mat;
    data_lock->unlock();

    QImage frame(currentFrame.data, currentFrame.cols, currentFrame.rows,
                currentFrame.step, QImage::Format_RGB888);
    QPixmap image = QPixmap::fromImage(frame);
    imageScene->clear();
    imageView->resetMatrix();
    imageScene->addPixmap(image);
    imageScene->update();
    imageView->setSceneRect(image.rect());

}

void MainWindow::populateSavedList(){
    QDir dir(Utilities::getDataPath());
    QStringList nameFilters;
    nameFilters << "*.jpg";
    QFileInfoList files = dir.entryInfoList(nameFilters, QDir::NoDotAndDotDot | QDir::Files, QDir::Name);

    foreach(QFileInfo photo, files){
        QString name = photo.baseName();
        QStandardItem *item = new QStandardItem();
        list_model->appendRow(item);
        QModelIndex index = list_model->indexFromItem(item);
        list_model->setData(index, QPixmap(photo.absolutePath()).scaledToHeight(145), Qt::DecorationRole);
        list_model->setData(index, name, Qt::DisplayRole);
    }

}

void MainWindow::appendSavedPhoto(QString name){
    QString photo_path = Utilities::getPhotoPath(name, "jpg");
    QStandardItem *item = new QStandardItem();
    list_model->appendRow(item);
    QModelIndex index = list_model->indexFromItem(item);
    list_model->setData(index, QPixmap(photo_path).scaledToHeight(145), Qt::DecorationRole);
    list_model->setData(index, name, Qt::DisplayRole);
    saved_list->scrollTo(index);
}

void MainWindow::takePhoto(){
    if(capturer != nullptr){
        capturer->takePhoto();
    }
}

void MainWindow::updateMasks(int status){
    if(capturer == nullptr){
        return;
    }

    QCheckBox *box = qobject_cast<QCheckBox*>(sender());
    for(int i = 0; i < CaptureThread::MASK_COUNT; i++){
        if(mask_checkboxes[i] == box){
            capturer->updateMaskFlag(static_cast<CaptureThread::MASK_TYPE>(i), status !=0);
        }
    }
}






















