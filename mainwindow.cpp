#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "styled_btn.h"
#include <vector>
#include <jpeglib.h>
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



    cosTable.resize(N, std::vector<double>(N, 0.0));
    initCosTable();
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
    }
}

//кнопка загрузки изображения: выводит на экран само изображение+его вес
//а я получаю битовые матрицы изображения с использованием цветовой субдискретизации 4 2 0
//провожу вычисления этих матриц именно здесь для того чтобы этими вычислениями не загружать программу в дальнейшем
void MainWindow::on_openbtn_clicked()
{
    filename=QFileDialog::getOpenFileName(this,"Выберите фотографию формата .tiff или .bmp","C:/", "Image Files (*.bmp *.tiff)");
    QImage image(filename);
    width = image.width();
    height = image.height();

    QFileInfo fileInfo(filename);
    double fileSizeKB = fileInfo.size() / 1024.0;
    QString fileSizeString = QString::number(fileSizeKB, 'f', 3);
    this->infobefore->setText("Исходный: " + fileSizeString + " KB");

    bitY.resize(height, std::vector<double>(width));
    std::vector<std::vector<double>> bitCr(height, std::vector<double>(width));
    std::vector<std::vector<double>> bitCb(height, std::vector<double>(width));

    this->labelbefore->setPixmap(QPixmap::fromImage(image).scaled(this->labelbefore->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));

    // Заполнение битовой матрицы
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            QColor color = image.pixelColor(x, y);
            int r = color.red();
            int g = color.green();
            int b = color.blue();

            // Преобразование RGB в YCbCr
            double yVal = 0.299 * r + 0.587 * g + 0.114 * b;
            double cbVal = 128 - 0.168736 * r - 0.331264 * g + 0.5 * b;
            double crVal = 128 + 0.5 * r - 0.418688 * g - 0.081312 * b;

            bitY[y][x]=yVal;
            bitCr[y][x]=crVal;
            bitCb[y][x]=cbVal;

        }
    }

    //пробую цветовую субдискретизацию 4 2 0 которая берет среднее значение пикселей
    compbitCr.resize(height, std::vector<double>(width));
    compbitCb.resize(height, std::vector<double>(width));
    for (int y=0;y<height;y+=2){
        for (int x=0;x<width;x+=2)
        {
            double avgCr=(bitCr[y][x]+bitCr[y+1][x]+bitCr[y][x+1]+bitCr[y+1][x+1])/4.0;
            double avgCb=(bitCb[y][x]+bitCb[y+1][x]+bitCb[y][x+1]+bitCb[y+1][x+1])/4.0;

            compbitCr[y][x]=avgCr;
            compbitCr[y+1][x]=avgCr;
            compbitCr[y][x+1]=avgCr;
            compbitCr[y+1][x+1]=avgCr;
            compbitCb[y][x]=avgCb;
            compbitCb[y+1][x]=avgCb;
            compbitCb[y][x+1]=avgCb;
            compbitCb[y+1][x+1]=avgCb;
        }
    }


}


void MainWindow::rle()
{

}

void MainWindow::hfn()
{

}

void MainWindow::lzw()
{

}
void MainWindow::dct(const std::vector<std::vector<double>>& block, std::vector<std::vector<double>>& dctBlock)
{

    double alpha_u, alpha_v, sum;
    for (int u = 0; u < N; ++u) {
        for (int v = 0; v < N; ++v) {
            sum = 0.0;
            for (int x = 0; x < N; ++x) {
                for (int y = 0; y < N; ++y) {
                    sum += block[x][y] * cosTable[x][u] * cosTable[y][v];
                }
            }
            alpha_u = (u == 0) ? sqrt(1.0 / N) : sqrt(2.0 / N);
            alpha_v = (v == 0) ? sqrt(1.0 / N) : sqrt(2.0 / N);
            dctBlock[u][v] = alpha_u * alpha_v * sum;
        }
    }



}

void MainWindow::afc()
{

}


