#include "system/process.hpp"
#include <QDir>
#include <QFile>
#include <QStringList>
#include <QTextStream>

static bool readProcessStats(int pid, QString &name, long &memoryKb)
{
    QFile cmdline(QString("/proc/%1/cmdline").arg(pid));
    if (!cmdline.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;

    QByteArray raw = cmdline.readAll();
    if (raw.isEmpty())
        return false;

    name = QString::fromUtf8(raw).split('\0').first();
    if (name.isEmpty())
        name = QString::number(pid);

    QFile status(QString("/proc/%1/status").arg(pid));
    if (!status.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;

    QTextStream in(&status);
    while (!in.atEnd())
    {
        const QString line = in.readLine();
        if (line.startsWith("VmRSS:"))
        {
            const QStringList parts = line.simplified().split(' ', Qt::SkipEmptyParts);
            if (parts.size() >= 2)
                memoryKb = parts.at(1).toLongLong();
            break;
        }
    }
    return true;
}

QVector<ProcessInfo> ProcessMonitor::list()
{
    QVector<ProcessInfo> processes;
    QDir procDir("/proc");
    const QStringList entries = procDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (const QString &entry : entries)
    {
        bool isPid = false;
        int pid = entry.toInt(&isPid);
        if (!isPid)
            continue;

        QString name;
        long memoryKb = 0;
        if (readProcessStats(pid, name, memoryKb))
        {
            processes.append({pid, name, 0.0, memoryKb});
        }
    }
    return processes;
}
