#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "QDragEnterEvent"
#include "QDropEvent"
#include "QMimeData"
#include "QDebug"
#include "QPixmap"
#include "QFileInfo"
#include "QFileDialog"
#include "QtGui"
#include "QMessageBox"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    // Setup UI
    ui->setupUi(this);
    setAcceptDrops(true);
    on_comboBoxDenoiseType_currentIndexChanged(0);
    ui->pushButtonRun->setEnabled(false);
    ui->pushButtonSave->setEnabled(false);
    ui->horizontalSlider_Brightness->setEnabled(false);
    ui->horizontalSlider_Constrast->setEnabled(false);
    ui->horizontalSlider_Hue->setEnabled(false);
    ui->horizontalSlider_Saturation->setEnabled(false);
    disableParamsUI();

    // Setup specific thread for image processing
    m_imageDenoizer.start();
    m_imageDenoizer.moveToThread(&m_imageDenoizer);

    // Connect image rendered to UI
    (void)QObject::connect(&m_imageDenoizer, SIGNAL(updatedDenoizeImg(QImage)), this, SLOT(updateDenoizeImage(QImage)));
    (void)QObject::connect(&m_imageDenoizer, SIGNAL(updatedEditedImg(QImage)), this, SLOT(updateEditedImage(QImage)));
}


MainWindow::~MainWindow()
{
    delete ui;

    //Exit Image Processing thread
    m_imageDenoizer.stop();
    while (m_imageDenoizer.isRunning());
}

/**
*************************************************************************
@verbatim
+ updateDenoizeImage() - Slot called when a new denoized image is received.
+                 Store the image in local and display it to the UI
+ ----------------
+ Parameters : image    Processed image to store and display
+ Returns    : NONE
@endverbatim
***************************************************************************/
void MainWindow::updateDenoizeImage(const QImage image)
{
    // Get label dimensions
    int w = ui->labelImgDenoized->width();
    int h = ui->labelImgDenoized->height();

    // Store image in local
    m_denoizedImg = image.copy();

    // Enable Save button
    ui->pushButtonSave->setEnabled(true);

    // Set a scaled pixmap to a w x h window keeping its aspect ratio
    ui->labelImgDenoized->setPixmap(QPixmap::fromImage(image.scaled(w, h, Qt::KeepAspectRatio)));
}

/**
*************************************************************************
@verbatim
+ updateEditedImage() - Slot called when a new edited image is received.
+                 Store the image in local and display it to the UI
+ ----------------
+ Parameters : image    Processed image to store and display
+ Returns    : NONE
@endverbatim
***************************************************************************/
void MainWindow::updateEditedImage(const QImage image)
{
    // Get label dimensions
    int w = ui->labelImgPrevious->width();
    int h = ui->labelImgPrevious->height();

    // Store image in local
    m_curImg = image.copy();

    // Set a scaled pixmap to a w x h window keeping its aspect ratio
    ui->labelImgPrevious->setPixmap(QPixmap::fromImage(image.scaled(w, h, Qt::KeepAspectRatio)));
}

/**
*************************************************************************
@verbatim
+ dragEnterEvent() - Overload dragEvent function to check file before allowing
+                    the drop. For now, check only if file has an URL
+ ----------------
+ Parameters : e        Qt class that contains information about current event
+ Returns    : NONE
@endverbatim
***************************************************************************/
void MainWindow::dragEnterEvent(QDragEnterEvent *e)
{
    if (e->mimeData()->hasUrls())
    {
        e->acceptProposedAction();
    }
}

