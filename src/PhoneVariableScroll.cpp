#include "PhoneVariableScroll.h"
#include "ui_PhoneVariableScroll.h"

PhoneVariableScroll::PhoneVariableScroll(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::PhoneVariableScroll)
{
    ui->setupUi(this);


    ui->spinBox_Divisor->setValue(_phoneVariables.SCREEN_DIVISIONS);
    ui->spinBox_Deadzone->setValue(_phoneVariables.WHEEL_NEUTRAL_ZONE_PX);
    ui->spinBox_ScrollLow->setValue(_phoneVariables.WHEEL_TICK_RATE_MAX);
    ui->spinBox_ScrollHigh->setValue(_phoneVariables.WHEEL_TICK_RATE_MIN);

    createInput();

    _adbProcessPipe = new QProcess(this);
    connect(_adbProcessPipe, &QProcess::readyRead, this, &PhoneVariableScroll::readAdbShell);
    connect(_adbProcessPipe, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), this, &PhoneVariableScroll::adbFinished);

    _wheelTimer = new QTimer(this);
    connect(_wheelTimer, &QTimer::timeout, this, &PhoneVariableScroll::wheelRun);

    on_pushButton_clicked();
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
    while (i < points)
        res << start + step * i++;

    return res;
}
void PhoneVariableScroll::readAdbShell()
{
    if (!_bStatusMessages)
    {
        _bStatusMessages = !_bStatusMessages;
        ui->statusbar->showMessage("ADB Bridge Connected");

        createScreenSize();
        updateGUI();
    }
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

                // Draw on GUI
                // Expensive, but fun
                if (_fingerY != fingerY)
                {
                    _fingerY = fingerY;
                    updateGUI(false);
                }

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

void PhoneVariableScroll::adbFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    ui->pushButton->setEnabled(true);
    ui->statusbar->showMessage("ADB bridge offline");
    _bStatusMessages = false;
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

void PhoneVariableScroll::generateLinspace()
{
    // Boundaries of neutral zone based on screen size
    int low = _screenSize.y()/2 - _phoneVariables.WHEEL_NEUTRAL_ZONE_PX/2;
    int high = low + _phoneVariables.WHEEL_NEUTRAL_ZONE_PX;

    // For drawing
    _deadZone = QPoint(low,high);

    // Setup linspaces from below and above neutral zone
    auto lowSpaces = linspace(0,low,_phoneVariables.SCREEN_DIVISIONS/2);
    auto highSpaces = linspace(high, _screenSize.y(),_phoneVariables.SCREEN_DIVISIONS/2);

    // Save linspaces.
    // Neutral zone doesn't need to be saved,
    // since it's out of the spaces
    _screenSpaces = lowSpaces + highSpaces;

    _screenDivisionsHalf = _phoneVariables.SCREEN_DIVISIONS / 2;
    // Setup wheel tick speed based on linspace sections
    _wheelTickRates = linspace(_phoneVariables.WHEEL_TICK_RATE_MIN,_phoneVariables.WHEEL_TICK_RATE_MAX,_phoneVariables.SCREEN_DIVISIONS/2-1);

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
        {
            qw "Screen size requested from ADB was wrong. Command requested: adb shell wm size";
            return;
        }

        updateGUI();
//        generateLinspace();

    });

    getScreenSize->start("adb", QStringList() << "shell" << "wm" << "size");
}

void PhoneVariableScroll::updateGUI(bool bWithLinspace)
{
    // Prevents dividing by 0 when
    // too early or d/c'd
    if (_screenSize.y() == 0)
    {
//        QTimer::singleShot(100, this, &PhoneVariableScroll::updateGUI);
        return;
    }

    // Load pixmap
    QPixmap phone = QPixmap(":/images/resources/phone.png");
    if(bWithLinspace)
        generateLinspace();
//    db _screenSpaces;

    double ratio = static_cast<double>(phone.height()) / static_cast<double>(_screenSize.y());

//    db ratio;
//    db phone.height();
//    db _screenSize;
    // (0, 232.5, 465,  697.5, 930, 1230, 1462.5, 1695, 1927.5, 2160)

    //(0, -37.5, -75, -112.5, -150, 150, 112.5, 75, 37.5, 0)
    // Experimentally found in image editor (part of the phone image that's display)
    int bottomMargin = 45;
    int topMargin = 47;
    int sideMargin = 10;

    QPoint deadZone = _deadZone * ratio;

    QPainter *paint = new QPainter(&phone);
    paint->fillRect(sideMargin-1,deadZone.x(),phone.width()-sideMargin*2+2,deadZone.y()-deadZone.x(),QColor(255,34,255,255));

    if (_fingerY != 0)
    {
        auto rawLoc = _fingerY*ratio;
        rawLoc *= (400-45-47);
        double fingerY = rawLoc / 400;
        fingerY += bottomMargin;
        db fingerY;
        // Create 12px square around finger
        int fatFinger = 12;
        fingerY -= fatFinger/2;
        paint->fillRect(phone.width()/2 - fatFinger/2,
                        fingerY,
                        fatFinger,
                        fatFinger,
                        Qt::darkGreen);
    }
    paint->setPen(*(new QColor(Qt::darkBlue)));
    //(0, 232.5, 465,  697.5, 930, 1230, 1462.5, 1695, 1927.5, 2160)
    // Need to make new screenspace for image
    // Setup linspaces from below and above neutral zone
    auto lowSpaces = linspace(bottomMargin,deadZone.x(),_screenDivisionsHalf);
    auto highSpaces = linspace(deadZone.y(), phone.height()-topMargin,_screenDivisionsHalf);

    // Save linspaces.
    // Neutral zone doesn't need to be saved,
    // since it's out of the spaces
    auto screenSpaces = lowSpaces + highSpaces;
    for (auto screenSpace : screenSpaces)
    {
        // Shrink to phone image size
//        screenSpace *= ratio;
//        db screenSpace;
        paint->fillRect(sideMargin-1,screenSpace,phone.width()-sideMargin*2,2, Qt::darkBlue);

    }
    ui->label_Image->setPixmap(phone);
}

void PhoneVariableScroll::on_checkBox_clicked(bool checked)
{
    _bInvertScrollDirection = checked;
}


void PhoneVariableScroll::on_pushButton_clicked()
{
    ui->statusbar->showMessage("Connecting to ADB Bridge...");
    _adbProcessPipe->start("adb", QStringList() << "shell" << "getevent");
    ui->pushButton->setEnabled(false);
}


void PhoneVariableScroll::on_spinBox_Divisor_valueChanged(int arg1)
{
    _phoneVariables.SCREEN_DIVISIONS = arg1;
    updateGUI();
}


void PhoneVariableScroll::on_spinBox_Deadzone_valueChanged(int arg1)
{
    _phoneVariables.WHEEL_NEUTRAL_ZONE_PX = arg1;
    updateGUI();
}


void PhoneVariableScroll::on_spinBox_ScrollLow_valueChanged(int arg1)
{
    _phoneVariables.WHEEL_TICK_RATE_MAX = arg1;
    updateGUI();
}


void PhoneVariableScroll::on_spinBox_ScrollHigh_valueChanged(int arg1)
{
    _phoneVariables.WHEEL_TICK_RATE_MIN = arg1;
    updateGUI();
}

