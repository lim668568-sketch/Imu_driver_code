#ifndef LOGGER_H
#define LOGGER_H

#include <QObject>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QMutex>

class Logger : public QObject
{
    Q_OBJECT

public:
    enum LogLevel {
        Debug,
        Info,
        Warning,
        Error
    };
    
    static Logger* instance();
    
    void setLogFile(const QString &filename);
    void setLogLevel(LogLevel level);
    void setMaxFileSize(qint64 size);
    
    void log(LogLevel level, const QString &message);
    void debug(const QString &message);
    void info(const QString &message);
    void warning(const QString &message);
    void error(const QString &message);
    
    void enableConsoleOutput(bool enabled);
    void enableFileOutput(bool enabled);
    
signals:
    void logMessage(LogLevel level, const QString &message);
    
private:
    explicit Logger(QObject *parent = nullptr);
    ~Logger();
    
    void writeLog(LogLevel level, const QString &message);
    void checkFileSize();
    void rotateLog();
    
    static Logger *m_instance;
    
    QFile m_logFile;
    QTextStream m_logStream;
    QMutex m_mutex;
    
    LogLevel m_logLevel;
    qint64 m_maxFileSize;
    bool m_consoleOutput;
    bool m_fileOutput;
};

#endif
