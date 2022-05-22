#include "xkeys.h"
#define db qDebug() << this <<
xkeys::xkeys(QObject * parent) : QObject(parent)
{
  key_setup();
}

void xkeys::key_down(int iKeyCode)
{
//  key_setup();
  // Send a fake key press event to the window.
  XKeyEvent event = createKeyEvent(true, iKeyCode);
  XTestFakeKeyEvent(xDisplay, event.keycode, True, 0);
  XFlush(xDisplay);
//  XCloseDisplay(xDisplay);
}

void xkeys::key_up(int iKeyCode)
{
//  key_setup();
  // Send a fake key release event to the window.
  XKeyEvent event = createKeyEvent(false, iKeyCode);
  XTestFakeKeyEvent(xDisplay, event.keycode, False, 0);
  XFlush(xDisplay);
//  XCloseDisplay(xDisplay);
}

void xkeys::key_setup()
{
  xDisplay = XOpenDisplay(0);

  if(xDisplay == NULL)
    qWarning() << "Unable to open XxDisplay";

  // Get the root window for the current xDisplay.
  xWinRoot = XDefaultRootWindow(xDisplay);
  // Find the window which has the current keyboard focus.
  XGetInputFocus(xDisplay, &xWinFocus, &xRevert);
}

XKeyEvent xkeys::createKeyEvent( bool press, int keycode)
{
  XKeyEvent event;
  event.display     = xDisplay;
  event.window      = xWinFocus;
  event.root        = xWinRoot;
  event.subwindow   = 0;
  event.time        = CurrentTime;
  event.x           = 1;
  event.y           = 1;
  event.x_root      = 1;
  event.y_root      = 1;
  event.same_screen = True;
  event.keycode     = XKeysymToKeycode(xDisplay,  keycode);
  event.state       = 0;
  db "Event Keycode - " << event.keycode << ", from: " <<  keycode;

  if(press)
    event.type = 2;
  else
    event.type = 3;

  return event;
}
