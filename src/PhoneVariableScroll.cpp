#include "PhoneVariableScroll.h"
#include "ui_PhoneVariableScroll.h"

PhoneVariableScroll::PhoneVariableScroll(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::PhoneVariableScroll)
{
    ui->setupUi(this);

//    _input = new UInput();

    struct uinput_setup usetup;

    _fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);

    /* enable mouse button left and relative events */
    ioctl(_fd, UI_SET_EVBIT, EV_KEY);
    ioctl(_fd, UI_SET_KEYBIT, BTN_LEFT);

    ioctl(_fd, UI_SET_EVBIT, EV_REL);
    ioctl(_fd, UI_SET_RELBIT, REL_X);
    ioctl(_fd, UI_SET_RELBIT, REL_WHEEL);
    ioctl(_fd, UI_SET_RELBIT, REL_Y);

    memset(&usetup, 0, sizeof(usetup));
    usetup.id.bustype = BUS_USB;
    usetup.id.vendor = 0x1234; /* sample vendor */
    usetup.id.product = 0x5678; /* sample product */
    strcpy(usetup.name, "Example device");

    ioctl(_fd, UI_DEV_SETUP, &usetup);
    ioctl(_fd, UI_DEV_CREATE);

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
        _wheelTickRates = linspace(WHEEL_TICK_RATE_MIN,WHEEL_TICK_RATE_MAX,SCREEN_DIVISIONS/2-1);
    });
    getScreenSize->start("adb", QStringList() << "shell" << "wm" << "size");

//    QTimer * temp = new QTimer;
//    connect(temp, &QTimer::timeout, this, [=]()
//    {
//        _input->emulate_touchpad_scroll(-100);
//    });
//    temp->start(500);


//    sleep(10);
    _wheelTimer = new QTimer(this);
    connect(_wheelTimer, &QTimer::timeout, this, &PhoneVariableScroll::wheelRun);

}

PhoneVariableScroll::~PhoneVariableScroll()
{
    ioctl(_fd, UI_DEV_DESTROY);
//    close(_fd);
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
                bool(packet.at(3).toInt()) ?
                _wheelTimer->start(1000) :
                _wheelTimer->stop();

            }
            // Finger Y movement event
            if (packet.at(2) == "0036")
            {
                int fingerY = packet.at(3).toInt(nullptr, 16);
                int index = -1;
                for(int i = 0; i < _screenSpaces.size(); ++i)
                {
                    if (fingerY <= _screenSpaces.at(i))
                    {
                        index = i;
                        break;
                    }
                }
                if (index == -1)
                    break;
                index = _screenDivisionsHalf - index;

                // Inverts index from center
                index = index > 0 ?
                        _screenDivisionsHalf - index : index < 0 ?
                        -1 * (_screenDivisionsHalf + index): 0;

                updateWheelIndex(index);
//                db fingerY;
            }
        }

    }
}

void PhoneVariableScroll::wheelRun()
{
    emit_uinput(_fd, EV_REL, REL_WHEEL, wheelRepeat.direction);
    emit_uinput(_fd, EV_SYN, SYN_REPORT, 0);
}

void PhoneVariableScroll::updateWheelIndex(int index)
{
    wheelRepeat.direction = index > 0 ? 1:-1;
    index = abs(index);
    if(wheelRepeat.repeatMasterCounter != index)
    {
        if(index < wheelRepeat.repeatMasterCounter)
            wheelRun();
        db index;
        wheelRepeat.repeatMasterCounter = index;
        wheelRepeat.repeat = index;
        if(index == 0) return;
//        wheelRun();
//        db "interval: " <<WHEEL_TICK_RATE*wheelRepeat.repeat;
        _wheelTimer->setInterval(_wheelTickRates.at(index-1));
    }
//    _wheelTimer->setInterval(1000);
}

void PhoneVariableScroll::emit_uinput(int fd, int type, int code, int val)
{
    struct input_event ie;

    ie.type = type;
    ie.code = code;
    ie.value = val;
    /* timestamp values below are ignored */
    ie.time.tv_sec = 0;
    ie.time.tv_usec = 0;

    write(fd, &ie, sizeof(ie));
}
