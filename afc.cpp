#include "afc.h"


/*
//создание ранговых и домеин блоков
void generate_blocks(int domainBlock_size,int rangeBlock_size,QImage &inputimage,std::vector<Block> &domainBlocks,std::vector<Block> &rangeBlocks)
{

    for(int y=0;y<inputimage.height();y+=domainBlock_size)
    {
        for(int x=0;x<inputimage.width();x+=domainBlock_size)
        {
            domainBlocks.push_back(Block{x,y,domainBlock_size,domainBlock_size});
        }
    }


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


double mse(QImage &rangeBlock,QImage &transformed,double &bestContrast,double &bestBrightness)
{
    double mse=0;
    int pix_count=rangeBlock.width()*rangeBlock.height();

    double sumRange = 0.0;
    double sumDomain = 0.0;
    double sumDomainSquared = 0.0;
    double sumRangeDomain = 0.0;


    // Вычисление сумм для средних значений, ковариации и дисперсии
    for (int y = 0; y < rangeBlock.height(); ++y) {
        for (int x = 0; x < rangeBlock.width(); ++x) {
            int rPixel=qRed(rangeBlock.pixel(x,y));
            int tPixel=qRed(transformed.pixel(x,y));

            sumRange += rPixel;
            sumDomain += tPixel;
            sumDomainSquared += tPixel * tPixel;
            sumRangeDomain += rPixel * tPixel;
        }
    }

    // Средние значения
    double meanRange = sumRange / pix_count;
    double meanDomain = sumDomain / pix_count;

    // Коэффициенты контрастности и яркости
    bestContrast = (sumRangeDomain - pix_count * meanRange * meanDomain) / (sumDomainSquared - pix_count * meanDomain * meanDomain);
    bestBrightness = meanRange - bestContrast * meanDomain;


    // Вычисление MSE с учетом контрастности и яркости
    for (int y = 0; y < rangeBlock.height(); ++y) {
        for (int x = 0; x < rangeBlock.width(); ++x) {
            int rPixel=qRed(rangeBlock.pixel(x,y));
            int tPixel=qRed(transformed.pixel(x,y));

            double adjustedPixel = bestContrast * tPixel + bestBrightness;
            mse += (rPixel - adjustedPixel) * (rPixel - adjustedPixel);
        }
    }
    return mse/pix_count;
}



//создаю вектор содержащий лучший вариант преобразованного домеин блока похожего на ранговый блок

std::vector<QImage> findSimilar(std::vector<Block> &rangeBlocks,std::vector<QImage> &all_transformed,QImage &inputimage)
{
    std::vector<QImage> similar;
    QImage best;
    double bestbr;
    double bestcont;
    for(Block rblock:rangeBlocks)
    {

        QImage rangepixture=inputimage.copy(rblock.x,rblock.y,rblock.width,rblock.height);
        double minDifference = std::numeric_limits<double>::max();
        for(QImage transblock:all_transformed)
        {
            double d=mse(rangepixture,transblock,bestcont,bestbr);
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
*/


//создание ранговых и домеин блоков
void generate_blocks(int domainBlock_size,int rangeBlock_size,QImage &inputimage,std::vector<Block> &domainBlocks,std::vector<Block> &rangeBlocks)
{

    for(quint16 y=0;y<inputimage.height();y+=domainBlock_size)
    {
        for(quint16 x=0;x<inputimage.width();x+=domainBlock_size)
        {
            domainBlocks.push_back(Block{x,y,domainBlock_size,domainBlock_size});
        }
    }

    for(quint16 y=0;y<inputimage.height();y+=rangeBlock_size)
    {
        for(quint16 x=0;x<inputimage.width();x+=rangeBlock_size)
        {
            rangeBlocks.push_back(Block{x,y,rangeBlock_size,rangeBlock_size});
        }
    }

}



//сохраняю изменный блок
Transformed_data apply_transform(quint16 &x,quint16 &y,QImage &upg,quint8 &direction,quint8 &angle)
{
    QTransform rotate;
    rotate.rotate(angle);

    if (direction==1)//отзеркаливание по горизонатали
    {
        return Transformed_data{x,y,upg.transformed(rotate).mirrored(true,false),direction,angle,0,1};
    }
    else if (direction==2)//отзеркаливание по вертикали
    {
        return Transformed_data{x,y,upg.transformed(rotate).mirrored(false,true),direction,angle,0,1};
    }
    else if (direction==3)//отзеркаливание по вертикали
    {
        return Transformed_data{x,y,upg.transformed(rotate).mirrored(true,true),direction,angle,0,1};
    }
    else//никак не отзеркаливаю
    {
        return Transformed_data{x,y,upg.transformed(rotate),direction,angle,0,1};
    }
}



