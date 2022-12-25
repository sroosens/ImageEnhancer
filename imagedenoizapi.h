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
    void start(void) { bRunning = true; QThread::start(); }
    void stop(void) { bRunning = false; QThread::exit(); }

    bool bDenoize(QString file, ProcessType type, ProcessParameters params);

    // Add other processing functions;

private slots:
    void run();

signals:
    void updatedImg(const QPixmap &frame);

private:
    bool bCheckParams(ProcessType type, ProcessParameters &params);
    bool bIsOdd(int num);

    bool bRunning;
};

#endif // IMAGEDENOIZE_H
