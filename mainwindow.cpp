#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "styled_btn.h"
#include "rle.h"
#include "huffman.h"
#include "lzw.h"
#include "afc.h"
#include "dct.h"

#include <QBuffer>
#include <cmath>




MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowFlags(Qt::FramelessWindowHint);//убирает главную рамку
    this->resize(950, 600);


    openbtn=new styled_btn(this);
    savebtn=new styled_btn(this);
    dobtn=new styled_btn(this);
    openbtn->setText("Загрузить");
    connect(openbtn, &styled_btn::clicked, this, &MainWindow::on_openbtn_clicked);
    savebtn->setText("Сохранить");
    connect(savebtn,&styled_btn::clicked,this,&MainWindow::on_savebtn_clicked);
    dobtn->setText("Преобразовать");
    connect(dobtn,&styled_btn::clicked,this,&MainWindow::on_dobtn_clicked);
    openbtn->move(50,40);
    savebtn->move(770,510);
    dobtn->move(500,40);


    choice=new QCheckBox(this);
    choice->setText("Сжатие с потерями информации");
    QString check_style = R"(
    QCheckBox {
        font-size: 15px;
        font-weight: 500;
    }
    )";
    choice->setStyleSheet(check_style);
    choice->move(53,510);//три пикселя добавил для красоты
    choice->setMinimumSize(300,100);


    algorigms_box=new QComboBox(this);
    algorigms_box->move(50,510);
    algorigms_box->setMinimumSize(350,30);
    algorigms_box->setStyleSheet("font-size:15px;font-weight: 500;");
    algorigms_box->addItem("Run-Length Encoding");
    algorigms_box->addItem("Алгоритм Хаффмана");
    algorigms_box->addItem("Алгоритм Лемпеля — Зива — Велча");
    algorigms_box->addItem("Сохранить в формате bmp/tiff/bin без сжатия");
    connect(choice, &QCheckBox::stateChanged, this, &MainWindow::update_cbox);


    infobefore=new QLabel(this);
    infoafter=new QLabel(this);
    infobefore->setMinimumSize(300,30);
    infoafter->setMinimumSize(300,30);
    infobefore->setText("Исходный:");
    infoafter->setText("Новый:");
    infobefore->setStyleSheet("font-size:20px;font-weight: 500;");
    infoafter->setStyleSheet("font-size:20px;font-weight: 500;");
    infobefore->move(200,50);
    infoafter->move(650,50);


    labelbefore=new QLabel(this);
    labelafter=new QLabel(this);
    labelbefore->setAlignment(Qt::AlignCenter);
    labelafter->setAlignment(Qt::AlignCenter);
    labelbefore->setMinimumSize(400,400);
    labelafter->setMinimumSize(400,400);
    labelbefore->move(50,100);
    labelafter->move(500,100);
    labelbefore->setFrameShape(QFrame::Box);
    labelafter->setFrameShape(QFrame::Box);

}


MainWindow::~MainWindow()
{
    delete ui;
}

//кнопка закрытия программы
void MainWindow::on_exit_clicked()
{
    close();
}

//кнопка свертывания программы
void MainWindow::on_minimaze_clicked()
{
    showMinimized();
}

//как и во всех программах - titlebar зажимая который я могу передвигать окошко
void MainWindow::on_titlebar_pressed()
{
    mov=true;
    startPos=QCursor::pos()-this->frameGeometry().topLeft();
    //startPos=QCursor::pos()-this->pos();
    while(ui->titlebar->isDown()&&mov==true)
    {
        QPoint Pos = QCursor::pos();
        this->move(Pos-startPos);
        QCoreApplication::processEvents();
        if((! ui->titlebar->isDown()))
        {
            mov=false;
        }
    }
}
void MainWindow::on_titlebar_released()
{
    mov=false;
}

//меняю элементы combobox в зависимости от chechbox
void MainWindow::update_cbox()
{
    algorigms_box->clear();
    if(this->choice->isChecked())
    {
        algorigms_box->addItem("Дискретное косинусное преобразование");
        algorigms_box->addItem("Алгоритм фрактального сжатия");
    }
    else
    {
        algorigms_box->addItem("Run-Length Encoding");
        algorigms_box->addItem("Алгоритм Хаффмана");
        algorigms_box->addItem("Алгоритм Лемпеля — Зива — Велча");
        algorigms_box->addItem("Сохранить в формате bmp/tiff/bin без сжатия");
    }
}




