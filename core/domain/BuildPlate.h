#ifndef BUILDPLATE_H
#define BUILDPLATE_H

#include "Model.h"
#include <unordered_map>
#include <vector>
#include <memory>
#include <utility>

namespace MarcSLM {
namespace Domain {

/**
 * @brief Manages the collection of models on the build plate
 * 
 * Uses unordered_map instead of vector to avoid pointer invalidation
 * when adding/removing models.
 * 
 * Responsibilities:
 * - Add/remove models
 * - Validate models are within build volume
 * - Detect collisions between models
 * - Provide model lookup by ID
 * 
 * Build volume is defined as a cylinder:
 * - Radius: horizontal extent (X-Y plane)
 * - Height: vertical extent (Z-axis)
 */
class BuildPlate {
public:
    /**
     * @brief Construct a build plate with specified dimensions
     * @param radius Build plate radius in millimeters
     * @param height Maximum build height in millimeters
     */
    BuildPlate(double radius = 100.0, double height = 200.0);
    
    // Model management
    int addModel(const Model& model);
    bool removeModel(int id);
    void clear();
    
    // Queries
    std::shared_ptr<Model> getModel(int id);
    std::shared_ptr<const Model> getModel(int id) const;
    std::vector<std::shared_ptr<Model>> getAllModels();
    std::vector<std::shared_ptr<const Model>> getAllModels() const;
    int modelCount() const { return static_cast<int>(m_modelsById.size()); }
    
    // Validation
    bool isInsideBuildVolume(const Model& model) const;
    bool hasCollisions() const;
    std::vector<std::pair<int, int>> detectCollisions() const;
    
    // Build volume properties
    double radius() const { return m_radius; }
    double height() const { return m_height; }
    void setDimensions(double radius, double height);
    
    // Statistics
    double usedVolume() const;
    double usedVolumePercentage() const;
    
private:
    std::unordered_map<int, std::shared_ptr<Model>> m_modelsById;  // ? Map instead of vector (safer!)
    double m_radius;
    double m_height;
    int m_nextId = 1;  // Auto-increment ID for new models
    
    bool isInsideCylinder(double x, double y, double z) const;
};

} // namespace Domain
} // namespace MarcSLM

#endif // BUILDPLATE_H
