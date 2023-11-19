#include "mainwindow.h"

// Формула распространения
int PL(float d, double fc){
    return 28 + 22*log10(d) + 20*log10(fc);
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    QGraphicsScene* scene = new QGraphicsScene();

    // Базовые переменные
    QPixmap map(1000, 1000);
    int wifi_x = 750;
    int wifi_y = 400;
    int tx = 23;
    int ant = 10;
    double fc = 5;

    // Маштаб (1 пискель = 1/scale метров)
    float scale = 0.1;


    QPainter p(&map);

    for(int i = 0; i < 1000; i++){
        for(int j = 0; j < 1000; j++){
            float d = sqrt(pow((wifi_x-i),2)+pow((wifi_y-j),2))/scale;
            float dBm = ant + tx - PL(d,fc);

            // Нормализация dBm от 0 -> 100
            dBm = ((dBm * -1) -44) * 2.55; // коэффициент можно менять 1.875=-144blue

            // Задание цвета на основе какой dBm
            if(dBm < 0) p.setPen(QColor(255,0,0, 255));
            else if(dBm < 64 ) p.setPen(QColor(255, dBm*4, 0, 255));
            else if(dBm < 128) p.setPen(QColor((255 - ((dBm-64)*4)), 255, 0, 255));
            else if(dBm < 192) p.setPen(QColor(0, 255, ((dBm-128)*4), 255));
            else if(dBm < 256) p.setPen(QColor(0, 255 - ((dBm-192)*4), 255, 255));
            else p.setPen(QColor(0,0,200, 255));

            p.drawPoint(i, j);
        }
    }
    // Отображение на Пиксельной карте
    scene->addPixmap(map);
    QGraphicsView* view = new QGraphicsView(scene);
    setCentralWidget(view);
}

MainWindow::~MainWindow()
{
}
