#include "rle.h"

quint64 RLE::encoding(QByteArray &compressedData,quint8 &method,unsigned short int &width,unsigned short int &height,QImage &inputimage)
{
    QBuffer buffer(&compressedData);
    buffer.open(QIODevice::WriteOnly);
    QDataStream out(&buffer);
    method=1;
    out<<method<<width<<height;
    quint16 counter = 1;
    QColor prevPixel = inputimage.pixelColor(0, 0);
    for (int y = 0; y < height; ++y)
    {
        int startX = (y == 0) ? 1 : 0; // начальное значение для x
        for (int x = startX; x < width; ++x)
        {
            QColor currentPixel = inputimage.pixelColor(x, y);

            if (currentPixel == prevPixel && counter < 65535)
            {
                ++counter;
            }
            else
            {
                // записываю текущий пиксель и его количество повторений
                out << counter;
                out << quint8(prevPixel.red());
                out << quint8(prevPixel.green());
                out << quint8(prevPixel.blue());

                // сбрасываю счетчик и обновляю текущий пиксель
                counter = 1;
                prevPixel = currentPixel;
            }
        }
    }

    // записываю последний пиксель и его количество повторений
    if (counter > 0)
    {
        out << counter;
        out << quint8(prevPixel.red());
        out << quint8(prevPixel.green());
        out << quint8(prevPixel.blue());
    }
    buffer.close();
    return buffer.size();

}
void RLE::decoding(QString filename,quint8 &method,unsigned short int &width,unsigned short int &height,QImage &inputimage)
{
    QFile file(filename);
    file.open(QIODevice::ReadOnly);
    QDataStream in(&file);

    in>>method>>width>>height;
    //QImage image(width, height, QImage::Format_RGB888);
    inputimage=QImage(width, height, QImage::Format_RGB888);
    int x = 0, y = 0;

    while (!in.atEnd())
    {
        quint16 counter;
        quint8 red, green, blue;

        // количество повторений и цвет пикселя
        in >> counter >> red >> green >> blue;

        QColor pixColor(red, green, blue);
        for (int i = 0; i < counter; ++i)
        {
            if (x >= width)
            {
                x = 0;
                ++y;
            }
            /*
            сделать обработчик ошибок во всех функция и во всем коде
            if (y >= height)
            {
                qWarning("Decoded data exceeds image dimensions");
                return;
            }
            */
            inputimage.setPixelColor(x, y, pixColor);
            ++x;
        }
    }
    file.close();
}

