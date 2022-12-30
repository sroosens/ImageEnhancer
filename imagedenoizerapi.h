#ifndef IMAGEDENOIZE_H
#define IMAGEDENOIZE_H

#include <QThread>

#include <opencv2/opencv.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include <QPixmap>

typedef enum
{
    TypeGaussianBlur = 0,
    TypeMedianBlur = 1,
    TypeNlMeans = 2
} ProcessType;

typedef struct
{
    // For GaussianBlur
    int sigma;
    int kernelSizeWidth;
    int kernelSizeHeight;
    // For MedianBlur
    int aperture;
} ProcessParameters;

class ImageDenoizeAPI : public QThread
{
  Q_OBJECT
public:
    ImageDenoizeAPI();
    ~ImageDenoizeAPI();

public slots:
    // Thread management
    void start(void) { bRunning = true; QThread::start(); }
    void stop(void) { bRunning = false; QThread::exit(); }

    // Load image
    bool bLoadImage(QString _file);

    // Image processes
    bool bApplyImageEditing(int _brigthness, int _contrast);
    bool bApplyDenoize(ProcessType _type, ProcessParameters _params);

    // Getter
    QImage GetImage();

    // Save file
    bool bSaveImage(QString _file, QImage _image);

    // Add other processing functions;

private slots:
    void run();

signals:
    void updatedDenoizeImg(const QImage &_frame);
    void updatedEditedImg(const QImage &_frame);

private:
    bool bCheckDenoizeParams(ProcessType _type, ProcessParameters &_params);
    bool bCheckImageEditingValues(int _brightness, int _contrast);
    bool bIsOdd(int _num);

    cv::Mat m_originalImg;
    cv::Mat m_curImg;
    bool bRunning;
};

#endif // IMAGEDENOIZE_H
