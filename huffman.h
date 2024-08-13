#ifndef HUFFMAN_H
#define HUFFMAN_H

#include <QDataStream>
#include <QBuffer>
#include <QColor>
#include <QImage>
#include <QFile>
#include <memory>
#include <queue>
#include <string>
#include<bitset>
#include <QDebug>


struct HuffmanNode {
    quint8 value;
    int frequency;
    std::shared_ptr<HuffmanNode> left;
    std::shared_ptr<HuffmanNode> right;

    HuffmanNode(quint8 val, int freq) : value(val), frequency(freq), left(nullptr), right(nullptr) {}
};


struct Compare {
    bool operator()(std::shared_ptr<HuffmanNode> const& a, std::shared_ptr<HuffmanNode> const& b) {
        return a->frequency > b->frequency;
    }
};

class Huffman
{
public:
    static quint64 encoding(QByteArray &compressedData,quint8 &method,unsigned short int &width,unsigned short int &height,QImage& inputimage);
    static void decoding(QString &filename,QByteArray &imageData,quint8 &method,unsigned short int &width,unsigned short int &height,QImage &inputimage );
    static std::shared_ptr<HuffmanNode> buildHuffmanTree(const std::vector<int> freq) ;
    static void generateHuffmanCodes(const std::shared_ptr<HuffmanNode>& node, const std::string& prefix, std::vector<std::string>& huffmanCodeArr);
};

#endif // HUFFMAN_H
