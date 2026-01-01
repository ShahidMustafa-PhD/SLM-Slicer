#ifndef BOUNDINGBOX_H
#define BOUNDINGBOX_H

#include <algorithm>
#include <cmath>

namespace MarcSLM {
namespace Domain {

/**
 * @brief Axis-Aligned Bounding Box (AABB) for 3D models
 * 
 * Represents the spatial extent of a model in world coordinates.
 * All dimensions are in millimeters.
 */
struct BoundingBox {
    double minX = 0.0;
    double maxX = 0.0;
    double minY = 0.0;
    double maxY = 0.0;
    double minZ = 0.0;
    double maxZ = 0.0;
    
    BoundingBox() = default;
    
    BoundingBox(double minx, double maxx, 
                double miny, double maxy, 
                double minz, double maxz)
        : minX(minx), maxX(maxx)
        , minY(miny), maxY(maxy)
        , minZ(minz), maxZ(maxz) {}
    
    // Dimensions
    double width() const { return maxX - minX; }
    double height() const { return maxZ - minZ; }
    double depth() const { return maxY - minY; }
    
    // Center point
    double centerX() const { return (minX + maxX) / 2.0; }
    double centerY() const { return (minY + maxY) / 2.0; }
    double centerZ() const { return (minZ + maxZ) / 2.0; }
    
    // Volume
    double volume() const { 
        return width() * height() * depth(); 
    }
    
    // Check if this box intersects another (AABB collision detection)
    bool intersects(const BoundingBox& other) const {
        return (minX <= other.maxX && maxX >= other.minX) &&
               (minY <= other.maxY && maxY >= other.minY) &&
               (minZ <= other.maxZ && maxZ >= other.minZ);
    }
    
    // Check if a point is inside this box
    bool contains(double x, double y, double z) const {
        return (x >= minX && x <= maxX) &&
               (y >= minY && y <= maxY) &&
               (z >= minZ && z <= maxZ);
    }
    
    // Expand box to include another box
    void expand(const BoundingBox& other) {
        minX = std::min(minX, other.minX);
        maxX = std::max(maxX, other.maxX);
        minY = std::min(minY, other.minY);
        maxY = std::max(maxY, other.maxY);
        minZ = std::min(minZ, other.minZ);
        maxZ = std::max(maxZ, other.maxZ);
    }
    
    // Check if box is valid (max > min)
    bool isValid() const {
        return maxX >= minX && maxY >= minY && maxZ >= minZ;
    }
};

} // namespace Domain
} // namespace MarcSLM

#endif // BOUNDINGBOX_H
