#pragma once

#include <QWidget>
#include <model/RGBFramebufferModel.h>

namespace Ui {
class RGBFramebufferWidget;
}

class RGBFramebufferWidget : public QWidget
{
    Q_OBJECT

public:
    explicit RGBFramebufferWidget(QWidget *parent = nullptr);
    ~RGBFramebufferWidget();

    void setModel(RGBFramebufferModel* model);

private slots:
    void on_doubleSpinBox_2_valueChanged(double arg1);

private:
    Ui::RGBFramebufferWidget *ui;
    RGBFramebufferModel* m_model;
};

