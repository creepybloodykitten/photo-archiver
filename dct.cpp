#include "dct.h"

/*
//пробую цветовую субдискретизацию 4 2 0 которая берет среднее значение пикселей
void color_subsampling(unsigned short int &width,unsigned short int &height,std::vector<std::vector<double>> &bitCr,std::vector<std::vector<double>> &bitCb)
{
    //точно ли обрабатывает без двойниковых контейнеров и не выходит ли за границы?
    for (int y=0;y<height;y+=2)
    {
        for (int x=0;x<width;x+=2)
        {
            double avgCr=(bitCr[y][x]+bitCr[y+1][x]+bitCr[y][x+1]+bitCr[y+1][x+1])/4.0;
            double avgCb=(bitCb[y][x]+bitCb[y+1][x]+bitCb[y][x+1]+bitCb[y+1][x+1])/4.0;

            bitCr[y][x]=avgCr;
            bitCr[y+1][x]=avgCr;
            bitCr[y][x+1]=avgCr;
            bitCr[y+1][x+1]=avgCr;
            bitCb[y][x]=avgCb;
            bitCb[y+1][x]=avgCb;
            bitCb[y][x+1]=avgCb;
            bitCb[y+1][x+1]=avgCb;
        }
    }
}



void dct_y(std::vector<std::vector<double>> &bitY,std::vector<int> &dct_bitY,unsigned short int &width,unsigned short int &height)
{
    //делаю вектор содержащий уже вычисленные значения необходимых косинусных функций
    std::vector<std::vector<double>> cosTable(8, std::vector<double>(8));
    for (int i = 0; i < 8; ++i)
    {
        for (int j = 0; j < 8; ++j)
        {
            cosTable[i][j] = std::cos((2 * i + 1) * j * 3.14159265358979323846 / (2 * 8));
        }
    }

    //таблица квантования для яркости
    double quant_Y[8][8] = {
        {16, 11, 10, 16,  24,  40,  51,  61},
        {12, 12, 14, 19,  26,  58,  60,  55},
        {14, 13, 16, 24,  40,  57,  69,  56},
        {14, 17, 22, 29,  51,  87,  80,  62},
        {18, 22, 37, 56,  68, 109, 103,  77},
        {24, 35, 55, 64,  81, 104, 113,  92},
        {49, 64, 78, 87, 103, 121, 120, 101},
        {72, 92, 95, 98, 112, 100, 103,  99}
    };

    int n=8;


    // извлечение всех блоков 8 на 8 пикселей
    for (int i = 0; i < height; i += n) {
        for (int j = 0; j < width; j += n) {
            // инициализирую пустой блок 8x8 и заполняю его значениями из основной матрицы bitY
            std::vector<std::vector<double>> blockY(8, std::vector<double>(8, 0.0));

            for (int x = 0; x < n; ++x)
            {
                for (int y = 0; y < n; ++y)
                {
                    blockY[x][y] = bitY[i + x][j + y];
                }
            }

            // блок 8 на 8 который будет хранить хранить значения блока после dct
            std::vector<std::vector<double>> dctBlockY(8, std::vector<double>(8, 0.0));

            double alpha_u=0;
            double alpha_v=0;
            double sum=0;
            //вычисление dct
            for (int u = 0; u < n; ++u)
            {
                for (int v = 0; v < n; ++v)
                {
                    sum = 0.0;
                    for (int x = 0; x < n; ++x)
                    {
                        for (int y = 0; y < n; ++y)
                        {
                            sum += blockY[x][y] * cosTable[x][u] * cosTable[y][v];
                        }
                    }
                    double alpha_u = (u == 0) ? 1.0 / std::sqrt(2.0) : 1.0;
                    double alpha_v = (v == 0) ? 1.0 / std::sqrt(2.0) : 1.0;
                    dctBlockY[u][v] = 0.25 * alpha_u * alpha_v * sum;
                }
            }

            //зигзаг чтение матрицы 8 на 8
            //я записываю в вектор только числа неравные нулю и также записываю их индексы одним числом
            for (int sum = 0; sum <= 2 * (n - 1); ++sum)
            {
                if (sum % 2 != 0)
                {
                    for (int i = std::max(0, sum - n + 1); i <= std::min(sum, n - 1); ++i)
                    {
                        int newval=static_cast<int>(std::round(dctBlockY[i][sum-i]/quant_Y[i][sum-i]));
                        if(newval!=0)
                        {
                            dct_bitY.push_back(i*8+sum-i);//позиция числа которую потом можно перевести в позицию в матрице
                            dct_bitY.push_back(newval);
                        }
                    }
                }
                else
                {
                    for (int i = std::min(sum, n - 1); i >= std::max(0, sum - n + 1); --i)
                    {
                        int newval=static_cast<int>(std::round(dctBlockY[i][sum-i]/quant_Y[i][sum-i]));
                        if(newval!=0)
                        {
                            dct_bitY.push_back(i*8+sum-i);//позиция числа которую потом можно перевести в позицию в матрице
                            dct_bitY.push_back(newval);
                        }
                    }
                }
            }

        }
    }

}




quint64 DCT::encoding(QByteArray &compressedData, quint8 &method,unsigned short int &width,unsigned short int &height,QImage &inputimage,QImage &outputimage)
{
    QBuffer buffer(&compressedData);
    buffer.open(QIODevice::WriteOnly);
    QDataStream out(&buffer);
    method=4;

    std::vector<std::vector<double>> bitY(height, std::vector<double>(width));
    std::vector<std::vector<double>> bitCr(height, std::vector<double>(width));
    std::vector<std::vector<double>> bitCb(height, std::vector<double>(width));

    // заполнение битовой матрицы
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            QColor color = inputimage.pixelColor(x, y);
            int r = color.red();
            int g = color.green();
            int b = color.blue();

            // Преобразование RGB в YCbCr //вычитаю в конце 128 так как перевожу из диапазона 0-255 в диапазон -128-127
            double yVal = (0.299 * r + 0.587 * g + 0.114 * b)-128;
            double cbVal = (128 - 0.168736 * r - 0.331264 * g + 0.5 * b)-128;
            double crVal = (128 + 0.5 * r - 0.418688 * g - 0.081312 * b)-128;

            bitY[y][x]=yVal;
            bitCr[y][x]=crVal;
            bitCb[y][x]=cbVal;

        }
    }

    //цветовая субдискретизация 4 2 0
    color_subsampling(width,height,bitCr,bitCb);

    std::vector<int> dct_bitY;
    dct_y(bitY,dct_bitY,width,height);

    out<<method<<width<<height;
    out<<static_cast<int>(dct_bitY.size());

    for (int data:dct_bitY)
    {
        out<<static_cast<signed char>(data);//если вдруг будут ошибки то измени тип данных который принимает больше значений мало ли какое то значение больше 127
    }

    for (int y=0;y<height;y+=2)
    {
        for (int x=0;x<width;x+=2)
        {
            out<<static_cast<quint8>(std::round(bitCr[y][x]))<<static_cast<quint8>(std::round(bitCb[y][x]));
        }
    }


    outputimage=QImage(width,height,QImage::Format_RGB888);
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            outputimage.setPixel(x,y,qRgb(bitY[y][x]+128,bitY[y][x]+128,bitY[y][x]+128));
        }
    }


    // for (int y = 0; y < 64; ++y)
    // {
    //     for (int x = 0; x < 64; ++x)
    //     {
    //         std::cout<<bitY[y][x]<<" ";
    //     }
    // }

    buffer.close();
    return buffer.size();


}

void DCT::decoding(QString &filename,QByteArray &imageData,quint8 &method,unsigned short int &width,unsigned short int &height,QImage &inputimage)
{

    std::vector<std::vector<double>> cosTable(8, std::vector<double>(8));
    for (int i = 0; i < 8; ++i)
    {
        for (int j = 0; j < 8; ++j)
        {
            cosTable[i][j] = std::cos((2 * i + 1) * j * 3.14159265358979323846 / (2 * 8));
        }
    }

    double quant_Y[8][8] = {
        {16, 11, 10, 16,  24,  40,  51,  61},
        {12, 12, 14, 19,  26,  58,  60,  55},
        {14, 13, 16, 24,  40,  57,  69,  56},
        {14, 17, 22, 29,  51,  87,  80,  62},
        {18, 22, 37, 56,  68, 109, 103,  77},
        {24, 35, 55, 64,  81, 104, 113,  92},
        {49, 64, 78, 87, 103, 121, 120, 101},
        {72, 92, 95, 98, 112, 100, 103,  99}
    };

    std::vector<std::vector<double>> bitY(height, std::vector<double>(width));
    std::vector<std::vector<double>> bitCr(height, std::vector<double>(width));
    std::vector<std::vector<double>> bitCb(height, std::vector<double>(width));
    int dct_Y_size=0;

    int n=8;

    QFile file(filename);
    file.open(QIODevice::ReadOnly);

    imageData = file.readAll();
    file.close();

    QBuffer buffer(&imageData);
    buffer.open(QIODevice::ReadOnly);
    QDataStream in(&buffer);



    in >> method >> width >> height;

    in>>dct_Y_size;
    std::vector<int> dct_bitY;

    for (int i=0;i<dct_Y_size;++i)
    {
        signed char ch=0;
        in>>ch;
        dct_bitY.push_back(static_cast<int>(ch));

    }


    //посути если я сначала считаю cr и сb то все оставшиеся будет y и тогда не надо записывать в int размер
    for (int y=0;y<height;y+=2)
    {
        for (int x=0;x<width;x+=2)
        {
            quint8 comp_cr;
            quint8 comp_cb;
            in>>comp_cr>>comp_cb;
            bitCr[y][x]=comp_cr;
            bitCr[y+1][x]=comp_cr;
            bitCr[y][x+1]=comp_cr;
            bitCr[y+1][x+1]=comp_cr;
            bitCb[y][x]=comp_cb;
            bitCb[y+1][x]=comp_cb;
            bitCb[y][x+1]=comp_cb;
            bitCb[y+1][x+1]=comp_cb;
        }
    }

    int posx=0;
    int posy=0;
    std::vector<int> temp;
    for(int i=0;i<dct_bitY.size();++i)
    {
        if(dct_bitY[i]==0)
        {
            temp.clear();
            temp.push_back(0);
        }
        else if(dct_bitY[i]!=0)
        {
            temp.push_back(dct_bitY[i]);
        }
        if(dct_bitY[i+1]==0 || i + 1 == dct_bitY.size())
        {
            std::vector<std::vector<double>> dctBlockY(8, std::vector<double>(8, 0.0));
            for(int j=0;j<temp.size();j+=2)//проверить насчет границ
            {
                int posX=temp[j]/n;//строка
                int posY=temp[j]%n;//столбец
                int val=temp[j+1];
                dctBlockY[posX][posY]=val*quant_Y[posX][posY];
            }

            //применение обратного дискретного косинусного преобразования
            std::vector<std::vector<double>> blockY(8, std::vector<double>(8, 0.0));
            double alpha_u=0;
            double alpha_v=0;
            double sum=0;
            for (int x = 0; x < n; ++x)
            {
                for (int y = 0; y < n; ++y)
                {
                    sum = 0.0;
                    for (int u = 0; u < n; ++u) {
                        for (int v = 0; v < n; ++v) {
                            alpha_u = (u == 0) ? 1.0 / std::sqrt(2.0) : 1.0;
                            alpha_v = (v == 0) ? 1.0 / std::sqrt(2.0) : 1.0;
                            sum += alpha_u * alpha_v * dctBlockY[u][v] * cosTable[x][u] * cosTable[y][v];
                        }
                    }
                    blockY[x][y] = 0.25 * sum;
                }
            }

            //теперь вставляю этот блок Y значений в основной bitY
            for(int y=0;y<n;++y)
            {
                for(int x=0;x<n;++x)
                {
                    bitY[y+posy][x+posx]=blockY[y][x];

                }
            }
            posx+=8;
            if (posx>width-1)
            {
                posx=0;
                posy+=8;
            }
        }

    }





    inputimage=QImage(width, height, QImage::Format_RGB888);

    // for (int y = 0; y < height; ++y)
    // {
    //     for (int x = 0; x < width; ++x)
    //     {
    //         double Y = bitY[y][x]+128;
    //         double Cb = bitCb[y][x]+128;
    //         double Cr = bitCr[y][x]+128;

    //         double R = Y + 1.402 * (Cr - 128);
    //         double G = Y - 0.344136 * (Cb - 128) - 0.714136 * (Cr - 128);
    //         double B = Y + 1.772 * (Cb - 128);

    //         //inputimage.setPixel(x,y,qRgb(static_cast<quint8>(R),static_cast<quint8>(G),static_cast<quint8>(B)));
    //         inputimage.setPixel(x,y,qRgb(R,G,B));
    //     }
    // }



    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            inputimage.setPixel(x,y,qRgb(bitY[y][x]+128,bitY[y][x]+128,bitY[y][x]+128));
        }
    }

    // for (int y = 0; y < 64; ++y)
    // {
    //     for (int x = 0; x < 64; ++x)
    //     {
    //         std::cout<<bitY[y][x]<<" ";
    //     }
    // }

}
*/



