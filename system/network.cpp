#include "system/network.hpp"
#include <fstream>
#include <sstream>
#include <string>

static QString convertKBtoMB(double kb)
{
    double mb = kb / 1024.0;
    return QString::number(mb, 'f', 2) + " MB";
}

static bool readNetworkTotals(unsigned long long &rx, unsigned long long &tx)
{
    std::ifstream file("/proc/net/dev");
    if (!file.is_open())
        return false;

    std::string line;
    // skip headers
    std::getline(file, line);
    std::getline(file, line);

    rx = 0;
    tx = 0;
    while (std::getline(file, line))
    {
        auto pos = line.find(':');
        if (pos == std::string::npos)
            continue;

        std::string data = line.substr(pos + 1);
        std::istringstream ss(data);
        unsigned long long ifaceRx = 0;
        unsigned long long ifaceTx = 0;
        ss >> ifaceRx;
        for (int i = 0; i < 7; ++i)
            ss >> std::ws >> std::skipws;
        ss >> ifaceTx;

        rx += ifaceRx;
        tx += ifaceTx;
    }
    return true;
}

QString Network::receive()
{
    unsigned long long rx = 0;
    unsigned long long tx = 0;
    if (!readNetworkTotals(rx, tx))
        return QStringLiteral("Unknown");
    return convertKBtoMB(static_cast<double>(rx) / 1024.0);
}

QString Network::transmit()
{
    unsigned long long rx = 0;
    unsigned long long tx = 0;
    if (!readNetworkTotals(rx, tx))
        return QStringLiteral("Unknown");
    return convertKBtoMB(static_cast<double>(tx) / 1024.0);
}
