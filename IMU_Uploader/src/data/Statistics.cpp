#include "Statistics.h"
#include <QtMath>
#include <QDebug>
#include <algorithm>

Statistics::Statistics(QObject *parent)
    : QObject(parent)
{
}

StatsResult Statistics::calculate(const QVector<float> &data)
{
    StatsResult result;
    result.count = data.size();
    
    if (data.isEmpty()) {
        result.mean = 0;
        result.variance = 0;
        result.stdDev = 0;
        result.min = 0;
        result.max = 0;
        result.range = 0;
        return result;
    }
    
    result.mean = calculateMean(data);
    result.variance = calculateVariance(data, result.mean);
    result.stdDev = calculateStdDev(result.variance);
    result.min = calculateMin(data);
    result.max = calculateMax(data);
    result.range = result.max - result.min;
    
    emit statisticsCalculated(result);
    return result;
}

StatsResult Statistics::calculateWindow(const QVector<float> &data, int windowSize)
{
    if (data.size() < windowSize) {
        return calculate(data);
    }
    
    QVector<float> windowData;
    for (int i = data.size() - windowSize; i < data.size(); ++i) {
        windowData.append(data[i]);
    }
    
    return calculate(windowData);
}

float Statistics::calculateMean(const QVector<float> &data)
{
    if (data.isEmpty()) return 0;
    
    float sum = 0;
    for (float value : data) {
        sum += value;
    }
    return sum / data.size();
}

float Statistics::calculateVariance(const QVector<float> &data, float mean)
{
    if (data.isEmpty()) return 0;
    
    float sumSquares = 0;
    for (float value : data) {
        float diff = value - mean;
        sumSquares += diff * diff;
    }
    return sumSquares / data.size();
}

float Statistics::calculateStdDev(float variance)
{
    return qSqrt(variance);
}

float Statistics::calculateMin(const QVector<float> &data)
{
    if (data.isEmpty()) return 0;
    
    float minVal = data[0];
    for (float value : data) {
        if (value < minVal) minVal = value;
    }
    return minVal;
}

float Statistics::calculateMax(const QVector<float> &data)
{
    if (data.isEmpty()) return 0;
    
    float maxVal = data[0];
    for (float value : data) {
        if (value > maxVal) maxVal = value;
    }
    return maxVal;
}

QVector<float> Statistics::calculateHistogram(const QVector<float> &data, int bins)
{
    QVector<float> histogram(bins, 0);
    
    if (data.isEmpty()) return histogram;
    
    float minVal = calculateMin(data);
    float maxVal = calculateMax(data);
    float range = maxVal - minVal;
    
    if (range == 0) {
        histogram[0] = data.size();
        return histogram;
    }
    
    float binWidth = range / bins;
    
    for (float value : data) {
        int binIndex = static_cast<int>((value - minVal) / binWidth);
        if (binIndex >= bins) binIndex = bins - 1;
        if (binIndex < 0) binIndex = 0;
        histogram[binIndex]++;
    }
    
    return histogram;
}

QVector<float> Statistics::calculateMovingAverage(const QVector<float> &data, int windowSize)
{
    QVector<float> result;
    
    if (data.size() < windowSize) {
        return data;
    }
    
    for (int i = windowSize - 1; i < data.size(); ++i) {
        float sum = 0;
        for (int j = i - windowSize + 1; j <= i; ++j) {
            sum += data[j];
        }
        result.append(sum / windowSize);
    }
    
    return result;
}
