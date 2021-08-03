#include "qform1.h"
#include "ui_qform1.h"
#include <math.h>

QForm1::QForm1(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::QForm1)
{
    ui->setupUi(this);

    QTimer1 = new QTimer(this);
    QSerialPort1 = new QSerialPort(this);
    QSerialSetup1 = new QSerialSetup(this, QSerialPort1);
    QPaintBox1 = new QPaintBox(0, 0, ui->widget);

    connect(QTimer1, &QTimer::timeout, this, &QForm1::OnTimer1);
    connect(QSerialPort1, &QSerialPort::readyRead, this, &QForm1::OnRxQSerialPort1);
    connect(QPaintBox1, &QPaintBox::OnMousePress, this, &QForm1::OnQPaintBox1MousePressed);
    connect(QPaintBox1, &QPaintBox::OnMouseRelease, this, &QForm1::OnQPaintBox1MouseReleased);
    connect(QPaintBox1, &QPaintBox::OnMouseMove, this, &QForm1::OnQPaintBox1MouseMoved);

    header = 0;
    timeout = 0;

    QSerialPort1->setPortName("");

    firstTime = true;
    leftButtonPressed = false;
    rightButtonPressed = false;

    v0 = 20.0;
    angle0 = 45.0;
    x0 = 0.16;
    y0 = 0.16;

    QTimer1->start(20);

    Label1 = new QLabel();
    Label1->size().setWidth(100);
    Label1->setAlignment(Qt::AlignCenter);
    Label1->setText(QString().asprintf("  (%5.2f, %5.2f) - %5.2f - %5.2f  ", x0, y0, 0.0, 0.0));

    Label2 = new QLabel();
    Label2->setText("  COM?:");

    Label3 = new QLabel();
    Label3->setText("  Control on X");

    ui->statusbar->addWidget(Label1, 0);
    ui->statusbar->addWidget(Label2, 1);
    ui->statusbar->addWidget(Label3, 1);

    controlMode = 0;


}

QForm1::~QForm1()
{
    delete ui;
}

