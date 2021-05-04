#include "FramebufferWidget.h"
#include "ui_FramebufferWidget.h"

FramebufferWidget::FramebufferWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FramebufferWidget)
{
    ui->setupUi(this);
}

FramebufferWidget::~FramebufferWidget()
{
    delete ui;
}

void FramebufferWidget::setModel(ImageModel *model)
{
    ui->graphicsView->setModel(model);
}
