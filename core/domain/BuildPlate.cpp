#include "BuildPlate.h"
#include <cmath>
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace MarcSLM {
namespace Domain {

BuildPlate::BuildPlate(double radius, double height)
    : m_radius(radius)
    , m_height(height)
    , m_nextId(1)
{
}

int BuildPlate::addModel(const Model& model) {
    int assignedId;
    
    if (model.id() < 0) {
        assignedId = m_nextId++;
        Model newModel(model.filePath(), assignedId);
        newModel.setTransform(model.transform());
        newModel.setBounds(model.bounds());
        newModel.setTriangleCount(model.triangleCount());
        newModel.setVolume(model.volume());
        m_modelsById[assignedId] = std::make_shared<Model>(newModel);  // ? Map insertion, no reallocation of existing elements!
    } else {
        assignedId = model.id();
        m_nextId = std::max(m_nextId, model.id() + 1);
        m_modelsById[assignedId] = std::make_shared<Model>(model);  // ? Safe insertion
    }
    
    return assignedId;
}

bool BuildPlate::removeModel(int id) {
    auto it = m_modelsById.find(id);
    if (it != m_modelsById.end()) {
        m_modelsById.erase(it);
        return true;
    }
    return false;
}

void BuildPlate::clear() {
    m_modelsById.clear();
    m_nextId = 1;
}

std::shared_ptr<Model> BuildPlate::getModel(int id) {
    auto it = m_modelsById.find(id);
    return (it != m_modelsById.end()) ? it->second : nullptr;
}

std::shared_ptr<const Model> BuildPlate::getModel(int id) const {
    auto it = m_modelsById.find(id);
    return (it != m_modelsById.end()) ? it->second : nullptr;  // ? Safe const pointer
}

std::vector<std::shared_ptr<Model>> BuildPlate::getAllModels() {
    std::vector<std::shared_ptr<Model>> result;
    result.reserve(m_modelsById.size());
    for (auto& pair : m_modelsById) {
        result.push_back(pair.second);
    }
    return result;
}

std::vector<std::shared_ptr<const Model>> BuildPlate::getAllModels() const {
    std::vector<std::shared_ptr<const Model>> result;
    result.reserve(m_modelsById.size());
    for (const auto& pair : m_modelsById) {
        result.push_back(pair.second);
    }
    return result;
}

bool BuildPlate::isInsideBuildVolume(const Model& model) const {
    BoundingBox bounds = model.worldBounds();
    
    // Check height constraint
    if (bounds.minZ < 0.0 || bounds.maxZ > m_height) {
        return false;
    }
    
    // Check if all 8 corners of bounding box are inside cylinder
    double corners[8][3] = {
        {bounds.minX, bounds.minY, bounds.minZ},
        {bounds.minX, bounds.minY, bounds.maxZ},
        {bounds.minX, bounds.maxY, bounds.minZ},
        {bounds.minX, bounds.maxY, bounds.maxZ},
        {bounds.maxX, bounds.minY, bounds.minZ},
        {bounds.maxX, bounds.minY, bounds.maxZ},
        {bounds.maxX, bounds.maxY, bounds.minZ},
        {bounds.maxX, bounds.maxY, bounds.maxZ}
    };
    
    for (int i = 0; i < 8; ++i) {
        if (!isInsideCylinder(corners[i][0], corners[i][1], corners[i][2])) {
            return false;
        }
    }
    
    return true;
}

bool BuildPlate::hasCollisions() const {
    return !detectCollisions().empty();
}

std::vector<std::pair<int, int>> BuildPlate::detectCollisions() const {
    std::vector<std::pair<int, int>> collisions;
    
    // Build vector of all models for collision detection
    auto allModels = getAllModels();
    
    for (size_t i = 0; i < allModels.size(); ++i) {
        for (size_t j = i + 1; j < allModels.size(); ++j) {
            if (allModels[i]->collidesWith(*allModels[j])) {
                collisions.push_back({allModels[i]->id(), allModels[j]->id()});
            }
        }
    }
    
    return collisions;
}

void BuildPlate::setDimensions(double radius, double height) {
    m_radius = radius;
    m_height = height;
}

double BuildPlate::usedVolume() const {
    double total = 0.0;
    for (const auto& pair : m_modelsById) {
        total += pair.second->volume();
    }
    return total;
}

double BuildPlate::usedVolumePercentage() const {
    double buildVolume = M_PI * m_radius * m_radius * m_height;
    if (buildVolume <= 0.0) return 0.0;
    
    return (usedVolume() / buildVolume) * 100.0;
}

bool BuildPlate::isInsideCylinder(double x, double y, double z) const {
    // Check Z bounds
    if (z < 0.0 || z > m_height) {
        return false;
    }
    
    // Check radial distance from center
    double distanceFromCenter = std::sqrt(x * x + y * y);
    return distanceFromCenter <= m_radius;
}

} // namespace Domain
} // namespace MarcSLM
