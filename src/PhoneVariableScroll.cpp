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


    QProcess * getScreenSize = new QProcess();
    connect(getScreenSize, &QProcess::readyRead, this, [=]()
    {
        auto line = getScreenSize->readAll().split(' ');
        if (line.size() == 3)
        {
            auto diff = line.at(2).split('x');
            _screenSize = QPoint(diff[0].toInt(), diff[1].toInt());
        }
        else
            qw "Screen size requested from ADB was wrong. Command requested: adb shell wm size";

        _screenSpaces = linspace(0,_screenSize.y(),SCREEN_DIVISIONS);
    });
    getScreenSize->start("adb", QStringList() << "shell" << "wm" << "size");
}

PhoneVariableScroll::~PhoneVariableScroll()
{
    delete ui;
}

QList<float> PhoneVariableScroll::linspace(float start, float end, int points)
{
    QList<float> res;
    float step = (end - start) / (points - 1);
    int i = 0;
    for (int x = 0; x < points; x++)
        res << start + step * i++;
    return res;
}

void PhoneVariableScroll::readAdbShell()
{
    auto output = _annoying->readAll();
    //
    QList<QByteArray> events = output.split('\n');
// "/dev/input/event4: 0000 0000 00000000"
    for (const auto &event : events)
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
