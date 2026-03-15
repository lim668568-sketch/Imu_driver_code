#include "WaveformPlot.h"
#include <QMouseEvent>
#include <QWheelEvent>
#include <QtMath>
#include <QDebug>

WaveformPlot::WaveformPlot(QWidget *parent)
    : QWidget(parent)
    , m_dataBuffer(nullptr)
    , m_timeWindow(10)
    , m_sampleRate(100.0f)
    , m_yMin(-180)
    , m_yMax(180)
    , m_autoScale(false)
    , m_gridEnabled(true)
    , m_gridColor(50, 50, 60)
    , m_paused(false)
    , m_showCursor(false)
    , m_updateTimer(new QTimer(this))
{
    setMinimumSize(400, 300);
    setMouseTracking(true);
    
    m_channelColors["Roll"] = QColor(255, 100, 100);
    m_channelColors["Pitch"] = QColor(100, 255, 100);
    m_channelColors["Yaw"] = QColor(100, 100, 255);
    m_channelColors["AccelX"] = QColor(255, 200, 100);
    m_channelColors["AccelY"] = QColor(100, 255, 200);
    m_channelColors["AccelZ"] = QColor(200, 100, 255);
    m_channelColors["GyroX"] = QColor(255, 150, 150);
    m_channelColors["GyroY"] = QColor(150, 255, 150);
    m_channelColors["GyroZ"] = QColor(150, 150, 255);
    
    m_channelVisible["Roll"] = true;
    m_channelVisible["Pitch"] = true;
    m_channelVisible["Yaw"] = true;
    m_channelVisible["AccelX"] = false;
    m_channelVisible["AccelY"] = false;
    m_channelVisible["AccelZ"] = false;
    m_channelVisible["GyroX"] = false;
    m_channelVisible["GyroY"] = false;
    m_channelVisible["GyroZ"] = false;
    
    connect(m_updateTimer, &QTimer::timeout, this, &WaveformPlot::updatePlot);
    m_updateTimer->start(33);
}

WaveformPlot::~WaveformPlot()
{
}

void WaveformPlot::setDataBuffer(DataBuffer *buffer)
{
    m_dataBuffer = buffer;
}

void WaveformPlot::setTimeWindow(int seconds)
{
    m_timeWindow = seconds;
    emit timeWindowChanged(seconds);
    update();
}

void WaveformPlot::setSampleRate(float rate)
{
    m_sampleRate = rate;
}

void WaveformPlot::showChannel(const QString &channel, bool visible)
{
    m_channelVisible[channel] = visible;
    update();
}

void WaveformPlot::setChannelColor(const QString &channel, const QColor &color)
{
    m_channelColors[channel] = color;
    update();
}

void WaveformPlot::setAutoScale(bool enabled)
{
    m_autoScale = enabled;
    update();
}

void WaveformPlot::setYRange(float min, float max)
{
    m_yMin = min;
    m_yMax = max;
    m_autoScale = false;
    emit yRangeChanged(min, max);
    update();
}

void WaveformPlot::enableGrid(bool enabled)
{
    m_gridEnabled = enabled;
    update();
}

void WaveformPlot::setGridColor(const QColor &color)
{
    m_gridColor = color;
    update();
}

void WaveformPlot::clearData()
{
    if (m_dataBuffer) {
        m_dataBuffer->clear();
    }
    update();
}

void WaveformPlot::pause(bool paused)
{
    m_paused = paused;
}

void WaveformPlot::setChannelVisible(Channel channel, bool visible)
{
    QString channelName;
    switch (channel) {
        case Roll: channelName = "Roll"; break;
        case Pitch: channelName = "Pitch"; break;
        case Yaw: channelName = "Yaw"; break;
        case AccelX: channelName = "AccelX"; break;
        case AccelY: channelName = "AccelY"; break;
        case AccelZ: channelName = "AccelZ"; break;
        case GyroX: channelName = "GyroX"; break;
        case GyroY: channelName = "GyroY"; break;
        case GyroZ: channelName = "GyroZ"; break;
    }
    m_channelVisible[channelName] = visible;
    update();
}

void WaveformPlot::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    painter.fillRect(rect(), QColor(25, 25, 35));
    
    if (m_gridEnabled) {
        drawGrid(painter);
    }
    
    drawAxes(painter);
    drawData(painter);
    drawLegend(painter);
    
    if (m_showCursor) {
        drawCursor(painter);
    }
}

void WaveformPlot::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    update();
}

void WaveformPlot::mousePressEvent(QMouseEvent *event)
{
    m_mousePos = event->pos();
    m_showCursor = true;
    update();
}

void WaveformPlot::mouseMoveEvent(QMouseEvent *event)
{
    if (m_showCursor) {
        m_mousePos = event->pos();
        update();
    }
}

void WaveformPlot::wheelEvent(QWheelEvent *event)
{
    float delta = event->angleDelta().y() / 120.0f;
    float range = m_yMax - m_yMin;
    float scale = 1.0f - delta * 0.1f;
    
    float newRange = range * scale;
    float center = (m_yMax + m_yMin) / 2;
    
    m_yMin = center - newRange / 2;
    m_yMax = center + newRange / 2;
    
    m_autoScale = false;
    update();
}

void WaveformPlot::drawGrid(QPainter &painter)
{
    painter.setPen(QPen(m_gridColor, 1));
    
    int margin = 60;
    int plotWidth = width() - margin - 20;
    int plotHeight = height() - 40;
    
    int hLines = 10;
    for (int i = 0; i <= hLines; ++i) {
        int y = 20 + i * plotHeight / hLines;
        painter.drawLine(margin, y, margin + plotWidth, y);
    }
    
    int vLines = m_timeWindow;
    for (int i = 0; i <= vLines; ++i) {
        int x = margin + i * plotWidth / vLines;
        painter.drawLine(x, 20, x, 20 + plotHeight);
    }
}