/*
//все что я тут изменил по сравннеию с веръним вариантом - убрал повторения временныъ переменных
//пробую цветовую субдискретизацию 4 2 0 которая берет среднее значение пикселей
void color_subsampling(unsigned short int &width,unsigned short int &height,std::vector<std::vector<double>> &bitCr,std::vector<std::vector<double>> &bitCb)
{
    //точно ли обрабатывает без двойниковых контейнеров и не выходит ли за границы?
    for (int y=0;y<height;y+=2)
    {
        for (int x=0;x<width;x+=2)
        {
            double avgCr=(bitCr[y][x]+bitCr[y+1][x]+bitCr[y][x+1]+bitCr[y+1][x+1])/4.0;
            double avgCb=(bitCb[y][x]+bitCb[y+1][x]+bitCb[y][x+1]+bitCb[y+1][x+1])/4.0;

            bitCr[y][x]=avgCr;
            bitCr[y+1][x]=avgCr;
            bitCr[y][x+1]=avgCr;
            bitCr[y+1][x+1]=avgCr;
            bitCb[y][x]=avgCb;
            bitCb[y+1][x]=avgCb;
            bitCb[y][x+1]=avgCb;
            bitCb[y+1][x+1]=avgCb;
        }
    }
}



void dct_y(std::vector<std::vector<double>> &bitY,std::vector<int> &dct_bitY,unsigned short int &width,unsigned short int &height)
{
    //делаю вектор содержащий уже вычисленные значения необходимых косинусных функций
    std::vector<std::vector<double>> cosTable(8, std::vector<double>(8));
    for (int i = 0; i < 8; ++i)
    {
        for (int j = 0; j < 8; ++j)
        {
            cosTable[i][j] = std::cos((2 * i + 1) * j * 3.14159265358979323846 / (2 * 8));
        }
    }

    //таблица квантования для яркости
    double quant_Y[8][8] = {
        {16, 11, 10, 16,  24,  40,  51,  61},
        {12, 12, 14, 19,  26,  58,  60,  55},
        {14, 13, 16, 24,  40,  57,  69,  56},
        {14, 17, 22, 29,  51,  87,  80,  62},
        {18, 22, 37, 56,  68, 109, 103,  77},
        {24, 35, 55, 64,  81, 104, 113,  92},
        {49, 64, 78, 87, 103, 121, 120, 101},
        {72, 92, 95, 98, 112, 100, 103,  99}
    };

    int n=8;


    // извлечение всех блоков 8 на 8 пикселей
    for (int i = 0; i < height; i += n) {
        for (int j = 0; j < width; j += n) {
            // инициализирую пустой блок 8x8 и заполняю его значениями из основной матрицы bitY
            std::vector<std::vector<double>> blockY(8, std::vector<double>(8, 0.0));

            for (int x1= 0; x1 < n; ++x1)
            {
                for (int y1 = 0; y1 < n; ++y1)
                {
                    blockY[x1][y1] = bitY[i + x1][j + y1];
                }
            }

            // блок 8 на 8 который будет хранить хранить значения блока после dct
            std::vector<std::vector<double>> dctBlockY(8, std::vector<double>(8, 0.0));

            double alpha_u=0;
            double alpha_v=0;
            double sum=0;
            //вычисление dct
            for (int u = 0; u < n; ++u)
            {
                for (int v = 0; v < n; ++v)
                {
                    sum = 0.0;
                    for (int x = 0; x < n; ++x)
                    {
                        for (int y = 0; y < n; ++y)
                        {
                            sum += blockY[x][y] * cosTable[x][u] * cosTable[y][v];
                        }
                    }
                    double alpha_u = (u == 0) ? 1.0 / std::sqrt(2.0) : 1.0;
                    double alpha_v = (v == 0) ? 1.0 / std::sqrt(2.0) : 1.0;
                    dctBlockY[u][v] = 0.25 * alpha_u * alpha_v * sum;
                }
            }

            //зигзаг чтение матрицы 8 на 8
            //я записываю в вектор только числа неравные нулю и также записываю их индексы одним числом
            for (int sum = 0; sum <= 2 * (n - 1); ++sum)
            {
                if (sum % 2 != 0)
                {
                    for (int ii = std::max(0, sum - n + 1); ii <= std::min(sum, n - 1); ++ii)
                    {
                        int newval=static_cast<int>(std::round(dctBlockY[ii][sum-ii]/quant_Y[ii][sum-ii]));
                        if(newval!=0)
                        {
                            dct_bitY.push_back(ii*8+sum-ii);//позиция числа которую потом можно перевести в позицию в матрице
                            dct_bitY.push_back(newval);
                        }
                    }
                }
                else
                {
                    for (int ii = std::min(sum, n - 1); ii >= std::max(0, sum - n + 1); --ii)
                    {
                        int newval=static_cast<int>(std::round(dctBlockY[ii][sum-ii]/quant_Y[ii][sum-ii]));
                        if(newval!=0)
                        {
                            dct_bitY.push_back(ii*8+sum-ii);//позиция числа которую потом можно перевести в позицию в матрице
                            dct_bitY.push_back(newval);
                        }
                    }
                }
            }

        }
    }

}




quint64 DCT::encoding(QByteArray &compressedData, quint8 &method,unsigned short int &width,unsigned short int &height,QImage &inputimage,QImage &outputimage)
{
    QBuffer buffer(&compressedData);
    buffer.open(QIODevice::WriteOnly);
    QDataStream out(&buffer);
    method=4;

    std::vector<std::vector<double>> bitY(height, std::vector<double>(width));
    std::vector<std::vector<double>> bitCr(height, std::vector<double>(width));
    std::vector<std::vector<double>> bitCb(height, std::vector<double>(width));

    // заполнение битовой матрицы
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            QColor color = inputimage.pixelColor(x, y);
            int r = color.red();
            int g = color.green();
            int b = color.blue();

            // Преобразование RGB в YCbCr //вычитаю в конце 128 так как перевожу из диапазона 0-255 в диапазон -128-127
            double yVal = (0.299 * r + 0.587 * g + 0.114 * b)-128;
            double cbVal = (128 - 0.168736 * r - 0.331264 * g + 0.5 * b)-128;
            double crVal = (128 + 0.5 * r - 0.418688 * g - 0.081312 * b)-128;

            bitY[y][x]=yVal;
            bitCr[y][x]=crVal;
            bitCb[y][x]=cbVal;

        }
    }

    //цветовая субдискретизация 4 2 0
    color_subsampling(width,height,bitCr,bitCb);

    std::vector<int> dct_bitY;
    dct_y(bitY,dct_bitY,width,height);

    out<<method<<width<<height;
    out<<static_cast<int>(dct_bitY.size());

    for (int data:dct_bitY)
    {
        out<<static_cast<signed char>(data);//если вдруг будут ошибки то измени тип данных который принимает больше значений мало ли какое то значение больше 127
    }

    for (int y=0;y<height;y+=2)
    {
        for (int x=0;x<width;x+=2)
        {
            out<<static_cast<quint8>(std::round(bitCr[y][x]))<<static_cast<quint8>(std::round(bitCb[y][x]));
        }
    }


    outputimage=QImage(width,height,QImage::Format_RGB888);
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            outputimage.setPixel(x,y,qRgb(bitY[y][x]+128,bitY[y][x]+128,bitY[y][x]+128));
        }
    }


    for (int y = 0; y < 64; ++y)
    {
        for (int x = 0; x < 64; ++x)
        {
            std::cout<<bitY[y][x]<<" ";
        }
    }

    buffer.close();
    return buffer.size();


}

void DCT::decoding(QString &filename,QByteArray &imageData,quint8 &method,unsigned short int &width,unsigned short int &height,QImage &inputimage)
{

    std::vector<std::vector<double>> cosTable(8, std::vector<double>(8));
    for (int i = 0; i < 8; ++i)
    {
        for (int j = 0; j < 8; ++j)
        {
            cosTable[i][j] = std::cos((2 * i + 1) * j * 3.14159265358979323846 / (2 * 8));
        }
    }

    double quant_Y[8][8] = {
        {16, 11, 10, 16,  24,  40,  51,  61},
        {12, 12, 14, 19,  26,  58,  60,  55},
        {14, 13, 16, 24,  40,  57,  69,  56},
        {14, 17, 22, 29,  51,  87,  80,  62},
        {18, 22, 37, 56,  68, 109, 103,  77},
        {24, 35, 55, 64,  81, 104, 113,  92},
        {49, 64, 78, 87, 103, 121, 120, 101},
        {72, 92, 95, 98, 112, 100, 103,  99}
    };

    std::vector<std::vector<double>> bitY(height, std::vector<double>(width,0.0));
    std::vector<std::vector<double>> bitCr(height, std::vector<double>(width));
    std::vector<std::vector<double>> bitCb(height, std::vector<double>(width));
    int dct_Y_size=0;

    int n=8;

    QFile file(filename);
    file.open(QIODevice::ReadOnly);

    imageData = file.readAll();
    file.close();

    QBuffer buffer(&imageData);
    buffer.open(QIODevice::ReadOnly);
    QDataStream in(&buffer);



    in >> method >> width >> height;

    in>>dct_Y_size;
    std::vector<int> dct_bitY;

    for (int i=0;i<dct_Y_size;++i)
    {
        signed char ch=0;
        in>>ch;
        dct_bitY.push_back(static_cast<int>(ch));

    }


    //посути если я сначала считаю cr и сb то все оставшиеся будет y и тогда не надо записывать в int размер
    for (int y=0;y<height;y+=2)
    {
        for (int x=0;x<width;x+=2)
        {
            quint8 comp_cr;
            quint8 comp_cb;
            in>>comp_cr>>comp_cb;
            bitCr[y][x]=comp_cr;
            bitCr[y+1][x]=comp_cr;
            bitCr[y][x+1]=comp_cr;
            bitCr[y+1][x+1]=comp_cr;
            bitCb[y][x]=comp_cb;
            bitCb[y+1][x]=comp_cb;
            bitCb[y][x+1]=comp_cb;
            bitCb[y+1][x+1]=comp_cb;
        }
    }

    int posx=0;
    int posy=0;
    std::vector<int> temp;
    for(int i=0;i<dct_bitY.size();++i)
    {
        if(dct_bitY[i]==0)
        {
            temp.clear();
            temp.push_back(0);
        }
        else if(dct_bitY[i]!=0)
        {
            temp.push_back(dct_bitY[i]);
        }
        if(dct_bitY[i+1]==0 || i + 1 == dct_bitY.size())
        {
            std::vector<std::vector<double>> dctBlockY(8, std::vector<double>(8, 0.0));
            for(int j=0;j<temp.size();j+=2)//проверить насчет границ
            {
                int posX=temp[j]/n;//строка
                int posY=temp[j]%n;//столбец
                int val=temp[j+1];
                dctBlockY[posX][posY]=val*quant_Y[posX][posY];
            }

            //применение обратного дискретного косинусного преобразования
            std::vector<std::vector<double>> blockY(8, std::vector<double>(8, 0.0));
            double alpha_u=0;
            double alpha_v=0;
            double sum=0;
            for (int x = 0; x < n; ++x)
            {
                for (int y = 0; y < n; ++y)
                {
                    sum = 0.0;
                    for (int u = 0; u < n; ++u) {
                        for (int v = 0; v < n; ++v) {
                            alpha_u = (u == 0) ? 1.0 / std::sqrt(2.0) : 1.0;
                            alpha_v = (v == 0) ? 1.0 / std::sqrt(2.0) : 1.0;
                            sum += alpha_u * alpha_v * dctBlockY[u][v] * cosTable[x][u] * cosTable[y][v];
                        }
                    }
                    blockY[x][y] = 0.25 * sum;
                }
            }

            //теперь вставляю этот блок Y значений в основной bitY
            for(int y=0;y<n;++y)
            {
                for(int x=0;x<n;++x)
                {
                    bitY[y+posy][x+posx]=blockY[y][x];

                }
            }
            posx+=8;
            if (posx>width-1)
            {
                posx=0;
                posy+=8;
            }
        }

    }





    inputimage=QImage(width, height, QImage::Format_RGB888);

    // for (int y = 0; y < height; ++y)
    // {
    //     for (int x = 0; x < width; ++x)
    //     {
    //         double Y = bitY[y][x]+128;
    //         double Cb = bitCb[y][x]+128;
    //         double Cr = bitCr[y][x]+128;

    //         double R = Y + 1.402 * (Cr - 128);
    //         double G = Y - 0.344136 * (Cb - 128) - 0.714136 * (Cr - 128);
    //         double B = Y + 1.772 * (Cb - 128);

    //         //inputimage.setPixel(x,y,qRgb(static_cast<quint8>(R),static_cast<quint8>(G),static_cast<quint8>(B)));
    //         inputimage.setPixel(x,y,qRgb(R,G,B));
    //     }
    // }



    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            inputimage.setPixel(x,y,qRgb(bitY[y][x]+128,bitY[y][x]+128,bitY[y][x]+128));
        }
    }

    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            std::cout<<bitY[y][x]<<" ";
        }
    }

}
*/






