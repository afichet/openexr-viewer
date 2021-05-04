#pragma once

#include <QWidget>

#include <model/FramebufferModel.h>

namespace Ui {
class FramebufferWidget;
}

class FramebufferWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FramebufferWidget(QWidget *parent = nullptr);
    ~FramebufferWidget();

    void setModel(FramebufferModel* model);

private slots:
    void on_sbMinValue_valueChanged(double arg1);

    void on_sbMaxValue_valueChanged(double arg1);

private:
    Ui::FramebufferWidget *ui;
    FramebufferModel* m_model;
};

