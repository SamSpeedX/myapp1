#include "system/settings.hpp"
#include <QSettings>

SettingsManager::SettingsManager()
{
}

int SettingsManager::cpuAlertThreshold() const
{
    QSettings settings(ORG, APP);
    return settings.value("alerts/cpu", 90).toInt();
}

void SettingsManager::setCpuAlertThreshold(int value)
{
    QSettings settings(ORG, APP);
    settings.setValue("alerts/cpu", value);
}

int SettingsManager::ramAlertThreshold() const
{
    QSettings settings(ORG, APP);
    return settings.value("alerts/ram", 90).toInt();
}

void SettingsManager::setRamAlertThreshold(int value)
{
    QSettings settings(ORG, APP);
    settings.setValue("alerts/ram", value);
}

int SettingsManager::pollIntervalSeconds() const
{
    QSettings settings(ORG, APP);
    return settings.value("poll/interval", 1).toInt();
}

void SettingsManager::setPollIntervalSeconds(int value)
{
    QSettings settings(ORG, APP);
    settings.setValue("poll/interval", value);
}
