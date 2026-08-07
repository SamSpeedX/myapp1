#pragma once

#include <QVector>
#include <QDateTime>

struct SamplePoint
{
    QDateTime timestamp;
    int cpuUsage;
    double ramUsedGb;
    double netRxMb;
    double netTxMb;
};

class HistoryManager
{
public:
    void addSample(int cpuUsage, double ramUsedGb, double netRxMb, double netTxMb);
    QVector<SamplePoint> samples() const;

private:
    QVector<SamplePoint> m_samples;
};
