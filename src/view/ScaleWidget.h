#pragma once

#include <QWidget>
#include <util/ColormapModule.h>

class ScaleWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ScaleWidget(QWidget *parent = nullptr);

    QSize minimumSizeHint() const override;
    QSize sizeHint       () const override;

public slots:
    void setMin(double value);
    void setMax(double value);
    void setColormap(ColormapModule::Map map);

protected:
    void paintEvent(QPaintEvent *e)  override;
    void resizeEvent(QResizeEvent *e) override;


private:
    double m_min;
    double m_max;
    Colormap * m_cmap;

    int m_width, m_height;
};