void WaveformPlot::drawAxes(QPainter &painter)
{
    painter.setPen(QPen(Qt::white, 2));
    
    int margin = 60;
    int plotWidth = width() - margin - 20;
    int plotHeight = height() - 40;
    
    painter.drawLine(margin, 20, margin, 20 + plotHeight);
    painter.drawLine(margin, 20 + plotHeight, margin + plotWidth, 20 + plotHeight);
    
    QFont font = painter.font();
    font.setPointSize(9);
    painter.setFont(font);
    
    int hLines = 10;
    for (int i = 0; i <= hLines; ++i) {
        int y = 20 + i * plotHeight / hLines;
        float value = m_yMax - i * (m_yMax - m_yMin) / hLines;
        painter.drawText(5, y + 5, QString::number(value, 'f', 1));
    }
    
    for (int i = 0; i <= m_timeWindow; i += m_timeWindow / 5) {
        int x = margin + i * plotWidth / m_timeWindow;
        painter.drawText(x - 10, height() - 5, QString::number(i) + "s");
    }
    
    painter.drawText(width() / 2 - 30, height() - 5, "Time (s)");
}

void WaveformPlot::drawData(QPainter &painter)
{
    if (!m_dataBuffer || m_dataBuffer->isEmpty()) {
        return;
    }
    
    int margin = 60;
    int plotWidth = width() - margin - 20;
    int plotHeight = height() - 40;
    
    auto drawChannel = [&](const QString &name, const QVector<DataPoint> &data) {
        if (!m_channelVisible[name] || data.isEmpty()) return;
        
        painter.setPen(QPen(m_channelColors[name], 2));
        
        int maxPoints = static_cast<int>(m_timeWindow * m_sampleRate);
        int startIdx = qMax(0, data.size() - maxPoints);
        
        QPainterPath path;
        bool first = true;
        
        for (int i = startIdx; i < data.size(); ++i) {
            float x = margin + mapToWidgetX(i - startIdx);
            float y = mapToWidgetY(data[i].value, m_yMin, m_yMax);
            
            if (first) {
                path.moveTo(x, y);
                first = false;
            } else {
                path.lineTo(x, y);
            }
        }
        
        painter.drawPath(path);
    };
    
    drawChannel("Roll", m_dataBuffer->getRollData());
    drawChannel("Pitch", m_dataBuffer->getPitchData());
    drawChannel("Yaw", m_dataBuffer->getYawData());
    drawChannel("AccelX", m_dataBuffer->getAccelXData());
    drawChannel("AccelY", m_dataBuffer->getAccelYData());
    drawChannel("AccelZ", m_dataBuffer->getAccelZData());
    drawChannel("GyroX", m_dataBuffer->getGyroXData());
    drawChannel("GyroY", m_dataBuffer->getGyroYData());
    drawChannel("GyroZ", m_dataBuffer->getGyroZData());
}

void WaveformPlot::drawLegend(QPainter &painter)
{
    QFont font = painter.font();
    font.setPointSize(9);
    painter.setFont(font);
    
    int x = width() - 150;
    int y = 30;
    int lineHeight = 20;
    
    for (auto it = m_channelVisible.begin(); it != m_channelVisible.end(); ++it) {
        if (it.value()) {
            painter.setPen(QPen(m_channelColors[it.key()], 3));
            painter.drawLine(x, y, x + 20, y);
            
            painter.setPen(Qt::white);
            painter.drawText(x + 25, y + 5, it.key());
            
            y += lineHeight;
        }
    }
}

void WaveformPlot::drawCursor(QPainter &painter)
{
    int margin = 60;
    int plotWidth = width() - margin - 20;
    int plotHeight = height() - 40;
    
    if (m_mousePos.x() >= margin && m_mousePos.x() <= margin + plotWidth &&
        m_mousePos.y() >= 20 && m_mousePos.y() <= 20 + plotHeight) {
        
        painter.setPen(QPen(Qt::yellow, 1, Qt::DashLine));
        painter.drawLine(m_mousePos.x(), 20, m_mousePos.x(), 20 + plotHeight);
        painter.drawLine(margin, m_mousePos.y(), margin + plotWidth, m_mousePos.y());
        
        float time = (m_mousePos.x() - margin) * m_timeWindow / plotWidth;
        float value = m_yMax - (m_mousePos.y() - 20) * (m_yMax - m_yMin) / plotHeight;
        
        painter.setPen(Qt::white);
        QFont font = painter.font();
        font.setPointSize(10);
        font.setBold(true);
        painter.setFont(font);
        
        QString text = QString("T: %1s, V: %2").arg(time, 0, 'f', 2).arg(value, 0, 'f', 2);
        painter.drawText(m_mousePos.x() + 10, m_mousePos.y() - 10, text);
    }
}

void WaveformPlot::updatePlot()
{
    if (!m_paused) {
        update();
    }
}

float WaveformPlot::mapToWidgetY(float value, float minVal, float maxVal)
{
    int margin = 60;
    int plotHeight = height() - 40;
    
    float normalized = (value - minVal) / (maxVal - minVal);
    return 20 + plotHeight - normalized * plotHeight;
}

float WaveformPlot::mapToWidgetX(float timestamp)
{
    int margin = 60;
    int plotWidth = width() - margin - 20;
    
    return timestamp * plotWidth / (m_timeWindow * m_sampleRate);
}
