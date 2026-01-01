#ifndef MODEL_H
#define MODEL_H

#include "Transform.h"
#include "BoundingBox.h"
#include <string>
#include <memory>

namespace MarcSLM {
namespace Domain {

/**
 * @brief Domain model representing a 3D model on the build plate
 * 
 * This is a pure business entity with no dependencies on UI frameworks
 * (Qt) or rendering libraries (VTK). It stores only the essential data
 * needed for model management and export to the slicer DLL.
 * 
 * Design principles:
 * - Immutable file path (models don't change their source file)
 * - Mutable transform (users can position/rotate models)
 * - Metadata cached for performance (triangle count, volume, bounds)
 */
class Model {
public:
    /**
     * @brief Default constructor (for use in containers like std::map)
     */
    Model() : m_id(-1), m_filePath(""), m_triangleCount(0), m_volume(0.0) {}
    
    /**
     * @brief Construct a new Model from an STL file path
     * @param filePath Absolute path to STL file
     * @param id Unique identifier (managed by BuildPlate)
     */
    explicit Model(const std::string& filePath, int id = -1);
    
    // Getters
    int id() const { return m_id; }
    std::string filePath() const { return m_filePath; }
    Transform transform() const { return m_transform; }
    BoundingBox bounds() const { return m_bounds; }
    int triangleCount() const { return m_triangleCount; }
    double volume() const { return m_volume; }
    
    // Setters
    void setTransform(const Transform& t) { m_transform = t; }
    void setBounds(const BoundingBox& b) { m_bounds = b; }
    void setTriangleCount(int count) { m_triangleCount = count; }
    void setVolume(double vol) { m_volume = vol; }
    
    /**
     * @brief Get axis-aligned bounding box in world coordinates
     * 
     * Applies the model's transform to the local bounds to compute
     * the world-space AABB. This is used for collision detection
     * and build volume validation.
     */
    BoundingBox worldBounds() const;
    
    /**
     * @brief Check if this model collides with another
     * @param other Another model to test against
     * @return true if bounding boxes intersect
     */
    bool collidesWith(const Model& other) const;
    
private:
    int m_id;
    std::string m_filePath;
    Transform m_transform;
    BoundingBox m_bounds;       // Local bounds (untransformed)
    int m_triangleCount = 0;
    double m_volume = 0.0;      // Volume in mm³
};

} // namespace Domain
} // namespace MarcSLM

#endif // MODEL_H
