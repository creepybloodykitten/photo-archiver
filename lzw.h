#ifndef LZW_H
#define LZW_H

#include <QDataStream>
#include <QBuffer>
#include <QFile>
#include <QDebug>
#include <QImage>
#include <unordered_map>
#include <vector>
#include <bitset>

class LZW
{
public:
    //static quint64 encoding(QByteArray &imageData,QByteArray &compressedData,QImage &inputimage,quint8 &method,unsigned short int &width,unsigned short int &height);
    //static void decoding(QString &filename,QByteArray &imageData,QImage &inputimage,quint8 &method,unsigned short int &width,unsigned short int &height);

    static quint64 encoding(QByteArray &imageData,QByteArray &compressedData,QImage &inputimage,quint8 &method,unsigned short int &width,unsigned short int &height);
    static void decoding(QString &filename,QByteArray &imageData,QImage &inputimage,quint8 &method,unsigned short int &width,unsigned short int &height);
};

#endif // LZW_H
