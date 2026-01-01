#include "PropertiesPanel.h"
#include "MainWindowViewModel.h"
#include "../core/domain/BuildPlate.h"
#include "../core/domain/Model.h"

#include <QFormLayout>
#include <QFileInfo>

namespace MarcSLM {
namespace Presentation {

PropertiesPanel::PropertiesPanel(MainWindowViewModel* viewModel, QWidget* parent)
    : QWidget(parent)
    , m_viewModel(viewModel)
{
    setupUI();
    applyIndustrialStyle();
    
    // Connect to view model
    connect(m_viewModel, &MainWindowViewModel::transformChanged,
            this, &PropertiesPanel::onTransformChanged);
}

void PropertiesPanel::setupUI() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(15);
    
    // Header
    auto* headerLabel = new QLabel("Properties", this);
    QFont headerFont = headerLabel->font();
    headerFont.setPointSize(12);
    headerFont.setBold(true);
    headerLabel->setFont(headerFont);
    mainLayout->addWidget(headerLabel);
    
    // Transform Group
    auto* transformGroup = new QGroupBox("Transform", this);
    auto* transformLayout = new QFormLayout(transformGroup);
    transformLayout->setSpacing(8);
    
    // Position
    m_posXSpin = createPositionSpinBox();
    m_posYSpin = createPositionSpinBox();
    m_posZSpin = createPositionSpinBox();
    
    transformLayout->addRow("Position X (mm):", m_posXSpin);
    transformLayout->addRow("Position Y (mm):", m_posYSpin);
    transformLayout->addRow("Position Z (mm):", m_posZSpin);
    
    // Rotation
    m_rotRollSpin = createRotationSpinBox();
    m_rotPitchSpin = createRotationSpinBox();
    m_rotYawSpin = createRotationSpinBox();
    
    transformLayout->addRow("Roll (°):", m_rotRollSpin);
    transformLayout->addRow("Pitch (°):", m_rotPitchSpin);
    transformLayout->addRow("Yaw (°):", m_rotYawSpin);
    
    mainLayout->addWidget(transformGroup);
    
    // Transform action buttons
    auto* transformButtonLayout = new QHBoxLayout();
    
    m_autoOrientButton = new QPushButton("?? Auto-Orient", this);
    m_resetButton = new QPushButton("Reset", this);
    
    transformButtonLayout->addWidget(m_autoOrientButton);
    transformButtonLayout->addWidget(m_resetButton);
    
    mainLayout->addLayout(transformButtonLayout);
    
    // Model Info Group
    auto* infoGroup = new QGroupBox("Model Info", this);
    auto* infoLayout = new QFormLayout(infoGroup);
    infoLayout->setSpacing(6);
    
    m_fileNameLabel = new QLabel("--", this);
    m_trianglesLabel = new QLabel("--", this);
    m_volumeLabel = new QLabel("--", this);
    m_boundsLabel = new QLabel("--", this);
    
    m_fileNameLabel->setWordWrap(true);
    
    infoLayout->addRow("File:", m_fileNameLabel);
    infoLayout->addRow("Triangles:", m_trianglesLabel);
    infoLayout->addRow("Volume:", m_volumeLabel);
    infoLayout->addRow("Bounds:", m_boundsLabel);
    
    mainLayout->addWidget(infoGroup);
    
    mainLayout->addStretch();
    
    // Connect spinbox signals
    connect(m_posXSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &PropertiesPanel::onTransformSpinBoxChanged);
    connect(m_posYSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &PropertiesPanel::onTransformSpinBoxChanged);
    connect(m_posZSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &PropertiesPanel::onTransformSpinBoxChanged);
    connect(m_rotRollSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &PropertiesPanel::onTransformSpinBoxChanged);
    connect(m_rotPitchSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &PropertiesPanel::onTransformSpinBoxChanged);
    connect(m_rotYawSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &PropertiesPanel::onTransformSpinBoxChanged);
    
    // Connect button signals
    connect(m_autoOrientButton, &QPushButton::clicked,
            this, &PropertiesPanel::onAutoOrientClicked);
    connect(m_resetButton, &QPushButton::clicked,
            this, &PropertiesPanel::onResetClicked);
    
    // Initially disabled
    setEnabled(false);
}

void PropertiesPanel::applyIndustrialStyle() {
    QString style = R"(
        QWidget {
            background-color: #2E3440;
            color: #ECEFF4;
            font-family: 'Segoe UI', sans-serif;
        }
        
        QLabel {
            color: #ECEFF4;
        }
        
        QGroupBox {
            border: 1px solid #4C566A;
            border-radius: 4px;
            margin-top: 10px;
            padding-top: 10px;
            font-weight: bold;
        }
        
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 5px;
        }
        
