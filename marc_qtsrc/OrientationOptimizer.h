#ifndef ORIENTATIONOPTIMIZER_H
#define ORIENTATIONOPTIMIZER_H

#include <QObject>
#include <QString>
#include <QVector>
#include <vtkSmartPointer.h>
#include <vtkActor.h>
#include <vtkTransform.h>
#include <vtkPolyData.h>
#include <vtkRenderer.h>
#include <vtkTriangleFilter.h>
#include <vtkPolyDataNormals.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkMath.h>
#include <vtkTriangle.h>
//#include "StlViewer.h"
#include "slmcommons.h"
#include <QObject>

class OrientationOptimizer : public QObject
{
    Q_OBJECT  // Enable Qt signals/slots
public:
    explicit OrientationOptimizer(QObject* parent = nullptr);  // add explicit constructor with parent
 
    void optimizeModelOrientation(ModelInfo& model, double overhangThresholdDegrees);

        double b_roll = 0;  // best_rotation about x axis
         double b_pitch = 0;  // best_rotation about y axis
        double  b_yaw = 0;  // best_rotation about z axis

    void setCriticalFaceIds(const QVector<vtkIdType>& ids); // Faces to preserve
    

signals:
    void logMessage(const QString& message);
private:
    QVector<vtkIdType> criticalFaceIds;

    double computeSupportVolume(vtkSmartPointer<vtkPolyData> mesh, double angleThreshold);
    double computeOverhangPenalty(vtkSmartPointer<vtkPolyData> mesh, double angleThreshold);
    double computeBuildHeight(double bounds[6]);
    double computeCriticalSurfacePenalty(vtkSmartPointer<vtkPolyData> mesh, double angleThreshold);

    vtkSmartPointer<vtkPolyData> getPolyData(vtkSmartPointer<vtkActor> actor);
};

#endif // ORIENTATIONOPTIMIZER_H
