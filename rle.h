#ifndef RLE_H
#define RLE_H

#include <QFile>
#include <QImage>
#include <QBuffer>

class RLE
{
public:
    static quint64 encoding(QByteArray& compressedData,quint8& method,unsigned short int &width,unsigned short int &height,QImage &inputimage);
    static void decoding(QString filename,quint8& method,unsigned short int& width,unsigned short int& height,QImage& inputimage);
};

#endif // RLE_H
