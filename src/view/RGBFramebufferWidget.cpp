#include "RGBFramebufferWidget.h"
#include "ui_RGBFramebufferWidget.h"

RGBFramebufferWidget::RGBFramebufferWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::RGBFramebufferWidget)
{
    ui->setupUi(this);
}

RGBFramebufferWidget::~RGBFramebufferWidget()
{
    delete ui;
}

void RGBFramebufferWidget::setModel(ImageModel *model)
{
    ui->graphicsView->setModel(model);
}