/**
*************************************************************************
@verbatim
+ dropEvent() - Overload dropEvent function to get and store current image
+               to process.
+ ----------------
+ Parameters : e        Qt class that contains information about current event
+ Returns    : NONE
@endverbatim
***************************************************************************/
void MainWindow::dropEvent(QDropEvent *e)
{
    foreach (const QUrl &url, e->mimeData()->urls())
    {
        QString fileName = url.toLocalFile();
        qDebug() << "Dropped file:" << fileName;

        //Check validity
        if(QFile(fileName).exists())
        {
            m_curFileName = fileName;

            // Get label dimensions
            int w = ui->labelImgPrevious->width();
            int h = ui->labelImgPrevious->height();

            // Set a scaled pixmap to a w x h window keeping its aspect ratio
            ui->labelImgPrevious->setPixmap(QPixmap(m_curFileName).scaled(w, h, Qt::KeepAspectRatio));

            // Enable Denoize sliders
            on_comboBoxDenoiseType_currentIndexChanged(ui->comboBoxDenoiseType->currentIndex());

            // Set local image
            if(!m_imageDenoizer.bLoadImage(m_curFileName))
            {
                QMessageBox::warning(this,"Error",
                                     "Error while loading image!\n"
                                     "File path shall be in ASCII standard (no é, è, ê, µ, ¨, ...) \n"
                                     "File format shall be .jpg, .png, .tiff");
            }
            else
            {
                // Enable Editing sliders
                ui->horizontalSlider_Brightness->setEnabled(true);
                ui->horizontalSlider_Constrast->setEnabled(true);
                ui->horizontalSlider_Hue->setEnabled(false);
                ui->horizontalSlider_Saturation->setEnabled(false);

                // Update UI to current image hue and saturation values
                ui->label_valueHue->setText(QString::number(m_imageDenoizer.GetImageHue()));
                ui->label_valueSaturation->setText(QString::number(m_imageDenoizer.GetImageSaturation()));
                ui->horizontalSlider_Hue->setValue(m_imageDenoizer.GetImageHue());
                ui->horizontalSlider_Saturation->setValue(m_imageDenoizer.GetImageSaturation());

                ui->horizontalSlider_Brightness->setValue(100);
                ui->horizontalSlider_Constrast->setValue(100);
            }

            // Enable denoize button
            ui->pushButtonRun->setEnabled(true);

            // Display details about image
            displayImgDetails();
        }
        else
        {
            QMessageBox::warning(this,"Error","File does not exist!");
        }
    }
}

/**
*************************************************************************
@verbatim
+ displayImgDetails() - Update image details label iaw current image file
+ ----------------
+ Parameters : NONE
+ Returns    : NONE
@endverbatim
***************************************************************************/
void MainWindow::displayImgDetails()
{
    // Update UI
    ui->label_Size->setText(QString::number(QFile(m_curFileName).size() / 1000) + " Ko");
    ui->label_Width->setText(QString::number(QPixmap(m_curFileName).width()) + " px");
    ui->label_Height->setText(QString::number(QPixmap(m_curFileName).height()) + " px");
    ui->label_Format->setText(QFileInfo(m_curFileName).suffix());
    ui->label_Name->setText(QFileInfo(m_curFileName).baseName());
}

/**
*************************************************************************
@verbatim
+ on_pushButtonRun_clicked() - Slot triggered Denoize button has been clicked.
+                              Get current parameters values related to current
+                              denoizing type and proceed to denoize process.
+ ----------------
+ Parameters : NONE
+ Returns    : NONE
@endverbatim
***************************************************************************/
void MainWindow::on_pushButtonRun_clicked()
{
    ProcessType type = (ProcessType)ui->comboBoxDenoiseType->currentIndex();
    ProcessParameters params;

    // Check Denoizing type selected and get values
    if( type == TypeGaussianBlur)
    {
        params.sigma = ui->label_valueSigma->text().toInt();
        params.kernelSizeWidth = ui->label_valueKW->text().toInt();
        params.kernelSizeHeight = ui->label_valueKH->text().toInt();
        qDebug() << params.sigma << " " << params.kernelSizeWidth << " " << params.kernelSizeHeight;
    }
    else if(type == TypeMedianBlur)
    {
        params.aperture = ui->label_valueAperture->text().toInt();
        qDebug() << params.aperture;
    }
    else if(type == TypeNlMeans)
    {
        //no parameter
    }
    else
    {
        qDebug() << "Unkown Denoizing type!";
        return;
    }

    // Proceed to Denoizing
    if(!m_imageDenoizer.bApplyDenoize(type, params))
    {
        QMessageBox::warning(this,"Error",
                             "Error while Denoizing!\n"
                             "Check parameters \n");
    }
}


