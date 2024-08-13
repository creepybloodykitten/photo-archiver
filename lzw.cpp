#include "lzw.h"

/*
quint64 LZW::encoding(QByteArray &imageData,QByteArray &compressedData,QImage &inputimage,quint8 &method,unsigned short int &width,unsigned short int &height)
{

    method=3;

    for (int y = 0; y < height; ++y) {
        const QRgb* row = reinterpret_cast<const QRgb*>(inputimage.scanLine(y));
        for (int x = 0; x < width; ++x) {
            quint8 red = qRed(row[x]);
            quint8 green = qGreen(row[x]);
            quint8 blue = qBlue(row[x]);
            imageData.append(red);
            imageData.append(green);
            imageData.append(blue);
        }
    }


    std::unordered_map<QByteArray, int> dictionary;
    int dictSize = 256;
    for (int i = 0; i < 256; i++)
        dictionary[QByteArray(1, i)] = i;

    QByteArray w;
    std::vector<int> compressed;

    for (int i = 0; i < imageData.size(); ++i) {
        QByteArray wc = w + imageData[i];
        if (dictionary.count(wc))
            w = wc;
        else {
            compressed.push_back(dictionary[w]);
            dictionary[wc] = dictSize++;
            w = QByteArray(1, imageData[i]);
        }
    }

    if (!w.isEmpty())
        compressed.push_back(dictionary[w]);

    QBuffer buffer(&compressedData);
    buffer.open(QIODevice::WriteOnly);
    QDataStream out(&buffer);
    out<<method<<width<<height;
    for (int code : compressed)
        out << static_cast<quint32>(code); // Запись данных как 32-битные целые числа
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
        dictionary[i] = QByteArray(1, static_cast<char>(i));

    QByteArray w;
    QByteArray result;

    quint32 k;
    in >> k;

    w = dictionary[k];
    result += w;

    while (!in.atEnd()) {
        in >> k;
        QByteArray entry;
        if (dictionary.count(k)) {
            entry = dictionary[k];
        } else if (k == dictSize) {
            entry = w + w[0];
        }
        result += entry;

        dictionary[dictSize++] = w + entry[0];
        w = entry;
    }


    file.close();
    inputimage=QImage(width, height, QImage::Format_RGB888);
    int index = 0;

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int r = static_cast<quint8>(result[index++]);
            int g = static_cast<quint8>(result[index++]);
            int b = static_cast<quint8>(result[index++]);
            QColor pixColor(r, g, b);
            inputimage.setPixelColor(x, y, pixColor);
        }
    }
}
*/

/*
quint64 LZW::encoding(QByteArray &imageData, QByteArray &compressedData, QImage &inputimage, quint8 &method, unsigned short int &width, unsigned short int &height) {
    method = 3;

    for (int y = 0; y < height; ++y) {
        const QRgb* row = reinterpret_cast<const QRgb*>(inputimage.scanLine(y));
        for (int x = 0; x < width; ++x) {
            quint8 red = qRed(row[x]);
            quint8 green = qGreen(row[x]);
            quint8 blue = qBlue(row[x]);
            imageData.append(red);
            imageData.append(green);
            imageData.append(blue);
        }
    }

    std::unordered_map<QByteArray, int> dictionary;
    int dictSize = 256;
    for (int i = 0; i < 256; i++)
        dictionary[QByteArray(1, i)] = i;

    QByteArray w;
    std::vector<int> compressed;

    for (int i = 0; i < imageData.size(); ++i) {
        QByteArray wc = w + imageData[i];
        if (dictionary.count(wc))
            w = wc;
        else {
            compressed.push_back(dictionary[w]);
            dictionary[wc] = dictSize++;
            w = QByteArray(1, imageData[i]);
        }
    }

    if (!w.isEmpty())
        compressed.push_back(dictionary[w]);

    QBuffer buffer(&compressedData);
    buffer.open(QIODevice::WriteOnly);
    QDataStream out(&buffer);
    out << method << width << height;

    int bits = 9;
    int max_bits = 12;
    int mask = (1 << bits) - 1;
    int bufferBits = 0;
    quint32 bufferData = 0;

    for (int code : compressed) {
        bufferData = (bufferData << bits) | code;
        bufferBits += bits;

        while (bufferBits >= 8) {
            bufferBits -= 8;
            out << static_cast<quint8>((bufferData >> bufferBits) & 0xFF);
        }

        if (dictSize > mask && bits < max_bits) {
            bits++;
            mask = (1 << bits) - 1;
        }
    }

    if (bufferBits > 0) {
        out << static_cast<quint8>((bufferData << (8 - bufferBits)) & 0xFF);
    }

    buffer.close();

    return buffer.size();
}

void LZW::decoding(QString &filename, QByteArray &imageData, QImage &inputimage, quint8 &method, unsigned short int &width, unsigned short int &height) {
    QFile file(filename);
    file.open(QIODevice::ReadOnly);
    QDataStream in(&file);

    in >> method >> width >> height;

    std::unordered_map<int, QByteArray> dictionary;
    int dictSize = 256;

    for (int i = 0; i < 256; i++)
        dictionary[i] = QByteArray(1, static_cast<char>(i));

    QByteArray w;
    QByteArray result;

    int bits = 9;
    int max_bits = 12;
    int mask = (1 << bits) - 1;
    int bufferBits = 0;
    quint32 bufferData = 0;
    quint32 k;

    while (!in.atEnd()) {
        while (bufferBits < bits && !in.atEnd()) {
            quint8 byte;
            in >> byte;
            bufferData = (bufferData << 8) | byte;
            bufferBits += 8;
        }

        k = (bufferData >> (bufferBits - bits)) & mask;
        bufferBits -= bits;

        QByteArray entry;
        if (dictionary.count(k)) {
            entry = dictionary[k];
        } else if (k == dictSize) {
            entry = w + w[0];
        }

        if (!entry.isEmpty()) {
            result += entry;
            dictionary[dictSize++] = w + entry[0];
            w = entry;
        }

        if (dictSize > mask && bits < max_bits) {
            bits++;
            mask = (1 << bits) - 1;
        }
    }

    file.close();
    inputimage = QImage(width, height, QImage::Format_RGB888);
    int index = 0;

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            if (index + 2 < result.size()) {
                int r = static_cast<quint8>(result[index++]);
                int g = static_cast<quint8>(result[index++]);
                int b = static_cast<quint8>(result[index++]);
                QColor pixColor(r, g, b);
                inputimage.setPixelColor(x, y, pixColor);
            }
        }
    }
}

*/



