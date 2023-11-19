#include "mainwindow.h"

// Маштаб затухания
float scaleAttenuationGlass = 0.7;

double attenuationGlass(double freq) {
    // Формула для стеклопакета
    return (2 + 0.2 * freq)*scaleAttenuationGlass;
}

double attenuationIRRGlass(double freq) {
    // Формула для IRR стекла
    return (23 + 3.0 * freq)*scaleAttenuationGlass;
}

double attenuationConcrete(double freq) {
    // Формула для бетона
    return (5 + 4.0 * freq)*scaleAttenuationGlass;
}

double attenuationWood(double freq) {
    // Формула для дерева
    return (4.85 + 0.12 * freq)*scaleAttenuationGlass;
}

struct Wall {
    int x;
    int y;
    int length;
    int width;
    int materialType;
};

// Функция для определения пересечения с одной стеной
bool isIntersectionWithWall(int x, int y, const Wall& wall) {
    return (x >= wall.x && x <= wall.x + wall.length && y >= wall.y && y <= wall.y + wall.width);
}

// Функция для подсчета пересечений со стенами
double countWallIntersections(int i, int j, int wifi_x, int wifi_y, const Wall walls[], int wallsCount) {
    double attenuation = 0.0;
    double fc = 5;

    int x = i;
    int y = j;
    int dx = abs(wifi_x - i);
    int dy = abs(wifi_y - j);
    int sx = i < wifi_x ? 1 : -1;
    int sy = j < wifi_y ? 1 : -1;
    int err = dx - dy;

    while (true) {
        for (int k = 0; k < wallsCount; ++k) {
            if (isIntersectionWithWall(x, y, walls[k])) {
                switch (walls[k].materialType) {
                case 1:
                    attenuation += attenuationGlass(fc);
                    break;
                case 2:
                    attenuation += attenuationIRRGlass(fc);
                    break;
                case 3:
                    attenuation += attenuationConcrete(fc);
                    break;
                case 4:
                    attenuation += attenuationWood(fc);
                    break;
                default:
                    break;
                }
            }
        }

        if (x == wifi_x && y == wifi_y) {
            break; // Достигнута конечная точка
        }

        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x += sx;
        }
        if (e2 < dx) {
            err += dx;
            y += sy;
        }
    }

    return attenuation;
}

// Формула распространения
float PL(float d, double fc){
    return 28 + 22*log10(d) + 20*log10(fc);
}


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent) {
    QGraphicsScene* scene = new QGraphicsScene();

    // Базовые переменные
    QPixmap map(1000, 1000);
    int wifi_x = 750;
    int wifi_y = 400;
    int tx = 23;
    int ant = 10;
    double fc = 5;

    // Маштаб (1 пискель = 1/scale метров)
    float scale = 0.2;

    // Создание 4 стен
    Wall walls[] = {
        {650, 200, 150, 20, 1},    // Стекло
        {300, 300, 4, 150, 2},    // IRR стекло
        {450, 650, 70, 5, 3},    // Бетон
        {750, 700, 120, 20, 4}    // Дерево
    };

    int wallsCount = sizeof(walls) / sizeof(walls[0]); // Количество стен

    QPainter p(&map);

    for (int i = 0; i < 1000; i++) {
        std::cout << "x=" << i << std::endl;
        for (int j = 0; j < 1000; j++) {
            //std::cout << "x=" << i << "          y=" << j << std::endl;

            float d = sqrt(pow((wifi_x - i), 2) + pow((wifi_y - j), 2)) / scale;
            float dBm = ant + tx - PL(d, fc);

            // Нормализация dBm от 0 -> 100
            dBm = ((dBm * -1) - 44) * 2.55; // коэффициент можно менять 1.875=-144blue

            int intersections = countWallIntersections(i, j, wifi_x, wifi_y, walls, wallsCount);

            // Учет влияния стен на увеличение dBm
            dBm += intersections;

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

    for (int i = 0; i < wallsCount; ++i) {
        int x = walls[i].x;
        int y = walls[i].y;
        int length = walls[i].length;
        int width = walls[i].width;
        // Добавляем стену в виде линии
        scene->addLine(x, y, x + length, y); // Верхняя горизонтальная линия
        scene->addLine(x, y, x, y + width);   // Левая вертикальная линия
        scene->addLine(x + length, y, x + length, y + width); // Правая вертикальная линия
        scene->addLine(x, y + width, x + length, y + width); // Нижняя горизонтальная линия
    }
    QGraphicsView* view = new QGraphicsView(scene);
    setCentralWidget(view);
}

MainWindow::~MainWindow()
{
}