/*
//пробую цветовую субдискретизацию 4 2 0 которая берет среднее значение пикселей
void color_subsampling(unsigned short int &width,unsigned short int &height,std::vector<std::vector<double>> &bitCr,std::vector<std::vector<double>> &bitCb)
{
    //точно ли обрабатывает без двойниковых контейнеров и не выходит ли за границы?
    for (int y=0;y<height;y+=2)
    {
        for (int x=0;x<width;x+=2)
        {
            double avgCr=(bitCr[y][x]+bitCr[y+1][x]+bitCr[y][x+1]+bitCr[y+1][x+1])/4.0;
            double avgCb=(bitCb[y][x]+bitCb[y+1][x]+bitCb[y][x+1]+bitCb[y+1][x+1])/4.0;

            bitCr[y][x]=avgCr;
            bitCr[y+1][x]=avgCr;
            bitCr[y][x+1]=avgCr;
            bitCr[y+1][x+1]=avgCr;
            bitCb[y][x]=avgCb;
            bitCb[y+1][x]=avgCb;
            bitCb[y][x+1]=avgCb;
            bitCb[y+1][x+1]=avgCb;
        }
    }
}



//рабочее dct без сжатия
void dct_y(std::vector<std::vector<double>> &bitY,std::vector<std::vector<int>> &dct_bitY,unsigned short int &width,unsigned short int &height)
{
    //делаю вектор содержащий уже вычисленные значения необходимых косинусных функций
    std::vector<std::vector<double>> cosTable(8, std::vector<double>(8));
    for (int i = 0; i < 8; ++i)
    {
        for (int j = 0; j < 8; ++j)
        {
            cosTable[i][j] = std::cos((2 * i + 1) * j * 3.14159265358979323846 / (2 * 8));
        }
    }

    //таблица квантования для яркости
    double quant_Y[8][8] = {
        {16, 11, 10, 16,  24,  40,  51,  61},
        {12, 12, 14, 19,  26,  58,  60,  55},
        {14, 13, 16, 24,  40,  57,  69,  56},
        {14, 17, 22, 29,  51,  87,  80,  62},
        {18, 22, 37, 56,  68, 109, 103,  77},
        {24, 35, 55, 64,  81, 104, 113,  92},
        {49, 64, 78, 87, 103, 121, 120, 101},
        {72, 92, 95, 98, 112, 100, 103,  99}
    };

    int n=8;


    // извлечение всех блоков 8 на 8 пикселей
    for (int i = 0; i < height; i += n) {
        for (int j = 0; j < width; j += n) {
            // инициализирую пустой блок 8x8 и заполняю его значениями из основной матрицы bitY
            std::vector<std::vector<double>> blockY(8, std::vector<double>(8, 0.0));

            for (int x1= 0; x1 < n; ++x1)
            {
                for (int y1 = 0; y1 < n; ++y1)
                {
                    blockY[x1][y1] = bitY[i + x1][j + y1];
                }
            }

            // блок 8 на 8 который будет хранить хранить значения блока после dct
            std::vector<std::vector<double>> dctBlockY(8, std::vector<double>(8, 0.0));

            double alpha_u=0;
            double alpha_v=0;
            double sum=0;
            //вычисление dct
            for (int u = 0; u < n; ++u)
            {
                for (int v = 0; v < n; ++v)
                {
                    sum = 0.0;
                    for (int x = 0; x < n; ++x)
                    {
                        for (int y = 0; y < n; ++y)
                        {
                            sum += blockY[x][y] * cosTable[x][u] * cosTable[y][v];
                        }
                    }
                    double alpha_u = (u == 0) ? 1.0 / std::sqrt(2.0) : 1.0;
                    double alpha_v = (v == 0) ? 1.0 / std::sqrt(2.0) : 1.0;
                    dctBlockY[u][v] = 0.25 * alpha_u * alpha_v * sum;
                }
            }


            for(int ii=0;ii<n;++ii)
            {
                for (int jj=0;jj<n;++jj)
                {
                    dct_bitY[i+ii][j+jj]=static_cast<int>(std::round(dctBlockY[ii][jj]/quant_Y[ii][jj]));
                }
            }


        }
    }

}

std::shared_ptr<HuffmanNode> buildHuffmanTree1(const std::vector<unsigned char> freq)
{
    std::priority_queue<std::shared_ptr<HuffmanNode>, std::vector<std::shared_ptr<HuffmanNode>>, Compare> root;

    for (int i = 0; i < freq.size(); ++i)
    {
        if (freq[i] > 0)
        {
            root.push(std::make_shared<HuffmanNode>(i, freq[i]));
        }
    }

    while (root.size() > 1)
    {
        std::shared_ptr<HuffmanNode> left = root.top();
        root.pop();
        std::shared_ptr<HuffmanNode> right = root.top();
        root.pop();

        std::shared_ptr<HuffmanNode> newNode = std::make_shared<HuffmanNode>(0, left->frequency + right->frequency);
        newNode->left = left;
        newNode->right = right;
        root.push(newNode);
    }

    return root.top();
}


void generateHuffmanCodes1(const std::shared_ptr<HuffmanNode>& node, const std::string& prefix, std::vector<std::string>& huffmanCodeArr)
{
    if (node->left == nullptr && node->right == nullptr)
    {
        huffmanCodeArr[node->value] = prefix;
    }
    else
    {
        if (node->left)
        {
            generateHuffmanCodes1(node->left, prefix + "0", huffmanCodeArr);
        }
        if (node->right)
        {
            generateHuffmanCodes1(node->right, prefix + "1", huffmanCodeArr);
        }
    }
}


quint64 DCT::encoding(QByteArray &compressedData, quint8 &method,unsigned short int &width,unsigned short int &height,QImage &inputimage,QImage &outputimage)
{
    QBuffer buffer(&compressedData);
    buffer.open(QIODevice::WriteOnly);
    QDataStream out(&buffer);
    method=4;

    std::vector<std::vector<double>> bitY(height, std::vector<double>(width));
    std::vector<std::vector<double>> bitCr(height, std::vector<double>(width));
    std::vector<std::vector<double>> bitCb(height, std::vector<double>(width));

    // заполнение битовой матрицы
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            QColor color = inputimage.pixelColor(x, y);
            int r = color.red();
            int g = color.green();
            int b = color.blue();

            // Преобразование RGB в YCbCr //вычитаю в конце 128 так как перевожу из диапазона 0-255 в диапазон -128-127
            double yVal = (0.299 * r + 0.587 * g + 0.114 * b)-128;
            double cbVal = (128 - 0.168736 * r - 0.331264 * g + 0.5 * b)-128;
            double crVal = (128 + 0.5 * r - 0.418688 * g - 0.081312 * b)-128;

            bitY[y][x]=yVal;
            bitCr[y][x]=crVal;
            bitCb[y][x]=cbVal;


        }
    }

    //цветовая субдискретизация 4 2 0
    color_subsampling(width,height,bitCr,bitCb);

    std::vector<std::vector<int>> dct_bitY(height, std::vector<int>(width,0));
    dct_y(bitY,dct_bitY,width,height);

    out<<method<<width<<height;

    // for (int y=0;y<height;++y)
    // {
    //     for (int x=0;x<width;++x)
    //     {
    //         out<<static_cast<signed char>(dct_bitY[y][x]);
    //     }
    // }

    // for (int y=0;y<height;y+=2)
    // {
    //     for (int x=0;x<width;x+=2)
    //     {
    //         out<<static_cast<quint8>(std::round(bitCr[y][x]))<<static_cast<quint8>(std::round(bitCb[y][x]));
    //     }
    // }

    std::vector<unsigned char> freq(256,0);
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            unsigned char val_y=dct_bitY[y][x]+128;
            unsigned char val_cb=bitCb[y][x]+128;
            unsigned char val_cr=bitCr[y][x]+128;
            freq[val_y]++;
            freq[val_cb]++;
            freq[val_cr]++;

        }
    }

    //создаю бинарное дерево и коды к нему
    std::shared_ptr<HuffmanNode> HuffmanTree = buildHuffmanTree1(freq);
    std::vector<std::string> huffmanCodeArr(256,"");
    generateHuffmanCodes1(HuffmanTree, "", huffmanCodeArr);
    //создаю строчку из битов
    std::string binar{""};
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            unsigned char val_y=dct_bitY[y][x]+128;
            unsigned char val_cb=bitCb[y][x]+128;
            unsigned char val_cr=bitCr[y][x]+128;
            const std::string& redCode = huffmanCodeArr[val_y];
            const std::string& greenCode = huffmanCodeArr[val_cb];
            const std::string& blueCode = huffmanCodeArr[val_cr];

            binar+=redCode+greenCode+blueCode;
        }
    }


    int module =binar.size()%8;
    out<<module;
    //сохраняю кол во повторений для создани кодов при расжатии
    for(int i=0;i <freq.size();i++)
    {
        int value=freq[i];
        out<<value;
    }

    //записываю байты
    int length = binar.size();
    for (int i = 0; i < length; i += 8) {
        std::string byteString = binar.substr(i, 8);

        // Если строка меньше 8 символов, дополняем её нулями
        if (byteString.size() < 8)
        {
            byteString.append(8 - byteString.size(), '0');
        }

        std::bitset<8> bits(byteString);
        quint8 byte = static_cast<quint8>(bits.to_ulong());

        out << byte;
    }



    outputimage=QImage(width,height,QImage::Format_RGB888);
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            outputimage.setPixel(x,y,qRgb(bitY[y][x]+128,bitY[y][x]+128,bitY[y][x]+128));
        }
    }


    buffer.close();
    return buffer.size();
}




void DCT::decoding(QString &filename,QByteArray &imageData,quint8 &method,unsigned short int &width,unsigned short int &height,QImage &inputimage)
{

    std::vector<std::vector<double>> cosTable(8, std::vector<double>(8));
    for (int i = 0; i < 8; ++i)
    {
        for (int j = 0; j < 8; ++j)
        {
            cosTable[i][j] = std::cos((2 * i + 1) * j * 3.14159265358979323846 / (2 * 8));
        }
    }

    double quant_Y[8][8] = {
        {16, 11, 10, 16,  24,  40,  51,  61},
        {12, 12, 14, 19,  26,  58,  60,  55},
        {14, 13, 16, 24,  40,  57,  69,  56},
        {14, 17, 22, 29,  51,  87,  80,  62},
        {18, 22, 37, 56,  68, 109, 103,  77},
        {24, 35, 55, 64,  81, 104, 113,  92},
        {49, 64, 78, 87, 103, 121, 120, 101},
        {72, 92, 95, 98, 112, 100, 103,  99}
    };

    std::vector<std::vector<double>> bitY(height, std::vector<double>(width,0.0));
    std::vector<std::vector<double>> bitCr(height, std::vector<double>(width));
    std::vector<std::vector<double>> bitCb(height, std::vector<double>(width));


    int n=8;

    QFile file(filename);
    file.open(QIODevice::ReadOnly);

    imageData = file.readAll();
    file.close();

    QBuffer buffer(&imageData);
    buffer.open(QIODevice::ReadOnly);
    QDataStream in(&buffer);



    in >> method >> width >> height;

    for (int y=0;y<height;y+=2)
    {
        for (int x=0;x<width;x+=2)
        {
            quint8 comp_cr;
            quint8 comp_cb;
            in>>comp_cr>>comp_cb;
            bitCr[y][x]=comp_cr;
            bitCr[y+1][x]=comp_cr;
            bitCr[y][x+1]=comp_cr;
            bitCr[y+1][x+1]=comp_cr;
            bitCb[y][x]=comp_cb;
            bitCb[y+1][x]=comp_cb;
            bitCb[y][x+1]=comp_cb;
            bitCb[y+1][x+1]=comp_cb;
        }
    }

    std::vector<std::vector<int>> dct_bitY(height, std::vector<int>(width,0));
    for (int y=0;y<height;++y)
    {
        for (int x=0;x<width;++x)
        {
            signed char ch=0;
            in>>ch;
            dct_bitY[y][x]=static_cast<int>(ch);
        }
    }


    for (int i = 0; i < height; i += n) {
        for (int j = 0; j < width; j += n) {
            std::vector<std::vector<double>> dctblockY(8, std::vector<double>(8, 0.0));

            for (int x1= 0; x1 < n; ++x1)
            {
                for (int y1 = 0; y1 < n; ++y1)
                {
                    dctblockY[x1][y1] = dct_bitY[i + x1][j + y1]*quant_Y[x1][y1];
                }
            }

            //применение обратного дискретного косинусного преобразования
            std::vector<std::vector<double>> blockY(8, std::vector<double>(8, 0.0));
            double alpha_u=0;
            double alpha_v=0;
            double sum=0;
            for (int x = 0; x < n; ++x)
            {
                for (int y = 0; y < n; ++y)
                {
                    sum = 0.0;
                    for (int u = 0; u < n; ++u) {
                        for (int v = 0; v < n; ++v) {
                            alpha_u = (u == 0) ? 1.0 / std::sqrt(2.0) : 1.0;
                            alpha_v = (v == 0) ? 1.0 / std::sqrt(2.0) : 1.0;
                            sum += alpha_u * alpha_v * dctblockY[u][v] * cosTable[x][u] * cosTable[y][v];
                        }
                    }
                    blockY[x][y] = 0.25 * sum;
                }
            }


            for(int ii=0;ii<n;++ii)
            {
                for (int jj=0;jj<n;++jj)
                {
                    bitY[i+ii][j+jj]=blockY[ii][jj];
                }
            }


        }
    }


    inputimage=QImage(width, height, QImage::Format_RGB888);
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            inputimage.setPixel(x,y,qRgb(bitY[y][x]+128,bitY[y][x]+128,bitY[y][x]+128));
        }
    }


}
*/
//пробую цветовую субдискретизацию 4 2 0 которая берет среднее значение пикселей
void color_subsampling(unsigned short int &width,unsigned short int &height,std::vector<std::vector<double>> &bitCr,std::vector<std::vector<double>> &bitCb)
{
    //точно ли обрабатывает без двойниковых контейнеров и не выходит ли за границы?
    for (int y=0;y<height;y+=2)
    {
        for (int x=0;x<width;x+=2)
        {
            double avgCr=(bitCr[y][x]+bitCr[y+1][x]+bitCr[y][x+1]+bitCr[y+1][x+1])/4.0;
            double avgCb=(bitCb[y][x]+bitCb[y+1][x]+bitCb[y][x+1]+bitCb[y+1][x+1])/4.0;

            bitCr[y][x]=avgCr;
            bitCr[y+1][x]=avgCr;
            bitCr[y][x+1]=avgCr;
            bitCr[y+1][x+1]=avgCr;
            bitCb[y][x]=avgCb;
            bitCb[y+1][x]=avgCb;
            bitCb[y][x+1]=avgCb;
            bitCb[y+1][x+1]=avgCb;
        }
    }
}




