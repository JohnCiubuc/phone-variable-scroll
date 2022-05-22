#ifndef PHONEVARIABLESCROLL_H
#define PHONEVARIABLESCROLL_H

#include <QMainWindow>
#include <QProcess>
#include <QTimer>
#include <vector>

#include "debug.h"
#include "CONFIG.h"

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
    std::vector<float> linspace(float start, float end, size_t points);
private slots:

    void readAdbShell();
    void createScreenSpaces();
private:
    Ui::PhoneVariableScroll *ui;
    QTimer * _wheelTimer;
    QProcess * _annoying;
    bool _bFingerActive = false;
    QPoint _screenSize;
};
#endif // PHONEVARIABLESCROLL_H
