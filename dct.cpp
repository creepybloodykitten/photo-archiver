#include "dct.h"

//цветовая субдискретизация 4 2 0 которая берет среднее значение пикселей
void color_subsampling(unsigned short int &width,unsigned short int &height,std::vector<std::vector<double>> &bitCr,std::vector<std::vector<double>> &bitCb)
{
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




void dct_y(std::vector<std::vector<double>> &bitY,std::vector<int> &dct_bitY,std::vector<std::vector<int>> &dct_bitY2D,unsigned short int &width,unsigned short int &height)
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
    for (int i = 0; i < height; i += n)
    {
        for (int j = 0; j < width; j += n)
        {
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
                    int newval=static_cast<int>(std::round(dctBlockY[ii][jj]/quant_Y[ii][jj]));
                    dct_bitY2D[i+ii][j+jj]=newval;
                    if ((ii==0 &&jj==0)||newval!=0)
                    {
                        dct_bitY.push_back(ii*8+jj);
                        dct_bitY.push_back(newval);
                    }
                }
            }




        }
    }


    //применяю все в обратном порядке чтобы вывести итоговое изображение для пользователя
    for (int i = 0; i < height; i += n)
    {
        for (int j = 0; j < width; j += n)
        {
            std::vector<std::vector<double>> dctblockY(8, std::vector<double>(8, 0.0));

            for (int x1= 0; x1 < n; ++x1)
            {
                for (int y1 = 0; y1 < n; ++y1)
                {
                    dctblockY[x1][y1] = dct_bitY2D[i + x1][j + y1]*quant_Y[x1][y1];
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
                    blockY[x][y] = 0.25* sum;

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
    std::vector<std::vector<int>> dct_bitY2D(height, std::vector<int>(width,0));
    dct_y(bitY,dct_bitY,dct_bitY2D,width,height);

    out<<method<<width<<height;
    out<<static_cast<int>(dct_bitY.size());

    for (int i=0;i<dct_bitY.size();++i)
    {
        out<<static_cast<signed char>(dct_bitY[i]);
    }


    for (int y=0;y<height;y+=2)
    {
        for (int x=0;x<width;x+=2)
        {
            out<<static_cast<signed char>(std::round(bitCr[y][x]))<<static_cast<signed char>(std::round(bitCb[y][x]));
        }
    }


    outputimage=QImage(width,height,QImage::Format_RGB888);
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            int y_value = bitY[y][x]+128;
            int cr_value = bitCr[y][x]+128;
            int cb_value = bitCb[y][x]+128;

            int r = static_cast<int>(y_value + 1.402 * (cr_value - 128));
            int g = static_cast<int>(y_value - 0.34414 * (cb_value - 128) - 0.71414 * (cr_value - 128));
            int b = static_cast<int>(y_value + 1.772 * (cb_value - 128));

            r = qBound(0, r, 255);
            g = qBound(0, g, 255);
            b = qBound(0, b, 255);

            outputimage.setPixel(x, y, qRgb(r, g, b));
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

    int dct_y_size=0;
    int n=8;

    QFile file(filename);
    file.open(QIODevice::ReadOnly);

    imageData = file.readAll();
    file.close();

    QBuffer buffer(&imageData);
    buffer.open(QIODevice::ReadOnly);
    QDataStream in(&buffer);



    in >> method >> width >> height;
    in>>dct_y_size;
    std::vector<int> dct_Y;


    for (int i=0;i<dct_y_size;++i)
    {
        signed char ch=0;
        in>>ch;
        dct_Y.push_back(static_cast<int>(ch));

    }

    for (int y=0;y<height;y+=2)
    {
        for (int x=0;x<width;x+=2)
        {
            signed char comp_cr;
            signed char comp_cb;
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

    int posx=0;
    int posy=0;
    std::vector<int> temp;
    //восстанавливаю матрицу dctbitY из 1D в 2D
    for(int i=0;i<dct_Y.size();i+=2)
    {
        int pos=dct_Y[i];
        int val=dct_Y[i+1];
        if(pos==0)
        {
            if(temp.empty())
            {
                temp.push_back(pos);
                temp.push_back(val);
            }
            else
            {
                std::vector<std::vector<int>> dctBlockY(8, std::vector<int>(8, 0));
                for(int j=0;j<temp.size();j+=2)
                {
                    int posX=temp[j]/n;//строка
                    int posY=temp[j]%n;//столбец
                    int temp_val=temp[j+1];
                    dctBlockY[posX][posY]=temp_val;
                }

                for(int ii=0;ii<n;++ii)
                {
                    for (int jj=0;jj<n;++jj)
                    {
                        dct_bitY[posy+ii][posx+jj]=dctBlockY[ii][jj];
                    }
                }

                posx+=8;
                if (posx>width-1)
                {
                    posx=0;
                    posy+=8;
                }

                temp.clear();
                temp.push_back(pos);
                temp.push_back(val);
            }
        }
        else if(pos!=0)
        {
            temp.push_back(pos);
            temp.push_back(val);
        }
    }



    for (int i = 0; i < height; i += n)
    {
        for (int j = 0; j < width; j += n)
        {
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
                    blockY[x][y] = 0.25* sum;

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
            int y_value = bitY[y][x]+128;
            int cr_value = bitCr[y][x]+128;
            int cb_value = bitCb[y][x]+128;

            int r = static_cast<int>(y_value + 1.402 * (cr_value - 128));
            int g = static_cast<int>(y_value - 0.34414 * (cb_value - 128) - 0.71414 * (cr_value - 128));
            int b = static_cast<int>(y_value + 1.772 * (cb_value - 128));

            r = qBound(0, r, 255);
            g = qBound(0, g, 255);
            b = qBound(0, b, 255);

            inputimage.setPixel(x, y, qRgb(r, g, b));
        }
    }

}

