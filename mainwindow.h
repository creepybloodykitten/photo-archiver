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
    int width;
    int height;
    std::vector<std::vector<double>> bitY;
    std::vector<std::vector<double>> compbitCr;
    std::vector<std::vector<double>> compbitCb;


    void initCosTable() {
        for (int i = 0; i < N; ++i) {
            for (int j = 0; j < N; ++j) {
                cosTable[i][j] = cos((2 * i + 1) * j * M_PI / (2 * N));
            }
        }
    }

    static const int N = 8;
    std::vector<std::vector<double>> cosTable;

    //методы
    void rle();
    void hfn();
    void lzw();
    void dct(const std::vector<std::vector<double>>& block, std::vector<std::vector<double>>& dctBlock);
    void afc();
};
#endif // MAINWINDOW_H
