#include "huffman.h"

quint64 Huffman::encoding(QByteArray &compressedData,quint8 &method,unsigned short int &width,unsigned short int &height,QImage &inputimage)
{
    QBuffer buffer(&compressedData);
    buffer.open(QIODevice::WriteOnly);
    QDataStream out(&buffer);
    method=2;

    //создаю и заполняю вектор частот
    std::vector<int> freq(256,0);

    for (int y = 0; y < height; ++y) {
        const QRgb* row = reinterpret_cast<const QRgb*>(inputimage.scanLine(y));
        for (int x = 0; x < width; ++x) {
            quint8 red = qRed(row[x]);
            quint8 green = qGreen(row[x]);
            quint8 blue = qBlue(row[x]);
            freq[red]++;
            freq[green]++;
            freq[blue]++;

        }
    }

    //создаю бинарное дерево и коды к нему
    std::shared_ptr<HuffmanNode> HuffmanTree = buildHuffmanTree(freq);
    std::vector<std::string> huffmanCodeArr(256,"");
    generateHuffmanCodes(HuffmanTree, "", huffmanCodeArr);

    //создаю строчку из битов
    std::string binar{""};
    for (int y = 0; y < height; ++y) {
        const QRgb* row = reinterpret_cast<const QRgb*>(inputimage.scanLine(y));
        for (int x = 0; x < width; ++x) {
            quint8 red = qRed(row[x]);
            quint8 green = qGreen(row[x]);
            quint8 blue = qBlue(row[x]);
            const std::string& redCode = huffmanCodeArr[red];
            const std::string& greenCode = huffmanCodeArr[green];
            const std::string& blueCode = huffmanCodeArr[blue];

            binar+=redCode+greenCode+blueCode;
        }
    }


    out << method << width << height;
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
        if (byteString.size() < 8) {
            byteString.append(8 - byteString.size(), '0');
        }

        std::bitset<8> bits(byteString);
        quint8 byte = static_cast<quint8>(bits.to_ulong());

        out << byte;
    }


    buffer.close();
    return buffer.size();

}




std::shared_ptr<HuffmanNode> Huffman::buildHuffmanTree(const std::vector<int> freq)
{
    std::priority_queue<std::shared_ptr<HuffmanNode>, std::vector<std::shared_ptr<HuffmanNode>>, Compare> root;

    for (int i = 0; i < freq.size(); ++i) {
        if (freq[i] > 0) {
            root.push(std::make_shared<HuffmanNode>(i, freq[i]));
        }
    }

    while (root.size() > 1) {
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


void Huffman::generateHuffmanCodes(const std::shared_ptr<HuffmanNode>& node, const std::string& prefix, std::vector<std::string>& huffmanCodeArr) {
    if (node->left == nullptr && node->right == nullptr)
    {
        huffmanCodeArr[node->value] = prefix;
    }
    else
    {
        if (node->left)
        {
            generateHuffmanCodes(node->left, prefix + "0", huffmanCodeArr);
        }
        if (node->right)
        {
            generateHuffmanCodes(node->right, prefix + "1", huffmanCodeArr);
        }
    }
}



void Huffman::decoding(QString &filename,QByteArray &imageData,quint8 &method,unsigned short int &width,unsigned short int &height,QImage &inputimage)
{

    QFile file(filename);
    file.open(QIODevice::ReadOnly);

    imageData = file.readAll();
    file.close();

    QBuffer buffer(&imageData);
    buffer.open(QIODevice::ReadOnly);
    QDataStream in(&buffer);

    in >> method >> width >> height;
    int module=0;
    in>>module;
    std::vector<int> freq(256,0);

    for(int i=0;i<freq.size();i++)//вытаскиваю повторения для создания кодов
    {
        int count=0;
        in>>count;
        freq[i]=count;
    }

    std::shared_ptr<HuffmanNode> HuffmanTree = buildHuffmanTree(freq);
    std::vector<std::string> huffmanCodeArr(256,"");
    generateHuffmanCodes(HuffmanTree, "", huffmanCodeArr);

    std::string binar{""};//создание битовой строки
    while (!in.atEnd())
    {
        quint8 byte;
        in>>byte;
        std::bitset<8> b(byte);
        binar+=b.to_string();
    }

    if (module != 0) {
        binar = binar.substr(0, binar.size() - (8 - module));
    }

    //расшифровываю битовую строку и сразу воссоздаю изображение
    inputimage=QImage(width, height, QImage::Format_RGB888);
    int colorcounter=0;
    int x=0;
    int y=0;
    quint8 red=0;
    quint8 green=0;
    quint8 blue=0;
    size_t bitIndex = 0;
    std::shared_ptr<HuffmanNode> currentNode;
    currentNode = HuffmanTree;
    while (bitIndex < binar.size())
    {
        while (currentNode->left != nullptr && currentNode->right != nullptr)
        {
            currentNode = binar[bitIndex++] == '1' ? currentNode->right : currentNode->left;
        }

        colorcounter+=1;

        if(colorcounter%3==1)
        {
            red=currentNode->value;
        }
        else if(colorcounter%3==2)
        {
            green=currentNode->value;
        }
        else if(colorcounter%3==0)
        {

            blue=currentNode->value;
            inputimage.setPixel(x, y, qRgb(red, green, blue));
            ++x;
            if (x >= width)
            {
                x = 0;
                ++y;
            }
        }
        currentNode = HuffmanTree;
    }

}
