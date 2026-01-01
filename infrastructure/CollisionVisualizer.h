#ifndef COLLISIONVISUALIZER_H
#define COLLISIONVISUALIZER_H

#include <vtkSmartPointer.h>
#include <vtkRenderer.h>
#include <vtkActor.h>
#include <map>
#include <vector>

namespace MarcSLM {

namespace Domain {
    class BuildPlate;
}

namespace Infrastructure {

/**
 * @brief Visualizes collision detection results
 * 
 * Features:
 * - Highlights colliding models in red
 * - Shows collision count
 * - Updates dynamically as models move
 */
class CollisionVisualizer {
public:
    explicit CollisionVisualizer(vtkSmartPointer<vtkRenderer> renderer);
    ~CollisionVisualizer() = default;
    
    /**
     * @brief Update collision visualization based on build plate state
     * @param buildPlate Domain object containing models
     * @param modelToActorMap Mapping from model ID to VTK actor ID
     */
    void update(const Domain::BuildPlate* buildPlate,
                const std::map<int, int>& modelToActorMap);
    
    /**
     * @brief Clear all collision highlights
     */
    void clearHighlights();
    
    /**
     * @brief Get number of collision pairs detected
     */
    int getCollisionCount() const { return m_collisionCount; }
    
    /**
     * @brief Enable/disable collision visualization
     */
    void setEnabled(bool enabled);
    bool isEnabled() const { return m_enabled; }
    
private:
    vtkSmartPointer<vtkRenderer> m_renderer;  // Now owned/shared via smart pointer
    bool m_enabled = true;
    int m_collisionCount = 0;
    
    // Map from actor ID to original color
    std::map<int, std::tuple<double, double, double>> m_originalColors;
    
    // Currently highlighted actors
    std::vector<int> m_highlightedActors;
    
    void highlightActor(int actorId, double r, double g, double b);
    void restoreActorColor(int actorId);
};

} // namespace Infrastructure
} // namespace MarcSLM

#endif // COLLISIONVISUALIZER_H