void QForm1::OnTimer1(){

    if(header){
        timeout--;
        if(!timeout)
            header = 0;
    }

    if(timeOutMesg){
        timeOutMesg--;
        if(!timeOutMesg){
            int ix, iy;

            ix = x0 / 0.01;
            iy = ui->widget->height() - (y0/0.01);
            QPaintBox1->getCanvas()->fill(Qt::black);
            DrawBall(ix, iy, 8, "");
        }
    }

    if(ui->actionSHOOT->isChecked()){
        int ix, iy;
        float x, y, vx, vy, t, p;
        float t1, t2, b;

        tx[0] = 0xE0;
        tx[1] = 0x0E;
        tx[2] = 0x03;
        tx[3] = 0x00;
        tx[4] = 0x3A;
        tx[5] = 0x00;
        tx[7] = uint8_t(0xE0 + 0x0E + 0x03 + 0x00 + 0x3A);

        x = x0 + vx0*dt;
        y = y0 + vy0*dt - 0.5*9.81*dt*dt;
        vy = vy0 - 9.81*dt;
        vx = vx0;

        dt = 0.01;

        ix = x / 0.01;
        iy = ui->widget->height() - (y/0.01);
    //    p = 1.0-ui->horizontalSlider->value()/100.0;
        p = 0.5;

        if(ix>=(ui->widget->width()-8) && vx>0){
            t = ((ui->widget->width()-8)*0.01 - x0)/vx0;
            y = y0 + vy0*t - 0.5*9.81*t*t;
            vy = vy0 - 9.81*t;

            dt = dt - t;

            ix = ui->widget->width() - 8;
            x = (ui->widget->width()-8)*0.01;
            vx *= -p;

            tx[5] = 0xA0;
            tx[6] = 0x0D;
        }
        if(ix<=8 && vx<0){
            t = (8*0.01 - x0)/vx0;
            y = y0 + vy0*t - 0.5*9.81*t*t;
            vy = vy0 - 9.81*t;

            dt = dt - t;

            ix = 8;
            x = 8*0.01;
            vx *= -p;

            tx[5] = 0xA1;
            tx[6] = 0x0D;
        }
        if(iy>=(ui->widget->height()-8) && vy<0){
            b = vy0*vy0 - 4*0.5*9.81*(8*0.01-y0);
            if(b > 0){
                t1 = (-vy0 + sqrt(b))/9.81;
                t2 = (-vy0 - sqrt(b))/9.81;
                if(t1>0 && t1<dt){
                    vy = vy0 - 9.81*t1;
                    dt = dt - t1;
                }
                if(t2>0 && t2<dt){
                    vy = vy0 - 9.81*t2;
                    dt = dt - t2;
                }
            }

            iy = ui->widget->height() - 8;
            y = 8*0.01;
            vy *= -p;
            vx *= p;

            tx[5] = 0xA2;
            tx[6] = 0x0D;
        }
        if(iy<=8 && vy>0){
            b = vy0*vy0 - 4*0.5*9.81*((ui->widget->height()-8)*0.01-y0);
            if(b > 0){
                t1 = (-vy0 + sqrt(b))/9.81;
                t2 = (-vy0 - sqrt(b))/9.81;
                if(t1>0 && t1<dt){
                    vy = vy0 - 9.81*t1;
                    dt = dt - t1;
                }
                if(t2>0 && t2<dt){
                    vy = vy0 - 9.81*t2;
                    dt = dt - t2;
                }
            }

            iy = 8;
            y = (ui->widget->height()-8)*0.01;
            vy *= -p;

            tx[5] = 0xA3;
            tx[6] = 0x0D;
        }

        if(!ui->actionSHOW_STEPS->isChecked())
            QPaintBox1->getCanvas()->fill(Qt::black);

        DrawBall(ix, iy, 8, "");

        if(tx[5]){
            tx[7] += (tx[5] + tx[6]);

            if(QSerialPort1->isOpen())
                QSerialPort1->write((char *)tx, 8);
        }

        if(y<=0.08 && fabs(vy)<=0.095 && fabs(vx)<=0.05){
            ui->actionSHOOT->setChecked(false);
            ui->actionSHOOT->setText("SHOOT BALL");
        }

        ui->lcdNumber_3->display(QString().asprintf("%5.2f", x0));
        ui->lcdNumber_4->display(QString().asprintf("%5.2f", y0));
        Label1->setText(QString().asprintf("  (%5.2f, %5.2f) - %5.2f - %5.2f  ", x, y, fabs(vy), fabs(vx)));

        x0 = x;
        y0 = y;
        vy0 = vy;
        vx0 = vx;
    }
}

void QForm1::OnRxQSerialPort1(){
    int count;

    count = QSerialPort1->bytesAvailable();
    if(count <= 0)
        return;

    uint8_t *buf = new uint8_t[count];

    QSerialPort1->read((char *)buf, count);
    for (int i=0; i<count; i++) {
        switch(header){
        case 0:
            if(buf[i] == 0xE0){
                header = 1;
                timeout = 10;
            }
            break;
        case 1:
            if(buf[i] == 0x0E)
                header = 2;
            else{
                header = 0;
                i--;
            }
            break;
        case 2:
            ndatos = buf[i];
            cks = buf[i];
            header = 3;
            break;
        case 3:
            ndatos += (buf[i]*256);
            nBytesRX = ndatos;
            cks += buf[i];
            header = 4;
            break;
        case 4:
            if(buf[i] == 0x3A){
                header = 5;
                indexw = 0;
                cks += (0xE0 + 0x0E + 0x3A);
            }
            else{
                header = 0;
                i--;
            }
            break;
        case 5:
            ndatos--;
            if(ndatos > 0){
                rx[indexw++] = buf[i];
                cks += buf[i];
            }
            else{
                header = 0;
                if(cks != buf[i])
                    rx[0] = 0xFF;
                DecodeCMD();
            }
            break;
        default:
            header = 0;
        }
    }
    delete [] buf;
}

