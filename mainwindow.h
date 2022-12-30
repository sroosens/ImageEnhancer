#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <imagedenoizerapi.h>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void updateDenoizeImage(const QImage image);
    void updateEditedImage(const QImage image);

private slots:
    void on_pushButtonRun_clicked();
    void on_pushButtonSave_clicked();
    void on_comboBoxDenoiseType_currentIndexChanged(int index);
    void on_horizontalSlider_Sigma_valueChanged(int value);
    void on_horizontalSlider_KernelWidth_valueChanged(int value);
    void on_horizontalSlider_KernelHeight_valueChanged(int value);
    void on_horizontalSlider_Aperture_valueChanged(int value);

    void on_horizontalSlider_Brightness_valueChanged(int value);

    void on_horizontalSlider_Constrast_valueChanged(int value);

    void on_horizontalSlider_Hue_valueChanged(int value);

    void on_horizontalSlider_Saturation_valueChanged(int value);

private:
    Ui::MainWindow *ui;

    // Overload event functions
    void dragEnterEvent(QDragEnterEvent *e);
    void dropEvent(QDropEvent *e);

    void displayImgDetails();
    void disableParamsUI();

    QString             m_curFileName;
    ImageDenoizeAPI     m_imageDenoizer;
    QImage              m_curImg;
    QImage              m_denoizedImg;
};

#endif // MAINWINDOW_H
