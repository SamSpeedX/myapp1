#include <QApplication>
#include <QFile>
#include <QHeaderView>
#include <QLabel>
#include <QProgressBar>
#include <QTableWidget>
#include <QTabWidget>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>
#include <QTextStream>
#include <fstream>
#include <string>
#include "system/cpu.hpp"
#include "system/ram.hpp"
#include "system/disk.hpp"
#include "system/network.hpp"
#include "system/process.hpp"

class SystemMonitor : public QWidget
{
    Q_OBJECT

public:
    SystemMonitor(QWidget *parent = nullptr)
        : QWidget(parent), cpuLabel(new QLabel(this)), cpuBar(new QProgressBar(this)), ramTotalLabel(new QLabel(this)), ramAvailableLabel(new QLabel(this)), ramFreeLabel(new QLabel(this)), diskTotalLabel(new QLabel(this)), diskUsedLabel(new QLabel(this)), diskFreeLabel(new QLabel(this)), netRxLabel(new QLabel(this)), netTxLabel(new QLabel(this)), processTable(new QTableWidget(this)), tabWidget(new QTabWidget(this)), timer(new QTimer(this)), prevTotal(0), prevIdle(0)
    {
        cpuBar->setRange(0, 100);

        QWidget *overviewTab = new QWidget(this);
        QVBoxLayout *overviewLayout = new QVBoxLayout(overviewTab);
        overviewLayout->addWidget(cpuLabel);
        overviewLayout->addWidget(cpuBar);
        overviewLayout->addWidget(ramTotalLabel);
        overviewLayout->addWidget(ramAvailableLabel);
        overviewLayout->addWidget(ramFreeLabel);
        overviewLayout->addWidget(diskTotalLabel);
        overviewLayout->addWidget(diskUsedLabel);
        overviewLayout->addWidget(diskFreeLabel);
        overviewLayout->addWidget(netRxLabel);
        overviewLayout->addWidget(netTxLabel);

        processTable->setColumnCount(4);
        processTable->setHorizontalHeaderLabels({"PID", "Name", "Memory (KB)", "CPU (%)"});
        processTable->horizontalHeader()->setStretchLastSection(true);

        QWidget *processTab = new QWidget(this);
        QVBoxLayout *processLayout = new QVBoxLayout(processTab);
        processLayout->addWidget(processTable);

        tabWidget->addTab(overviewTab, "Overview");
        tabWidget->addTab(processTab, "Processes");

        QVBoxLayout *layout = new QVBoxLayout(this);
        layout->addWidget(tabWidget);
        setLayout(layout);
        setWindowTitle("My Spec");
        resize(640, 480);

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
    QLabel *diskTotalLabel;
    QLabel *diskUsedLabel;
    QLabel *diskFreeLabel;
    QLabel *netRxLabel;
    QLabel *netTxLabel;
    QTableWidget *processTable;
    QTabWidget *tabWidget;
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
        const QString cpuUsageText = CPU::usage();
        const QString totalRam = RAM::total();
        const QString availableRam = RAM::available();
        const QString freeRam = RAM::free();
        const QString totalDisk = Disk::total();
        const QString usedDisk = Disk::used();
        const QString freeDisk = Disk::free();
        const QString rx = Network::receive();
        const QString tx = Network::transmit();
        const QVector<ProcessInfo> processes = ProcessMonitor::list();

        cpuLabel->setText(QString("CPU Usage: %1").arg(cpuUsageText));
        QString cpuValueString = cpuUsageText;
        cpuValueString.remove(" %");
        cpuBar->setValue(cpuValueString.toInt());
        ramTotalLabel->setText(QString("RAM Total: %1").arg(totalRam));
        ramAvailableLabel->setText(QString("RAM Available: %1").arg(availableRam));
        ramFreeLabel->setText(QString("RAM Free: %1").arg(freeRam));
        diskTotalLabel->setText(QString("Disk Total: %1").arg(totalDisk));
        diskUsedLabel->setText(QString("Disk Used: %1").arg(usedDisk));
        diskFreeLabel->setText(QString("Disk Free: %1").arg(freeDisk));
        netRxLabel->setText(QString("Network Receive: %1").arg(rx));
        netTxLabel->setText(QString("Network Transmit: %1").arg(tx));

        processTable->setRowCount(processes.size());
        for (int row = 0; row < processes.size(); ++row)
        {
            const ProcessInfo &info = processes[row];
            processTable->setItem(row, 0, new QTableWidgetItem(QString::number(info.pid)));
            processTable->setItem(row, 1, new QTableWidgetItem(info.name));
            processTable->setItem(row, 2, new QTableWidgetItem(QString::number(info.memoryKb)));
            processTable->setItem(row, 3, new QTableWidgetItem(QString::number(info.cpu, 'f', 1)));
        }
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
