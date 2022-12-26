#include "imagedenoizerapi.h"

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
+ bDenoize() - Apply Denoizing process to image using type and parameters and
+              transfer the result via signal
+ ----------------
+ Parameters : file     string containing filename
+              type     type of denoizing process
+              params   parameters related to the requested type
+              out      image to fill with rendered frame
+ Returns    : TRUE if success; FALSE otherwise
@endverbatim
***************************************************************************/
bool ImageDenoizeAPI::bApplyDenoize(QString _file, ProcessType _type, ProcessParameters _params)
{
    bool bOK = true;
    QImage output;

    bOK = bDenoize(_file, _type, _params, output);

    if(bOK && !output.isNull())
    {
        // Transmit denoized image to who is interested
        emit updatedImg(output);
    }

    return bOK;
}

/**
*************************************************************************
@verbatim
+ bDenoize() - Apply Denoizing process to image using type and parameters and
+              transfer the result in passed arg
+ ----------------
+ Parameters : file     string containing filename
+              type     type of denoizing process
+              params   parameters related to the requested type
+              out      image to fill with rendered frame
+ Returns    : TRUE if success; FALSE otherwise
@endverbatim
***************************************************************************/
bool ImageDenoizeAPI::bApplyDenoize(QString _file, ProcessType _type, ProcessParameters _params, QImage &_out)
{
    bool bOK = true;
    QImage output;

    bOK = bDenoize(_file, _type, _params, output);

    if(bOK && !output.isNull())
    {
        // Fill output passed in arg
        _out = output.copy();
    }

    return bOK;
}

/**
*************************************************************************
@verbatim
+ bSaveImage() - Save image at target location
+ ----------------
+ Parameters : file     string containing filename
+              image    image data to be saved
+ Returns    : TRUE if success; FALSE otherwise
@endverbatim
***************************************************************************/
bool ImageDenoizeAPI::bSaveImage(QString _file, QImage _image)
{
    if(_image.byteCount() <= 0)
    {
        qDebug() << __func__ << " Error Image Size == null!";
        return false;
    }

    if(_image.save(_file))
        return true;
    else
    {
        qDebug() << __func__ << " Could not save image!";
        return false;
    }
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
bool ImageDenoizeAPI::bCheckParams(ProcessType _type, ProcessParameters &params)
{
    bool bOK = false;

    switch(_type)
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
bool ImageDenoizeAPI::bIsOdd(int _num)
{
  int i = 0;
  bool odd = false;

  while (i != _num)
  {
    odd = !odd;
    i = i + 1;
  }

  return odd;
}

/**
*************************************************************************
@verbatim
+ bDenoize() - Denoize image using type and parameters and
+              transfer the result in passed arg
+ ----------------
+ Parameters : file     string containing filename
+              type     type of denoizing process
+              params   parameters related to the requested type
+              out      image to fill with rendered frame
+ Returns    : TRUE if success; FALSE otherwise
@endverbatim
***************************************************************************/
bool ImageDenoizeAPI::bDenoize(QString _file, ProcessType _type, ProcessParameters _params, QImage &_output)
{
    cv::Mat input;
    cv::Mat tmp;
    cv::Mat output;

    input = cv::imread(_file.toStdString());

    if(input.empty())
    {
        qDebug() << "Error while loading file into Object Mat!";
        return false;
    }

    // Check if encoded parameters are in range & make them odd
    if(!bCheckParams(_type, _params))
    {
        qDebug() << __func__ << " Bad parameters!";
        return false;
    }

    // Apply Denoizing type
    switch(_type)
    {
    case TypeGaussianBlur:
        qDebug() << "Apply GaussianBlur Denoizing type";
        cv::GaussianBlur(input, tmp, cv::Size(_params.kernelSizeWidth, _params.kernelSizeHeight), (float)(_params.sigma / 10));
        break;
    case TypeMedianBlur:
        qDebug() << "Apply MedianBlur Denoizing type";
        cv::medianBlur(input, tmp, _params.aperture);
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

    _output = QImage(output.data, output.cols, output.rows, output.step, QImage::Format_RGB888).copy();

    return true;
}

