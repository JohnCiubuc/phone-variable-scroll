#include "PhoneVariableScroll.h"
#include "ui_PhoneVariableScroll.h"

PhoneVariableScroll::PhoneVariableScroll(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::PhoneVariableScroll)
{
    ui->setupUi(this);

    createInput();
    createScreenSize();

    _adbProcessPipe = new QProcess(this);
    connect(_adbProcessPipe, &QProcess::readyRead, this, &PhoneVariableScroll::readAdbShell);

    _wheelTimer = new QTimer(this);
    connect(_wheelTimer, &QTimer::timeout, this, &PhoneVariableScroll::wheelRun);

    _adbProcessPipe->start("adb", QStringList() << "shell" << "getevent");
}

PhoneVariableScroll::~PhoneVariableScroll()
{
    ioctl(_fd, UI_DEV_DESTROY);
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
    auto output = _adbProcessPipe->readAll();
    QList<QByteArray> events = output.split('\n');
    for (const auto &event : events)
    {
        QList<QByteArray> packet = event.split(' ');
        if (packet.size() == 4)
        {
            // Finger detection event
            if (packet.at(2) == "0145")
            {
                // If i don't assign this, my IDE says its an error
                // when sticking it in the ternary
                auto b = bool(packet.at(3).toInt());

                // start/stop timer if finger is on/off screen
                b ?
                _wheelTimer->start() :
                _wheelTimer->stop();

            }
            // Finger Y movement event
            if (packet.at(2) == "0036")
            {
                int fingerY = packet.at(3).toInt(nullptr, 16);
                int index = -1;
                db fingerY;

                // Get screen space index
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
                // Larger numbers nearest to neutral zone
                // Smallest numbers further from neutral zone
                index = index > 0 ?
                        _screenDivisionsHalf - index : index < 0 ?
                        -1 * (_screenDivisionsHalf + index): 0;

                updateWheelIndex(index);
            }
        }
    }
}

void PhoneVariableScroll::wheelRun()
{
    _wheelTimer->setInterval(_wheelTimerInterval);
    emit_uinput(_fd, EV_REL, REL_WHEEL, wheelRepeat.direction);
    emit_uinput(_fd, EV_SYN, SYN_REPORT, 0);
}

void PhoneVariableScroll::updateWheelIndex(int index)
{
    // Set wheel direction
    wheelRepeat.direction = index > 0 ? 1:-1;

    // Invert direction if true
    if (_bInvertScrollDirection)
        wheelRepeat.direction *= -1;

    // Patch to prevent quick scroll down on neutral zone
    if(index == 0) wheelRepeat.direction = 0;

    // Remove direction so we just know the speed
    index = abs(index);

    // If we are in a new section
    if(wheelRepeat.index != index)
    {
        // If we're closer to the edges of the phone
        // from the previous section, trigger a wheel
        // to make it more instant feedback
        if(index < wheelRepeat.index)
            wheelRun();

        // We were previous in neutral zone
        // But now we are in scroll zone
        bool bExitNeutral = (wheelRepeat.index == 0);

        // Update index
        wheelRepeat.index = index;

        // wheel neutral zone
        // Stop timer
        if(index == 0)
        {
            _wheelTimer->stop();
            return;
        }
        // Update intervals
        _wheelTimerInterval =_wheelTickRates.at(index-1);

        // Resume timer if we just exited neutral
        if(bExitNeutral) _wheelTimer->start();
    }
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

void PhoneVariableScroll::createInput()
{
    struct uinput_setup usetup;

    _fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);

    // We just need relative scroll wheel
    ioctl(_fd, UI_SET_EVBIT, EV_REL);
    ioctl(_fd, UI_SET_RELBIT, REL_WHEEL);

    memset(&usetup, 0, sizeof(usetup));
    usetup.id.bustype = BUS_USB;
    usetup.id.vendor = 0x1234;
    usetup.id.product = 0x5678;
    strcpy(usetup.name, "PhoneVariableScroll-VirtualWheel");

    ioctl(_fd, UI_DEV_SETUP, &usetup);
    ioctl(_fd, UI_DEV_CREATE);
}

void PhoneVariableScroll::createScreenSize()
{
    QProcess * getScreenSize = new QProcess();
    // converts adb output (example: 'Physical size: 1080x2160') into a QPoint
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

        // Boundaries of neutral zone based on screen size
        int low = _screenSize.y()/2 - WHEEL_NEUTRAL_ZONE_PX/2;
        int high = low + WHEEL_NEUTRAL_ZONE_PX;

        // Setup linspaces from below and above neutral zone
        auto lowSpaces = linspace(0,low,SCREEN_DIVISIONS/2);
        auto highSpaces = linspace(high, _screenSize.y(),SCREEN_DIVISIONS/2);

        // Save linspaces.
        // Neutral zone doesn't need to be saved,
        // since it's out of the spaces
        _screenSpaces = lowSpaces + highSpaces;

        // Setup wheel tick speed based on linspace sections
        _wheelTickRates = linspace(WHEEL_TICK_RATE_MIN,WHEEL_TICK_RATE_MAX,SCREEN_DIVISIONS/2-1);
    });

    getScreenSize->start("adb", QStringList() << "shell" << "wm" << "size");
}

void PhoneVariableScroll::on_checkBox_clicked(bool checked)
{
    _bInvertScrollDirection = checked;
}

