#include <QApplication>
#include <QFile>
#include <QLabel>
#include <QProgressBar>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>
#include <QTextStream>
#include <fstream>
#include <string>

class SystemMonitor : public QWidget
{
    Q_OBJECT

public:
    SystemMonitor(QWidget *parent = nullptr)
        : QWidget(parent), cpuLabel(new QLabel(this)), cpuBar(new QProgressBar(this)), ramTotalLabel(new QLabel(this)), ramAvailableLabel(new QLabel(this)), ramFreeLabel(new QLabel(this)), timer(new QTimer(this)), prevTotal(0), prevIdle(0)
    {
        cpuBar->setRange(0, 100);

        QVBoxLayout *layout = new QVBoxLayout(this);
        layout->addWidget(cpuLabel);
        layout->addWidget(cpuBar);
        layout->addWidget(ramTotalLabel);
        layout->addWidget(ramAvailableLabel);
        layout->addWidget(ramFreeLabel);
        setLayout(layout);
        setWindowTitle("My Spec");
        resize(360, 200);

        connect(timer, &QTimer::timeout, this, &SystemMonitor::updateUsage);
        timer->start(1000);
        updateUsage();
    }

private:
    QLabel *cpuLabel;
    QProgressBar *cpuBar;
    QLabel *ramTotalLabel;
    QLabel *ramAvailableLabel;
    QLabel *ramFreeLabel;
    QTimer *timer;
    quint64 prevTotal;
    quint64 prevIdle;

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

    QString readMemInfoValue(const QString &key) const
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

    quint64 parseCpuValue(const QStringList &tokens, int index) const
    {
        if (index < 0 || index >= tokens.size())
            return 0;
        bool ok = false;
        quint64 value = tokens.at(index).toULongLong(&ok);
        return ok ? value : 0;
    }

    void updateUsage()
    {
        int cpuUsage = readCpuUsage();
        const QString totalRam = convertKBtoGB(readMemInfoValue("MemTotal"));
        const QString availableRam = convertKBtoGB(readMemInfoValue("MemAvailable"));
        const QString freeRam = convertKBtoGB(readMemInfoValue("MemFree"));

        cpuLabel->setText(QString("CPU Usage: %1 %").arg(cpuUsage));
        cpuBar->setValue(cpuUsage);
        ramTotalLabel->setText(QString("RAM Total: %1").arg(totalRam));
        ramAvailableLabel->setText(QString("RAM Available: %1").arg(availableRam));
        ramFreeLabel->setText(QString("RAM Free: %1").arg(freeRam));
    }

    int readCpuUsage()
    {
        QFile file("/proc/stat");
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
            return 0;

        QTextStream in(&file);
        const QString line = in.readLine().trimmed();
        QStringList tokens = line.split(' ', Qt::SkipEmptyParts);
        if (tokens.size() < 5 || tokens.at(0) != "cpu")
            return 0;

        quint64 user = parseCpuValue(tokens, 1);
        quint64 nice = parseCpuValue(tokens, 2);
        quint64 system = parseCpuValue(tokens, 3);
        quint64 idle = parseCpuValue(tokens, 4);
        quint64 iowait = parseCpuValue(tokens, 5);
        quint64 irq = parseCpuValue(tokens, 6);
        quint64 softirq = parseCpuValue(tokens, 7);
        quint64 steal = parseCpuValue(tokens, 8);

        quint64 total = user + nice + system + idle + iowait + irq + softirq + steal;
        quint64 busy = total - idle;

        quint64 deltaTotal = total - prevTotal;
        quint64 deltaIdle = idle - prevIdle;

        prevTotal = total;
        prevIdle = idle;

        if (deltaTotal == 0)
            return 0;

        int usage = static_cast<int>((100 * (deltaTotal - deltaIdle)) / deltaTotal);
        return qBound(0, usage, 100);
    }
};

#include "main.moc"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    SystemMonitor monitor;
    monitor.show();
    return app.exec();
}
