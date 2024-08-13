#include "afc.h"

/*
void generate_blocks(int domainBlock_size,int rangeBlock_size,QImage &inputimage,std::vector<Block> &domainBlocks,std::vector<Block> &rangeBlocks)
{
    for(int y=0;y<inputimage.height();y+=domainBlock_size)
    {
        for(int x=0;x<inputimage.width();x+=domainBlock_size)
        {
            domainBlocks.push_back(Block{x,y,domainBlock_size,domainBlock_size});
        }
    }

    for(int y=0;y<inputimage.height();y+=rangeBlock_size)
    {
        for(int x=0;x<inputimage.width();x+=rangeBlock_size)
        {
            rangeBlocks.push_back(Block{x,y,rangeBlock_size,rangeBlock_size});
        }
    }

}



double calculateDifference(const Block& rangeBlock, const QImage& transformedRangeBlock, const Block& domainBlock,const QImage& inputimage) {
    double mse = 0.0;
    int n = rangeBlock.width * rangeBlock.height;

    for (int y = 0; y < rangeBlock.height; ++y) {
        for (int x = 0; x < rangeBlock.width; ++x) {
            // Проверка, чтобы координаты оставались в пределах изображения
            if (domainBlock.x + x >= inputimage.width() || domainBlock.y + y >= inputimage.height() ||
                x >= transformedRangeBlock.width() || y >= transformedRangeBlock.height()) {
                continue; // Пропустить пиксели, которые выходят за пределы изображения
            }

            int rangePixel = transformedRangeBlock.pixel(x, y);
            int domainPixel = inputimage.pixel(domainBlock.x + x, domainBlock.y + y);

            mse += (rangePixel - domainPixel) * (rangePixel - domainPixel);
        }
    }


    return mse / n;
}


QImage applyTransformation(const Block& rangeBlock, const Transformation& transformation,const QImage& inputimage) {
    QImage block = inputimage.copy(rangeBlock.x, rangeBlock.y, rangeBlock.width, rangeBlock.height);

    // Масштабирование
    if (transformation.scale != 1.0) {
        block = block.scaled(block.width() * transformation.scale, block.height() * transformation.scale);
    }

    // Поворот
    QTransform transform;
    transform.rotate(transformation.rotation);
    block = block.transformed(transform);

    // Отражение
    if (transformation.flipHorizontal) {
        block = block.mirrored(true, false);
    }
    if (transformation.flipVertical) {
        block = block.mirrored(false, true);
    }

    // Изменение контрастности и яркости
    for (int y = 0; y < block.height(); ++y) {
        for (int x = 0; x < block.width(); ++x) {
            int pixelValue = qGray(block.pixel(x, y));
            int newPixelValue = static_cast<int>(transformation.contrast * pixelValue + transformation.brightness);
            newPixelValue = qBound(0, newPixelValue, 255);
            block.setPixel(x, y, qRgb(newPixelValue, newPixelValue, newPixelValue));
        }
    }

    return block;
}





Transformation findBestMatch(const Block &rangeBlock,std::vector<Block> &domainBlocks,const QImage &inputimage) {
    Transformation bestTransformation;
    double minDifference = std::numeric_limits<double>::max();

    for (const Block& domainBlock : domainBlocks) {
        for (double scale : {1.0, 0.5}) { // Scale down if necessary
            for (double rotation : {0.0, 90.0, 180.0, 270.0}) {
                for (bool flipH : {false, true}) {
                    for (bool flipV : {false, true}) {
                        Transformation transformation = {domainBlock, rangeBlock, scale, rotation, 0.0, 1.0, flipH, flipV};
                        QImage transformedRangeBlock = applyTransformation(rangeBlock, transformation,inputimage);
                        double difference = calculateDifference(rangeBlock, transformedRangeBlock, domainBlock,inputimage);

                        if (difference < minDifference) {
                            minDifference = difference;
                            bestTransformation = transformation;
                        }
                    }
                }
            }
        }
    }

    return bestTransformation;
}



quint64 AFC::encoding(QByteArray &compressedData,quint8 &method,unsigned short int &width,unsigned short int &height,QImage &inputimage)
{
    std::vector<Transformation> transformations;
    std::vector<Block> domainBlocks;
    std::vector<Block> rangeBlocks;
    generate_blocks(8,4,inputimage,domainBlocks,rangeBlocks);
    qInfo()<<rangeBlocks.size();
    for(Block block:rangeBlocks)
    {
        Transformation t = findBestMatch(block,domainBlocks,inputimage);
        transformations.push_back(t);
    }
    return 1;
}
*/


