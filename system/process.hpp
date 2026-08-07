#pragma once

#include <QString>
#include <QVector>

struct ProcessInfo
{
    int pid;
    QString name;
    double cpu;
    long memoryKb;
};

class ProcessMonitor
{
public:
    static QVector<ProcessInfo> list();
};
