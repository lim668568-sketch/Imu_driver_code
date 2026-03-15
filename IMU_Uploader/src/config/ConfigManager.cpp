#include "ConfigManager.h"
#include <QCoreApplication>
#include <QDebug>

ConfigManager* ConfigManager::m_instance = nullptr;

ConfigManager* ConfigManager::instance()
{
    if (!m_instance) {
        m_instance = new ConfigManager();
    }
    return m_instance;
}

ConfigManager::ConfigManager(QObject *parent)
    : QObject(parent)
{
    QString configPath = QCoreApplication::applicationDirPath() + "/config.ini";
    m_settings = new QSettings(configPath, QSettings::IniFormat, this);
    load();
}

ConfigManager::~ConfigManager()
{
    save();
    delete m_settings;
}

void ConfigManager::setValue(const QString &key, const QVariant &value)
{
    m_cache[key] = value;
    m_settings->setValue(key, value);
    emit configChanged(key);
}

QVariant ConfigManager::value(const QString &key, const QVariant &defaultValue) const
{
    if (m_cache.contains(key)) {
        return m_cache[key];
    }
    return m_settings->value(key, defaultValue);
}

void ConfigManager::setSerialConfig(const QString &portName, int baudRate)
{
    setValue("Serial/PortName", portName);
    setValue("Serial/BaudRate", baudRate);
}

QString ConfigManager::getPortName() const
{
    return value("Serial/PortName", "").toString();
}

int ConfigManager::getBaudRate() const
{
    return value("Serial/BaudRate", 115200).toInt();
}

void ConfigManager::setPlotConfig(int timeWindow, float yMin, float yMax)
{
    setValue("Plot/TimeWindow", timeWindow);
    setValue("Plot/YMin", yMin);
    setValue("Plot/YMax", yMax);
}

int ConfigManager::getTimeWindow() const
{
    return value("Plot/TimeWindow", 10).toInt();
}

float ConfigManager::getYMin() const
{
    return value("Plot/YMin", -180.0f).toFloat();
}

float ConfigManager::getYMax() const
{
    return value("Plot/YMax", 180.0f).toFloat();
}

void ConfigManager::setWindowGeometry(const QByteArray &geometry)
{
    setValue("Window/Geometry", geometry);
}

QByteArray ConfigManager::getWindowGeometry() const
{
    return value("Window/Geometry").toByteArray();
}

void ConfigManager::setWindowState(const QByteArray &state)
{
    setValue("Window/State", state);
}

QByteArray ConfigManager::getWindowState() const
{
    return value("Window/State").toByteArray();
}

void ConfigManager::save()
{
    m_settings->sync();
}

void ConfigManager::load()
{
    QStringList keys = m_settings->allKeys();
    for (const QString &key : keys) {
        m_cache[key] = m_settings->value(key);
    }
}

void ConfigManager::reset()
{
    m_settings->clear();
    m_cache.clear();
    save();
}
