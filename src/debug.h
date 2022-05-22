#ifndef DEBUG_H
#define DEBUG_H
#include <QDebug>
#define db qDebug() << this <<
#define qw qWarning() << this <<
#endif // DEBUG_H
