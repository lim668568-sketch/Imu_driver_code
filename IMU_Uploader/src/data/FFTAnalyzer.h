#ifndef FFTANALYZER_H
#define FFTANALYZER_H

#include <QObject>
#include <QVector>
#include <complex>

struct FFTResult {
    QVector<float> frequencies;
    QVector<float> magnitudes;
    float peakFrequency;
    float peakMagnitude;
    float noiseDensity;
};

class FFTAnalyzer : public QObject
{
    Q_OBJECT

public:
    explicit FFTAnalyzer(QObject *parent = nullptr);
    
    FFTResult analyze(const QVector<float> &data, float sampleRate);
    FFTResult analyzeWindow(const QVector<float> &data, float sampleRate, int windowSize);
    
    void setWindowSize(int size);
    int getWindowSize() const;
    
    enum WindowFunction {
        Rectangular,
        Hanning,
        Hamming,
        Blackman
    };
    
    void setWindowFunction(WindowFunction func);
    
signals:
    void fftCompleted(const FFTResult &result);
    
private:
    void applyWindowFunction(QVector<std::complex<float>> &data);
    void fft(QVector<std::complex<float>> &data);
    int nextPowerOfTwo(int n);
    
    int m_windowSize;
    WindowFunction m_windowFunc;
};

#endif