void QForm1::DecodeCMD(){
    float step;
    int x, y;
    _work w;

    tx[0] = 0xE0;
    tx[1] = 0x0E;
    tx[2] = 0x03;
    tx[3] = 0x00;
    tx[4] = 0x3A;
    tx[5] = rx[0];
    tx[6] = 0x0D;
    tx[7] = uint8_t(0x0E + 0xE0 + 0x03u + 0x00 + 0x3A);

    switch(rx[0]){
    case 0xD0://Increment - Decrement
        w.u8[0] = rx[1];
        w.u8[1] = rx[2];
        step = 1.0*w.i16[0]/100;
        switch (controlMode) {
        case 0://x0
            x0 += step;
            x = x0/0.01;
            y = ui->widget->height()-(y0/0.01);
            if(x < 8)
                x = 8;
            if(x > (ui->widget->width()-8))
                x = (ui->widget->width()-8);
            ui->lcdNumber_3->display(QString().asprintf("%.2f", x0));
            QPaintBox1->getCanvas()->fill(Qt::black);
            DrawBall(x, y, 8, "");
            break;
        case 1://y0
            y0 += step;
            if(y0 < 0.08)
                y0 = 0.08;
            x = x0/0.01;
            y = ui->widget->height()-(y0/0.01);
            if(y > (ui->widget->height()-8))
                y = (ui->widget->height()-8);
            if(y < 8)
                y = 8;
            ui->lcdNumber_4->display(QString().asprintf("%.2f", y0));
            QPaintBox1->getCanvas()->fill(Qt::black);
            DrawBall(x, y, 8, "");
            break;
        case 2://v0
            v0 += step;
            if(v0 < 0)
                v0 = 0;
            ui->lcdNumber->display(QString().asprintf("%0.2f", v0));
            break;
        case 3://angle0
            angle0 += step;
            if(angle0 >= 360.0)
                angle0 -= 360.0;
            if(angle0 < 0)
                angle0 += 360.0;
            ui->lcdNumber_2->display(QString().asprintf("%0.2f", angle0));
            break;
        }
        break;
    case 0xD1://START - STOP
        if(!ui->actionSHOOT->isChecked()){
            ui->actionSHOOT->setChecked(true);
            dt = 0.01;
            x0 = ui->lcdNumber_3->value();
            y0 = ui->lcdNumber_4->value();
            v0 = ui->lcdNumber->value();
            angle0 = ui->lcdNumber_2->value();

            vx0 = v0*cos(angle0*M_PI/180);
            vy0 = v0*sin(angle0*M_PI/180);

            ui->actionSHOOT->setText("STOP");
            tx[6] = 0x01;
        }
        else{
            ui->actionSHOOT->setChecked(false);
            ui->actionSHOOT->setText("SHOOT BALL");
            tx[6] = 0x00;
        }
        break;
    case 0xD2://Control Mode
        controlMode++;
        if(controlMode == 4)
            controlMode = 0;
        if(controlMode == 0)
            Label3->setText("Control on X");
        if(controlMode == 1)
            Label3->setText("Control on Y");
        if(controlMode == 2)
            Label3->setText("Control on V0");
        if(controlMode == 3)
            Label3->setText("Control on Angle0");
        break;
    case 0xF0: //ALIVE
        timeOutMesg = 40;
        x = x0 / 0.01;
        y = ui->widget->height() - (y0/0.01);
        DrawBall(x, y, 8, "CONTROL CONNECTED!!!");
        tx[5] = 0x00;
        break;
    case 0xFF: //BAD checksum
        tx[6] = 0xFD;
        break;
    default: //NO CMD
        tx[6] = 0xFE;
    }

    if(tx[5]){
        tx[5] = rx[0];
        tx[7] += (tx[5] + tx[6]);

        if(QSerialPort1->isOpen())
            QSerialPort1->write((char *)tx, 8);
    }

}