//кнопка загрузки изображения: выводит на экран само изображение+его вес
//а я получаю битовые матрицы изображения с использованием цветовой субдискретизации 4 2 0
//провожу вычисления этих матриц именно здесь для того чтобы этими вычислениями не загружать программу в дальнейшем
void MainWindow::on_openbtn_clicked()
{
    compressedData.clear();
    imageData.clear();//очищаю данные прошлых декомпрессий
    filename=QFileDialog::getOpenFileName(this,"Выберите фотографию формата tiff/bmp/bin","C:/", "Image File (*.bmp *.tiff);; Image Data (*.bin);");
    if (filename.endsWith(".bin", Qt::CaseInsensitive))//пользователь загрузил bin (Qt::CaseInsensitive - сравнение без учета регистра)
    {
        QFile file(filename);
        file.open(QIODevice::ReadOnly);
        QDataStream in(&file);
        in>>method>>width>>height;


        if (method==0)
        {
            QImage image(width, height, QImage::Format_RGB888);
            // чтение данных из файла и заполнение изображения
            for (int y = 0; y < height; ++y) {
                for (int x = 0; x < width; ++x) {
                    quint8 red, green, blue;
                    in >> red >> green >> blue; // считываем 3 байта (RGB) для текущего пикселя
                    QColor pixelColor(red, green, blue);
                    image.setPixelColor(x, y, pixelColor);
                }
            }

            inputimage=image.copy();
        }
        file.close();


        if(method==1)
        {
            RLE::decoding(filename,method,width,height,inputimage);
        }

        if(method==2)
        {
            Huffman::decoding(filename,imageData,method,width,height,inputimage);
        }

        if(method==3)
        {
            LZW::decoding(filename,imageData,inputimage,method,width,height);
        }
        if(method==4)
        {
            DCT::decoding(filename,imageData,method,width,height,inputimage);

        }

        if(method==5)
        {
            AFC::decoding(filename,imageData,method,width,height,inputimage);
        }

    }
    else //пользователь загрузил изображение
    {
        inputimage.load(filename);
        width = inputimage.width();
        height = inputimage.height();
    }


    QFileInfo fileInfo(filename);
    double fileSizeKB = fileInfo.size() / 1024.0;
    QString fileSizeString = QString::number(fileSizeKB, 'f', 3);
    this->infobefore->setText("Исходный: " + fileSizeString + " KB");
    this->labelbefore->setPixmap(QPixmap::fromImage(inputimage).scaled(this->labelbefore->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));

}


