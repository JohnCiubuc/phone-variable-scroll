#include "PhoneVariableScroll.h"
#include "ui_PhoneVariableScroll.h"

PhoneVariableScroll::PhoneVariableScroll(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::PhoneVariableScroll)
{
    ui->setupUi(this);
    _annoying = new QProcess(this);
    connect(_annoying, &QProcess::readyRead, this, &PhoneVariableScroll::readAdbShell);
    _annoying->start("adb", QStringList() << "shell" << "getevent");
}

PhoneVariableScroll::~PhoneVariableScroll()
{
    delete ui;
}

std::vector<float> PhoneVariableScroll::linspace(float start, float end, size_t points)
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
    auto output = _annoying->readAll();
    //
    QList<QByteArray> events = output.split('\n');
// "/dev/input/event4: 0000 0000 00000000"
    for (auto event : events)
    {
        QList<QByteArray> packet = event.split(' ');
        if (packet.size() == 4)
        {
            // Finger detection event
            if (packet.at(2) == "0145")
            {
                _bFingerActive = bool(packet.at(3).toInt());
                db _bFingerActive;
            }
            // Finger Y movement event
            if (packet.at(2) == "0036")
            {
                int fingerY = packet.at(3).toInt(nullptr, 16);
                db fingerY;
            }
        }

    }
}

