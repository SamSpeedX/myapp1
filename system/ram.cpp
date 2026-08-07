#include "system/ram.hpp"
#include <fstream>
#include <string>

static QString convertKBtoGB(const QString &value)
{
    int pos = 0;
    while (pos < value.size() && !value[pos].isDigit() && value[pos] != '-')
        pos++;

    bool ok = false;
    long long kb = value.mid(pos).toLongLong(&ok);
    if (!ok)
        return value;

    double gb = static_cast<double>(kb) / (1024.0 * 1024.0);
    return QString::number(gb, 'f', 2) + " GB";
}

static QString readMemInfoValue(const QString &key)
{
    std::ifstream file("/proc/meminfo");
    if (!file.is_open())
        return QStringLiteral("Unknown");

    std::string line;
    const std::string target = key.toStdString() + ":";
    while (std::getline(file, line))
    {
        if (line.rfind(target, 0) == 0)
        {
            std::string value = line.substr(target.size());
            return QString::fromStdString(value);
        }
    }
    return QStringLiteral("Unknown");
}

QString RAM::total()
{
    return convertKBtoGB(readMemInfoValue("MemTotal"));
}

QString RAM::available()
{
    return convertKBtoGB(readMemInfoValue("MemAvailable"));
}

QString RAM::free()
{
    return convertKBtoGB(readMemInfoValue("MemFree"));
}
