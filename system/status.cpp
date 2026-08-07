#include "system/status.hpp"

QString SystemStatus::summary(int cpuUsage, int ramUsage)
{
    if (cpuUsage < 50 && ramUsage < 50)
        return QStringLiteral("System status is good.");
    if (cpuUsage < 80 && ramUsage < 80)
        return QStringLiteral("System status is moderate.");
    return QStringLiteral("System status is high.");
}