//создание общего списка измененных домеин блоков
void transform(std::vector<Transformed_data> &all_transformed,std::vector<Block> &domainBlocks,QImage &inputimage)
{

    for(Block block:domainBlocks)
    {
        QImage upg=inputimage.copy(block.x,block.y,block.width,block.height).scaled(4,4, Qt::IgnoreAspectRatio);//потом сравнить с использованием Qt::FastTransformation

        for(quint8 direction:{0,1,2,3})
        {
            for(quint8 rotation : {0, 90, 180, 270})
            {
                Transformed_data transformed=apply_transform(block.x,block.y,upg,direction,rotation);
                all_transformed.push_back(transformed);
            }
        }
    }

}


double mse(QImage &rangepixt,Transformed_data &transformed)
{
    double mse=0;
    int pix_count=rangepixt.width()*rangepixt.height();

    double sumRange = 0.0;
    double sumDomain = 0.0;
    double sumDomainSquared = 0.0;
    double sumRangeDomain = 0.0;

    double bestContrast=0;
    double bestBrightness=0;

    // Вычисление сумм для средних значений, ковариации и дисперсии
    for (int y = 0; y < rangepixt.height(); ++y) {
        for (int x = 0; x < rangepixt.width(); ++x) {
            int rPixel=qRed(rangepixt.pixel(x,y));
            int tPixel=qRed(transformed.img.pixel(x,y));

            sumRange += rPixel;
            sumDomain += tPixel;
            sumDomainSquared += tPixel * tPixel;
            sumRangeDomain += rPixel * tPixel;
        }
    }

    // Средние значения
    double meanRange = sumRange / pix_count;
    double meanDomain = sumDomain / pix_count;

    // Коэффициенты контрастности и яркости
    bestContrast = (sumRangeDomain - pix_count * meanRange * meanDomain) / (sumDomainSquared - pix_count * meanDomain * meanDomain);
    bestBrightness = meanRange - bestContrast * meanDomain;

    transformed.contrast=bestContrast;
    transformed.brightness=bestBrightness;

    // Вычисление MSE с учетом контрастности и яркости
    for (int y = 0; y < rangepixt.height(); ++y)
    {
        for (int x = 0; x < rangepixt.width(); ++x)
        {
            int rPixel=qRed(rangepixt.pixel(x,y));
            int tPixel=qRed(transformed.img.pixel(x,y));

            double adjustedPixel = bestContrast * tPixel + bestBrightness;
            mse += (rPixel - adjustedPixel) * (rPixel - adjustedPixel);
        }
    }
    return mse/pix_count;
}



//создаю вектор содержащий лучший вариант преобразованного домеин блока похожего на ранговый блок

