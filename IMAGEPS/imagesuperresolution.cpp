#include "imagesuperresolution.h"
#include "ui_imagesuperresolution.h"

ImageSuperResolution::ImageSuperResolution(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ImageSuperResolution)
{
    ui->setupUi(this);
    setWindowTitle(QString::fromLocal8Bit("影像重建处理"));
    setWindowIcon(QIcon(u8":/resource/menu/数据预处理/影像赋投影.png"));
}

ImageSuperResolution::~ImageSuperResolution()
{
    delete ui;
}
