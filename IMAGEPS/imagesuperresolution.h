#ifndef IMAGESUPERRESOLUTION_H
#define IMAGESUPERRESOLUTION_H

#include <QDialog>
#include <QIcon>
#include <QAction>
#include <QProcess>
#include <QDir>

#include "PublicFunctions.h"

namespace Ui {
class ImageSuperResolution;
}

class ImageSuperResolution : public QDialog
{
    Q_OBJECT

public:
    explicit ImageSuperResolution(QWidget *parent = nullptr);
    ~ImageSuperResolution();

private:
    Ui::ImageSuperResolution *ui;
};

#endif // IMAGESUPERRESOLUTION_H
