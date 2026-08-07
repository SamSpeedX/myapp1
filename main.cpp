#include <QApplication>
#include <QFile>
#include <QFormLayout>
#include <QHeaderView>
#include <QLabel>
#include <QPainter>
#include <QPaintEvent>
#include <QProgressBar>
#include <QPushButton>
#include <QSpinBox>
#include <QTableWidget>
#include <QTabWidget>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>
#include <QTextStream>
#include <fstream>
#include <sstream>
#include <string>
#include "system/alerts.hpp"
#include "system/cpu.hpp"
#include "system/disk.hpp"
#include "system/history.hpp"
#include "system/network.hpp"
#include "system/process.hpp"
#include "system/ram.hpp"
#include "system/settings.hpp"
#include "system/status.hpp"

class HistoryChart : public QWidget
{
    Q_OBJECT

public:
    explicit HistoryChart(QWidget *parent = nullptr)
        : QWidget(parent)
    {
        setMinimumHeight(180);
    }

    void setSamples(const QVector<SamplePoint> &samples)
    {
        m_samples = samples;
        update();
    }

protected:
    void paintEvent(QPaintEvent *event) override
    {
        Q_UNUSED(event)
        QPainter painter(this);
        painter.fillRect(rect(), palette().window());
        painter.setRenderHint(QPainter::Antialiasing, true);

        QRect area = rect().adjusted(10, 10, -10, -10);
        painter.setPen(Qt::gray);
        painter.drawRect(area);

        if (m_samples.isEmpty())
        {
            painter.setPen(Qt::darkGray);
            painter.drawText(area, Qt::AlignCenter, "Waiting for history samples...");
            return;
        }

        double maxValue = 100.0;
        for (const SamplePoint &sample : m_samples)
            maxValue = std::max(maxValue, std::max(sample.ramUsedGb * 15.0, std::max(sample.netRxMb, sample.netTxMb)));

        int count = m_samples.size();
        if (count < 2)
            return;

        auto point = [&](int index, double value)
        {
            double x = area.left() + (area.width() * index) / static_cast<double>(count - 1);
            double y = area.bottom() - (value / maxValue) * area.height();
            return QPointF(x, y);
        };

        painter.setPen(QPen(Qt::red, 2));
        for (int i = 1; i < count; ++i)
        {
            painter.drawLine(point(i - 1, m_samples[i - 1].cpuUsage), point(i, m_samples[i].cpuUsage));
        }

        painter.setPen(QPen(Qt::blue, 2));
        for (int i = 1; i < count; ++i)
        {
            painter.drawLine(point(i - 1, m_samples[i - 1].ramUsedGb * 15.0), point(i, m_samples[i].ramUsedGb * 15.0));
        }

        painter.setPen(QPen(Qt::green, 2));
        for (int i = 1; i < count; ++i)
        {
            painter.drawLine(point(i - 1, m_samples[i - 1].netRxMb), point(i, m_samples[i].netRxMb));
        }

        painter.setPen(QPen(Qt::magenta, 2));
        for (int i = 1; i < count; ++i)
        {
            painter.drawLine(point(i - 1, m_samples[i - 1].netTxMb), point(i, m_samples[i].netTxMb));
        }

        painter.setPen(Qt::black);
        painter.drawText(area.left() + 10, area.top() + 15, "CPU %");
        painter.setPen(Qt::red);
        painter.drawText(area.left() + 10, area.top() + 30, "RAM GB * 15");
        painter.setPen(Qt::green);
        painter.drawText(area.left() + 10, area.top() + 45, "Net RX MB");
        painter.setPen(Qt::magenta);
        painter.drawText(area.left() + 10, area.top() + 60, "Net TX MB");
    }

private:
    QVector<SamplePoint> m_samples;
};

