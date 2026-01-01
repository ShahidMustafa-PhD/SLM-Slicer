#ifndef BUILDVOLUMEVISU ALIZER_H
#define BUILDVOLUME VISUALIZER_H

#include <vtkSmartPointer.h>
#include <vtkRenderer.h>
#include <vtkActor.h>
#include <vtkCylinderSource.h>
#include <vtkLineSource.h>

namespace MarcSLM {
namespace Infrastructure {

/**
 * @brief Visualizes build volume boundaries
 * 
 * Features:
 * - Cylindrical boundary wireframe
 * - Height limit indicators
 * - Grid on build plate
 * - Toggleable visibility
 */
class BuildVolumeVisualizer {
public:
    explicit BuildVolumeVisualizer(vtkRenderer* renderer);
    ~BuildVolumeVisualizer();
    
    /**
     * @brief Set build volume dimensions
     * @param radius Cylinder radius in mm
     * @param height Cylinder height in mm
     */
    void setDimensions(double radius, double height);
    
    /**
     * @brief Enable/disable visualization
     */
    void setVisible(bool visible);
    bool isVisible() const { return m_visible; }
    
    /**
     * @brief Set wireframe color
     */
    void setColor(double r, double g, double b);
    
    /**
     * @brief Set line width
     */
    void setLineWidth(float width);
    
private:
    vtkRenderer* m_renderer;  // Not owned
    
    // Visualization actors
    vtkSmartPointer<vtkActor> m_cylinderActor;
    vtkSmartPointer<vtkActor> m_topCircleActor;
    vtkSmartPointer<vtkActor> m_bottomCircleActor;
    vtkSmartPointer<vtkActor> m_gridActor;
    
    // Geometry sources
    vtkSmartPointer<vtkCylinderSource> m_cylinderSource;
    
    double m_radius = 100.0;
    double m_height = 200.0;
    bool m_visible = true;
    
    void createVisualization();
    void updateVisualization();
    void createCylinderWireframe();
    void createHeightLimitIndicators();
    void createBuildPlateGrid();
};

} // namespace Infrastructure
} // namespace MarcSLM

#endif // BUILDVOLUMEVISUALIZER_H
