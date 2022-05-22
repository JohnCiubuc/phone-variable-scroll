#include "PhoneVariableScroll.h"
#include "ui_PhoneVariableScroll.h"

PhoneVariableScroll::PhoneVariableScroll(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::PhoneVariableScroll)
{
    ui->setupUi(this);
    QProcess * annoying = new QProcess(this);
    connect(annoying, &QProcess::readyRead, this, &PhoneVariableScroll::readAdbShell);
    annoying->start("adb", QStringList() << "shell" << "getevent");
}

PhoneVariableScroll::~PhoneVariableScroll()
{
    delete ui;
}

std::vector<float> PhoneVariableScroll::linspacer(float start, float end, size_t points)
{
    std::vector<float> res(points);
    float step = (end - start) / (points - 1);
    size_t i = 0;
    for (auto& e : res)
    {
        e = start + step * i++;
    }
    return res;
}

void PhoneVariableScroll::readAdbShell()
{
    db "ready";
}