/**
*************************************************************************
@verbatim
+ on_pushButtonSave_clicked() - Slot triggered save button has been clicked.
+                               Open dialog box and save current file to
+                               desired location.
+ ----------------
+ Parameters : NONE
+ Returns    : NONE
@endverbatim
***************************************************************************/
void MainWindow::on_pushButtonSave_clicked()
{
    QString filename = QFileDialog::getSaveFileName(this, "Save file", QDir::currentPath(), "Images (*.png *.tiff *.jpg)");

    if(m_imageDenoizer.bSaveImage(filename + "." + QFileInfo(m_curFileName).suffix(), m_denoizedImg))
    {
        qDebug() << "Denoized file saved!";
    }
    else
    {
        QMessageBox::warning(this,"Error","Error while saving denoized file!");
    }
}

/**
*************************************************************************
@verbatim
+ on_comboBoxDenoiseType_currentIndexChanged() - Slot triggered when value
+                                               from combo box has changed.
+                                               Update UI objects parameters
+                                               related to current denoizing
+                                               type.
+ ----------------
+ Parameters : index    updated index from combobox
+ Returns    : NONE
@endverbatim
***************************************************************************/
void MainWindow::on_comboBoxDenoiseType_currentIndexChanged(int index)
{
    ProcessType type = (ProcessType)index;

    // Disable every label and sliders
    disableParamsUI();

    if(type == TypeGaussianBlur)
    {
        // Sigma
        ui->label_sigma_2->setEnabled(true);
        ui->label_valueSigma->setEnabled(true);
        ui->horizontalSlider_Sigma->setEnabled(true);

        // Kernel W/H
        ui->label_kernelHeight->setEnabled(true);
        ui->label_kernelWidth->setEnabled(true);
        ui->label_valueKW->setEnabled(true);
        ui->label_valueKH->setEnabled(true);
        ui->horizontalSlider_KernelHeight->setEnabled(true);
        ui->horizontalSlider_KernelWidth->setEnabled(true);
    }
    else if(type == TypeMedianBlur)
    {
        // Aperture
        ui->label_aperture->setEnabled(true);
        ui->label_valueAperture->setEnabled(true);
        ui->horizontalSlider_Aperture->setEnabled(true);
    }
    else if(type == TypeNlMeans)
    {
        //no parameters
    }
    else
    {
        //do nothing
    }
}

/**
*************************************************************************
@verbatim
+ disableParamsUI() - Disable UI objects related to parameters
+ ----------------
+ Parameters : NONE
+ Returns    : NONE
@endverbatim
***************************************************************************/
void MainWindow::disableParamsUI()
{
    // Title
    ui->label_aperture->setEnabled(false);
    ui->label_sigma_2->setEnabled(false);
    ui->label_kernelHeight->setEnabled(false);
    ui->label_kernelWidth->setEnabled(false);
    // Value
    ui->label_valueAperture->setEnabled(false);
    ui->label_valueSigma->setEnabled(false);
    ui->label_valueKW->setEnabled(false);
    ui->label_valueKH->setEnabled(false);
    // Slider
    ui->horizontalSlider_Aperture->setEnabled(false);
    ui->horizontalSlider_KernelHeight->setEnabled(false);
    ui->horizontalSlider_KernelWidth->setEnabled(false);
    ui->horizontalSlider_Sigma->setEnabled(false);
}

/**
*************************************************************************
@verbatim
+ on_horizontalSlider_Sigma_valueChanged() - Slot triggered when value
+                                               from slider has changed.
+                                               Update related label with
+                                               new value.
+ ----------------
+ Parameters : value    updated value
+              params   reference to parameters related to the requested type
+ Returns    : NONE
@endverbatim
***************************************************************************/
void MainWindow::on_horizontalSlider_Sigma_valueChanged(int value)
{
    ui->label_valueSigma->setText(QString::number(value));
}

