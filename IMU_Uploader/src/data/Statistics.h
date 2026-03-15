#ifndef STATISTICS_H
#define STATISTICS_H

#include <QObject>
#include <QVector>

struct StatsResult {
    float mean;
    float variance;
    float stdDev;
    float min;
    float max;
    float range;
    int count;
};

class Statistics : public QObject
{
    Q_OBJECT

public:
    explicit Statistics(QObject *parent = nullptr);
    
    StatsResult calculate(const QVector<float> &data);
    StatsResult calculateWindow(const QVector<float> &data, int windowSize);
    
    float calculateMean(const QVector<float> &data);
    float calculateVariance(const QVector<float> &data, float mean);
    float calculateStdDev(float variance);
    float calculateMin(const QVector<float> &data);
    float calculateMax(const QVector<float> &data);
    
    QVector<float> calculateHistogram(const QVector<float> &data, int bins);
    QVector<float> calculateMovingAverage(const QVector<float> &data, int windowSize);
    
signals:
    void statisticsCalculated(const StatsResult &result);
};

#endif