std::vector<Transformed_data> findSimilar(std::vector<Block> &rangeBlocks,std::vector<Transformed_data> &all_transformed,QImage &inputimage)
{
    std::vector<Transformed_data> similar;
    Transformed_data best;
    for(Block rblock:rangeBlocks)
    {

        QImage rangepixture=inputimage.copy(rblock.x,rblock.y,rblock.width,rblock.height);
        double minDifference = std::numeric_limits<double>::max();
        for(Transformed_data transblock:all_transformed)
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

    quint8 rangeBlock_size=4;
    quint8 domainBlock_size=8;
    generate_blocks(domainBlock_size,rangeBlock_size,inputimage,domainBlocks,rangeBlocks);

    std::vector<Transformed_data> all_transformed;
    transform(all_transformed,domainBlocks,inputimage);

    qInfo()<<all_transformed.size();
    //индекс рангового блока совпадает с индексом самого подходящего для него преобразованного домеин блока
    std::vector<Transformed_data> bestmatches = findSimilar(rangeBlocks,all_transformed,inputimage);

    QBuffer buffer(&compressedData);
    buffer.open(QIODevice::WriteOnly);
    QDataStream out(&buffer);
    method=5;
    out<<method<<width<<height;
    out<<rangeBlock_size<<domainBlock_size;
    for(Transformed_data block: bestmatches)
    {
        out<<block.x<<block.y<<block.direction<<block.angle<<block.brightness<<block.contrast;
    }

    return buffer.size();
}

void AFC::decoding(QString &filename,QByteArray &imageData,quint8 &method,unsigned short int &width,unsigned short int &height,QImage &inputimage)
{
    QFile file(filename);
    file.open(QIODevice::ReadOnly);
    QDataStream in(&file);

    quint8 rangeBlock_size=0;
    quint8 domainBlock_size=0;
    in>>method>>width>>height;
    in>>rangeBlock_size>>domainBlock_size;

    inputimage=QImage(width, height, QImage::Format_RGB888);
    QPainter painter(&inputimage);

    inputimage.fill(0);

    std::vector<Block> rangeBlocks;

    for(quint16 y=0;y<inputimage.height();y+=rangeBlock_size)
    {
        for(quint16 x=0;x<inputimage.width();x+=rangeBlock_size)
        {
            rangeBlocks.push_back(Block{x,y,rangeBlock_size,rangeBlock_size});
        }
    }

    /*
    for(int iter=0;iter<20;iter++)
    {
        for(Block rblock:rangeBlocks)
        {
            quint16 x, y;
            quint8 direction, angle;
            quint8 brightness, contrast;

            // Чтение параметров для каждого рангового блока
            in >> x >> y >> direction >> angle >> brightness >> contrast;

            QImage domain=inputimage.copy(x,y,domainBlock_size,domainBlock_size).scaled(4,4, Qt::IgnoreAspectRatio);
            Transformed_data trans_domain=apply_transform(x,y,domain,direction,angle);

            for (int ry = 0; ry < rangeBlock_size; ++ry)
            {
                for (int rx = 0; rx < rangeBlock_size; ++rx)
                {
                    int pixVal=qRed(trans_domain.img.pixel(rx,ry));
                    int newpix=static_cast<int>(contrast * pixVal + brightness);
                    trans_domain.img.setPixel(rx,ry,qRgb(newpix,newpix,newpix));
                }
            }

            painter.drawImage(rblock.x,rblock.y,trans_domain.img);

        }
    }
    */

    std::vector<Transformed_data> all_transformed;



    for(Block rblock:rangeBlocks)
    {
        quint16 x, y;
        quint8 direction, angle;
        quint8 brightness, contrast;

        // Чтение параметров для каждого рангового блока
        in >> x >> y >> direction >> angle >> brightness >> contrast;

        QImage domain=inputimage.copy(x,y,domainBlock_size,domainBlock_size).scaled(4,4, Qt::IgnoreAspectRatio);
        Transformed_data trans_domain=apply_transform(x,y,domain,direction,angle);

        for (int ry = 0; ry < rangeBlock_size; ++ry)
        {
            for (int rx = 0; rx < rangeBlock_size; ++rx)
            {
                int pixVal=qRed(trans_domain.img.pixel(rx,ry));
                int newpix=static_cast<int>(contrast * pixVal + brightness);
                trans_domain.img.setPixel(rx,ry,qRgb(newpix,newpix,newpix));
            }
        }

        all_transformed.push_back(trans_domain);

        painter.drawImage(rblock.x,rblock.y,trans_domain.img);

    }

    int it=0;
    for(Block rblock:rangeBlocks)
    {
        Transformed_data td=all_transformed[it];
        QImage domain=inputimage.copy(td.x,td.y,domainBlock_size,domainBlock_size).scaled(4,4, Qt::IgnoreAspectRatio);
        //Transformed_data trans_domain=apply_transform(td.x,td.y,domain,td.direction,td.angle);
        //painter.drawImage(rblock.x,rblock.y,trans_domain.img);
        painter.drawImage(rblock.x,rblock.y,domain);
        it+=1;
    }



}

/*
void AFC::decoding(QString &filename,QByteArray &imageData,quint8 &method,unsigned short int &width,unsigned short int &height,QImage &inputimage)
{
    QFile file(filename);
    file.open(QIODevice::ReadOnly);
    QDataStream in(&file);

    quint8 rangeBlock_size=0;
    quint8 domainBlock_size=0;
    in>>method>>width>>height;
    in>>rangeBlock_size>>domainBlock_size;

    inputimage=QImage(width, height, QImage::Format_RGB888);
    inputimage.fill(0);



    std::vector<Block> domainBlocks;
    std::vector<Block> rangeBlocks;

    generate_blocks(domainBlock_size, rangeBlock_size, inputimage, domainBlocks, rangeBlocks);


    int counter=0;

    for (int iteration = 0; iteration < 20; ++iteration)
    {
        QImage newImage = inputimage.copy();  // Создание независимой копии изображения

        for (const Block &rblock : rangeBlocks)
        {
            quint16 x, y;
            quint8 direction, angle;
            quint8 brightness, contrast;

            // Чтение параметров для каждого рангового блока
            in >> x >> y >> direction >> angle >> brightness >> contrast;

            // Получение соответствующего доменного блока
            QImage domainBlock = inputimage.copy(x, y, domainBlock_size, domainBlock_size)
                                     .scaled(rangeBlock_size, rangeBlock_size);

            // Применение обратного преобразования
            QImage restoredBlock = apply_transform(x, y, domainBlock, direction, angle).img;



            // Применение коррекции контрастности и яркости
            for (int ry = 0; ry < rangeBlock_size; ++ry)
            {
                for (int rx = 0; rx < rangeBlock_size; ++rx)
                {
                    int pixelValue = qGray(restoredBlock.pixel(rx, ry));
                    int adjustedPixel = static_cast<int>(contrast * pixelValue + brightness);

                        // Применение ограничения значений пикселя
                    adjustedPixel = qBound(0, adjustedPixel, 255);

                    restoredBlock.setPixel(rx, ry, qRgb(adjustedPixel, adjustedPixel, adjustedPixel));
                }
            }


            // Вставка восстановленного блока в новое изображение
            for (int ry = 0; ry < rangeBlock_size; ++ry)
            {
                for (int rx = 0; rx < rangeBlock_size; ++rx)
                {
                    int grayValue = qGray(restoredBlock.pixel(rx, ry));
                    newImage.setPixel(rblock.x + rx, rblock.y + ry, qRgb(grayValue, grayValue, grayValue));
                }
            }
        }



        // Обновление изображения для следующей итерации
        inputimage = newImage;
    }


}
*/
