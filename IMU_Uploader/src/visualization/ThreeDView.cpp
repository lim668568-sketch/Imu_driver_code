#include "ThreeDView.h"
#include <QMouseEvent>
#include <QWheelEvent>
#include <QtMath>
#include <QDebug>

ThreeDView::ThreeDView(QWidget *parent)
    : QOpenGLWidget(parent)
    , m_roll(0)
    , m_pitch(0)
    , m_yaw(0)
    , m_cameraDistance(5.0f)
    , m_cameraRotX(30.0f)
    , m_cameraRotY(45.0f)
    , m_modelType(Airplane)
    , m_updateTimer(new QTimer(this))
{
    setMinimumSize(400, 400);
    
    connect(m_updateTimer, &QTimer::timeout, this, QOverload<>::of(&ThreeDView::update));
    m_updateTimer->start(33);
}

ThreeDView::~ThreeDView()
{
    makeCurrent();
    doneCurrent();
}

void ThreeDView::setRoll(float roll)
{
    m_roll = roll;
}

void ThreeDView::setPitch(float pitch)
{
    m_pitch = pitch;
}

void ThreeDView::setYaw(float yaw)
{
    m_yaw = yaw;
}

void ThreeDView::setEulerAngles(float roll, float pitch, float yaw)
{
    m_roll = roll;
    m_pitch = pitch;
    m_yaw = yaw;
}

void ThreeDView::resetView()
{
    m_cameraDistance = 5.0f;
    m_cameraRotX = 30.0f;
    m_cameraRotY = 45.0f;
    update();
}

void ThreeDView::setModelType(ModelType type)
{
    m_modelType = type;
    update();
}

ThreeDView::ModelType ThreeDView::getModelType() const
{
    return m_modelType;
}

void ThreeDView::initializeGL()
{
    initializeOpenGLFunctions();
    
    glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_NORMALIZE);
    glShadeModel(GL_SMOOTH);
    
    setupLighting();
}

void ThreeDView::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    float aspect = static_cast<float>(w) / static_cast<float>(h);
    gluPerspective(45.0, aspect, 0.1, 100.0);
    glMatrixMode(GL_MODELVIEW);
}

void ThreeDView::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    
    glTranslatef(0, 0, -m_cameraDistance);
    glRotatef(m_cameraRotX, 1, 0, 0);
    glRotatef(m_cameraRotY, 0, 1, 0);
    
    drawGrid();
    drawAxes();
    
    glPushMatrix();
    
    glRotatef(m_yaw, 0, 1, 0);
    glRotatef(m_pitch, 1, 0, 0);
    glRotatef(-m_roll, 0, 0, 1);
    
    drawModel();
    
    glPopMatrix();
}

void ThreeDView::drawModel()
{
    switch (m_modelType) {
        case Airplane:
            drawAirplane();
            break;
        case Car:
            drawCar();
            break;
        case Phone:
            drawPhone();
            break;
        case Cube:
        default:
            drawCube();
            break;
    }
}

void ThreeDView::drawAirplane()
{
    setMaterial(0.3f, 0.5f, 0.8f);
    
    glBegin(GL_TRIANGLES);
    
    glNormal3f(0, 0, 1);
    glVertex3f(0, 0, 1.5f);
    glVertex3f(-0.3f, 0, 0);
    glVertex3f(0.3f, 0, 0);
    
    glNormal3f(0, 0, -1);
    glVertex3f(0, 0, 1.5f);
    glVertex3f(0.3f, 0, 0);
    glVertex3f(-0.3f, 0, 0);
    
    glEnd();
    
    glBegin(GL_QUADS);
    
    glNormal3f(0, 1, 0);
    glVertex3f(-0.3f, 0, 0);
    glVertex3f(-0.3f, 0, -1.0f);
    glVertex3f(0.3f, 0, -1.0f);
    glVertex3f(0.3f, 0, 0);
    
    glEnd();
    
    setMaterial(0.8f, 0.3f, 0.3f);
    glBegin(GL_TRIANGLES);
    
    glNormal3f(0, 1, 0);
    glVertex3f(0, 0, -0.5f);
    glVertex3f(-1.5f, 0, -0.8f);
    glVertex3f(-0.3f, 0, -0.8f);
    
    glVertex3f(0, 0, -0.5f);
    glVertex3f(0.3f, 0, -0.8f);
    glVertex3f(1.5f, 0, -0.8f);
    
    glEnd();
    
    setMaterial(0.3f, 0.8f, 0.3f);
    glBegin(GL_TRIANGLES);
    
    glNormal3f(0, 0, 1);
    glVertex3f(0, 0, -0.7f);
    glVertex3f(-0.1f, 0.5f, -1.0f);
    glVertex3f(0.1f, 0.5f, -1.0f);
    
    glEnd();
}