void dct_y(std::vector<std::vector<double>> &bitY,std::vector<std::vector<int>> &dct_bitY,unsigned short int &width,unsigned short int &height)
{
    //делаю вектор содержащий уже вычисленные значения необходимых косинусных функций
    std::vector<std::vector<double>> cosTable(8, std::vector<double>(8));
    for (int i = 0; i < 8; ++i)
    {
        for (int j = 0; j < 8; ++j)
        {
            cosTable[i][j] = std::cos((2 * i + 1) * j * 3.14159265358979323846 / (2 * 8));
        }
    }

    //таблица квантования для яркости
    double quant_Y[8][8] = {
        {16, 11, 10, 16,  24,  40,  51,  61},
        {12, 12, 14, 19,  26,  58,  60,  55},
        {14, 13, 16, 24,  40,  57,  69,  56},
        {14, 17, 22, 29,  51,  87,  80,  62},
        {18, 22, 37, 56,  68, 109, 103,  77},
        {24, 35, 55, 64,  81, 104, 113,  92},
        {49, 64, 78, 87, 103, 121, 120, 101},
        {72, 92, 95, 98, 112, 100, 103,  99}
    };

    int n=8;


    // извлечение всех блоков 8 на 8 пикселей
    for (int i = 0; i < height; i += n) {
        for (int j = 0; j < width; j += n) {
            // инициализирую пустой блок 8x8 и заполняю его значениями из основной матрицы bitY
            std::vector<std::vector<double>> blockY(8, std::vector<double>(8, 0.0));

            for (int x1= 0; x1 < n; ++x1)
            {
                for (int y1 = 0; y1 < n; ++y1)
                {
                    blockY[x1][y1] = bitY[i + x1][j + y1];
                }
            }

            // блок 8 на 8 который будет хранить хранить значения блока после dct
            std::vector<std::vector<double>> dctBlockY(8, std::vector<double>(8, 0.0));

            double alpha_u=0;
            double alpha_v=0;
            double sum=0;
            //вычисление dct
            for (int u = 0; u < n; ++u)
            {
                for (int v = 0; v < n; ++v)
                {
                    sum = 0.0;
                    for (int x = 0; x < n; ++x)
                    {
                        for (int y = 0; y < n; ++y)
                        {
                            sum += blockY[x][y] * cosTable[x][u] * cosTable[y][v];
                        }
                    }
                    double alpha_u = (u == 0) ? 1.0 / std::sqrt(2.0) : 1.0;
                    double alpha_v = (v == 0) ? 1.0 / std::sqrt(2.0) : 1.0;
                    dctBlockY[u][v] = 0.25 * alpha_u * alpha_v * sum;
                }
            }


            for(int ii=0;ii<n;++ii)
            {
                for (int jj=0;jj<n;++jj)
                {
                    dct_bitY[i+ii][j+jj]=static_cast<int>(std::round(dctBlockY[ii][jj]/quant_Y[ii][jj]));
                }
            }


        }
    }

}



