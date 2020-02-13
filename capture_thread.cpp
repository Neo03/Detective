#include <QTime>
#include <QDebug>

#include "utilities.h"
#include "capture_thread.h"

CaptureThread::CaptureThread(int camera, QMutex *lock):
    running(false), cameraID(camera), videoPath(""), data_lock(lock)
{
    frame_width = frame_height = 0;
    taking_photo = false;
}

CaptureThread::CaptureThread(QString videoPath, QMutex *lock):
    running(false), cameraID(-1), videoPath(videoPath), data_lock(lock)
{
    frame_width = frame_height = 0;
    taking_photo = false;

}

CaptureThread::~CaptureThread() {
}

void CaptureThread::run() {
    running = true;
    //cv::VideoCapture cap(cameraID);
    cv::VideoCapture cap("/root/Video/Cat1_1.mp4");
    cv::Mat tmp_frame;

    frame_width = cap.get(cv::CAP_PROP_FRAME_WIDTH);
    frame_height = cap.get(cv::CAP_PROP_FRAME_HEIGHT);

    classifier = new cv::CascadeClassifier(OPENCV_DATA_DIR "haarcascades/haarcascade_frontalcatface_extended.xml");
    // classifier = new cv::CascadeClassifier("../boston-bull/cascade.xml");
    // classifier = new cv::CascadeClassifier("../no-entry/cascade.xml");
    while(running) {        
        cap >> tmp_frame;
        if (tmp_frame.empty()) {
            break;
        }
        // tmp_frame = cv::imread("../boston-bull/boston-bull-predict.png");
        // tmp_frame = cv::imread("../no-entry/no-entry-predict.png");
        if(taking_photo) {
            takePhoto(tmp_frame);
        }

#ifdef TIME_MEASURE
        int64 t0 = cv::getTickCount();
#endif
        // определение объектов
        detectObjects(tmp_frame);

#ifdef TIME_MEASURE
        int64 t1 = cv::getTickCount();
        double t = (t1-t0) * 1000 /cv::getTickFrequency();
        qDebug() << "Detecting time on a single frame: " << t <<"ms";
#endif

        cvtColor(tmp_frame, tmp_frame, cv::COLOR_BGR2RGB);
        data_lock->lock();
        frame = tmp_frame;
        data_lock->unlock();
        emit frameCaptured(&frame);
    }
    cap.release();
    delete classifier;
    classifier = nullptr;
    running = false;
}


void CaptureThread::takePhoto(cv::Mat &frame)
{
    QString photo_name = Utilities::newPhotoName();
    QString photo_path = Utilities::getPhotoPath(photo_name, "jpg");
    cv::imwrite(photo_path.toStdString(), frame);
    emit photoTaken(photo_name);
    taking_photo = false;
}


void CaptureThread::detectObjects(cv::Mat &frame)
{
    vector<cv::Rect> objects;

    classifier->detectMultiScale(frame, objects, 1.3, 5);

    cv::Scalar color = cv::Scalar(0, 0, 255); // red

    //рисуем ограниченные квадраты
    for(size_t i = 0; i < objects.size(); i++){
        cv::rectangle(frame, objects[i], color, 2);
    }
}
































