/*
quint64 LZW::encoding(QByteArray &imageData, QByteArray &compressedData, QImage &inputimage, quint8 &method, unsigned short int &width, unsigned short int &height) {
    method = 3;

    for (int y = 0; y < height; ++y) {
        const QRgb* row = reinterpret_cast<const QRgb*>(inputimage.scanLine(y));
        for (int x = 0; x < width; ++x) {
            quint8 red = qRed(row[x]);
            quint8 green = qGreen(row[x]);
            quint8 blue = qBlue(row[x]);
            imageData.append(red);
            imageData.append(green);
            imageData.append(blue);
        }
    }

    std::unordered_map<QByteArray, int> dictionary;
    int dictSize = 256;
    for (int i = 0; i < 256; i++)
        dictionary[QByteArray(1, i)] = i;

    QByteArray w;
    std::vector<int> compressed;

    for (int i = 0; i < imageData.size(); ++i) {
        QByteArray wc = w + imageData[i];
        if (dictionary.count(wc))
            w = wc;
        else {
            compressed.push_back(dictionary[w]);
            if (dictSize < 65536) // Limit dictionary size
                dictionary[wc] = dictSize++;
            w = QByteArray(1, imageData[i]);
        }
    }

    if (!w.isEmpty())
        compressed.push_back(dictionary[w]);

    QBuffer buffer(&compressedData);
    buffer.open(QIODevice::WriteOnly);
    QDataStream out(&buffer);
    out << method << width << height;
    for (int code : compressed)
        out << static_cast<quint16>(code); // Use 16-bit codes
    buffer.close();

    return buffer.size();
}

void LZW::decoding(QString &filename, QByteArray &imageData, QImage &inputimage, quint8 &method, unsigned short int &width, unsigned short int &height) {
    QFile file(filename);
    file.open(QIODevice::ReadOnly);
    QDataStream in(&file);

    in >> method >> width >> height;

    std::unordered_map<int, QByteArray> dictionary;
    int dictSize = 256;

    for (int i = 0; i < 256; i++)
        dictionary[i] = QByteArray(1, static_cast<char>(i));

    QByteArray w;
    QByteArray result;

    quint16 k;
    in >> k;

    w = dictionary[k];
    result += w;

    while (!in.atEnd()) {
        in >> k;
        QByteArray entry;
        if (dictionary.count(k)) {
            entry = dictionary[k];
        } else if (k == dictSize) {
            entry = w + w[0];
        }
        result += entry;

        if (dictSize < 65536) // Limit dictionary size
            dictionary[dictSize++] = w + entry[0];
        w = entry;
    }

    file.close();
    inputimage = QImage(width, height, QImage::Format_RGB888);
    int index = 0;

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int r = static_cast<quint8>(result[index++]);
            int g = static_cast<quint8>(result[index++]);
            int b = static_cast<quint8>(result[index++]);
            QColor pixColor(r, g, b);
            inputimage.setPixelColor(x, y, pixColor);
        }
    }
}
*/