quint64 DCT::encoding(QByteArray &compressedData, quint8 &method,unsigned short int &width,unsigned short int &height,QImage &inputimage,QImage &outputimage)
{
    QBuffer buffer(&compressedData);
    buffer.open(QIODevice::WriteOnly);
    QDataStream out(&buffer);
    method=4;

    std::vector<std::vector<double>> bitY(height, std::vector<double>(width));
    std::vector<std::vector<double>> bitCr(height, std::vector<double>(width));
    std::vector<std::vector<double>> bitCb(height, std::vector<double>(width));

    // заполнение битовой матрицы
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            QColor color = inputimage.pixelColor(x, y);
            int r = color.red();
            int g = color.green();
            int b = color.blue();

            // Преобразование RGB в YCbCr //вычитаю в конце 128 так как перевожу из диапазона 0-255 в диапазон -128-127
            double yVal = (0.299 * r + 0.587 * g + 0.114 * b)-128;
            double cbVal = (128 - 0.168736 * r - 0.331264 * g + 0.5 * b)-128;
            double crVal = (128 + 0.5 * r - 0.418688 * g - 0.081312 * b)-128;

            bitY[y][x]=yVal;
            bitCr[y][x]=crVal;
            bitCb[y][x]=cbVal;


        }
    }

    //цветовая субдискретизация 4 2 0
    color_subsampling(width,height,bitCr,bitCb);

    std::vector<std::vector<int>> dct_bitY(height, std::vector<int>(width,0));
    dct_y(bitY,dct_bitY,width,height);

    out<<method<<width<<height;

    // for (int y=0;y<height;++y)
    // {
    //     for (int x=0;x<width;++x)
    //     {
    //         out<<static_cast<signed char>(dct_bitY[y][x]);
    //     }
    // }

    for (int y=0;y<height;y+=2)
    {
        for (int x=0;x<width;x+=2)
        {
            out<<static_cast<quint8>(std::round(bitCr[y][x]))<<static_cast<quint8>(std::round(bitCb[y][x]));
        }
    }

    quint16 counter = 1;

    int prevval = dct_bitY[0][0];
    for (int y = 0; y < height; ++y)
    {
        int startX = (y == 0) ? 1 : 0; // начальное значение для x
        for (int x = startX; x < width; ++x)
        {
            int currentval=dct_bitY[y][x];

            if (currentval == prevval && counter < 65535)
            {
                ++counter;
            }
            else
            {
                // записываю текущий пиксель и его количество повторений
                out << counter;
                out<<static_cast<signed char>(prevval);

                // сбрасываю счетчик и обновляю текущий пиксель
                counter = 1;
                prevval = currentval;

            }
        }
    }

    // записываю последний пиксель и его количество повторений
    if (counter > 0)
    {
        out << counter;
        out<<static_cast<signed char>(prevval);
    }


    outputimage=QImage(width,height,QImage::Format_RGB888);
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            outputimage.setPixel(x,y,qRgb(bitCb[y][x]+128,bitCb[y][x]+128,bitCb[y][x]+128));
        }
    }


    buffer.close();
    return buffer.size();
}




