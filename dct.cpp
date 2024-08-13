#include "dct.h"



/*




    std::vector<std::vector<double>> bitY;
    std::vector<std::vector<double>> compbitCr;
    std::vector<std::vector<double>> compbitCb;


    void initCosTable() {
        for (int i = 0; i < N; ++i) {
            for (int j = 0; j < N; ++j) {
                cosTable[i][j] = cos((2 * i + 1) * j * M_PI / (2 * N));
            }
        }
    }

    static const int N = 8;
    std::vector<std::vector<double>> cosTable;

    void dct(const std::vector<std::vector<double>>& block, std::vector<std::vector<double>>& dctBlock);





bitY.resize(height, std::vector<double>(width));
std::vector<std::vector<double>> bitCr(height, std::vector<double>(width));
std::vector<std::vector<double>> bitCb(height, std::vector<double>(width));

this->labelbefore->setPixmap(QPixmap::fromImage(image).scaled(this->labelbefore->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));

// заполнение битовой матрицы
for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width; ++x) {
        QColor color = image.pixelColor(x, y);
        int r = color.red();
        int g = color.green();
        int b = color.blue();

        // Преобразование RGB в YCbCr
        double yVal = 0.299 * r + 0.587 * g + 0.114 * b;
        double cbVal = 128 - 0.168736 * r - 0.331264 * g + 0.5 * b;
        double crVal = 128 + 0.5 * r - 0.418688 * g - 0.081312 * b;

        bitY[y][x]=yVal;
        bitCr[y][x]=crVal;
        bitCb[y][x]=cbVal;

    }
}

//пробую цветовую субдискретизацию 4 2 0 которая берет среднее значение пикселей
compbitCr.resize(height, std::vector<double>(width));
compbitCb.resize(height, std::vector<double>(width));
for (int y=0;y<height;y+=2){
    for (int x=0;x<width;x+=2)
    {
        double avgCr=(bitCr[y][x]+bitCr[y+1][x]+bitCr[y][x+1]+bitCr[y+1][x+1])/4.0;
        double avgCb=(bitCb[y][x]+bitCb[y+1][x]+bitCb[y][x+1]+bitCb[y+1][x+1])/4.0;

        compbitCr[y][x]=avgCr;
        compbitCr[y+1][x]=avgCr;
        compbitCr[y][x+1]=avgCr;
        compbitCr[y+1][x+1]=avgCr;
        compbitCb[y][x]=avgCb;
        compbitCb[y+1][x]=avgCb;
        compbitCb[y][x+1]=avgCb;
        compbitCb[y+1][x+1]=avgCb;
    }
}



void MainWindow::dct(const std::vector<std::vector<double>>& block, std::vector<std::vector<double>>& dctBlock)
{

    double alpha_u, alpha_v, sum;
    for (int u = 0; u < N; ++u) {
        for (int v = 0; v < N; ++v) {
            sum = 0.0;
            for (int x = 0; x < N; ++x) {
                for (int y = 0; y < N; ++y) {
                    sum += block[x][y] * cosTable[x][u] * cosTable[y][v];
                }
            }
            alpha_u = (u == 0) ? sqrt(1.0 / N) : sqrt(2.0 / N);
            alpha_v = (v == 0) ? sqrt(1.0 / N) : sqrt(2.0 / N);
            dctBlock[u][v] = alpha_u * alpha_v * sum;
        }
    }
}

else if(algorigms_box->currentText()=="Дискретное косинусное преобразование")
{

    std::vector<std::vector<double>> dctbitY(height, std::vector<double>(width, 0.0));
    std::vector<std::vector<double>> dctbitCr(height, std::vector<double>(width, 0.0));
    std::vector<std::vector<double>> dctbitCb(height, std::vector<double>(width, 0.0));
    // Применение DCT к каждому блоку 8x8
    for (int i = 0; i < height; i += 8) {
        for (int j = 0; j < width; j += 8) {
            // Извлечение блока 8x8
            std::vector<std::vector<double>> blockY(8, std::vector<double>(8, 0.0));
            std::vector<std::vector<double>> blockCr(8, std::vector<double>(8, 0.0));
            std::vector<std::vector<double>> blockCb(8, std::vector<double>(8, 0.0));


            for (int x = 0; x < 8; ++x) {
                for (int y = 0; y < 8; ++y) {
                    blockY[x][y] = bitY[i + x][j + y];
                    blockCr[x][y] = compbitCr[i + x][j + y];
                    blockCb[x][y] = compbitCb[i + x][j + y];
                }
            }

            // Выполнение DCT для блока
            std::vector<std::vector<double>> dctBlockY(8, std::vector<double>(8, 0.0));
            std::vector<std::vector<double>> dctBlockCr(8, std::vector<double>(8, 0.0));
            std::vector<std::vector<double>> dctBlockCb(8, std::vector<double>(8, 0.0));
            dct(blockY, dctBlockY);
            //dct(blockCr,dctBlockCr);
            //dct(blockCb,dctBlockCb);


            // Запись результатов DCT обратно в изображение
            for (int x = 0; x < 8; ++x) {
                for (int y = 0; y < 8; ++y) {
                    dctbitY[i + x][j + y] = dctBlockY[x][y];
                    dctbitCr[i + x][j + y] = dctBlockCr[x][y];
                    dctbitCb[i + x][j + y] = dctBlockCb[x][y];
                }
            }
        }
    }
}
*/