void adaptiveReset(std::unordered_map<QByteArray, int> &dictionary, std::unordered_map<QByteArray, int> &usageCount, int &dictSize) {
    // Порог использования для удаления редко используемых последовательностей
    const int usageThreshold = 2;

    for (auto it = usageCount.begin(); it != usageCount.end();) {
        if (it->second < usageThreshold) {
            dictionary.erase(it->first); // Удаление из словаря
            it = usageCount.erase(it); // Удаление из счетчика использования
            dictSize--;
        } else {
            ++it;
        }
    }
}


quint64 LZW::encoding(QByteArray &imageData, QByteArray &compressedData, QImage &inputimage, quint8 &method, unsigned short int &width, unsigned short int &height) {
    method = 3;

    imageData.reserve(width * height * 3); // Резервируем память для imageData

    for (int y = 0; y < height; ++y) {
        const QRgb* row = reinterpret_cast<const QRgb*>(inputimage.scanLine(y));
        for (int x = 0; x < width; ++x) {
            quint8 red = qRed(row[x]);
            quint8 green = qGreen(row[x]);
            quint8 blue = qBlue(row[x]);
            imageData.append(red);
            imageData.append(green);
            imageData.append(blue);
        }
    }

    std::unordered_map<QByteArray, int> dictionary;
    std::unordered_map<QByteArray, int> usageCount; // Частота использования
    int dictSize = 256;

    for (int i = 0; i < 256; i++) {
        dictionary[QByteArray(1, i)] = i;
        usageCount[QByteArray(1, i)] = 1;
    }

    QByteArray w;
    std::vector<int> compressed;
    compressed.reserve(imageData.size() / 2); // Резервируем память для сжатых данных

    for (int i = 0; i < imageData.size(); ++i) {
        w.append(imageData[i]); // Избегаем создания временных объектов
        if (dictionary.count(w)) {
            usageCount[w]++; // Увеличение частоты использования
        } else {
            compressed.push_back(dictionary[w.left(w.size() - 1)]);
            if (dictSize < 65536) {
                dictionary[w] = dictSize++;
                usageCount[w] = 1;
            } else {
                adaptiveReset(dictionary, usageCount, dictSize);
                dictionary[w] = dictSize++;
                usageCount[w] = 1;
            }
            w = QByteArray(1, imageData[i]);
        }
    }

    if (!w.isEmpty())
        compressed.push_back(dictionary[w]);

    QBuffer buffer(&compressedData);
    buffer.open(QIODevice::WriteOnly);
    QDataStream out(&buffer);
    out << method << width << height;
    for (int code : compressed)
        out << static_cast<quint16>(code); // Использование 16-битных кодов
    buffer.close();

    return buffer.size();
}



void LZW::decoding(QString &filename, QByteArray &imageData, QImage &inputimage, quint8 &method, unsigned short int &width, unsigned short int &height) {
    QFile file(filename);
    file.open(QIODevice::ReadOnly);
    QDataStream in(&file);

    in >> method >> width >> height;

    std::unordered_map<int, QByteArray> dictionary;
    int dictSize = 256;

    for (int i = 0; i < 256; i++)
        dictionary[i] = QByteArray(1, static_cast<char>(i));

    QByteArray w;
    QByteArray result;

    quint16 k;
    in >> k;

    w = dictionary[k];
    result += w;

    while (!in.atEnd()) {
        in >> k;
        QByteArray entry;
        if (dictionary.count(k)) {
            entry = dictionary[k];
        } else if (k == dictSize) {
            entry = w + w[0];
        }
        result += entry;

        if (dictSize < 65536) {
            dictionary[dictSize++] = w + entry[0];
        } else {
            // Адаптивный сброс словаря также может быть реализован в декодировании,
            // чтобы словарь не переполнялся (по аналогии с кодированием).
        }
        w = entry;
    }

    file.close();
    inputimage = QImage(width, height, QImage::Format_RGB888);
    int index = 0;

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int r = static_cast<quint8>(result[index++]);
            int g = static_cast<quint8>(result[index++]);
            int b = static_cast<quint8>(result[index++]);
            QColor pixColor(r, g, b);
            inputimage.setPixelColor(x, y, pixColor);
        }
    }
}

