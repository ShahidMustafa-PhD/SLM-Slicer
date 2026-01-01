#ifndef PROPERTIESPANEL_H
#define PROPERTIESPANEL_H

#include <QWidget>
#include <QLabel>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QGroupBox>
#include <QVBoxLayout>

#include "../core/domain/Transform.h"

namespace MarcSLM {
namespace Presentation {

class MainWindowViewModel;

/**
 * @brief Industrial-grade properties panel for the right side
 * 
 * Features:
 * - Transform controls (position + rotation)
 * - Precise numeric input with spinboxes
 * - Model information display
 * - Auto-Orient button
 * - Reset transform button
 * - Configuration assignment
 */
class PropertiesPanel : public QWidget {
    Q_OBJECT
    
public:
    explicit PropertiesPanel(MainWindowViewModel* viewModel, QWidget* parent = nullptr);
    
public slots:
    void onModelSelected(int modelId);
    void onTransformChanged(int modelId);
    void clearSelection();
    
signals:
    void transformUpdated(int modelId, const Domain::Transform& transform);
    void autoOrientRequested(int modelId);
    void resetTransformRequested(int modelId);
    
private slots:
    void onTransformSpinBoxChanged();
    void onAutoOrientClicked();
    void onResetClicked();
    
private:
    MainWindowViewModel* m_viewModel;
    int m_currentModelId = -1;
    bool m_updating = false;  // Prevent feedback loops
    
    // Transform controls
    QDoubleSpinBox* m_posXSpin;
    QDoubleSpinBox* m_posYSpin;
    QDoubleSpinBox* m_posZSpin;
    
    QDoubleSpinBox* m_rotRollSpin;
    QDoubleSpinBox* m_rotPitchSpin;
    QDoubleSpinBox* m_rotYawSpin;
    
    // Model info labels
    QLabel* m_fileNameLabel;
    QLabel* m_trianglesLabel;
    QLabel* m_volumeLabel;
    QLabel* m_boundsLabel;
    
    // Buttons
    QPushButton* m_autoOrientButton;
    QPushButton* m_resetButton;
    
    void setupUI();
    void applyIndustrialStyle();
    void updateTransformControls(const Domain::Transform& transform);
    void updateModelInfo();
    
    QDoubleSpinBox* createPositionSpinBox();
    QDoubleSpinBox* createRotationSpinBox();
};

} // namespace Presentation
} // namespace MarcSLM

#endif // PROPERTIESPANEL_H
