#ifndef PHONEVARIABLESCROLL_H
#define PHONEVARIABLESCROLL_H

#include <QMainWindow>
#include <QProcess>
#include <QTimer>
#include <vector>

#include "debug.h"
#include "CONFIG.h"
#include <uInput.hpp>

using namespace uInputPlus;
QT_BEGIN_NAMESPACE
namespace Ui
{
class PhoneVariableScroll;
}
QT_END_NAMESPACE

class PhoneVariableScroll : public QMainWindow
{
    Q_OBJECT

public:
    PhoneVariableScroll(QWidget *parent = nullptr);
    ~PhoneVariableScroll();
    QList<float> linspace(float start, float end, int points);
private slots:

    void readAdbShell();
private:
    Ui::PhoneVariableScroll *ui;
    QTimer * _wheelTimer;
    QProcess * _annoying;
    bool _bFingerActive = false;
    QPoint _screenSize;


    // This is... this isn't right is it?
    const int _screenDivisionsHalf = SCREEN_DIVISIONS / 2;
    QList<float> _screenSpaces;
    uInput * _input;
};
#endif // PHONEVARIABLESCROLL_H