void QForm1::OnQPaintBox1MousePressed(QMouseEvent *event){
    int x, y;

    x = event->pos().x();
    y = event->pos().y();
    if(QPaintBox1->getCanvas()->toImage().pixelColor(x, y)==Qt::red){
        if(event->button() == Qt::MouseButton::LeftButton)
            leftButtonPressed = true;
        else{
            if(event->button() == Qt::MouseButton::RightButton)
                rightButtonPressed = true;
        }
    }
}

void QForm1::OnQPaintBox1MouseReleased(QMouseEvent *event){
    Q_UNUSED(event);

    leftButtonPressed = false;
    rightButtonPressed = false;
}

void QForm1::OnQPaintBox1MouseMoved(QMouseEvent *event){
    int x = event->pos().x();
    int y = event->pos().y();
    float xaux, yaux;
    int xC, yC;


    if(leftButtonPressed){
        if(x < 8)
            x = 8;
        if(x > (ui->widget->width()-8))
            x = ui->widget->width()-8;

        if(y < 8)
            y = 8;
        if(y > (ui->widget->height()-8))
            y = ui->widget->height()-8;

        x0 = x*0.01;
        y0 = (ui->widget->height()-y)*0.01;

        ui->lcdNumber_3->display(QString().asprintf("%.2f", x0));
        ui->lcdNumber_4->display(QString().asprintf("%.2f", y0));

        QPaintBox1->getCanvas()->fill(Qt::black);

        DrawBall(x, y, 8, "");
    }

    if(rightButtonPressed){
        xaux = x*0.01;
        yaux = (ui->widget->height()-y)*0.01;
        xC = x0/0.01;
        yC = ui->widget->height()-y0/0.01;

        angle0 = atan2(xaux-x0, yaux-y0)*180/M_PI;
        angle0 = 90 - angle0;
        if(angle0 < 0)
            angle0 = 360 + angle0;

        v0 = 10*sqrt((xaux-x0)*(xaux-x0)+(yaux-y0)*(yaux-y0));

        ui->lcdNumber->display(QString().asprintf("%0.2f", v0));
        ui->lcdNumber_2->display(QString().asprintf("%0.2f", angle0));

        QPaintBox1->getCanvas()->fill(Qt::black);

        DrawBall(xC, yC, 8, "");

        QPainter paint(QPaintBox1->getCanvas());
        QPen pen;
        QBrush brush;

        pen.setWidth(1);
        pen.setColor(Qt::cyan);
        brush.setColor(Qt::cyan);
        brush.setStyle(Qt::SolidPattern);
        paint.setPen(pen);
        paint.setBrush(brush);
        paint.drawLine(xC, yC, x, y);
        paint.drawEllipse(x-2, y-2, 4, 4);

        QPaintBox1->update();
    }
}

void QForm1::DrawBall(int x, int y, int radius, QString msg){
    QPen pen;
    QBrush brush;
    QFont font;
    QPainter paint(QPaintBox1->getCanvas());

    pen.setWidth(5);
    pen.setColor(Qt::cyan);
    paint.setPen(pen);
    if(x == radius)
        paint.drawLine(3, 3, 3, ui->widget->height()-4);

    if(x == (ui->widget->width()-radius))
        paint.drawLine(ui->widget->width()-4, 3, ui->widget->width()-4, ui->widget->height()-4);

    if(y == radius)
        paint.drawLine(3, 3, ui->widget->width()-4, 3);

    if(y == (ui->widget->height()-radius))
        paint.drawLine(3, ui->widget->height()-4, ui->widget->width()-4, ui->widget->height()-4);

    pen.setWidth(1);
    pen.setColor(Qt::red);
    brush.setColor(Qt::red);
    brush.setStyle(Qt::SolidPattern);
    paint.setPen(pen);
    paint.setBrush(brush);
//    paint.drawEllipse((x-radius)+6, ui->widget->height()-(y+radius+6), 2*radius, 2*radius);
    paint.drawEllipse((x-radius), (y-radius), 2*radius, 2*radius);

    if(msg != ""){
        font.setFamily("Courier");
        font.setPointSize(20);
        font.setBold(true);
        paint.setFont(font);
        x = QFontMetrics(font).horizontalAdvance(msg);
        y = QFontMetrics(font).height();
        paint.drawText((ui->widget->width()-x)/2,
                       (ui->widget->height()-y)/2, msg);
        timeOutMesg = 50;
    }

    QPaintBox1->update();
}



