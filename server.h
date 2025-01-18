#ifndef SERVER_H
#define SERVER_H

#include <QString>
#include <QColor>
#include "vector2d.h"

struct Server {
    QString name;
    Vector2D position;
    QColor color;
};

#endif // SERVER_H
