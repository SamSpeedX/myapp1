#include "system/history.hpp"

void HistoryManager::addSample(int cpuUsage, double ramUsedGb, double netRxMb, double netTxMb)
{
    SamplePoint sample;
    sample.timestamp = QDateTime::currentDateTime();
    sample.cpuUsage = cpuUsage;
    sample.ramUsedGb = ramUsedGb;
    sample.netRxMb = netRxMb;
    sample.netTxMb = netTxMb;

    m_samples.append(sample);
    while (m_samples.size() > 120)
        m_samples.removeFirst();
}

QVector<SamplePoint> HistoryManager::samples() const
{
    return m_samples;
}
