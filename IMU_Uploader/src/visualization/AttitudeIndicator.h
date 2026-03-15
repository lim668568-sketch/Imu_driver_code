#ifndef ATTITUDEINDICATOR_H
#define ATTITUDEINDICATOR_H

#include <QWidget>
#include <QPainter>
#include <QColor>

class AttitudeIndicator : public QWidget
{
    Q_OBJECT

public:
    explicit AttitudeIndicator(QWidget *parent = nullptr);
    ~AttitudeIndicator();
    
    void setRoll(float roll);
    void setPitch(float pitch);
    void setYaw(float yaw);
    void setEulerAngles(float roll, float pitch, float yaw);
    
    float getRoll() const;
    float getPitch() const;
    float getYaw() const;
    
    void setRollColor(const QColor &color);
    void setPitchColor(const QColor &color);
    void setBackgroundColor(const QColor &color);
    
signals:
    void rollChanged(float roll);
    void pitchChanged(float pitch);
    void yawChanged(float yaw);
    
protected:
    void paintEvent(QPaintEvent *event) override;
    
private:
    void drawHorizon(QPainter &painter);
    void drawRollIndicator(QPainter &painter);
    void drawPitchIndicator(QPainter &painter);
    void drawYawIndicator(QPainter &painter);
    void drawCompass(QPainter &painter);
    void drawDataDisplay(QPainter &painter);
    
    float m_roll;
    float m_pitch;
    float m_yaw;
    
    QColor m_rollColor;
    QColor m_pitchColor;
    QColor m_backgroundColor;
};

#endif