void DCT::decoding(QString &filename,QByteArray &imageData,quint8 &method,unsigned short int &width,unsigned short int &height,QImage &inputimage)
{

    std::vector<std::vector<double>> cosTable(8, std::vector<double>(8));
    for (int i = 0; i < 8; ++i)
    {
        for (int j = 0; j < 8; ++j)
        {
            cosTable[i][j] = std::cos((2 * i + 1) * j * 3.14159265358979323846 / (2 * 8));
        }
    }

    double quant_Y[8][8] = {
        {16, 11, 10, 16,  24,  40,  51,  61},
        {12, 12, 14, 19,  26,  58,  60,  55},
        {14, 13, 16, 24,  40,  57,  69,  56},
        {14, 17, 22, 29,  51,  87,  80,  62},
        {18, 22, 37, 56,  68, 109, 103,  77},
        {24, 35, 55, 64,  81, 104, 113,  92},
        {49, 64, 78, 87, 103, 121, 120, 101},
        {72, 92, 95, 98, 112, 100, 103,  99}
    };

    std::vector<std::vector<double>> bitY(height, std::vector<double>(width,0.0));
    std::vector<std::vector<double>> bitCr(height, std::vector<double>(width));
    std::vector<std::vector<double>> bitCb(height, std::vector<double>(width));


    int n=8;

    QFile file(filename);
    file.open(QIODevice::ReadOnly);

    imageData = file.readAll();
    file.close();

    QBuffer buffer(&imageData);
    buffer.open(QIODevice::ReadOnly);
    QDataStream in(&buffer);



    in >> method >> width >> height;

    for (int y=0;y<height;y+=2)
    {
        for (int x=0;x<width;x+=2)
        {
            quint8 comp_cr;
            quint8 comp_cb;
            in>>comp_cr>>comp_cb;
            bitCr[y][x]=comp_cr;
            bitCr[y+1][x]=comp_cr;
            bitCr[y][x+1]=comp_cr;
            bitCr[y+1][x+1]=comp_cr;
            bitCb[y][x]=comp_cb;
            bitCb[y+1][x]=comp_cb;
            bitCb[y][x+1]=comp_cb;
            bitCb[y+1][x+1]=comp_cb;
        }
    }


    std::vector<std::vector<int>> dct_bitY(height, std::vector<int>(width,0));
    // for (int y=0;y<height;++y)
    // {
    //     for (int x=0;x<width;++x)
    //     {
    //         signed char ch=0;
    //         in>>ch;
    //         dct_bitY[y][x]=static_cast<int>(ch);
    //     }
    // }


    int x = 0, y = 0;

    while (!in.atEnd())
    {
        quint16 counter;
        signed char val;

        // количество повторений и цвет пикселя
        in >> counter >> val;

        for (int i = 0; i < counter; ++i)
        {
            if (x >= width)
            {
                x = 0;
                ++y;
            }
            dct_bitY[y][x]=val;
            ++x;
        }
    }



    for (int i = 0; i < height; i += n) {
        for (int j = 0; j < width; j += n) {
            std::vector<std::vector<double>> dctblockY(8, std::vector<double>(8, 0.0));

            for (int x1= 0; x1 < n; ++x1)
            {
                for (int y1 = 0; y1 < n; ++y1)
                {
                    dctblockY[x1][y1] = dct_bitY[i + x1][j + y1]*quant_Y[x1][y1];
                }
            }

            //применение обратного дискретного косинусного преобразования
            std::vector<std::vector<double>> blockY(8, std::vector<double>(8, 0.0));
            double alpha_u=0;
            double alpha_v=0;
            double sum=0;
            for (int x = 0; x < n; ++x)
            {
                for (int y = 0; y < n; ++y)
                {
                    sum = 0.0;
                    for (int u = 0; u < n; ++u) {
                        for (int v = 0; v < n; ++v) {
                            alpha_u = (u == 0) ? 1.0 / std::sqrt(2.0) : 1.0;
                            alpha_v = (v == 0) ? 1.0 / std::sqrt(2.0) : 1.0;
                            sum += alpha_u * alpha_v * dctblockY[u][v] * cosTable[x][u] * cosTable[y][v];
                        }
                    }
                    blockY[x][y] = 0.25 * sum;
                }
            }


            for(int ii=0;ii<n;++ii)
            {
                for (int jj=0;jj<n;++jj)
                {
                    bitY[i+ii][j+jj]=blockY[ii][jj];
                }
            }


        }
    }


    inputimage=QImage(width, height, QImage::Format_RGB888);
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            inputimage.setPixel(x,y,qRgb(bitCb[y][x]+128,bitCb[y][x]+128,bitCb[y][x]+128));
        }
    }

    // for (int y = 0; y < height; ++y)
    // {
    //     for (int x = 0; x < width; ++x) {
    //         int y_value = bitY[y][x]+128;
    //         int cr_value = bitCr[y][x]+128;
    //         int cb_value = bitCb[y][x]+128;

    //         int r = static_cast<int>(y_value + 1.402 * (cr_value - 128));
    //         int g = static_cast<int>(y_value - 0.34414 * (cb_value - 128) - 0.71414 * (cr_value - 128));
    //         int b = static_cast<int>(y_value + 1.772 * (cb_value - 128));

    //         // Ограничение значений до 0-255
    //         r = qBound(0, r, 255);
    //         g = qBound(0, g, 255);
    //         b = qBound(0, b, 255);

    //         inputimage.setPixel(x, y, qRgb(r, g, b));
    //     }
    // }

}