        QDoubleSpinBox {
            background-color: #3B4252;
            border: 1px solid #4C566A;
            border-radius: 3px;
            padding: 4px;
            color: #ECEFF4;
        }
        
        QDoubleSpinBox:focus {
            border: 1px solid #5E81AC;
        }
        
        QPushButton {
            background-color: #4C566A;
            color: white;
            border: none;
            border-radius: 4px;
            padding: 8px 12px;
            font-weight: bold;
        }
        
        QPushButton:hover {
            background-color: #5E81AC;
        }
        
        QPushButton:pressed {
            background-color: #81A1C1;
        }
        
        QPushButton:disabled {
            background-color: #3B4252;
            color: #666;
        }
    )";
    
    setStyleSheet(style);
}

QDoubleSpinBox* PropertiesPanel::createPositionSpinBox() {
    auto* spin = new QDoubleSpinBox(this);
    spin->setRange(-1000.0, 1000.0);
    spin->setDecimals(2);
    spin->setSingleStep(1.0);
    spin->setValue(0.0);
    return spin;
}

QDoubleSpinBox* PropertiesPanel::createRotationSpinBox() {
    auto* spin = new QDoubleSpinBox(this);
    spin->setRange(-180.0, 180.0);
    spin->setDecimals(1);
    spin->setSingleStep(5.0);
    spin->setValue(0.0);
    spin->setWrapping(true);
    return spin;
}

void PropertiesPanel::onModelSelected(int modelId) {
    m_currentModelId = modelId;
    setEnabled(true);
    
    // Update controls with model's current transform
    Domain::Transform transform = m_viewModel->getModelTransform(modelId);
    updateTransformControls(transform);
    updateModelInfo();
}

void PropertiesPanel::onTransformChanged(int modelId) {
    if (modelId == m_currentModelId) {
        Domain::Transform transform = m_viewModel->getModelTransform(modelId);
        updateTransformControls(transform);
    }
}

void PropertiesPanel::clearSelection() {
    m_currentModelId = -1;
    setEnabled(false);
    
    m_fileNameLabel->setText("--");
    m_trianglesLabel->setText("--");
    m_volumeLabel->setText("--");
    m_boundsLabel->setText("--");
}

void PropertiesPanel::onTransformSpinBoxChanged() {
    if (m_updating || m_currentModelId < 0) {
        return;
    }
    
    // Read values from spinboxes
    Domain::Transform transform;
    transform.x = m_posXSpin->value();
    transform.y = m_posYSpin->value();
    transform.z = m_posZSpin->value();
    transform.roll = m_rotRollSpin->value();
    transform.pitch = m_rotPitchSpin->value();
    transform.yaw = m_rotYawSpin->value();
    
    // Update view model
    m_viewModel->updateModelTransform(m_currentModelId, transform);
    emit transformUpdated(m_currentModelId, transform);
}

void PropertiesPanel::onAutoOrientClicked() {
    if (m_currentModelId >= 0) {
        emit autoOrientRequested(m_currentModelId);
    }
}

void PropertiesPanel::onResetClicked() {
    if (m_currentModelId < 0) {
        return;
    }
    
    Domain::Transform resetTransform;
    m_viewModel->updateModelTransform(m_currentModelId, resetTransform);
    emit resetTransformRequested(m_currentModelId);
}

void PropertiesPanel::updateTransformControls(const Domain::Transform& transform) {
    m_updating = true;
    
    m_posXSpin->setValue(transform.x);
    m_posYSpin->setValue(transform.y);
    m_posZSpin->setValue(transform.z);
    m_rotRollSpin->setValue(transform.roll);
    m_rotPitchSpin->setValue(transform.pitch);
    m_rotYawSpin->setValue(transform.yaw);
    
    m_updating = false;
}

void PropertiesPanel::updateModelInfo() {
    if (m_currentModelId < 0) {
        return;
    }
    
    // TODO: Get model from BuildPlate via ViewModel
    // For now, show placeholder
    m_fileNameLabel->setText("model.stl");
    m_trianglesLabel->setText("--");
    m_volumeLabel->setText("-- mm³");
    m_boundsLabel->setText("-- × -- × -- mm");
}

} // namespace Presentation
} // namespace MarcSLM
