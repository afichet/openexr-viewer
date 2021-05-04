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
    delete m_model;
}

void RGBFramebufferWidget::setModel(RGBFramebufferModel *model)
{
    m_model = model;
    ui->graphicsView->setModel(m_model);
}


void RGBFramebufferWidget::on_doubleSpinBox_2_valueChanged(double arg1)
{
    m_model->setExposure(arg1);
}
