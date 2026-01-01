#ifndef STLVIEWER_H
#define STLVIEWER_H

#include <QWidget>
#include <QVector>
#include <QString>
#include <QPushButton>
#include <QFutureWatcher>
#include <QtConcurrent>

#include <vtkSmartPointer.h>
#include <vtkRenderer.h>
#include <vtkActor.h>
#include <QVTKOpenGLNativeWidget.h>
#include <vtkCellPicker.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkInteractorStyleTrackballActor.h>
#include <vtkCylinderSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkAxesActor.h>
#include <vtkOrientationMarkerWidget.h>
#include <vtkProperty.h>
#include <vtkLineSource.h>
#include <vtkDiskSource.h>
#include <vtkTransform.h>
#include <vtkCaptionActor2D.h>
#include <vtkTextActor.h>
#include <vtkTextProperty.h>

#include "CustomInteractorStyle.h"
#include "OrientationOptimizer.h"
#include "slmcommons.h"
#include <QFileDialog>

class StlViewer : public QVTKOpenGLNativeWidget
{
    Q_OBJECT

public:
    StlViewer(QWidget* parent = nullptr);

    void addModel(const QString& stlFilePath);
    QStringList getLoadedModelNames() const;

    void setModelColor(int index, double r, double g, double b);
    void removeModel(int index);
    void setBackgroundColor(double r, double g, double b);
    void arrangeModelsOnPlatter();

    void addGroundPlate();
    void clearBuildPlate();
    QVector<int> getdeletedmodels();
	std::vector<InternalGuiModel> getModels() const;

signals:
    void logMessage(const QString& message);

protected:
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dropEvent(QDropEvent* event) override;

private:
    void finalizeModelArrangement();

    QVTKOpenGLNativeWidget* vtkWidget;
    vtkSmartPointer<vtkRenderer> renderer;
    QVector<ModelInfo> models;

    vtkSmartPointer<vtkCellPicker> picker;
    vtkSmartPointer<CustomInteractorStyle> interactorStyle;

    QPushButton* rotateXBtn;
    QPushButton* rotateYBtn;
    QPushButton* rotateZBtn;
    QPushButton* arrangeBtn;
    QPushButton* pn_toggleButton;
    QPushButton* add_ModelBtn;
	QVector<int> deletedmodels;
    enum RotationAxis { None, XAxis, YAxis, ZAxis };
    RotationAxis selectedRotationAxis = None;

    void showRotationButtons(bool show);
    void rotateSelectedModel(double xDeg, double yDeg, double zDeg);
    void OrientationOptimizerfnc();
    int index = 0;
    double dir = 9;
    vtkSmartPointer<vtkOrientationMarkerWidget> axesWidget;

    QFutureWatcher<void> optimizationWatcher;
    bool optimizationNeeded = false;

    OrientationOptimizer* optimizer = nullptr;  // ✅ Add optimizer as a member
	double build_plate_radius = 100; // Radius of the build plate for arranging models



private slots:
    void onOrientationOptimizationFinished();
};

#endif // STLVIEWER_H
