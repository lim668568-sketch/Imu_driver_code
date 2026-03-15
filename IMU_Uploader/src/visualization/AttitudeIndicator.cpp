#include "AttitudeIndicator.h"
#include <QtMath>
#include <QFont>
#include <QDebug>

AttitudeIndicator::AttitudeIndicator(QWidget *parent)
    : QWidget(parent)
    , m_roll(0)
    , m_pitch(0)
    , m_yaw(0)
    , m_rollColor(255, 100, 100)
    , m_pitchColor(100, 255, 100)
    , m_backgroundColor(30, 30, 40)
{
    setMinimumSize(300, 300);
}

AttitudeIndicator::~AttitudeIndicator()
{
}

void AttitudeIndicator::setRoll(float roll)
{
    if (m_roll != roll) {
        m_roll = roll;
        emit rollChanged(roll);
        update();
    }
}

void AttitudeIndicator::setPitch(float pitch)
{
    if (m_pitch != pitch) {
        m_pitch = pitch;
        emit pitchChanged(pitch);
        update();
    }
}

void AttitudeIndicator::setYaw(float yaw)
{
    if (m_yaw != yaw) {
        m_yaw = yaw;
        emit yawChanged(yaw);
        update();
    }
}

void AttitudeIndicator::setEulerAngles(float roll, float pitch, float yaw)
{
    m_roll = roll;
    m_pitch = pitch;
    m_yaw = yaw;
    update();
}

float AttitudeIndicator::getRoll() const
{
    return m_roll;
}

float AttitudeIndicator::getPitch() const
{
    return m_pitch;
}

float AttitudeIndicator::getYaw() const
{
    return m_yaw;
}

void AttitudeIndicator::setRollColor(const QColor &color)
{
    m_rollColor = color;
}

void AttitudeIndicator::setPitchColor(const QColor &color)
{
    m_pitchColor = color;
}

void AttitudeIndicator::setBackgroundColor(const QColor &color)
{
    m_backgroundColor = color;
}

void AttitudeIndicator::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    painter.fillRect(rect(), m_backgroundColor);
    
    drawHorizon(painter);
    drawRollIndicator(painter);
    drawPitchIndicator(painter);
    drawYawIndicator(painter);
    drawCompass(painter);
    drawDataDisplay(painter);
}

void AttitudeIndicator::drawHorizon(QPainter &painter)
{
    int centerX = width() / 2;
    int centerY = height() / 2;
    int radius = qMin(width(), height()) / 3;
    
    painter.save();
    painter.translate(centerX, centerY);
    painter.rotate(m_roll);
    
    int pitchOffset = static_cast<int>(m_pitch * radius / 45.0f);
    
    QPainterPath skyPath;
    skyPath.addPie(-radius, -radius + pitchOffset, radius * 2, radius * 2, 0, 180 * 16);
    painter.fillPath(skyPath, QColor(70, 130, 180));
    
    QPainterPath groundPath;
    groundPath.addPie(-radius, -radius + pitchOffset, radius * 2, radius * 2, 180 * 16, 180 * 16);
    painter.fillPath(groundPath, QColor(139, 90, 43));
    
    painter.setPen(QPen(Qt::white, 2));
    painter.drawLine(-radius, pitchOffset, radius, pitchOffset);
    
    painter.restore();
}

void AttitudeIndicator::drawRollIndicator(QPainter &painter)
{
    int centerX = width() / 2;
    int topY = 30;
    int radius = qMin(width(), height()) / 3;
    
    painter.save();
    painter.translate(centerX, topY + radius);
    
    painter.setPen(QPen(m_rollColor, 2));
    painter.setBrush(Qt::NoBrush);
    
    int arcRadius = radius + 20;
    QRectF arcRect(-arcRadius, -arcRadius, arcRadius * 2, arcRadius * 2);
    painter.drawArc(arcRect, -60 * 16, 120 * 16);
    
    for (int angle = -60; angle <= 60; angle += 10) {
        qreal rad = qDegreesToRadians(static_cast<qreal>(-angle + 90));
        int x1 = static_cast<int>(arcRadius * qCos(rad));
        int y1 = static_cast<int>(arcRadius * qSin(rad));
        int x2 = static_cast<int>((arcRadius - 10) * qCos(rad));
        int y2 = static_cast<int>((arcRadius - 10) * qSin(rad));
        
        painter.drawLine(x1, y1, x2, y2);
    }
    
    qreal rollRad = qDegreesToRadians(static_cast<qreal>(-m_roll + 90));
    int pointerX = static_cast<int>((arcRadius - 5) * qCos(rollRad));
    int pointerY = static_cast<int>((arcRadius - 5) * qSin(rollRad));
    
    painter.setBrush(m_rollColor);
    QPolygonF pointer;
    pointer << QPointF(pointerX, pointerY)
            << QPointF(pointerX - 5, pointerY - 10)
            << QPointF(pointerX + 5, pointerY - 10);
    painter.drawPolygon(pointer);
    
    painter.restore();
}

