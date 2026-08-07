#pragma once

#include <QString>

class Disk
{
public:
    static QString total();
    static QString used();
    static QString free();
};
