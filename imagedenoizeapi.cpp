#include "imagedenoizapi.h"

#include <opencv2/opencv.hpp>

#include <QImage>
#include <QPixmap>
#include <QDebug>

ImageDenoizeAPI::ImageDenoizeAPI()
{

}

ImageDenoizeAPI::~ImageDenoizeAPI()
{
}

void ImageDenoizeAPI::run()
{
    while(bRunning)
    {
        // Run the object on separated thread for real time processing
        // For now, just sleep the thread and allow time for other stuff
        msleep(1);
    }
}

/**
*************************************************************************
@verbatim
+ bDenoize() - Apply Denoizing process to image using type and parameters
+ ----------------
+ Parameters : file     string containing filename
+              type     type of denoizing process
+              params   parameters related to the requested type
+ Returns    : TRUE if success; FALSE otherwise
@endverbatim
***************************************************************************/
bool ImageDenoizeAPI::bDenoize(QString file, ProcessType type, ProcessParameters params)
{
    cv::Mat input;
    cv::Mat tmp;
    cv::Mat output;

    input = cv::imread(file.toStdString());

    if(input.empty())
    {
        qDebug() << "Error while loading file into Object Mat!";
        return false;
    }

    // Check if encoded parameters are in range & make them odd
    if(!bCheckParams(type, params))
    {
        qDebug() << __func__ << " Bad parameters!";
        return false;
    }

    // Apply Denoizing type
    switch(type)
    {
    case TypeGaussianBlur:
        qDebug() << "Apply GaussianBlur Denoizing type";
        cv::GaussianBlur(input, tmp, cv::Size(params.kernelSizeWidth, params.kernelSizeHeight), (float)(params.sigma / 10));
        break;
    case TypeMedianBlur:
        qDebug() << "Apply MedianBlur Denoizing type";
        cv::medianBlur(input, tmp, params.aperture);
        break;
    case TypeNlMeans:
        qDebug() << "Apply NlMeans Denoizing type";
        cv::fastNlMeansDenoisingColored(input, tmp);
        break;
    default:
        qDebug() << __func__ << " Unkown type!";
        return false;
        break;
    }

    // Change coding order from BGR to RGB
    cv::cvtColor(tmp, output, cv::COLOR_BGR2RGB);

    // Transmit denoized image to who is interested
    emit updatedImg(QPixmap::fromImage(QImage(output.data, output.cols, output.rows, output.step, QImage::Format_RGB888)));

    return true;
}


/**
*************************************************************************
@verbatim
+ bCheckParams() - Checks input parameters related to the current denoizing type.
+                  If requested parameters are not odd, make them odd
+ ----------------
+ Parameters : file     string containing filename
+              params   reference to parameters related to the requested type
+ Returns    : TRUE if params are OK; FALSE otherwise
@endverbatim
***************************************************************************/
bool ImageDenoizeAPI::bCheckParams(ProcessType type, ProcessParameters &params)
{
    bool bOK = false;

    switch(type)
    {
    case TypeGaussianBlur:
        // Check value ranges according to GaussianBlur() specification
        if( ((params.kernelSizeHeight > 0) && (params.kernelSizeHeight < 25)) &&
            ((params.kernelSizeWidth > 0) && (params.kernelSizeWidth < 25)) &&
            ((params.sigma > 0) && (params.sigma < 100)) )
        {
            // Values cannot be odd
            if(!bIsOdd(params.kernelSizeHeight))
            {
                params.kernelSizeHeight += 1;
            }
            if(!bIsOdd(params.kernelSizeWidth))
            {
                params.kernelSizeWidth += 1;
            }

            bOK = true;
        }
        break;
    case TypeMedianBlur:
        // Check value ranges according to MedianBlur() specification
        if( (params.aperture > 1) && (params.aperture < 25) )
        {
            // Values cannot be odd
            if(!bIsOdd(params.aperture))
            {
                params.aperture +=1;
            }
            bOK = true;
        }
        break;
    case TypeNlMeans:
        bOK = true;
        break;
    default:
        bOK = false;
    }

    return bOK;
}

/**
*************************************************************************
@verbatim
+ bCheckParams() - Checks if input integer is an odd number
+ ----------------
+ Parameters : num    input number
+ Returns    : TRUE if integer is an odd number; FALSE otherwise
@endverbatim
***************************************************************************/
bool ImageDenoizeAPI::bIsOdd(int num)
{
  int i = 0;
  bool odd = false;

  while (i != num)
  {
    odd = !odd;
    i = i + 1;
  }

  return odd;
}

