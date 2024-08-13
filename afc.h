#ifndef AFC_H
#define AFC_H

#include <QFile>
#include <QImage>
#include <QDebug>

struct Block
{
    int x;
    int y;
    int height;
    int width;
};

struct Transform
{
    QImage img;
    int direction;
    int angle;
    int brightness;
    int contrast;
};
/*
struct Transformation {
    Block domainRect;
    Block rangeRect;
    double scale;
    double rotation;
    double brightness;
    double contrast;
    bool flipHorizontal;
    bool flipVertical;
};
*/

class AFC
{
public:
    static quint64 encoding(QByteArray &compressedData,quint8 &method,unsigned short int &width,unsigned short int &height,QImage &inputimage);
    static void decoding();





};

#endif // AFC_H
