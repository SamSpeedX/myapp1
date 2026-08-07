#include "system/disk.hpp"
#include <sys/statvfs.h>

static QString convertBytesToGB(unsigned long long bytes)
{
    double gb = static_cast<double>(bytes) / (1024.0 * 1024.0 * 1024.0);
    return QString::number(gb, 'f', 2) + " GB";
}

static bool queryDisk(const QString &path, unsigned long long &total, unsigned long long &free)
{
    struct statvfs fs;
    if (statvfs(path.toUtf8().constData(), &fs) != 0)
        return false;

    total = static_cast<unsigned long long>(fs.f_blocks) * fs.f_frsize;
    free = static_cast<unsigned long long>(fs.f_bfree) * fs.f_frsize;
    return true;
}

QString Disk::total()
{
    unsigned long long total = 0;
    unsigned long long free = 0;
    if (!queryDisk("/", total, free))
        return QStringLiteral("Unknown");
    return convertBytesToGB(total);
}

QString Disk::used()
{
    unsigned long long total = 0;
    unsigned long long free = 0;
    if (!queryDisk("/", total, free))
        return QStringLiteral("Unknown");
    return convertBytesToGB(total - free);
}

QString Disk::free()
{
    unsigned long long total = 0;
    unsigned long long free = 0;
    if (!queryDisk("/", total, free))
        return QStringLiteral("Unknown");
    return convertBytesToGB(free);
}
