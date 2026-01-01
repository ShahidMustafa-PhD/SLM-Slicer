#include "OrientationOptimizer.h"

#include <vtkDataSetMapper.h>
#include <vtkPolyDataMapper.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkTriangle.h>
#include <vtkCellData.h>
#include <vtkMath.h>
#include <vtkPointData.h>
#include <vtkCell.h>
#include <vtkPolyDataNormals.h>
#include <vtkSmartPointer.h>
#include <vtkTransform.h>

#include <QDebug>
#include <limits>
OrientationOptimizer::OrientationOptimizer(QObject* parent)
    : QObject(parent)
{
    // constructor body if needed
}

// Set user-defined critical face IDs
void OrientationOptimizer::setCriticalFaceIds(const QVector<vtkIdType>& ids) {
    criticalFaceIds = ids;
}

// Optimize orientation based on minimal support area (lower Z-facing surfaces)
void OrientationOptimizer::optimizeModelOrientation(ModelInfo& model, double overhangThresholdDegrees) {
    if (!model.actor) {
        qWarning("optimizeModelOrientation: model.actor is null.");
        return;
    }

    auto mesh = getPolyData(model.actor);
    if (!mesh) {
        //qWarning("optimizeModelOrientation: Failed to get vtkPolyData from actor.");
        emit logMessage(QString(" Failed to get vtkPolyData from actor."));
        return;
    }

    double angleThreshold = vtkMath::RadiansFromDegrees(overhangThresholdDegrees);
    double bestScore = std::numeric_limits<double>::max();
    vtkSmartPointer<vtkTransform> bestTransform = vtkSmartPointer<vtkTransform>::New();

    double b_roll = 0.0, b_pitch = 0.0;

    for (double pitch = 0; pitch <= 180; pitch += 5) {
        for (double roll = 0; roll <= 360; roll += 10) {
            auto transform = vtkSmartPointer<vtkTransform>::New();
            transform->Identity();
            transform->RotateY(pitch);
            transform->RotateX(roll);

            auto transformer = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
            transformer->SetInputData(mesh);
            transformer->SetTransform(transform);
            transformer->Update();

            auto rotated = transformer->GetOutput();
            if (!rotated) {
                qWarning("optimizeModelOrientation: Transformation failed.");
                continue;
            }

            double supportVol = computeSupportVolume(rotated, angleThreshold);
            double score = supportVol;

            if (score < bestScore) {
                bestScore = score;
                bestTransform->DeepCopy(transform);
                b_roll = roll;
                b_pitch = pitch;
            }
        }
    }

    model.actor->SetUserTransform(bestTransform);
    model.best_orientation_angles[0] = b_roll;
    model.best_orientation_angles[1] = b_pitch;
    model.best_orientation_angles[2] = 0.0; // yaw assumed 0

    emit logMessage(QString("Best Orientation Found - Roll: %1, Pitch: %2").arg(b_roll).arg(b_pitch));

}

vtkSmartPointer<vtkPolyData> OrientationOptimizer::getPolyData(vtkSmartPointer<vtkActor> actor) {
    if (!actor) return nullptr;

    auto mapper = vtkPolyDataMapper::SafeDownCast(actor->GetMapper());
    if (!mapper) {
        qWarning("getPolyData: Mapper is not vtkPolyDataMapper.");
        return nullptr;
    }

    mapper->Update();

    auto input = mapper->GetInput();
    if (!input) {
        qWarning("getPolyData: Mapper input is null.");
        return nullptr;
    }

    auto polyData = vtkPolyData::SafeDownCast(input);
    if (!polyData) {
        qWarning("getPolyData: Input is not vtkPolyData.");
        return nullptr;
    }

    return polyData;
}


// Compute support volume (total area of faces needing support)
double OrientationOptimizer::computeSupportVolume(vtkSmartPointer<vtkPolyData> mesh, double angleThreshold) {
    if (!mesh) return 0.0;

    auto normalGen = vtkSmartPointer<vtkPolyDataNormals>::New();
    normalGen->SetInputData(mesh);
    normalGen->ComputePointNormalsOff();
    normalGen->ComputeCellNormalsOn();
    normalGen->Update();

    auto polyData = normalGen->GetOutput();
    if (!polyData) return 0.0;

    auto normals = polyData->GetCellData()->GetNormals();
    if (!normals) return 0.0;

    double totalArea = 0.0;
    double supportArea = 0.0;

    for (vtkIdType i = 0; i < polyData->GetNumberOfCells(); ++i) {
        auto cell = polyData->GetCell(i);
        if (!cell || cell->GetCellType() != VTK_TRIANGLE) continue;

        double p0[3], p1[3], p2[3];
        polyData->GetPoint(cell->GetPointId(0), p0);
        polyData->GetPoint(cell->GetPointId(1), p1);
        polyData->GetPoint(cell->GetPointId(2), p2);

        double area = vtkTriangle::TriangleArea(p0, p1, p2);
        totalArea += area;

        double* normal = normals->GetTuple(i);
        if (normal[2] < std::cos(angleThreshold)) {
            supportArea += area;
        }
    }

    return supportArea;
}

// Compute overhang penalty (currently same as support volume)
double OrientationOptimizer::computeOverhangPenalty(vtkSmartPointer<vtkPolyData> mesh, double angleThreshold) {
    return computeSupportVolume(mesh, angleThreshold);
}

// Evaluate penalty based on critical face overhang
double OrientationOptimizer::computeCriticalSurfacePenalty(vtkSmartPointer<vtkPolyData> mesh, double angleThreshold) {
    if (!mesh || criticalFaceIds.isEmpty()) return 0.0;

    auto normalGen = vtkSmartPointer<vtkPolyDataNormals>::New();
    normalGen->SetInputData(mesh);
    normalGen->ComputePointNormalsOff();
    normalGen->ComputeCellNormalsOn();
    normalGen->Update();

    auto polyData = normalGen->GetOutput();
    if (!polyData) return 0.0;

    auto normals = polyData->GetCellData()->GetNormals();
    if (!normals) return 0.0;

    double penalty = 0.0;
    int count = 0;

    for (vtkIdType id : criticalFaceIds) {
        if (id < 0 || id >= polyData->GetNumberOfCells()) continue;

        double* normal = normals->GetTuple(id);
        if (normal && normal[2] < std::cos(angleThreshold)) {
            penalty += 1.0;
        }
        ++count;
    }

    return count > 0 ? (penalty / count) : 0.0;
}

// Compute model height from bounds (Z-axis)
double OrientationOptimizer::computeBuildHeight(double bounds[6]) {
    return bounds[5] - bounds[4];
}
