#ifndef WAVEFORMPLOT_H
#define WAVEFORMPLOT_H

#include <QWidget>
#include <QPainter>
#include <QVector>
#include <QTimer>
#include <QMap>
#include "src/data/DataBuffer.h"

class WaveformPlot : public QWidget
{
    Q_OBJECT

public:
    explicit WaveformPlot(QWidget *parent = nullptr);
    ~WaveformPlot();
    
    void setDataBuffer(DataBuffer *buffer);
    void setTimeWindow(int seconds);
    void setSampleRate(float rate);
    
    void showChannel(const QString &channel, bool visible);
    void setChannelColor(const QString &channel, const QColor &color);
    
    void setAutoScale(bool enabled);
    void setYRange(float min, float max);
    
    void enableGrid(bool enabled);
    void setGridColor(const QColor &color);
    
    void clearData();
    void pause(bool paused);
    
    enum Channel {
        Roll,
        Pitch,
        Yaw,
        AccelX,
        AccelY,
        AccelZ,
        GyroX,
        GyroY,
        GyroZ
    };
    
    void setChannelVisible(Channel channel, bool visible);
    
signals:
    void timeWindowChanged(int seconds);
    void yRangeChanged(float min, float max);
    
protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    
private:
    void drawGrid(QPainter &painter);
    void drawAxes(QPainter &painter);
    void drawData(QPainter &painter);
    void drawLegend(QPainter &painter);
    void drawCursor(QPainter &painter);
    
    void updatePlot();
    float mapToWidgetY(float value, float minVal, float maxVal);
    float mapToWidgetX(float timestamp);
    
    DataBuffer *m_dataBuffer;
    
    int m_timeWindow;
    float m_sampleRate;
    
    float m_yMin;
    float m_yMax;
    bool m_autoScale;
    
    bool m_gridEnabled;
    QColor m_gridColor;
    
    QMap<QString, bool> m_channelVisible;
    QMap<QString, QColor> m_channelColors;
    
    bool m_paused;
    QPoint m_mousePos;
    bool m_showCursor;
    
    QTimer *m_updateTimer;
};

#endif