void QForm1::paintEvent(QPaintEvent *event){
    Q_UNUSED(event);

    if(firstTime){
        firstTime = false;
        QPen pen;
        QBrush brush;
        QPainter paint(QPaintBox1->getCanvas());

        QPaintBox1->getCanvas()->fill(Qt::black);

        pen.setWidth(1);
        pen.setColor(Qt::red);
        brush.setColor(Qt::red);
        brush.setStyle(Qt::SolidPattern);
        paint.setPen(pen);
        paint.setBrush(brush);
        paint.drawEllipse(7, ui->widget->height()-(16+7), 16, 16);

        QPaintBox1->update();
    }
}


void QForm1::on_actionSET_PORT_triggered()
{
    if(QSerialPort1->isOpen()){
        QSerialPort1->close();
        ui->actionOPEN_PORT->setText("OPEN PORT");
    }

    QSerialSetup1->exec();
    Label2->setText(" CLOSED - " + QSerialSetup1->getSerialPortConfig());

}

void QForm1::on_actionOPEN_PORT_triggered()
{
    if(QSerialPort1->isOpen()){
        QSerialPort1->close();
        Label2->setText(" CLOSED - " + QSerialSetup1->getSerialPortConfig());
        ui->actionOPEN_PORT->setText("OPEN PORT");
    }
    else{
        if(QSerialPort1->portName() == ""){
            QMessageBox::information(this, "SERIAL PORT", "Select a valid Serial PORT");
            return;
        }
        if(QSerialPort1->open(QSerialPort::ReadWrite)){
            ui->actionOPEN_PORT->setText("CLOSE PORT");
            Label2->setText(" OPENED - " + QSerialSetup1->getSerialPortConfig());
        }
        else{
            QMessageBox::information(this, "SERIAL PORT", "Can't OPEN PORT = "+QSerialPort1->portName());
        }
    }
}

void QForm1::on_actionSHOOT_triggered()
{
    if(!ui->actionSHOOT->isChecked()){
        ui->actionSHOOT->setText("SHOOT BALL");
    }
    else{
//        dt = ui->lineEdit_5->text().toFloat(&ok);
//        if(!ok)
//            return;
        dt = 0.01;
        x0 = ui->lcdNumber_3->value();
        y0 = ui->lcdNumber_4->value();
        v0 = ui->lcdNumber->value();
        angle0 = ui->lcdNumber_2->value();

        vx0 = v0*cos(angle0*M_PI/180);
        vy0 = v0*sin(angle0*M_PI/180);

        ui->actionSHOOT->setText("STOP");
    }

}

void QForm1::on_actionTEST_COM_triggered()
{
    if(ui->actionSHOOT->isChecked())
        return;

    tx[0] = 0xE0;
    tx[1] = 0x0E;
    tx[2] = 0x02;
    tx[3] = 0x00;
    tx[4] = 0x3A;
    tx[5] = 0xF0;
    tx[6] = uint8_t(0xE0 + 0x0E + 0x02 + 0x00 + 0x3A + 0xF0);
    if(QSerialPort1->isOpen())
        QSerialPort1->write((char *)tx, 7);
}
