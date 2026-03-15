#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include <QObject>
#include <QSettings>
#include <QString>
#include <QVariant>
#include <QMap>

class ConfigManager : public QObject
{
    Q_OBJECT

public:
    static ConfigManager* instance();
    
    void setValue(const QString &key, const QVariant &value);
    QVariant value(const QString &key, const QVariant &defaultValue = QVariant()) const;
    
    void setSerialConfig(const QString &portName, int baudRate);
    QString getPortName() const;
    int getBaudRate() const;
    
    void setPlotConfig(int timeWindow, float yMin, float yMax);
    int getTimeWindow() const;
    float getYMin() const;
    float getYMax() const;
    
    void setWindowGeometry(const QByteArray &geometry);
    QByteArray getWindowGeometry() const;
    
    void setWindowState(const QByteArray &state);
    QByteArray getWindowState() const;
    
    void save();
    void load();
    void reset();
    
signals:
    void configChanged(const QString &key);
    
private:
    explicit ConfigManager(QObject *parent = nullptr);
    ~ConfigManager();
    
    static ConfigManager *m_instance;
    QSettings *m_settings;
    
    QMap<QString, QVariant> m_cache;
};

#endif