/**
*************************************************************************
@verbatim
+ on_horizontalSlider_KernelWidth_valueChanged() - Slot triggered when value
+                                               from slider has changed.
+                                               Update related label with
+                                               new value.
+ ----------------
+ Parameters : value    updated value
+              params   reference to parameters related to the requested type
+ Returns    : NONE
@endverbatim
***************************************************************************/
void MainWindow::on_horizontalSlider_KernelWidth_valueChanged(int value)
{
    ui->label_valueKW->setText(QString::number(value));
}

/**
*************************************************************************
@verbatim
+ on_horizontalSlider_KernelHeight_valueChanged() - Slot triggered when value
+                                               from slider has changed.
+                                               Update related label with
+                                               new value.
+ ----------------
+ Parameters : value    updated value
+              params   reference to parameters related to the requested type
+ Returns    : NONE
@endverbatim
***************************************************************************/
void MainWindow::on_horizontalSlider_KernelHeight_valueChanged(int value)
{
    ui->label_valueKH->setText(QString::number(value));
}

/**
*************************************************************************
@verbatim
+ on_horizontalSlider_Aperture_valueChanged() - Slot triggered when value
+                                               from slider has changed.
+                                               Update related label with
+                                               new value.
+ ----------------
+ Parameters : value    updated value
+              params   reference to parameters related to the requested type
+ Returns    : NONE
@endverbatim
***************************************************************************/
void MainWindow::on_horizontalSlider_Aperture_valueChanged(int value)
{
    ui->label_valueAperture->setText(QString::number(value));
}


void MainWindow::on_horizontalSlider_Brightness_valueChanged(int value)
{
    ui->label_valueBright->setText(QString::number(value));

    // Get current params
    int contrast = ui->label_valueConstrast->text().toInt();
    int hue = ui->label_valueHue->text().toInt();
    int saturation = ui->label_valueSaturation->text().toInt();

    if(!m_imageDenoizer.bApplyImageEditing(value, contrast, hue, saturation))
    {
        QMessageBox::warning(this,"Error",
                             "Error while updating brightness!\n"
                             "Check parameters\n");
    }
}

void MainWindow::on_horizontalSlider_Constrast_valueChanged(int value)
{
    ui->label_valueConstrast->setText(QString::number(value));

    // Get current params
    int brightness = ui->label_valueBright->text().toInt();
    int hue = ui->label_valueHue->text().toInt();
    int saturation = ui->label_valueSaturation->text().toInt();

    if(!m_imageDenoizer.bApplyImageEditing(brightness, value, hue, saturation))
    {
        QMessageBox::warning(this,"Error",
                             "Error while updating contrast!\n"
                             "Check parameters\n");
    }
}

void MainWindow::on_horizontalSlider_Hue_valueChanged(int value)
{
    ui->label_valueHue->setText(QString::number(value));

    // Get current params
    int brightness = ui->label_valueBright->text().toInt();
    int saturation = ui->label_valueSaturation->text().toInt();
    int contrast = ui->label_valueConstrast->text().toInt();

    if(!m_imageDenoizer.bApplyImageEditing(brightness, contrast, value, saturation))
    {
        QMessageBox::warning(this,"Error",
                             "Error while updating hue!\n"
                             "Check parameters\n");
    }
}

void MainWindow::on_horizontalSlider_Saturation_valueChanged(int value)
{
    ui->label_valueSaturation->setText(QString::number(value));

    // Get current params
    int brightness = ui->label_valueBright->text().toInt();
    int hue = ui->label_valueHue->text().toInt();
    int contrast = ui->label_valueConstrast->text().toInt();

    if(!m_imageDenoizer.bApplyImageEditing(brightness, contrast, hue, value))
    {
        QMessageBox::warning(this,"Error",
                             "Error while updating saturation!\n"
                             "Check parameters\n");
    }
}
