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

bool ImageDenoizeAPI::bLoadImage(QString _file)
{
    cv::Mat input;
    cv::Mat output;

    input = cv::imread(_file.toStdString());

    if(input.empty())
    {
        qDebug() << "Error while loading file into Object Mat!";
        return false;
    }

    // Store original image (used as base reference)
    m_originalImg = input.clone();
    // Set current image as initial
    m_curImg = m_originalImg.clone();

    // Convert the image back to the RGB color space
    cv::cvtColor(m_originalImg, output, cv::COLOR_BGR2RGB);

    // Transmit original image to who is interested
    emit updatedEditedImg(QImage(output.data, output.cols, output.rows, output.step, QImage::Format_RGB888).copy());

    return true;
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
+ bApplyImageEditing() - Apply Brightness & Constrat to an image. The result
+                        is transferred via signal
+ ----------------
+ Parameters : _brightness brightness value between 1 and 200
+              _contrast   constrast value between 1 and 200
+ Returns    : TRUE if success; FALSE otherwise
@endverbatim
***************************************************************************/
bool ImageDenoizeAPI::bApplyImageEditing(int _brigthness, int _contrast, int _hue, int _saturation)
{
    bool bOK = true;
    cv::Mat tmp;
    cv::Mat out;
    cv::Mat hsvImage;
    std::vector<cv::Mat> channels;

    if(m_originalImg.empty())
    {
        qDebug() << "Error while loading file into Object Mat!";
        return false;
    }

    // Get original image
    m_curImg = m_originalImg.clone();

    // Check if request brightness value is valid
    if(!bCheckImageEditingValues(_brigthness, _contrast, _hue, _saturation))
    {
        qDebug() << __func__ << " Bad brightness value!";
        return false;
    }

    /*
     * Brightness & Contrast
     */
    // Increase/Decrease brightness & contrast
    m_curImg.convertTo(tmp, -1, ((double)_contrast / 100), _brigthness - 100);

    /*
     * Hue & Saturation
     */
    // Convert the image to the HSV color space
    cv::cvtColor(tmp, hsvImage, cv::COLOR_BGR2HSV);

    // Split the image into its individual channels
    cv::split(hsvImage, channels);

    // Modify the hue channel
    //channels[0] = _hue;

    // Modify the saturation channel
    //channels[1] = _saturation;

    // Merge the modified channels back into a single image
    cv::merge(channels, hsvImage);

    // Convert the image back to the BGR color space
    cv::cvtColor(hsvImage, out, cv::COLOR_HSV2BGR);

    // Update current image
    m_curImg = out.clone();

    // Convert the image to the RGB color space
    cv::cvtColor(m_curImg, out, cv::COLOR_BGR2RGB);

    if(bOK && !out.empty())
    {
        // Transmit processed image to who is interested
        emit updatedEditedImg(QImage(out.data, out.cols, out.rows, out.step, QImage::Format_RGB888).copy());
    }

    return bOK;
}

/**
*************************************************************************
@verbatim
+ bApplyDenoize() - Apply Denoizing process to current image using type and parameters and
+              transfer the result via signal
+ ----------------
+ Parameters : type     type of denoizing process
+              params   parameters related to the requested type
+ Returns    : TRUE if success; FALSE otherwise
@endverbatim
***************************************************************************/
bool ImageDenoizeAPI::bApplyDenoize(ProcessType _type, ProcessParameters _params)
{
    bool bOK = true;
    cv::Mat in;
    cv::Mat tmp;
    cv::Mat out;

    if(m_curImg.empty())
    {
        qDebug() << "Error while loading file into Object Mat!";
        return false;
    }

    // Check if encoded parameters are in range & make them odd
    if(!bCheckDenoizeParams(_type, _params))
    {
        qDebug() << __func__ << " Bad parameters!";
        return false;
    }

    // Get current image
    in = m_curImg.clone();

    // Apply Denoizing type
    switch(_type)
    {
    case TypeGaussianBlur:
        qDebug() << "Apply GaussianBlur Denoizing type";
        cv::GaussianBlur(in, tmp, cv::Size(_params.kernelSizeWidth, _params.kernelSizeHeight), (float)(_params.sigma / 10));
        break;
    case TypeMedianBlur:
        qDebug() << "Apply MedianBlur Denoizing type";
        cv::medianBlur(in, tmp, _params.aperture);
        break;
    case TypeNlMeans:
        qDebug() << "Apply NlMeans Denoizing type";
        cv::fastNlMeansDenoisingColored(in, tmp);
        break;
    default:
        qDebug() << __func__ << " Unkown type!";
        bOK = false;
        break;
    }

    // Change coding order from BGR to RGB
    cv::cvtColor(tmp, out, cv::COLOR_BGR2RGB);

    if(bOK && !out.empty())
    {
        // Transmit denoized image to who is interested
        emit updatedDenoizeImg(QImage(out.data, out.cols, out.rows, out.step, QImage::Format_RGB888).copy());
    }

    return bOK;
}

/**
*************************************************************************
@verbatim
+ GetImage() - Return current modified image into a QImage
+ ----------------
+ Parameters : NONE
+ Returns    : QImage the current modified image
@endverbatim
***************************************************************************/
QImage ImageDenoizeAPI::GetImage()
{
    return QImage(m_curImg.data, m_curImg.cols, m_curImg.rows, m_curImg.step, QImage::Format_RGB888).copy();
}

int ImageDenoizeAPI::GetImageSaturation()
{
    // Convert the image to the HSV color space
    cv::Mat hsvImage;
    cv::cvtColor(m_curImg, hsvImage, cv::COLOR_BGR2HSV);

    // Split the image into its individual channels
    std::vector<cv::Mat> channels;
    cv::split(hsvImage, channels);

    // Retrieve the saturation channel
    cv::Mat saturation = channels[1];

    // Calculate the mean saturation value for the entire image
    cv::Scalar meanSaturation = cv::mean(saturation);

    qDebug() << "Original Image Saturation: " + QString::number(meanSaturation[0]);

    return meanSaturation[0];
}

int ImageDenoizeAPI::GetImageHue()
{
    // Convert the image to the HSV color space
    cv::Mat hsvImage;
    cv::cvtColor(m_curImg, hsvImage, cv::COLOR_BGR2HSV);

    // Split the image into its individual channels
    std::vector<cv::Mat> channels;
    cv::split(hsvImage, channels);

    // Retrieve the saturation channel
    cv::Mat hue = channels[0];

    // Calculate the mean saturation value for the entire image
    cv::Scalar meanHue = cv::mean(hue);

    qDebug() << "Original Image Hue: " + QString::number(meanHue[0]);

    return meanHue[0];
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
+ bCheckDenoizeParams() - Checks input parameters related to the current denoizing type.
+                  If requested parameters are not odd, make them odd
+ ----------------
+ Parameters : type     denoizing process type
+              params   reference to parameters related to the requested type
+ Returns    : TRUE if params are OK; FALSE otherwise
@endverbatim
***************************************************************************/
bool ImageDenoizeAPI::bCheckDenoizeParams(ProcessType _type, ProcessParameters &params)
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
+ bCheckImageEditingValues() - Checks input brightness & contrast values
+ ----------------
+ Parameters : brightness value of the brightness
+              constrast value of the constrast
+ Returns    : TRUE if params are OK; FALSE otherwise
@endverbatim
***************************************************************************/
bool ImageDenoizeAPI::bCheckImageEditingValues(int _brightness, int _contrast, int _hue, int _saturation)
{
    bool bOK = true;

    if( (_brightness < 1) || (_brightness > 200) )
        bOK = false;

    if( (_contrast < 1) || (_contrast > 200) )
        bOK = false;

    if( (_hue < 0) || (_hue > 179) )
        bOK = false;

    if( (_saturation < 0) || (_saturation > 255) )
        bOK = false;

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
