#ifndef QFORM1_H
#define QFORM1_H

#include <QMainWindow>
#include <qpaintbox.h>
#include <qserialsetup.h>
#include <QtSerialPort/QSerialPort>
#include <QTimer>
#include <QPainter>
#include <QMessageBox>

typedef union{
    uint8_t     u8[4];
    uint16_t    u16[2];
    int16_t     i16[2];
    uint32_t    u32;
    int32_t     i32;
    float       f;
}_work;

QT_BEGIN_NAMESPACE
namespace Ui { class QForm1; }
QT_END_NAMESPACE

class QForm1 : public QMainWindow
{
    Q_OBJECT

public:
    QForm1(QWidget *parent = nullptr);
    ~QForm1();

private slots:
    void OnRxQSerialPort1();
    void OnTimer1();

    void OnQPaintBox1MousePressed(QMouseEvent *event);
    void OnQPaintBox1MouseReleased(QMouseEvent *event);
    void OnQPaintBox1MouseMoved(QMouseEvent *event);

    void paintEvent(QPaintEvent *event);

    void on_actionSET_PORT_triggered();

    void on_actionOPEN_PORT_triggered();

    void on_actionSHOOT_triggered();

    void on_actionTEST_COM_triggered();

private:
    Ui::QForm1 *ui;

    QSerialSetup *QSerialSetup1;
    QTimer *QTimer1;
    QSerialPort *QSerialPort1;
    QPaintBox *QPaintBox1;

    QLabel *Label1;
    QLabel *Label2;
    QLabel *Label3;

    void DrawBall(int x, int y, int radius, QString msg);
    void DecodeCMD();

    uint8_t rx[2048], timeout, header, cks;
    uint8_t tx[32];
    uint16_t ndatos, nBytesRX, indexw;

    bool firstTime, leftButtonPressed, rightButtonPressed;

    int timeOutMesg, controlMode;

    float angle0, v0, x0, y0, vx0, vy0, dt;



};
#endif // QFORM1_H
