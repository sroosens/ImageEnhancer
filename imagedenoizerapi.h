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

    // Denoize process
    bool bApplyDenoize(QString _file, ProcessType _type, ProcessParameters _params);
    bool bApplyDenoize(QString _file, ProcessType _type, ProcessParameters _params, QImage &_out);

    // Save file
    bool bSaveImage(QString _file, QImage _image);

    // Add other processing functions;

private slots:
    void run();

signals:
    void updatedImg(const QImage &_frame);

private:
    bool bCheckParams(ProcessType _type, ProcessParameters &_params);
    bool bIsOdd(int _num);
    bool bDenoize(QString _file, ProcessType _type, ProcessParameters _params, QImage &_output);

    bool bRunning;
};

#endif // IMAGEDENOIZE_H
