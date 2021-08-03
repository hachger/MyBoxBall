#ifndef QPAINTBOX_H
#define QPAINTBOX_H

#include <QWidget>
#include <QPainter>
#include <QMouseEvent>

class QPaintBox : public QWidget
{
    Q_OBJECT
public:
    explicit QPaintBox(int aWidth = 200, int aHeigth = 200, QWidget *parent = nullptr);

    void setWidth(int awidth);
    void setHeight(int aheight);
    QPixmap *getCanvas(void);

signals:
    /**
     * @brief OnMousePress
     * Señal que dice si un boton del mouse se presiono
     * @param event
     * out: event
     */

    void OnMousePress(QMouseEvent *event);
    void OnMouseRelease(QMouseEvent *event);
    void OnMouseMove(QMouseEvent *event);

public slots:

protected:
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
    void resizeEvent(QResizeEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;


private:
    QPixmap *pixelCanvas;

};

#endif // QPAINTBOX_H
