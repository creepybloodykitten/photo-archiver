#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMouseEvent>
#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QFileDialog>
#include <QPixmap>
#include <QImage>
#include "styled_btn.h"


QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();



private slots:

    void on_exit_clicked();

    void on_minimaze_clicked();

    void on_titlebar_pressed();

    void on_titlebar_released();

    void update_cbox();

    void on_openbtn_clicked();

    void on_dobtn_clicked();

    void on_savebtn_clicked();



private:
    Ui::MainWindow *ui;
    //глобальные переменные
    bool mov;
    QPoint startPos;
    double px,py;
    QCheckBox *choice;
    QComboBox* algorigms_box;
    styled_btn* openbtn;
    styled_btn* savebtn;
    styled_btn* dobtn;
    QLabel *infobefore;
    QLabel *infoafter;
    QLabel *labelbefore;
    QLabel *labelafter;
    QByteArray imageData;
    QByteArray compressedData;
    QString filename;
    QImage inputimage;
    QImage outputimage;
    quint8 method; //0-без алгоритма 1-rle 2-hfn 3-lzw 4-dct 5-afc
    quint64 datasize;
    unsigned short int width;
    unsigned short int height;

};
#endif // MAINWINDOW_H
