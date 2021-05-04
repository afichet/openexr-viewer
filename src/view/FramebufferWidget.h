#pragma once

#include <QWidget>

#include <model/ImageModel.h>

namespace Ui {
class FramebufferWidget;
}

class FramebufferWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FramebufferWidget(QWidget *parent = nullptr);
    ~FramebufferWidget();

    void setModel(ImageModel* model);

private:
    Ui::FramebufferWidget *ui;
};