//создание ранговых и домеин блоков
void generate_blocks(int domainBlock_size,int rangeBlock_size,QImage &inputimage,std::vector<Block> &domainBlocks,std::vector<Block> &rangeBlocks)
{
    /*
    for(int y=0;y<inputimage.height();y+=domainBlock_size)
    {
        for(int x=0;x<inputimage.width();x+=domainBlock_size)
        {
            domainBlocks.push_back(Block{x,y,domainBlock_size,domainBlock_size});
        }
    }
    */

    for(int y=0;y<inputimage.height()-8;y+=2)
    {
        for(int x=0;x<inputimage.width()-8;x+=2)
        {
            domainBlocks.push_back(Block{x,y,domainBlock_size,domainBlock_size});
        }
    }

    for(int y=0;y<inputimage.height();y+=rangeBlock_size)
    {
        for(int x=0;x<inputimage.width();x+=rangeBlock_size)
        {
            rangeBlocks.push_back(Block{x,y,rangeBlock_size,rangeBlock_size});
        }
    }

}



//сохраняю изменный блок
QImage apply_transform(QImage &upg,int &direction,int &angle)
{
    QTransform rotate;
    rotate.rotate(angle);

    if (direction==1)//отзеркаливание по горизонатали
    {
        return upg.transformed(rotate).mirrored(true,false);
    }
    else if (direction==2)//отзеркаливание по вертикали
    {
        return upg.transformed(rotate).mirrored(false,true);
    }
    else if (direction==3)//отзеркаливание по вертикали
    {
        return upg.transformed(rotate).mirrored(true,true);
    }
    else//никак не отзеркаливаю
    {
        return upg.transformed(rotate);
    }
}



//создание общего списка измененных домеин блоков
void transform(std::vector<QImage> &all_transformed,std::vector<Block> &domainBlocks,QImage &inputimage)
{

    for(Block block:domainBlocks)
    {
        QImage upg=inputimage.copy(block.x,block.y,block.width,block.height).scaled(4,4, Qt::IgnoreAspectRatio);//потом сравнить с использованием Qt::FastTransformation

        for(int direction:{0,1,2,3})
        {
            for(int rotation : {0, 90, 180, 270})
            {
                QImage transformed=apply_transform(upg,direction,rotation);
                all_transformed.push_back(transformed);
            }
        }
    }

}

//среднеквадратичное отклонение которое считает разницу между соотвествующими позициями значений пикселей
double mse(QImage &rangeBlock,QImage &transformed)
{
    double mse=0;
    int pix_count=rangeBlock.width()*rangeBlock.height();
    for(int y = 0; y < rangeBlock.height(); ++y)
    {
        for(int x = 0; x < rangeBlock.width(); ++x)
        {
            int rPixel=qRed(rangeBlock.pixel(x,y));
            int tPixel=qRed(transformed.pixel(x,y));

            mse+=(rPixel-tPixel)*(rPixel-tPixel);
        }
    }
    return mse/pix_count;
}

//создаю вектор содержащий лучший вариант преобразованного домеин блока похожего на ранговый блок
std::vector<QImage> findSimilar(std::vector<Block> &rangeBlocks,std::vector<QImage> &all_transformed,QImage &inputimage)
{
    std::vector<QImage> similar;
    QImage best;
    for(Block rblock:rangeBlocks)
    {

        QImage rangepixture=inputimage.copy(rblock.x,rblock.y,rblock.width,rblock.height);
        double minDifference = std::numeric_limits<double>::max();
        for(QImage transblock:all_transformed)
        {
            double d=mse(rangepixture,transblock);
            if (d<minDifference)
            {
                minDifference=d;
                best=transblock;
            }
        }
        similar.push_back(best);
    }

    return similar;
}


quint64 AFC::encoding(QByteArray &compressedData,quint8 &method,unsigned short int &width,unsigned short int &height,QImage &inputimage)
{

    std::vector<Block> domainBlocks;
    std::vector<Block> rangeBlocks;

    generate_blocks(8,4,inputimage,domainBlocks,rangeBlocks);

    std::vector<QImage> all_transformed;
    transform(all_transformed,domainBlocks,inputimage);

    qInfo()<<all_transformed.size();
    //индекс рангового блока совпадает с индексом самого подходящего для него преобразованного домеин блока
    std::vector<QImage> bestmatches = findSimilar(rangeBlocks,all_transformed,inputimage);

    return 1;
}
