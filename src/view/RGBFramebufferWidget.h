#pragma once

#include <QWidget>
#include <model/ImageModel.h>

namespace Ui {
class RGBFramebufferWidget;
}

class RGBFramebufferWidget : public QWidget
{
    Q_OBJECT

public:
    explicit RGBFramebufferWidget(QWidget *parent = nullptr);
    ~RGBFramebufferWidget();

    void setModel(ImageModel* model);

private:
    Ui::RGBFramebufferWidget *ui;
};

