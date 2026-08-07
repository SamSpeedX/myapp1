#pragma once

#include <QString>

class SettingsManager
{
public:
    SettingsManager();

    int cpuAlertThreshold() const;
    void setCpuAlertThreshold(int value);

    int ramAlertThreshold() const;
    void setRamAlertThreshold(int value);

    int pollIntervalSeconds() const;
    void setPollIntervalSeconds(int value);

private:
    static constexpr const char *ORG = "MySpec";
    static constexpr const char *APP = "MySpecMonitor";
};
