#ifndef DCT_H
#define DCT_H

#include <QBuffer>
#include <QDataStream>
#include <QImage>
#include <QDebug>
#include <QFile>
#include <cmath>
#include <memory>
#include <queue>
#include<bitset>
#include <string>
#include <iostream>

#include "huffman.h"




class DCT
{
public:

    double crcb_quant_matrix[8*8]=
        { 17, 18, 24, 47, 99, 99, 99, 99,
         18, 21, 26, 66, 99, 99, 99, 99,
         24, 26, 56, 99, 99, 99, 99, 99,
         47, 66, 99, 99, 99, 99, 99, 99,
         99, 99, 99, 99, 99, 99, 99, 99,
         99, 99, 99, 99, 99, 99, 99, 99,
         99, 99, 99, 99, 99, 99, 99, 99,
         99, 99, 99, 99, 99, 99, 99, 99 };

    static quint64 encoding(QByteArray &compressedData, quint8 &method,unsigned short int &width,unsigned short int &height,QImage &inputimage, QImage &outputimage);
    static void decoding(QString &filename,QByteArray &imageData,quint8 &method,unsigned short int &width,unsigned short int &height,QImage &inputimage);






};

#endif // DCT_H
