#include "system/alerts.hpp"

QString AlertManager::cpuAlert(int usage, int threshold)
{
    if (usage >= threshold)
        return QString("CPU usage is high (%1%).").arg(usage);
    return QString();
}

QString AlertManager::ramAlert(int usage, int threshold)
{
    if (usage >= threshold)
        return QString("RAM usage is high (%1%).").arg(usage);
    return QString();
}
