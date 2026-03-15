#include "FFTAnalyzer.h"
#include <QtMath>
#include <QDebug>

FFTAnalyzer::FFTAnalyzer(QObject *parent)
    : QObject(parent)
    , m_windowSize(1024)
    , m_windowFunc(Hanning)
{
}

FFTResult FFTAnalyzer::analyze(const QVector<float> &data, float sampleRate)
{
    FFTResult result;
    
    if (data.isEmpty()) {
        return result;
    }
    
    int n = nextPowerOfTwo(data.size());
    QVector<std::complex<float>> fftData(n);
    
    for (int i = 0; i < data.size(); ++i) {
        fftData[i] = std::complex<float>(data[i], 0);
    }
    for (int i = data.size(); i < n; ++i) {
        fftData[i] = std::complex<float>(0, 0);
    }
    
    applyWindowFunction(fftData);
    fft(fftData);
    
    int halfN = n / 2;
    result.frequencies.resize(halfN);
    result.magnitudes.resize(halfN);
    
    float freqResolution = sampleRate / n;
    
    for (int i = 0; i < halfN; ++i) {
        result.frequencies[i] = i * freqResolution;
        result.magnitudes[i] = std::abs(fftData[i]) / n * 2;
    }
    
    result.peakMagnitude = 0;
    result.peakFrequency = 0;
    for (int i = 1; i < halfN; ++i) {
        if (result.magnitudes[i] > result.peakMagnitude) {
            result.peakMagnitude = result.magnitudes[i];
            result.peakFrequency = result.frequencies[i];
        }
    }
    
    float sum = 0;
    for (int i = halfN / 2; i < halfN; ++i) {
        sum += result.magnitudes[i] * result.magnitudes[i];
    }
    result.noiseDensity = qSqrt(sum / (halfN / 2));
    
    emit fftCompleted(result);
    return result;
}

FFTResult FFTAnalyzer::analyzeWindow(const QVector<float> &data, float sampleRate, int windowSize)
{
    if (data.size() < windowSize) {
        return analyze(data, sampleRate);
    }
    
    QVector<float> windowData;
    for (int i = data.size() - windowSize; i < data.size(); ++i) {
        windowData.append(data[i]);
    }
    
    return analyze(windowData, sampleRate);
}

void FFTAnalyzer::setWindowSize(int size)
{
    m_windowSize = nextPowerOfTwo(size);
}

int FFTAnalyzer::getWindowSize() const
{
    return m_windowSize;
}

void FFTAnalyzer::setWindowFunction(WindowFunction func)
{
    m_windowFunc = func;
}

void FFTAnalyzer::applyWindowFunction(QVector<std::complex<float>> &data)
{
    int n = data.size();
    
    for (int i = 0; i < n; ++i) {
        float windowValue = 1.0f;
        
        switch (m_windowFunc) {
            case Hanning:
                windowValue = 0.5f * (1 - qCos(2 * M_PI * i / (n - 1)));
                break;
            case Hamming:
                windowValue = 0.54f - 0.46f * qCos(2 * M_PI * i / (n - 1));
                break;
            case Blackman:
                windowValue = 0.42f - 0.5f * qCos(2 * M_PI * i / (n - 1)) 
                            + 0.08f * qCos(4 * M_PI * i / (n - 1));
                break;
            case Rectangular:
            default:
                windowValue = 1.0f;
                break;
        }
        
        data[i] *= windowValue;
    }
}

void FFTAnalyzer::fft(QVector<std::complex<float>> &data)
{
    int n = data.size();
    
    if (n <= 1) return;
    
    QVector<std::complex<float>> even(n / 2);
    QVector<std::complex<float>> odd(n / 2);
    
    for (int i = 0; i < n / 2; ++i) {
        even[i] = data[2 * i];
        odd[i] = data[2 * i + 1];
    }
    
    fft(even);
    fft(odd);
    
    for (int k = 0; k < n / 2; ++k) {
        float angle = -2 * M_PI * k / n;
        std::complex<float> t = std::polar(1.0f, angle) * odd[k];
        data[k] = even[k] + t;
        data[k + n / 2] = even[k] - t;
    }
}

int FFTAnalyzer::nextPowerOfTwo(int n)
{
    int power = 1;
    while (power < n) {
        power <<= 1;
    }
    return power;
}
