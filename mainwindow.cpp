#include "mainwindow.h"

struct Point {
    int x;
    int y;
};

struct pt {
    int x, y;
};

std::vector<Point> bresenhamLine(int x1, int y1, int x2, int y2) {
    std::vector<Point> points;
    bool steep = abs(y2 - y1) > abs(x2 - x1);
    if (steep) {
        std::swap(x1, y1);
        std::swap(x2, y2);
    }
    if (x1 > x2) {
        std::swap(x1, x2);
        std::swap(y1, y2);
    }
    int dx = x2 - x1;
    int dy = abs(y2 - y1);
    int error = dx / 2;
    int ystep = (y1 < y2) ? 1 : -1;
    int y = y1;
    for (int x = x1; x <= x2; x++) {
        if (steep) {
            points.push_back({y, x});
        } else {
            points.push_back({x, y});
        }
        error -= dy;
        if (error < 0) {
            y += ystep;
            error += dx;
        }
    }
    return points;
}
using namespace std;
inline int area (pt a, pt b, pt c) {
    return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
}

inline bool intersect_1 (int a, int b, int c, int d) {
    if (a > b)  swap(a, b);
    if (c > d)  swap(c, d);
    return max(a,c) <= min(b,d);
}

bool intersect (pt a, pt b, pt c, pt d) {
    return /*intersect_1 (a.x, b.x, c.x, d.x)
        && intersect_1 (a.y, b.y, c.y, d.y)
        && */area(a,b,c) * area(a,b,d) <= 0
        && area(c,d,a) * area(c,d,b) <= 0;
}

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

        float scale = 0.1; //1 пискель = 1/scale метров

        QPainter p(&map);

        //стенка
        pt a;
        a.x = 500;
        a.y = 350;
        pt b;
        b.x = 503;
        b.y = 450;

        pt c;
        c.x = wifi_x;
        c.y = wifi_y;

        //поиск минимума
        float d = 1/scale;
        float PL = 28 + 22*log10(d) + 20*log10(fc);
        float dBm = ant + tx - PL;
        max_dbm = dBm;

        int x1 = 500;
        int y1 = 350;
        int x2 = 503;
        int y2 = 450;

        std::vector<Point> pixelCoordinates = bresenhamLine(x1, y1, x2, y2);
        std::vector<Point> extraLine = bresenhamLine(x1+1, y1, x2+1, y2);

        // Увеличьте размер оригинального массива для объединения
        pixelCoordinates.resize(pixelCoordinates.size() + extraLine.size());

        // Скопируйте элементы из нового массива в оригинальный
        std::copy(extraLine.begin(), extraLine.end(), pixelCoordinates.end() - extraLine.size());
        for(int i = 0; i < 1000; i++){
            for(int j = 0; j < 1000; j++){
                    std::cout << "x=" << i << "          y=" << j << std::endl;
                d = sqrt(pow((wifi_x-i),2)+pow((wifi_y-j),2))/scale;
                PL = 28 + 22*log10(d) + 20*log10(fc);
                dBm = ant + tx - PL;

                pt D;
                D.x = i;
                D.y = j;
                if (intersect(a,b,c,D) != 0) {
                    std::vector<Point> linePoints = bresenhamLine(i, j, wifi_x, wifi_y);
                    for (const Point& pixel : linePoints) {
                        int x = pixel.x;
                        int y = pixel.y;
    //                    std::cout << "x=" << x << "          y=" << y << std::endl;
                        for (const Point& point : pixelCoordinates) {
                            if (x == point.x && y == point.y) {
                                //std::cout << "true" << std::endl;
                                dBm -= 20;
                                break; // Если нашли совпадение, можно выйти из цикла проверки pixelCoordinates
                            }
                        }
                    }
                }


                if (d == 0) dBm = 10;

                if (dBm < min_dbm) min_dbm = dBm;


                dBm = ((dBm * -1) -44) * 2.55; // коэффициент можно менять 1.875=-144blue

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
        scene->addLine(x1,y1,x2,y2);
        QGraphicsView* view = new QGraphicsView(scene);
        setCentralWidget(view);
        for (const Point& point : pixelCoordinates) {
                std::cout << "X: " << point.x << ", Y: " << point.y << std::endl;
            }
}



MainWindow::~MainWindow()
{
}
