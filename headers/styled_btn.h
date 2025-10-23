#ifndef STYLED_BTN_H
#define STYLED_BTN_H

#include <QPushButton>

class styled_btn : public QPushButton
{
    Q_OBJECT

public:
    explicit styled_btn(QWidget *parent = nullptr);

};

#endif // STYLED_BTN_H