void AttitudeIndicator::drawPitchIndicator(QPainter &painter)
{
    int centerX = width() / 2;
    int centerY = height() / 2;
    
    painter.save();
    painter.translate(centerX, centerY);
    
    painter.setPen(QPen(Qt::white, 2));
    painter.drawLine(-50, 0, -10, 0);
    painter.drawLine(10, 0, 50, 0);
    painter.drawLine(0, -5, 0, 5);
    
    QFont font = painter.font();
    font.setPointSize(10);
    font.setBold(true);
    painter.setFont(font);
    painter.drawText(-50, -10, 100, 20, Qt::AlignCenter, QString::number(m_pitch, 'f', 1) + "°");
    
    painter.restore();
}

void AttitudeIndicator::drawYawIndicator(QPainter &painter)
{
    int centerX = width() / 2;
    int bottomY = height() - 50;
    int radius = 40;
    
    painter.save();
    painter.translate(centerX, bottomY);
    
    painter.setPen(QPen(Qt::white, 2));
    painter.setBrush(Qt::NoBrush);
    painter.drawEllipse(-radius, -radius, radius * 2, radius * 2);
    
    for (int i = 0; i < 8; ++i) {
        qreal angle = i * 45.0;
        qreal rad = qDegreesToRadians(angle - 90);
        
        int x = static_cast<int>((radius - 10) * qCos(rad));
        int y = static_cast<int>((radius - 10) * qSin(rad));
        
        QString label;
        switch (i) {
            case 0: label = "N"; break;
            case 2: label = "E"; break;
            case 4: label = "S"; break;
            case 6: label = "W"; break;
            default: label = "";
        }
        
        if (!label.isEmpty()) {
            QFont font = painter.font();
            font.setPointSize(8);
            font.setBold(true);
            painter.setFont(font);
            painter.drawText(x - 10, y - 10, 20, 20, Qt::AlignCenter, label);
        }
    }
    
    qreal yawRad = qDegreesToRadians(static_cast<qreal>(-m_yaw - 90));
    int arrowX = static_cast<int>((radius - 15) * qCos(yawRad));
    int arrowY = static_cast<int>((radius - 15) * qSin(yawRad));
    
    painter.setBrush(QColor(255, 200, 0));
    QPolygonF arrow;
    arrow << QPointF(arrowX, arrowY)
          << QPointF(arrowX - 4, arrowY + 8)
          << QPointF(arrowX + 4, arrowY + 8);
    painter.drawPolygon(arrow);
    
    painter.restore();
}

void AttitudeIndicator::drawCompass(QPainter &painter)
{
    int rightX = width() - 80;
    int centerY = height() / 2;
    int radius = 60;
    
    painter.save();
    painter.translate(rightX, centerY);
    
    painter.setPen(QPen(QColor(100, 200, 255), 2));
    painter.setBrush(Qt::NoBrush);
    painter.drawEllipse(-radius, -radius, radius * 2, radius * 2);
    
    QFont font = painter.font();
    font.setPointSize(12);
    font.setBold(true);
    painter.setFont(font);
    
    for (int i = 0; i < 12; ++i) {
        qreal angle = i * 30.0 - m_yaw;
        qreal rad = qDegreesToRadians(angle - 90);
        
        int x = static_cast<int>((radius - 20) * qCos(rad));
        int y = static_cast<int>((radius - 20) * qSin(rad));
        
        QString label;
        switch (i) {
            case 0: label = "N"; painter.setPen(QPen(Qt::red, 2)); break;
            case 3: label = "E"; painter.setPen(QPen(Qt::white, 2)); break;
            case 6: label = "S"; painter.setPen(QPen(Qt::white, 2)); break;
            case 9: label = "W"; painter.setPen(QPen(Qt::white, 2)); break;
            default: label = QString::number(i * 30); painter.setPen(QPen(Qt::gray, 2)); break;
        }
        
        painter.drawText(x - 15, y - 10, 30, 20, Qt::AlignCenter, label);
    }
    
    painter.restore();
}

void AttitudeIndicator::drawDataDisplay(QPainter &painter)
{
    QFont font = painter.font();
    font.setPointSize(11);
    font.setBold(true);
    painter.setFont(font);
    
    int leftX = 20;
    int topY = 20;
    int lineHeight = 25;
    
    painter.setPen(m_rollColor);
    painter.drawText(leftX, topY, QString("Roll:  %1°").arg(m_roll, 6, 'f', 2));
    
    painter.setPen(m_pitchColor);
    painter.drawText(leftX, topY + lineHeight, QString("Pitch: %1°").arg(m_pitch, 6, 'f', 2));
    
    painter.setPen(QColor(100, 200, 255));
    painter.drawText(leftX, topY + lineHeight * 2, QString("Yaw:   %1°").arg(m_yaw, 6, 'f', 2));
}
