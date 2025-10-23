#include "styled_btn.h"

styled_btn::styled_btn(QWidget *parent):QPushButton (parent)
{
    this->setText("new custom button");
    QString styleSheet = R"(
    styled_btn {
        border-radius: 8px;

        font-size: 15px;
        font-weight: 600;
    }
    styled_btn:enabled {
        color: black;
        background-color: white;

    }
    styled_btn:hover:!pressed {
        color: black;
        background-color: #999999;

    }
    styled_btn:pressed {
        color: black;
        background-color: #6e6c68;

    }
    )";
    this->setStyleSheet(styleSheet);
    this->resize(130,50);
}


