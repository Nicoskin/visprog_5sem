#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    QGraphicsScene* scene = new QGraphicsScene();

        QPixmap map(1000, 1000);
        int wifi_x = 750;
        int wifi_y = 400;
        int tx = 23;
        int ant = 10;
        double fc = 5;
        int min_dbm = -60;
        int max_dbm = -50;

        float scale = 2 ; //1 пискель = 1/scale метров

        QPainter p(&map);

        //поиск минимума
        float d = 1/scale;
        float PL = 28 + 22*log10(d) + 20*log10(fc);
        float dBm = ant + tx - PL;
        max_dbm = dBm;

        for(int i = 0; i < 1000; i++){
            for(int j = 0; j < 1000; j++){
                d = sqrt(pow((wifi_x-i),2)+pow((wifi_y-j),2))/scale;
                PL = 28 + 22*log10(d) + 20*log10(fc);
                dBm = ant + tx - PL;

                if (d == 0) dBm = 10;

                if (dBm < min_dbm) min_dbm = dBm;
                //if (dBm > max_dbm) max_dbm = dBm;

                dBm = ((dBm * -1) -8) * 3.86; // коэффициент можно менять 1.875=-144blue
                //dBm = int(dBm);

                if(dBm < 0) p.setPen(QColor(255,0,0, 255));
                else if(dBm < 64 ) p.setPen(QColor(255, dBm*4, 0, 255));
                else if(dBm < 128) p.setPen(QColor((255 - ((dBm-64)*4)), 255, 0, 255));
                else if(dBm < 192) p.setPen(QColor(0, 255, ((dBm-128)*4), 255));
                else if(dBm < 256) p.setPen(QColor(0, 255 - ((dBm-192)*4), 255, 255));
                else p.setPen(QColor(0,0,200, 255));

                    p.drawPoint(i, j);
            }
        }
        float k = (min_dbm - max_dbm) * -1;
        std::cout << " min=" << min_dbm;
        std::cout << " max=" << max_dbm<< "    разница=" << k << "    коэффициент=" << 255/k << "\n";
        p.end();
        scene->addPixmap(map);
        QGraphicsView* view = new QGraphicsView(scene);
        setCentralWidget(view);
}

MainWindow::~MainWindow()
{
}
