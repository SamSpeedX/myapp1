#pragma once

#include <QString>

class SystemStatus
{
public:
    static QString summary(int cpuUsage, int ramUsage);
};