void MainWindow::on_dobtn_clicked()
{
    if(filename.isEmpty())
    {
        ui->statusbar->showMessage("Вы не выбрали изображение для сжатия");
    }
    else if(!filename.isEmpty())
    {
        ui->statusbar->clearMessage();
        if(algorigms_box->currentText()=="Run-Length Encoding")
        {
            rle();
        }
        else if(algorigms_box->currentText()=="Алгоритм Хаффмана")
        {
            hfn();
        }
        else if(algorigms_box->currentText()=="Алгоритм Лемпеля — Зива — Велча")
        {
            lzw();
        }
        else if(algorigms_box->currentText()=="Дискретное косинусное преобразование")
        {

            std::vector<std::vector<double>> dctbitY(height, std::vector<double>(width, 0.0));
            std::vector<std::vector<double>> dctbitCr(height, std::vector<double>(width, 0.0));
            std::vector<std::vector<double>> dctbitCb(height, std::vector<double>(width, 0.0));
            // Применение DCT к каждому блоку 8x8
            for (int i = 0; i < height; i += 8) {
                for (int j = 0; j < width; j += 8) {
                    // Извлечение блока 8x8
                    std::vector<std::vector<double>> blockY(8, std::vector<double>(8, 0.0));
                    std::vector<std::vector<double>> blockCr(8, std::vector<double>(8, 0.0));
                    std::vector<std::vector<double>> blockCb(8, std::vector<double>(8, 0.0));


                    for (int x = 0; x < 8; ++x) {
                        for (int y = 0; y < 8; ++y) {
                            blockY[x][y] = bitY[i + x][j + y];
                            blockCr[x][y] = compbitCr[i + x][j + y];
                            blockCb[x][y] = compbitCb[i + x][j + y];
                        }
                    }

                    // Выполнение DCT для блока
                    std::vector<std::vector<double>> dctBlockY(8, std::vector<double>(8, 0.0));
                    std::vector<std::vector<double>> dctBlockCr(8, std::vector<double>(8, 0.0));
                    std::vector<std::vector<double>> dctBlockCb(8, std::vector<double>(8, 0.0));
                    dct(blockY, dctBlockY);
                    //dct(blockCr,dctBlockCr);
                    //dct(blockCb,dctBlockCb);


                    // Запись результатов DCT обратно в изображение
                    for (int x = 0; x < 8; ++x) {
                        for (int y = 0; y < 8; ++y) {
                            dctbitY[i + x][j + y] = dctBlockY[x][y];
                            dctbitCr[i + x][j + y] = dctBlockCr[x][y];
                            dctbitCb[i + x][j + y] = dctBlockCb[x][y];
                        }
                    }
                }
            }
            ui->statusbar->showMessage("Успешно сжато");




        }
        else if(algorigms_box->currentText()=="Алгоритм фрактального сжатия")
        {
            afc();
        }
        /*
        QPixmap picafter;
        //picafter.loadFromData(compressedData);
        picafter.loadFromData(imageData,"JPEG");
        this->labelafter->setPixmap(picafter.scaled(this->labelafter->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));

        double fileSizeKB = static_cast<double>(compressedData.size()) / 1024;
        QString fileSizeString = QString::number(fileSizeKB, 'f', 3);
        this->infoafter->setText("Сжатый: "+fileSizeString+" KB");

        */
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


    if(filename.isEmpty())
    {
        ui->statusbar->showMessage("Вы не выбрали изображение для сжатия");
    }
    else if(compressedData.isEmpty())
    {
        /*
        QString outputFilePath = QFileDialog::getSaveFileName(nullptr, "Save Image", "", "JPEG Files (*.jpg *.jpeg)");
        QImage image(width, height, QImage::Format_RGB32);
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                double yVal = bitY[y][x];
                double cbVal = compbitCb[y][x];
                double crVal = compbitCr[y][x];

                // Обратное преобразование YCbCr в RGB
                int r = std::round(yVal + 1.402 * (crVal - 128));
                int g = std::round(yVal - 0.344136 * (cbVal - 128) - 0.714136 * (crVal - 128));
                int b = std::round(yVal + 1.772 * (cbVal - 128));

                // Ограничиваем значения от 0 до 255
                r = std::clamp(r, 0, 255);
                g = std::clamp(g, 0, 255);
                b = std::clamp(b, 0, 255);

                // Устанавливаем пиксель в результирующем изображении
                image.setPixelColor(x, y, QColor(r, g, b));

            }
        }
        struct jpeg_compress_struct cinfo;
        struct jpeg_error_mgr jerr;

        cinfo.err = jpeg_std_error(&jerr);
        jpeg_create_compress(&cinfo);

        const char* ffilename=filename.toStdString().c_str();
        FILE* outfile = fopen(ffilename, "wb");


        jpeg_stdio_dest(&cinfo, outfile);

        cinfo.image_width = width;
        cinfo.image_height = height;
        cinfo.input_components = 3;
        cinfo.in_color_space = JCS_RGB;

        jpeg_set_defaults(&cinfo);
        jpeg_set_quality(&cinfo, 100, TRUE); // Устанавливаем качество на 100 (без сжатия)

        jpeg_start_compress(&cinfo, TRUE);

        JSAMPROW row_pointer;
        std::vector<unsigned char> row_data(width * 3);

        while (cinfo.next_scanline < cinfo.image_height) {
            for (int x = 0; x < width; ++x) {
                QColor color = image.pixelColor(x, cinfo.next_scanline);
                row_data[x * 3] = color.red();
                row_data[x * 3 + 1] = color.green();
                row_data[x * 3 + 2] = color.blue();
            }
            row_pointer = row_data.data();
            jpeg_write_scanlines(&cinfo, &row_pointer, 1);
        }

        jpeg_finish_compress(&cinfo);
        jpeg_destroy_compress(&cinfo);
        fclose(outfile);
        */





        /*
        ui->statusbar->showMessage("Вы не сжали изображение");
        QImage outputImage(width, height, QImage::Format_RGB32);
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                //Pixel pixel = bitMatrix[y][x];
                //outputImage.setPixel(x, y, qRgb(pixel.r, pixel.g, pixel.b));
                int pixelValue = static_cast<int>(compbitCb[y][x]);
                outputImage.setPixel(x, y, qRgb(pixelValue,pixelValue,pixelValue));
            }
        }
        QString filePath = QFileDialog::getSaveFileName(this, "Сохранить изображение", "", "JPEG Image (*.jpg *.jpeg)");
        outputImage.save(filePath, "JPEG");
        ui->statusbar->showMessage("Успешно");
        */






        /*
        QImage outputImage(width, height, QImage::Format_RGB32);
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                double yVal = bitY[y][x];
                double cbVal = compbitCb[y][x];
                double crVal = compbitCr[y][x];

                // Обратное преобразование YCbCr в RGB
                int r = std::round(yVal + 1.402 * (crVal - 128));
                int g = std::round(yVal - 0.344136 * (cbVal - 128) - 0.714136 * (crVal - 128));
                int b = std::round(yVal + 1.772 * (cbVal - 128));

                // Ограничиваем значения от 0 до 255
                r = std::clamp(r, 0, 255);
                g = std::clamp(g, 0, 255);
                b = std::clamp(b, 0, 255);

                // Устанавливаем пиксель в результирующем изображении
                outputImage.setPixelColor(x, y, QColor(r, g, b));

            }
        }
        QString filePath = QFileDialog::getSaveFileName(this, "Сохранить изображение", "", "JPEG Image (*.jpg *.jpeg)");
        outputImage.save(filePath, "JPEG");
        ui->statusbar->showMessage("Успешно");
        */
    }
    else
    {

        /*
        QImage image;
        if (image.loadFromData(imageData, "JPEG")) {
            QString filePath = QFileDialog::getSaveFileName(this, "Сохранить изображение", "", "JPEG Image (*.jpg *.jpeg)");
            if (!filePath.isEmpty()) {
                image.save(filePath, "JPEG");
            }
        } else {
            qWarning("Не удалось загрузить изображение из QByteArray");
        }
        */
    }

}


/*
// Инициализация битовой матрицы
struct Pixel {
    int r;
    int g;
    int b;
};
std::vector<std::vector<Pixel>> bitMatrix(height, std::vector<Pixel>(width));
std::vector<std::vector<double>> bitY(height, std::vector<double>(width));


// Создание QImage из битовой матрицы
QImage outputImage(width, height, QImage::Format_RGB32);
for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width; ++x) {
        //Pixel pixel = bitMatrix[y][x];
        //outputImage.setPixel(x, y, qRgb(pixel.r, pixel.g, pixel.b));
        int pixelValue = static_cast<int>(compbitCb[y][x]);
        outputImage.setPixel(x, y, qRgb(pixelValue,pixelValue,pixelValue));
    }
}
*/
