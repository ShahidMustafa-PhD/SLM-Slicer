#include "Model.h"
#include <cmath>

namespace MarcSLM {
namespace Domain {

Model::Model(const std::string& filePath, int id)
    : m_id(id)
    , m_filePath(filePath)
    , m_transform()
    , m_bounds()
    , m_triangleCount(0)
    , m_volume(0.0)
{
}

BoundingBox Model::worldBounds() const {
    // For now, return a simple translation of local bounds
    // TODO: Implement proper rotation-aware AABB computation
    // when orientation optimization is integrated
    
    BoundingBox world = m_bounds;
    world.minX += m_transform.x;
    world.maxX += m_transform.x;
    world.minY += m_transform.y;
    world.maxY += m_transform.y;
    world.minZ += m_transform.z;
    world.maxZ += m_transform.z;
    
    return world;
}

bool Model::collidesWith(const Model& other) const {
    BoundingBox myWorld = worldBounds();
    BoundingBox otherWorld = other.worldBounds();
    
    return myWorld.intersects(otherWorld);
}

} // namespace Domain
} // namespace MarcSLM
