#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "styled_btn.h"






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
    connect(choice, &QCheckBox::stateChanged, this, &MainWindow::update_cbox);

    infobefore=new QLabel(this);
    infoafter=new QLabel(this);
    infobefore->setMinimumSize(300,30);
    infoafter->setMinimumSize(300,30);
    infobefore->setText("Исходный:");
    infoafter->setText("Сжатый:");
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

void MainWindow::on_exit_clicked()
{
    close();
}

void MainWindow::on_minimaze_clicked()
{
    showMinimized();
}

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
    }
}


void MainWindow::on_openbtn_clicked()
{
    QString filename=QFileDialog::getOpenFileName(this,"Выберите фотографию формата .tiff или .bmp","C:/", "Image Files (*.bmp *.tiff)");
    QFile file(filename);
    if (file.open(QIODevice::ReadOnly))
    {
        imageData = file.readAll();
        file.close();

        //QPixmap picbefore;
        //picbefore.loadFromData(imageData);
        //this->labelbefore->setPixmap(picbefore.scaled(this->labelbefore->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));

        //QImage image;
        //if (image.loadFromData(imageData, "TIFF"))
        //{
        //    this->labelbefore->setPixmap(QPixmap::fromImage(image).scaled(this->labelbefore->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        //}

        QImageReader reader(filename);
        reader.setAutoTransform(true);
        const QImage newImage = reader.read();

        if (!newImage.isNull()) {
            QPixmap pixmap = QPixmap::fromImage(newImage);
            this->labelbefore->setPixmap(pixmap);
        } else {
            // Обработка ошибки загрузки изображения
            qDebug() << "Failed to load image:" << reader.errorString();
        }

        double fileSizeKB = static_cast<double>(imageData.size()) / 1024;
        QString fileSizeString = QString::number(fileSizeKB, 'f', 3);
        this->infobefore->setText("Исходный: "+fileSizeString+" KB");
    }
}


QByteArray MainWindow::rle(const QByteArray &imageData)
{
    int size = imageData.size();
    int i = 0;

    while (i < size) {
        char currentChar = imageData.at(i);
        int count = 1;

        while (i + count < size && imageData.at(i + count) == currentChar) {
            ++count;
        }

        compressedData.append(currentChar);
        compressedData.append(static_cast<char>(count));
        i += count;
    }

    /*
    while (i < size) {
        char currentChar = imageData.at(i);
        int count = 1;

        while (i + count < size && imageData.at(i + count) == currentChar) {
            ++count;
            if (count == 255) { // Ограничиваем количество до 255, чтобы поместить в один байт
                break;
            }
        }

        if (count > 1 || (count == 1 && (i == 0 || currentChar != imageData.at(i - 1)))) {
            compressedData.append(currentChar);
            compressedData.append(static_cast<char>(count));
        } else {
            // Если текущий символ такой же, как предыдущий, и count == 1, добавляем несжатый байт
            compressedData.append('\0');
            compressedData.append(currentChar);
        }

        i += count;
    }
    */
    return compressedData;

}

QByteArray MainWindow::hfn(const QByteArray &imageData)
{
    return compressedData;
}

QByteArray MainWindow::lzw(const QByteArray &imageData)
{
    return compressedData;
}

QByteArray MainWindow::dtc(const QByteArray &imageData)
{
    return compressedData;
}

QByteArray MainWindow::afc(const QByteArray &imageData)
{
    return compressedData;
}

void MainWindow::on_dobtn_clicked()
{
    if(imageData.isEmpty()&&compressedData.isEmpty())
    {
        ui->statusbar->showMessage("Вы не выбрали изображение для сжатия");
    }
    else if(!imageData.isEmpty()&&compressedData.isEmpty())
    {
        ui->statusbar->clearMessage();
        if(algorigms_box->currentText()=="Run-Length Encoding")
        {
            rle(imageData);
        }
        else if(algorigms_box->currentText()=="Алгоритм Хаффмана")
        {
            hfn(imageData);
        }
        else if(algorigms_box->currentText()=="Алгоритм Лемпеля — Зива — Велча")
        {
            lzw(imageData);
        }
        else if(algorigms_box->currentText()=="Дискретное косинусное преобразование")
        {
            dtc(imageData);
        }
        else if(algorigms_box->currentText()=="Алгоритм фрактального сжатия")
        {
            afc(imageData);
        }
        QPixmap picafter;
        //picafter.loadFromData(compressedData);
        picafter.loadFromData(imageData,"JPEG");
        this->labelafter->setPixmap(picafter.scaled(this->labelafter->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));

        double fileSizeKB = static_cast<double>(compressedData.size()) / 1024;
        QString fileSizeString = QString::number(fileSizeKB, 'f', 3);
        this->infoafter->setText("Сжатый: "+fileSizeString+" KB");
    }
}

void MainWindow::on_savebtn_clicked()
{
    /*
    QString selectedFolderPath = QFileDialog::getExistingDirectory(this, "Выберите путь сохранения изображения", "C:/");
    QImage save_img;
    if(save_img.loadFromData(compressedData,"JPEG"))
    {
        save_img.save(selectedFolderPath,"JPEG");
    }
    */


    if(imageData.isEmpty())
    {
        ui->statusbar->showMessage("Вы не выбрали изображение для сжатия");
    }
    else if(compressedData.isEmpty())
    {
        ui->statusbar->showMessage("Вы не сжали изображение");
    }
    else
    {
        QImage image;
        if (image.loadFromData(imageData, "JPEG")) {
            QString filePath = QFileDialog::getSaveFileName(this, "Сохранить изображение", "", "JPEG Image (*.jpg *.jpeg)");
            if (!filePath.isEmpty()) {
                image.save(filePath, "JPEG");
            }
        } else {
            qWarning("Не удалось загрузить изображение из QByteArray");
        }
    }

}




