#pragma once

#include <QString>

class AlertManager
{
public:
    static QString cpuAlert(int usage, int threshold);
    static QString ramAlert(int usage, int threshold);
};
