#ifndef THREEDVIEW_H
#define THREEDVIEW_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QMatrix4x4>
#include <QVector3D>
#include <QTimer>

class ThreeDView : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    explicit ThreeDView(QWidget *parent = nullptr);
    ~ThreeDView();
    
    void setRoll(float roll);
    void setPitch(float pitch);
    void setYaw(float yaw);
    void setEulerAngles(float roll, float pitch, float yaw);
    
    void resetView();
    
    enum ModelType {
        Airplane,
        Car,
        Phone,
        Cube
    };
    
    void setModelType(ModelType type);
    ModelType getModelType() const;
    
protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;
    
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    
private:
    void drawModel();
    void drawAirplane();
    void drawCar();
    void drawPhone();
    void drawCube();
    void drawAxes();
    void drawGrid();
    
    void setupLighting();
    void setMaterial(float r, float g, float b);
    
    float m_roll;
    float m_pitch;
    float m_yaw;
    
    float m_cameraDistance;
    float m_cameraRotX;
    float m_cameraRotY;
    
    QPoint m_lastMousePos;
    
    ModelType m_modelType;
    
    QTimer *m_updateTimer;
};

#endif