void MainWindow::on_dobtn_clicked()
{
    imageData.clear();
    compressedData.clear();
    //очищаю данные прошлых сжатий

    if(filename.isEmpty())
    {
        ui->statusbar->showMessage("Вы ничего не выбрали");
    }

    else if(algorigms_box->currentText()=="Сохранить в формате bmp/tiff/bin без сжатия")
    {
        ui->statusbar->showMessage("При таком варианте вы можете только сохранить изображение");
    }

    else if (filename.endsWith(".bin", Qt::CaseInsensitive))//если bin, то задача - декомпрессия
    {
        this->labelafter->setPixmap(QPixmap::fromImage(inputimage).scaled(this->labelafter->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        QBuffer buffer(&imageData);
        buffer.open(QIODevice::WriteOnly);
        inputimage.save(&buffer,"BMP");
        double fileSizeKB = static_cast<double>(buffer.size()) / 1024;
        QString fileSizeString = QString::number(fileSizeKB, 'f', 3);
        this->infoafter->setText("Новый: "+fileSizeString+" KB");
        ui->statusbar->showMessage("Успешно разжато");
    }
    else// если bmp/tiff, то задача - сжать
    {
        ui->statusbar->clearMessage();
        if(algorigms_box->currentText()=="Run-Length Encoding")
        {
            datasize=RLE::encoding(compressedData,method,width,height,inputimage);
            this->labelafter->setPixmap(QPixmap::fromImage(inputimage).scaled(this->labelafter->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        }
        else if(algorigms_box->currentText()=="Алгоритм Хаффмана")
        {
            datasize=Huffman::encoding(compressedData,method,width,height,inputimage);
            this->labelafter->setPixmap(QPixmap::fromImage(inputimage).scaled(this->labelafter->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));

        }
        else if(algorigms_box->currentText()=="Алгоритм Лемпеля — Зива — Велча")
        {
            datasize=LZW::encoding(imageData,compressedData,inputimage,method,width,height);
            this->labelafter->setPixmap(QPixmap::fromImage(inputimage).scaled(this->labelafter->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        }
        else if(algorigms_box->currentText()=="Дискретное косинусное преобразование")
        {
            datasize=DCT::encoding(compressedData,method,width,height,inputimage,outputimage);
            this->labelafter->setPixmap(QPixmap::fromImage(outputimage).scaled(this->labelafter->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        }
        else if(algorigms_box->currentText()=="Алгоритм фрактального сжатия")
        {
            datasize=AFC::encoding(compressedData,method,width,height,inputimage);
            this->labelafter->setPixmap(QPixmap::fromImage(inputimage).scaled(this->labelafter->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        }
        double fileSizeKB = static_cast<double>(datasize) / 1024;
        QString fileSizeString = QString::number(fileSizeKB, 'f', 3);
        this->infoafter->setText("Новый: "+fileSizeString+" KB");
        ui->statusbar->showMessage("Успешно сжато");

    }

}

void MainWindow::on_savebtn_clicked()
{
    if(filename.isEmpty())
    {
        ui->statusbar->showMessage("Вы не выбрали изображение");
    }



    else if (algorigms_box->currentText()=="Сохранить в формате bmp/tiff/bin без сжатия")
    {

        if(filename.endsWith(".bmp", Qt::CaseInsensitive) || filename.endsWith(".tiff", Qt::CaseInsensitive))
        {
            QString filePath = QFileDialog::getSaveFileName(this, "Выберите куда сохранить сжатое изображение", "", "Image Data (*.bin);");
            QFile file(filePath);
            file.open(QIODevice::WriteOnly );
            QDataStream out(&file);
            //записываю размер файла и метод который используется для сжатия
            method=0;
            out<<method<<width<<height;
            for (int y = 0; y < height; ++y)
            {
                for (int x = 0; x < width; ++x)
                {
                    QColor pixelColor = inputimage.pixelColor(x, y);
                    quint8 red = pixelColor.red();
                    quint8 green = pixelColor.green();
                    quint8 blue = pixelColor.blue();
                    out << red << green << blue;
                }
            }
            file.close();
            ui->statusbar->showMessage("Успешно сохранено");
        }

        else if (filename.endsWith(".bin", Qt::CaseInsensitive))
        {
            QString filePath = QFileDialog::getSaveFileName(this, "Выберите куда сохранить изображение", "", "Image BMP File (*.bmp);;Image BMP File(*.tiff);");
            if(filePath.endsWith(".bmp", Qt::CaseInsensitive))
            {
                inputimage.save(filePath,"BMP");
            }
            else
            {
                inputimage.save(filePath,"TIFF");
            }
            ui->statusbar->showMessage("Успешно сохранено");
        }

    }




    else if (filename.endsWith(".bmp", Qt::CaseInsensitive) || filename.endsWith(".tiff", Qt::CaseInsensitive))
    {
        if(method==1||method==2||method==3||method==4||method==5)
        {
            QString filePath = QFileDialog::getSaveFileName(this, "Выберите куда сохранить сжатое изображение", "", "Image Data (*.bin);");
            QFile file(filePath);
            file.open(QIODevice::WriteOnly);
            file.write(compressedData);
            file.close();

        }
        ui->statusbar->showMessage("Успешно сохранено");
    }




    else if(filename.endsWith(".bin", Qt::CaseInsensitive))
    {
        if(method==1||method==2||method==3)
        {
            QString filePath = QFileDialog::getSaveFileName(this, "Выберите куда сохранить изображение", "", "Image BMP File (*.bmp);;Image TIFF File(*.tiff);");
            if(filePath.endsWith(".bmp", Qt::CaseInsensitive))
            {
                inputimage.save(filePath,"BMP");
            }
            else
            {
                inputimage.save(filePath,"TIFF");
            }
        }
        ui->statusbar->showMessage("Успешно сохранено");
    }



    else
    {
        ui->statusbar->showMessage("Вы не преобразовали изображение");
    }
}