class SystemMonitor : public QWidget
{
    Q_OBJECT

public:
    SystemMonitor(QWidget *parent = nullptr)
        : QWidget(parent), cpuLabel(new QLabel(this)), cpuBar(new QProgressBar(this)), ramTotalLabel(new QLabel(this)), ramAvailableLabel(new QLabel(this)), ramFreeLabel(new QLabel(this)), diskTotalLabel(new QLabel(this)), diskUsedLabel(new QLabel(this)), diskFreeLabel(new QLabel(this)), netRxLabel(new QLabel(this)), netTxLabel(new QLabel(this)), statusLabel(new QLabel(this)), alertLabel(new QLabel(this)), historyChart(new HistoryChart(this)), cpuAlertSpin(new QSpinBox(this)), ramAlertSpin(new QSpinBox(this)), pollIntervalSpin(new QSpinBox(this)), saveSettingsButton(new QPushButton("Save Settings", this)), processTable(new QTableWidget(this)), tabWidget(new QTabWidget(this)), timer(new QTimer(this))
    {
        cpuBar->setRange(0, 100);
        cpuBar->setTextVisible(true);

        QWidget *overviewTab = new QWidget(this);
        QVBoxLayout *overviewLayout = new QVBoxLayout(overviewTab);
        overviewLayout->addWidget(cpuLabel);
        overviewLayout->addWidget(cpuBar);
        overviewLayout->addWidget(statusLabel);
        overviewLayout->addWidget(alertLabel);
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

        QWidget *historyTab = new QWidget(this);
        QVBoxLayout *historyLayout = new QVBoxLayout(historyTab);
        historyLayout->addWidget(new QLabel("History chart (CPU, RAM, Network)", this));
        historyLayout->addWidget(historyChart);

        QWidget *settingsTab = new QWidget(this);
        QFormLayout *settingsLayout = new QFormLayout(settingsTab);
        cpuAlertSpin->setRange(10, 100);
        ramAlertSpin->setRange(10, 100);
        pollIntervalSpin->setRange(1, 60);
        cpuAlertSpin->setValue(settingsManager.cpuAlertThreshold());
        ramAlertSpin->setValue(settingsManager.ramAlertThreshold());
        pollIntervalSpin->setValue(settingsManager.pollIntervalSeconds());
        settingsLayout->addRow("CPU alert threshold (%)", cpuAlertSpin);
        settingsLayout->addRow("RAM alert threshold (%)", ramAlertSpin);
        settingsLayout->addRow("Poll interval (sec)", pollIntervalSpin);
        settingsLayout->addRow(saveSettingsButton);

        tabWidget->addTab(overviewTab, "Overview");
        tabWidget->addTab(processTab, "Processes");
        tabWidget->addTab(historyTab, "History");
        tabWidget->addTab(settingsTab, "Settings");

        QVBoxLayout *layout = new QVBoxLayout(this);
        layout->addWidget(tabWidget);
        setLayout(layout);
        setWindowTitle("My Spec");
        resize(800, 600);

        connect(timer, &QTimer::timeout, this, &SystemMonitor::updateUsage);
        connect(saveSettingsButton, &QPushButton::clicked, this, &SystemMonitor::saveSettings);

        timer->start(settingsManager.pollIntervalSeconds() * 1000);
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
    QLabel *statusLabel;
    QLabel *alertLabel;
    HistoryChart *historyChart;
    QSpinBox *cpuAlertSpin;
    QSpinBox *ramAlertSpin;
    QSpinBox *pollIntervalSpin;
    QPushButton *saveSettingsButton;
    QTableWidget *processTable;
    QTabWidget *tabWidget;
    QTimer *timer;
    SettingsManager settingsManager;
    HistoryManager historyManager;

    static bool parseKbValue(const QString &value, double &kb)
    {
        int pos = 0;
        while (pos < value.size() && !value[pos].isDigit() && value[pos] != '-')
            pos++;
        bool ok = false;
        long long parsedKb = value.mid(pos).toLongLong(&ok);
        if (!ok)
            return false;
        kb = static_cast<double>(parsedKb);
        return true;
    }

    static QString convertKBtoGB(const QString &value)
    {
        double kb = 0.0;
        if (!parseKbValue(value, kb))
            return value.trimmed();

        double gb = kb / (1024.0 * 1024.0);
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

    bool parseValue(const QString &text, double &out) const
    {
        QString copy = text.trimmed();
        int pos = copy.indexOf(' ');
        if (pos > 0)
            copy = copy.left(pos);

        bool ok = false;
        out = copy.toDouble(&ok);
        return ok;
    }

    void saveSettings()
    {
        settingsManager.setCpuAlertThreshold(cpuAlertSpin->value());
        settingsManager.setRamAlertThreshold(ramAlertSpin->value());
        settingsManager.setPollIntervalSeconds(pollIntervalSpin->value());
        timer->start(pollIntervalSpin->value() * 1000);
        statusLabel->setText("Settings saved.");
    }

    void updateUsage()
    {
        const QString cpuUsageText = CPU::usage();
        QString cpuValueString = cpuUsageText;
        cpuValueString.remove(" %");
        int cpuUsage = cpuValueString.toInt();

        const QString totalRam = RAM::total();
        const QString availableRam = RAM::available();
        const QString freeRam = RAM::free();
        const QString totalDisk = Disk::total();
        const QString usedDisk = Disk::used();
        const QString freeDisk = Disk::free();
        const QString rx = Network::receive();
        const QString tx = Network::transmit();
        const QVector<ProcessInfo> processes = ProcessMonitor::list();

        double totalKb = 0.0;
        double availableKb = 0.0;
        int ramUsage = 0;
        if (parseKbValue(readMemInfoValue("MemTotal"), totalKb) && parseKbValue(readMemInfoValue("MemAvailable"), availableKb) && totalKb > 0)
        {
            ramUsage = static_cast<int>(((totalKb - availableKb) * 100.0) / totalKb);
        }

        double usedGb = 0.0;
        parseValue(totalRam, usedGb);
        double rxMb = 0.0;
        double txMb = 0.0;
        parseValue(rx, rxMb);
        parseValue(tx, txMb);

        historyManager.addSample(cpuUsage, (totalKb - availableKb) / (1024.0 * 1024.0), rxMb, txMb);
        historyChart->setSamples(historyManager.samples());

        cpuLabel->setText(QString("CPU Usage: %1").arg(cpuUsageText));
        cpuBar->setValue(cpuUsage);
        ramTotalLabel->setText(QString("RAM Total: %1").arg(totalRam));
        ramAvailableLabel->setText(QString("RAM Available: %1").arg(availableRam));
        ramFreeLabel->setText(QString("RAM Free: %1").arg(freeRam));
        diskTotalLabel->setText(QString("Disk Total: %1").arg(totalDisk));
        diskUsedLabel->setText(QString("Disk Used: %1").arg(usedDisk));
        diskFreeLabel->setText(QString("Disk Free: %1").arg(freeDisk));
        netRxLabel->setText(QString("Network Receive: %1").arg(rx));
        netTxLabel->setText(QString("Network Transmit: %1").arg(tx));

        const QString status = SystemStatus::summary(cpuUsage, ramUsage);
        statusLabel->setText(QString("Status: %1").arg(status));

        const QString cpuAlert = AlertManager::cpuAlert(cpuUsage, settingsManager.cpuAlertThreshold());
        const QString ramAlertText = AlertManager::ramAlert(ramUsage, settingsManager.ramAlertThreshold());
        QString combinedAlert;
        if (!cpuAlert.isEmpty())
            combinedAlert += cpuAlert;
        if (!ramAlertText.isEmpty())
        {
            if (!combinedAlert.isEmpty())
                combinedAlert += " ";
            combinedAlert += ramAlertText;
        }
        if (combinedAlert.isEmpty())
        {
            combinedAlert = "No alerts.";
            alertLabel->setStyleSheet("color: green");
        }
        else
        {
            alertLabel->setStyleSheet("color: red");
        }
        alertLabel->setText(QString("Alerts: %1").arg(combinedAlert));

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
};

#include "main.moc"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    SystemMonitor monitor;
    monitor.show();
    return app.exec();
}
