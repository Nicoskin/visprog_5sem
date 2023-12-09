#include "mainwindow.h"

struct Wall {
    int x;
    int y;
    int length;
    int width;
    int materialType;
};

class AttenuationModel {
public:
    float scaleAttenuationGlass = 0.7;

    double attenuationGlass(double freq) {
        // Формула для стеклопакета
        return (2 + 0.2 * freq) * scaleAttenuationGlass;
    }

    double attenuationIRRGlass(double freq) {
        // Формула для IRR стекла
        return (23 + 0.3 * freq) * scaleAttenuationGlass;
    }

    double attenuationConcrete(double freq) {
        // Формула для бетона
        return (5 + 4 * freq) * scaleAttenuationGlass * 0.11; // *0.13 чтоб покрасивее было
    }

    double attenuationWood(double freq) {
        // Формула для дерева
        return (4.85 + 0.12 * freq)*scaleAttenuationGlass;
    }
};

class Heatmap {
private:
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
        AttenuationModel model;
        while (true) {
            for (int k = 0; k < wallsCount; ++k) {
                if (isIntersectionWithWall(x, y, walls[k])) {
                    switch (walls[k].materialType) {
                    case 1:
                        attenuation += model.attenuationGlass(fc);
                        break;
                    case 2:
                        attenuation += model.attenuationIRRGlass(fc);
                        break;
                    case 3:
                        attenuation += model.attenuationConcrete(fc);
                        break;
                    case 4:
                        attenuation += model.attenuationWood(fc);
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

    // Красивая загрузка
    void showLoadingBar(int progress) {
        const int barWidth = 50; // Ширина полосы загрузки
        const int blockSize = 100 / barWidth; // Количество блоков на 1%

        std::cout << "[";
        int completedBlocks = progress / blockSize;

        // Для корректного отображения прогресса, увеличиваем completedBlocks
        for (int i = 0; i < barWidth; ++i) {
            if (i < completedBlocks)
                std::cout << "="; // Символ полной загрузки
            else
                std::cout << ".";
        }
        std::cout << "] " << progress << "%\r";
        std::cout.flush();
    }

    // Формула распространения
    float PL(float d, double fc){
        return 28 + 22*log10(d) + 20*log10(fc);
    }

public:
    // Рисует тепловую карту с учётом стен
    void drawWirelessMap(QPainter &p, int wifi_x, int wifi_y, float scale, int ant, int tx, float fc, const Wall walls[], int wallsCount) {
        for (int i = 0; i < 1000; i++) {
            showLoadingBar(i / 10);
            for (int j = 0; j < 1000; j++) {
                float d = sqrt(pow((wifi_x - i), 2) + pow((wifi_y - j), 2)) / scale;
                float dBm = ant + tx - PL(d, fc);
                dBm = ((dBm * -1) - 44) * 2.55;
                int intersections = countWallIntersections(i, j, wifi_x, wifi_y, walls, wallsCount);
                dBm += intersections;

                if (dBm < 0) p.setPen(QColor(255, 0, 0, 255));
                else if (dBm < 64) p.setPen(QColor(255, dBm * 4, 0, 255));
                else if (dBm < 128) p.setPen(QColor((255 - ((dBm - 64) * 4)), 255, 0, 255));
                else if (dBm < 192) p.setPen(QColor(0, 255, ((dBm - 128) * 4), 255));
                else if (dBm < 256) p.setPen(QColor(0, 255 - ((dBm - 192) * 4), 255, 255));
                else p.setPen(QColor(0, 0, 150, 255));

                p.drawPoint(i, j);
            }
        }
    }

    // Рисует стены
    void drawWallsOnMap(QPainter &p, const Wall walls[], int wallsCount) {
        for (int i = 0; i < wallsCount; ++i) {
            int x = walls[i].x;
            int y = walls[i].y;
            int length = walls[i].length;
            int width = walls[i].width;
            int materialType = walls[i].materialType;

            switch (materialType) {
            case 1:
                p.setPen(QColor(66, 170, 255));
                break;
            case 2:
                p.setPen(QColor(186, 85, 211));
                break;
            case 3:
                p.setPen(QColor(128, 128, 128));
                break;
            case 4:
                p.setPen(QColor(101, 67, 33));
                break;
            default:
                break;
            }

            for (int px = x; px < x + length; ++px) {
                for (int py = y; py < y + width; ++py) {
                    p.drawPoint(px, py);
                }
            }
        }
    }

};

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

    float scale = 0.2; // Маштаб (1 пискель = 1/scale метров)

    // Создание 4 стен
    Wall walls[] = {
        {650, 300, 150, 60, 1},   // Стекло
        {500, 300, 6,  150, 2},   // IRR стекло
        {350, 650, 270, 50, 3},   // Бетон
        {750, 700, 120, 20, 4}    // Дерево
    };

    QPainter p(&map);

    Heatmap heatmat;
    heatmat.drawWirelessMap(p, wifi_x, wifi_y, scale, ant, tx, fc, walls, sizeof(walls) / sizeof(walls[0]));
    heatmat.drawWallsOnMap(p, walls, sizeof(walls) / sizeof(walls[0]));

    scene->addPixmap(map);
    QGraphicsView* view = new QGraphicsView(scene);
    setCentralWidget(view);
}

MainWindow::~MainWindow()
{
}
