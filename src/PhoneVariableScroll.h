#ifndef PHONEVARIABLESCROLL_H
#define PHONEVARIABLESCROLL_H

#include <QMainWindow>
#include <QProcess>
#include <QTimer>
#include <vector>

#include "debug.h"

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
    std::vector<float> linspacer(float start, float end, size_t points);
private slots:

    void readAdbShell();
private:
    Ui::PhoneVariableScroll *ui;
    QTimer * _wheelTimer;
};
#endif // PHONEVARIABLESCROLL_H