void ThreeDView::drawCar()
{
    setMaterial(0.8f, 0.4f, 0.2f);
    
    glBegin(GL_QUADS);
    
    glNormal3f(0, 0, 1);
    glVertex3f(-0.5f, -0.3f, 0.8f);
    glVertex3f(0.5f, -0.3f, 0.8f);
    glVertex3f(0.5f, 0.3f, 0.8f);
    glVertex3f(-0.5f, 0.3f, 0.8f);
    
    glNormal3f(0, 0, -1);
    glVertex3f(-0.5f, -0.3f, -0.8f);
    glVertex3f(-0.5f, 0.3f, -0.8f);
    glVertex3f(0.5f, 0.3f, -0.8f);
    glVertex3f(0.5f, -0.3f, -0.8f);
    
    glEnd();
    
    setMaterial(0.6f, 0.3f, 0.1f);
    glBegin(GL_QUADS);
    
    glNormal3f(0, 1, 0);
    glVertex3f(-0.3f, -0.25f, 0.3f);
    glVertex3f(0.3f, -0.25f, 0.3f);
    glVertex3f(0.3f, -0.25f, -0.3f);
    glVertex3f(-0.3f, -0.25f, -0.3f);
    
    glEnd();
}

void ThreeDView::drawPhone()
{
    setMaterial(0.2f, 0.2f, 0.3f);
    
    glBegin(GL_QUADS);
    
    glNormal3f(0, 0, 1);
    glVertex3f(-0.4f, -0.8f, 0.05f);
    glVertex3f(0.4f, -0.8f, 0.05f);
    glVertex3f(0.4f, 0.8f, 0.05f);
    glVertex3f(-0.4f, 0.8f, 0.05f);
    
    glNormal3f(0, 0, -1);
    glVertex3f(-0.4f, -0.8f, -0.05f);
    glVertex3f(-0.4f, 0.8f, -0.05f);
    glVertex3f(0.4f, 0.8f, -0.05f);
    glVertex3f(0.4f, -0.8f, -0.05f);
    
    glNormal3f(1, 0, 0);
    glVertex3f(0.4f, -0.8f, -0.05f);
    glVertex3f(0.4f, 0.8f, -0.05f);
    glVertex3f(0.4f, 0.8f, 0.05f);
    glVertex3f(0.4f, -0.8f, 0.05f);
    
    glNormal3f(-1, 0, 0);
    glVertex3f(-0.4f, -0.8f, -0.05f);
    glVertex3f(-0.4f, -0.8f, 0.05f);
    glVertex3f(-0.4f, 0.8f, 0.05f);
    glVertex3f(-0.4f, 0.8f, -0.05f);
    
    glEnd();
    
    setMaterial(0.1f, 0.3f, 0.8f);
    glBegin(GL_QUADS);
    glNormal3f(0, 0, 1);
    glVertex3f(-0.35f, -0.7f, 0.051f);
    glVertex3f(0.35f, -0.7f, 0.051f);
    glVertex3f(0.35f, 0.7f, 0.051f);
    glVertex3f(-0.35f, 0.7f, 0.051f);
    glEnd();
}

