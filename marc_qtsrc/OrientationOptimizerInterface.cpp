#include "OrientationOptimizerInterface.h"


#include <QDebug>
#include <QtConcurrent>

// Helper struct to pair model index with reference to ModelInfo
struct ModelJob {
    int index;
    ModelInfo* model;
};

OrientationOptimizerInterface::OrientationOptimizerInterface(QObject* parent)
    : QObject(parent), models(nullptr),
    w1(1.0), w2(1.0), w3(1.0), w4(1.0),
    overhangThreshold(45.0) {
}

void OrientationOptimizerInterface::setModels(QVector<ModelInfo>* modelsRef)
{
    models = modelsRef;
}

void OrientationOptimizerInterface::setCriticalFaces(const QMap<int, QVector<vtkIdType>>& modelCriticalFaces)
{
    criticalFaceMap = modelCriticalFaces;
}

void OrientationOptimizerInterface::setWeights(double weight1, double weight2, double weight3, double weight4)
{
    w1 = weight1;
    w2 = weight2;
    w3 = weight3;
    w4 = weight4;
}

QVector<QVector<double>>  OrientationOptimizerInterface::startOptimization(double theta)
{
  QVector<QVector<double>> best_transforms;
    if (!models || models->isEmpty())
        return best_transforms;

    overhangThreshold = theta;

    best_transforms.reserve(models->size()); // optional: performance optimization

    for (int i = 0; i < models->size(); ++i) {
        ModelInfo& model = (*models)[i];

        OrientationOptimizer optimizer;

        if (criticalFaceMap.contains(i)) {
            optimizer.setCriticalFaceIds(criticalFaceMap[i]);
        }
        
        QVector<double>  bestTransform;
           optimizer.optimizeModelOrientation(model, theta);

       // model.transform = bestTransform;
       // if (model.actor)
           // model.actor->SetUserTransform(bestTransform);

        //best_transforms.push_back(bestTransform);

       // emit modelOptimized(i); // optional: for UI updates
    }
    return  best_transforms;
   // emit optimizationFinished(); // optional: if you're tracking full completion
}

