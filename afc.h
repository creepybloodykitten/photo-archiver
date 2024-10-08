#ifndef AFC_H
#define AFC_H

#include <QFile>
#include <QImage>
#include <QDebug>
#include <QBuffer>

#include <QPainter>

struct Block
{
    quint16 x;
    quint16 y;
    int height;
    int width;
};

struct Transformed_data
{
    quint16 x;
    quint16 y;
    QImage img;
    quint8 direction;
    quint8 angle;
    quint8 brightness;
    quint8 contrast;
};


class AFC
{
public:
    static quint64 encoding(QByteArray &compressedData,quint8 &method,unsigned short int &width,unsigned short int &height,QImage &inputimage);
    static void decoding(QString &filename,QByteArray &imageData,quint8 &method,unsigned short int &width,unsigned short int &height,QImage &inputimage);
};

#endif // AFC_H