void ThreeDView::drawCube()
{
    setMaterial(0.5f, 0.5f, 0.8f);
    
    glBegin(GL_QUADS);
    
    glNormal3f(0, 0, 1);
    glVertex3f(-0.5f, -0.5f, 0.5f);
    glVertex3f(0.5f, -0.5f, 0.5f);
    glVertex3f(0.5f, 0.5f, 0.5f);
    glVertex3f(-0.5f, 0.5f, 0.5f);
    
    glNormal3f(0, 0, -1);
    glVertex3f(-0.5f, -0.5f, -0.5f);
    glVertex3f(-0.5f, 0.5f, -0.5f);
    glVertex3f(0.5f, 0.5f, -0.5f);
    glVertex3f(0.5f, -0.5f, -0.5f);
    
    glNormal3f(0, 1, 0);
    glVertex3f(-0.5f, 0.5f, -0.5f);
    glVertex3f(-0.5f, 0.5f, 0.5f);
    glVertex3f(0.5f, 0.5f, 0.5f);
    glVertex3f(0.5f, 0.5f, -0.5f);
    
    glNormal3f(0, -1, 0);
    glVertex3f(-0.5f, -0.5f, -0.5f);
    glVertex3f(0.5f, -0.5f, -0.5f);
    glVertex3f(0.5f, -0.5f, 0.5f);
    glVertex3f(-0.5f, -0.5f, 0.5f);
    
    glNormal3f(1, 0, 0);
    glVertex3f(0.5f, -0.5f, -0.5f);
    glVertex3f(0.5f, 0.5f, -0.5f);
    glVertex3f(0.5f, 0.5f, 0.5f);
    glVertex3f(0.5f, -0.5f, 0.5f);
    
    glNormal3f(-1, 0, 0);
    glVertex3f(-0.5f, -0.5f, -0.5f);
    glVertex3f(-0.5f, -0.5f, 0.5f);
    glVertex3f(-0.5f, 0.5f, 0.5f);
    glVertex3f(-0.5f, 0.5f, -0.5f);
    
    glEnd();
}

void ThreeDView::drawAxes()
{
    glDisable(GL_LIGHTING);
    glLineWidth(2.0f);
    
    glBegin(GL_LINES);
    
    glColor3f(1, 0, 0);
    glVertex3f(0, 0, 0);
    glVertex3f(2, 0, 0);
    
    glColor3f(0, 1, 0);
    glVertex3f(0, 0, 0);
    glVertex3f(0, 2, 0);
    
    glColor3f(0, 0, 1);
    glVertex3f(0, 0, 0);
    glVertex3f(0, 0, 2);
    
    glEnd();
    
    glEnable(GL_LIGHTING);
}

void ThreeDView::drawGrid()
{
    glDisable(GL_LIGHTING);
    glColor3f(0.3f, 0.3f, 0.3f);
    glLineWidth(1.0f);
    
    glBegin(GL_LINES);
    for (int i = -5; i <= 5; ++i) {
        glVertex3f(i, -2, 0);
        glVertex3f(i, 2, 0);
        glVertex3f(-5, i, 0);
        glVertex3f(5, i, 0);
    }
    glEnd();
    
    glEnable(GL_LIGHTING);
}

void ThreeDView::setupLighting()
{
    GLfloat lightPos[] = {5.0f, 5.0f, 10.0f, 1.0f};
    GLfloat lightAmbient[] = {0.3f, 0.3f, 0.3f, 1.0f};
    GLfloat lightDiffuse[] = {0.8f, 0.8f, 0.8f, 1.0f};
    GLfloat lightSpecular[] = {1.0f, 1.0f, 1.0f, 1.0f};
    
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);
    
    glEnable(GL_LIGHT0);
}

void ThreeDView::setMaterial(float r, float g, float b)
{
    GLfloat ambient[] = {r * 0.3f, g * 0.3f, b * 0.3f, 1.0f};
    GLfloat diffuse[] = {r, g, b, 1.0f};
    GLfloat specular[] = {0.5f, 0.5f, 0.5f, 1.0f};
    GLfloat shininess[] = {50.0f};
    
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shininess);
}

void ThreeDView::mousePressEvent(QMouseEvent *event)
{
    m_lastMousePos = event->pos();
}

void ThreeDView::mouseMoveEvent(QMouseEvent *event)
{
    int dx = event->x() - m_lastMousePos.x();
    int dy = event->y() - m_lastMousePos.y();
    
    if (event->buttons() & Qt::LeftButton) {
        m_cameraRotY += dx * 0.5f;
        m_cameraRotX += dy * 0.5f;
        
        if (m_cameraRotX > 90) m_cameraRotX = 90;
        if (m_cameraRotX < -90) m_cameraRotX = -90;
    }
    
    m_lastMousePos = event->pos();
    update();
}

void ThreeDView::wheelEvent(QWheelEvent *event)
{
    float delta = event->angleDelta().y() / 120.0f;
    m_cameraDistance -= delta * 0.5f;
    
    if (m_cameraDistance < 2.0f) m_cameraDistance = 2.0f;
    if (m_cameraDistance > 20.0f) m_cameraDistance = 20.0f;
    
    update();
}
