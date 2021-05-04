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
    delete m_model;
}


void FramebufferWidget::setModel(FramebufferModel *model)
{
    m_model = model;
    ui->graphicsView->setModel(model);
}



void FramebufferWidget::on_sbMinValue_valueChanged(double arg1)
{
    ui->sbMaxValue->setMinimum(arg1);
    m_model->setMinValue(arg1);
}

void FramebufferWidget::on_sbMaxValue_valueChanged(double arg1)
{
    ui->sbMinValue->setMaximum(arg1);
    m_model->setMaxValue(arg1);
}
