#ifndef ORIENTATIONOPTIMIZERINTERFACE_H
#define ORIENTATIONOPTIMIZERINTERFACE_H

#include <QObject>
#include <QVector>
#include <QMap>
#include <QMutex>
#include <QFutureWatcher>
#include <QPointer>
#include <QThreadPool>

#include <vtkSmartPointer.h>
#include <vtkActor.h>
#include <vtkTransform.h>
#include "StlViewer.h"
#include <QtConcurrent/QtConcurrent>
#include "OrientationOptimizer.h"

class OrientationOptimizer;

class OrientationOptimizerInterface : public QObject {
    Q_OBJECT

public:
    explicit OrientationOptimizerInterface(QObject* parent = nullptr);

    void setModels(QVector<ModelInfo>* modelsRef);
    void setCriticalFaces(const QMap<int, QVector<vtkIdType>>& modelCriticalFaces);
    void setWeights(double weight1, double weight2, double weight3, double weight4);
    QVector<QVector<double>> startOptimization(double theta);

signals:
    void modelOptimized(int index);
    void optimizationFinished();

private:
    QVector<ModelInfo>* models;
    QMap<int, QVector<vtkIdType>> criticalFaceMap;

    double w1, w2, w3, w4;
    double overhangThreshold;
};

#endif // ORIENTATIONOPTIMIZERINTERFACE_H
