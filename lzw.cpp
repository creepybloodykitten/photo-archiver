#include "lzw.h"


quint64 LZW::encoding(QByteArray &imageData,QByteArray &compressedData,QImage &inputimage,quint8 &method,unsigned short int &width,unsigned short int &height)
{
    method = 3;

    imageData.reserve(width * height * 3);

    // считывание пикселей изображения в imageData
    for (int y = 0; y < height; ++y) {
        const QRgb* row = reinterpret_cast<const QRgb*>(inputimage.scanLine(y));
        for (int x = 0; x < width; ++x) {
            imageData.append(qRed(row[x]));
            imageData.append(qGreen(row[x]));
            imageData.append(qBlue(row[x]));
        }
    }


    std::unordered_map<QByteArray, int> dictionary;
    int dictSize = 256;
    for (int i = 0; i < 256; ++i)
        dictionary[QByteArray(1, i)] = i;

    QByteArray w;
    std::vector<int> compressed;
    compressed.reserve(imageData.size());

    // Алгоритм сжатия
    for (int i = 0; i < imageData.size(); ++i)
    {
        QByteArray wc = w + imageData[i];
        if (dictionary.find(wc) != dictionary.end()) {
            w = wc;
        } else {
            compressed.push_back(dictionary[w]);
            dictionary[wc] = dictSize++;
            w = QByteArray(1, imageData[i]);
        }
    }

    // добавление последнего элемента в compressed
    if (!w.isEmpty())
        compressed.push_back(dictionary[w]);


    QBuffer buffer(&compressedData);
    buffer.open(QIODevice::WriteOnly);
    QDataStream out(&buffer);
    out << method << width << height;

    //00 - конец
    //01 - 8 бит (0-255)
    //10 - 16 бит (0-65_535)
    //11 - 24 бит (0-16_777_215)
    quint8 data = 0;
    int pos = 6;

    std::vector<quint8> info_bytes;
    info_bytes.reserve((compressed.size()) / 4);

    for (int i = 0; i < compressed.size(); i++)
    {
        int code = compressed[i];

        if (i == compressed.size() - 1)
        {
            data = data | (0b00 << pos);
            info_bytes.push_back(data);
            break;
        }

        else if (code <= 255)
        {
            data = data | (0b01 << pos);
        }

        else if (code <= 65535)
        {
            data = data | (0b10 << pos);
        }

        else if (code <= 16777215)
        {
            data = data | (0b11 << pos);
        }

        pos -= 2;
        if (pos < 0) {
            pos = 6;
            info_bytes.push_back(data);
            data = 0;
        }
    }

    // запись данных в поток
    size_t info_byte_index = 0;
    for (int i = 0; i < compressed.size(); ++i)
    {
        if (i % 4 == 0 && info_byte_index < info_bytes.size())
        {
            out << info_bytes[info_byte_index++];
        }

        quint32 code = static_cast<quint32>(compressed[i]);

        if (code <= 255)
        {
            out << static_cast<quint8>(code);
        }
        else if (code <= 65535)
        {
            out << static_cast<quint8>((code >> 8) & 0xFF) << static_cast<quint8>(code & 0xFF);
        }
        else
        {
            out << static_cast<quint8>((code >> 16) & 0xFF)<< static_cast<quint8>((code >> 8) & 0xFF)<< static_cast<quint8>(code & 0xFF);
        }
    }

    buffer.close();
    return buffer.size();
}

void LZW::decoding(QString &filename,QByteArray &imageData,QImage &inputimage,quint8 &method,unsigned short int &width,unsigned short int &height)
{

    QFile file(filename);
    file.open(QIODevice::ReadOnly);
    QDataStream in(&file);

    in >> method >> width >> height;

    std::unordered_map<int, QByteArray> dictionary;
    int dictSize = 256;
    for (int i = 0; i < 256; i++)
    {
        dictionary[i] = QByteArray(1, i);
    }

    QByteArray w;
    QByteArray entry;


    quint8 info_byte = 0;
    int pos = 6;

    // чтение информации о длинах и кодов
    std::vector<int> codes;
    while (!in.atEnd())
    {
        // читаю инфо-байт каждые 4 кода
        if (codes.size() % 4 == 0)
        {
            in >> info_byte;
            pos = 6;
        }

        // извлекаю длину кода (2 бита на каждый код)
        quint8 code_length = (info_byte >> pos) & 0b11;
        pos -= 2;

        // читаю код в зависимости от его длины
        quint32 code = 0;
        if (code_length == 0b00)// Последний код
        {
            break;
        }
        else if (code_length == 0b01)// 8-битный код
        {
            quint8 byte1;
            in >> byte1;
            code = byte1;
        }
        else if (code_length == 0b10)// 16-битный код
        {
            quint8 byte1, byte2;
            in >> byte1 >> byte2;
            code = (byte1 << 8) | byte2;
        }
        else if (code_length == 0b11)// 24-битный код
        {
            quint8 byte1, byte2, byte3;
            in >> byte1 >> byte2 >> byte3;
            code = (byte1 << 16) | (byte2 << 8) | byte3;
        }

        codes.push_back(code);
    }

    // декодирование кодов в исходные пиксели

    int prevCode = codes[0];
    imageData.append(dictionary[prevCode]);

    for (int i = 1; i < codes.size(); ++i)
    {
        int code = codes[i];
        if (dictionary.count(code)) {
            entry = dictionary[code];
        } else if (code == dictSize) {
            entry = dictionary[prevCode] + dictionary[prevCode][0];
        } else {
            return ; // ошибка в данных
        }

        imageData.append(entry);

        // добавление нового элемента в словарь
        dictionary[dictSize++] = dictionary[prevCode] + entry[0];
        prevCode = code;
    }


    // восстанавливаем изображение из imageData
    inputimage = QImage(width, height, QImage::Format_RGB32);
    int index = 0;

    for (int y = 0; y < height; ++y) {
        QRgb *row = reinterpret_cast<QRgb *>(inputimage.scanLine(y));
        for (int x = 0; x < width; ++x) {
            if (index + 2 >= imageData.size())
            {
                return;  // недостаточно данных для восстановления изображения
            }
            quint8 red = static_cast<quint8>(imageData[index++]);
            quint8 green = static_cast<quint8>(imageData[index++]);
            quint8 blue = static_cast<quint8>(imageData[index++]);

            row[x] = qRgb(red, green, blue);
        }
    }

    file.close();

}


