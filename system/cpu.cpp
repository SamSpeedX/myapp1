#include "system/cpu.hpp"
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>

static quint64 parseCpuValue(const QStringList &tokens, int index)
{
    if (index < 0 || index >= tokens.size())
        return 0;
    bool ok = false;
    quint64 value = tokens.at(index).toULongLong(&ok);
    return ok ? value : 0;
}

QString CPU::usage()
{
    QFile file("/proc/stat");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return QStringLiteral("Unknown");

    QTextStream in(&file);
    const QString line = in.readLine().trimmed();
    QStringList tokens = line.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
    if (tokens.size() < 5 || tokens.at(0) != "cpu")
        return QStringLiteral("Unknown");

    static quint64 prevTotal = 0;
    static quint64 prevIdle = 0;

    quint64 user = parseCpuValue(tokens, 1);
    quint64 nice = parseCpuValue(tokens, 2);
    quint64 system = parseCpuValue(tokens, 3);
    quint64 idle = parseCpuValue(tokens, 4);
    quint64 iowait = parseCpuValue(tokens, 5);
    quint64 irq = parseCpuValue(tokens, 6);
    quint64 softirq = parseCpuValue(tokens, 7);
    quint64 steal = parseCpuValue(tokens, 8);

    quint64 total = user + nice + system + idle + iowait + irq + softirq + steal;
    quint64 deltaTotal = total - prevTotal;
    quint64 deltaIdle = idle - prevIdle;

    prevTotal = total;
    prevIdle = idle;

    if (deltaTotal == 0)
        return QStringLiteral("0 %");

    int usage = static_cast<int>((100 * (deltaTotal - deltaIdle)) / deltaTotal);
    return QString::number(qBound(0, usage, 100)) + " %";
}
