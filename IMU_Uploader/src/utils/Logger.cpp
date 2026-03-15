#include "Logger.h"
#include <QCoreApplication>
#include <QDebug>
#include <QDir>

Logger* Logger::m_instance = nullptr;

Logger* Logger::instance()
{
    if (!m_instance) {
        m_instance = new Logger();
    }
    return m_instance;
}

Logger::Logger(QObject *parent)
    : QObject(parent)
    , m_logLevel(Info)
    , m_maxFileSize(10 * 1024 * 1024)
    , m_consoleOutput(true)
    , m_fileOutput(true)
{
    QString logPath = QCoreApplication::applicationDirPath() + "/logs";
    QDir dir(logPath);
    if (!dir.exists()) {
        dir.mkpath(logPath);
    }
    
    QString logFile = logPath + "/imu_uploader_" + 
                     QDateTime::currentDateTime().toString("yyyy-MM-dd") + ".log";
    setLogFile(logFile);
}

Logger::~Logger()
{
    if (m_logFile.isOpen()) {
        m_logStream.flush();
        m_logFile.close();
    }
}

void Logger::setLogFile(const QString &filename)
{
    QMutexLocker locker(&m_mutex);
    
    if (m_logFile.isOpen()) {
        m_logFile.close();
    }
    
    m_logFile.setFileName(filename);
    if (m_logFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        m_logStream.setDevice(&m_logFile);
    }
}

void Logger::setLogLevel(LogLevel level)
{
    m_logLevel = level;
}

void Logger::setMaxFileSize(qint64 size)
{
    m_maxFileSize = size;
}

void Logger::log(LogLevel level, const QString &message)
{
    if (level >= m_logLevel) {
        writeLog(level, message);
    }
}

void Logger::debug(const QString &message)
{
    log(Debug, message);
}

void Logger::info(const QString &message)
{
    log(Info, message);
}

void Logger::warning(const QString &message)
{
    log(Warning, message);
}

void Logger::error(const QString &message)
{
    log(Error, message);
}

void Logger::enableConsoleOutput(bool enabled)
{
    m_consoleOutput = enabled;
}

void Logger::enableFileOutput(bool enabled)
{
    m_fileOutput = enabled;
}

void Logger::writeLog(LogLevel level, const QString &message)
{
    QMutexLocker locker(&m_mutex);
    
    QString levelStr;
    switch (level) {
        case Debug:   levelStr = "DEBUG"; break;
        case Info:    levelStr = "INFO"; break;
        case Warning: levelStr = "WARN"; break;
        case Error:   levelStr = "ERROR"; break;
    }
    
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
    QString logMessage = QString("[%1] [%2] %3").arg(timestamp, levelStr, message);
    
    if (m_consoleOutput) {
        switch (level) {
            case Debug:   qDebug() << logMessage; break;
            case Info:    qInfo() << logMessage; break;
            case Warning: qWarning() << logMessage; break;
            case Error:   qCritical() << logMessage; break;
        }
    }
    
    if (m_fileOutput && m_logFile.isOpen()) {
        m_logStream << logMessage << "\n";
        m_logStream.flush();
        
        checkFileSize();
    }
    
    emit logMessage(level, message);
}

void Logger::checkFileSize()
{
    if (m_logFile.size() > m_maxFileSize) {
        rotateLog();
    }
}

void Logger::rotateLog()
{
    m_logStream.flush();
    m_logFile.close();
    
    QString oldName = m_logFile.fileName();
    QString backupName = oldName + ".bak";
    
    QFile::remove(backupName);
    QFile::rename(oldName, backupName);
    
    m_logFile.setFileName(oldName);
    m_logFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text);
    m_logStream.setDevice(&m_logFile);
}
