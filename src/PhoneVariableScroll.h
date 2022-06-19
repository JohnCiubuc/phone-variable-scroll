#ifndef PHONEVARIABLESCROLL_H
#define PHONEVARIABLESCROLL_H

#include <QMainWindow>
#include <QProcess>
#include <QTimer>
#include <vector>

#include <linux/uinput.h>
#include <cstring>
#include <thread>
#include <fstream>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <QPainter>

#define DEBUG

#include "debug.h"
#include "CONFIG.h"



QT_BEGIN_NAMESPACE
namespace Ui
{
class PhoneVariableScroll;

}
QT_END_NAMESPACE

struct phoneVariables_s
{
    int SCREEN_DIVISIONS = 10;
    int WHEEL_TICK_RATE_MIN = 20;
    int WHEEL_TICK_RATE_MAX = 200;
    int WHEEL_NEUTRAL_ZONE_PX = 300;
} ;

class PhoneVariableScroll : public QMainWindow
{
    Q_OBJECT

    struct wheelRepeat_s
    {
        int index;
        int direction;
    } wheelRepeat;

public:
    PhoneVariableScroll(QWidget *parent = nullptr);
    ~PhoneVariableScroll();
    QList<float> linspace(float start, float end, int points);
private slots:

    void updateGUI();
    void generateLinspace();
    void readAdbShell();
    void adbFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void wheelRun();
    void updateWheelIndex(int);


    void emit_uinput(int fd, int type, int code, int val);
    void on_checkBox_clicked(bool checked);

    void on_pushButton_clicked();

private:
    void createInput();
    void createScreenSize();
    Ui::PhoneVariableScroll *ui;
    QTimer * _wheelTimer;
    QProcess * _adbProcessPipe;
    bool _bFingerActive = false;
    QPoint _screenSize;


    // This is... this isn't right is it?
    int _screenDivisionsHalf;
    QList<float> _screenSpaces;
    QList<float> _wheelTickRates;
    int _fd;


    int _wheelTimerInterval;
    bool _bInvertScrollDirection = false;
    bool _bStatusMessages = false;

    phoneVariables_s _phoneVariables;
    int _fingerY = 0;
};
#endif // PHONEVARIABLESCROLL_H
