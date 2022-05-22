#ifndef XKEYS_H
#define XKEYS_H

// Simulate Key Press in Linux
// Adapated from Adam Pierce - http://www.doctort.org/adam/

#include <QObject>
#include <QDebug>

#include <X11/Xlib.h>
#include <X11/extensions/XTest.h>
#undef Bool
#undef CursorShape
#undef Expose
#undef KeyPress
#undef KeyRelease
#undef FocusIn
#undef FocusOut
#undef FontChange
#undef None
#undef Status
#undef Unsorted


class xkeys : public QObject
{
    Q_OBJECT
public:
    explicit xkeys(QObject * parent = 0);
    XKeyEvent createKeyEvent(bool press, int keycode);

signals:

public slots:
    // check diks
    void pressKey_Dik(int iKeyCode)
    {
        key_down(iKeyCode);
    };
    void releaseKey_Dik(int iKeyCode)
    {
        key_up(iKeyCode);
    };
    void key_down(int iKeyCode);
    void key_up(int iKeyCode);
private slots:
    void key_setup();

private:
    Display * xDisplay ;
    Window xWinRoot;
    Window xWinFocus;
    int    xRevert;
};

#endif // XKEYS_H
