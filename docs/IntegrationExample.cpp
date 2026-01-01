/**
 * @file IntegrationExample.cpp
 * @brief Example showing how to integrate the new architecture into existing MainWindow
 * 
 * This file demonstrates the refactoring pattern for migrating from the old
 * tightly-coupled architecture to the new clean architecture.
 */

#include "presentation/MainWindowViewModel.h"
#include "presentation/ModelListWidget.h"
#include "presentation/PropertiesPanel.h"
#include "infrastructure/VtkStlFileLoader.h"
#include "infrastructure/VtkModelRenderer.h"
#include "infrastructure/MarcDllAdapter.h"
#include "core/domain/BuildPlate.h"

#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QPushButton>
#include <QProgressBar>
#include <QTextEdit>
#include <QFileDialog>
#include <QVTKOpenGLNativeWidget.h>

/**
 * STEP-BY-STEP MIGRATION GUIDE
 * ============================
 * 
 * Step 1: Create domain objects (replace old data structures)
 * -----------------------------------------------------------
 */
void setupDomainLayer(QMainWindow* mainWindow) {
    // Create build plate (replaces scattered model storage)
    auto* buildPlate = new Domain::BuildPlate(100.0, 200.0);
    
    // Store as member variable in MainWindow
    // mainWindow->m_buildPlate = buildPlate;
}

/**
 * Step 2: Create infrastructure services
 * ---------------------------------------
 */
void setupInfrastructure(QMainWindow* mainWindow, vtkRenderer* renderer) {
    // Create STL file loader (replaces inline vtkSTLReader code)
    auto* stlLoader = new Infrastructure::VtkStlFileLoader();
    
    // Create model renderer (replaces manual VTK actor management)
    auto* modelRenderer = new Infrastructure::VtkModelRenderer(renderer);
    
    // Create DLL adapter (replaces MainWindow::SliceModelviaDLL code)
    auto* dllAdapter = new Infrastructure::MarcDllAdapter();
    
    // Store as member variables
    // mainWindow->m_stlLoader = stlLoader;
    // mainWindow->m_modelRenderer = modelRenderer;
    // mainWindow->m_dllAdapter = dllAdapter;
}

/**
 * Step 3: Create ViewModel
 * -------------------------
 */
void setupViewModel(QMainWindow* mainWindow,
                   Domain::BuildPlate* buildPlate,
                   Application::IStlFileLoader* stlLoader,
                   Application::IModelRenderer* renderer,
                   Infrastructure::MarcDllAdapter* dllAdapter)
{
    // Create ViewModel (replaces direct use case calls in UI)
    auto* viewModel = new Presentation::MainWindowViewModel(
        buildPlate,
        stlLoader,
        renderer,
        dllAdapter,
        mainWindow
    );
    
    // Connect signals for UI feedback
    QObject::connect(viewModel, &Presentation::MainWindowViewModel::progressUpdate,
                    [](const QString& msg) {
                        // Update status bar or log window
                        qDebug() << msg;
                    });
    
    QObject::connect(viewModel, &Presentation::MainWindowViewModel::errorOccurred,
                    [mainWindow](const QString& error) {
                        QMessageBox::critical(mainWindow, "Error", error);
                    });
    
    // mainWindow->m_viewModel = viewModel;
}

/**
 * Step 4: Create UI widgets
 * --------------------------
 */
void setupUIWidgets(QMainWindow* mainWindow, Presentation::MainWindowViewModel* viewModel) {
    // Create model list widget (left panel)
    auto* modelListWidget = new Presentation::ModelListWidget(viewModel, mainWindow);
    
    // Create properties panel (right panel)
    auto* propertiesPanel = new Presentation::PropertiesPanel(viewModel, mainWindow);
    
    // Connect signals between widgets
    QObject::connect(modelListWidget, &Presentation::ModelListWidget::modelSelected,
                    propertiesPanel, &Presentation::PropertiesPanel::onModelSelected);
    
    QObject::connect(modelListWidget, &Presentation::ModelListWidget::addModelRequested,
                    [mainWindow, viewModel]() {
                        QString fileName = QFileDialog::getOpenFileName(
                            mainWindow,
                            "Open STL File",
                            "",
                            "STL Files (*.stl);;All Files (*)"
                        );
                        
                        if (!fileName.isEmpty()) {
                            viewModel->addModel(fileName);
                        }
                    });
    
    QObject::connect(modelListWidget, &Presentation::ModelListWidget::removeModelRequested,
                    viewModel, &Presentation::MainWindowViewModel::removeModel);
    
    // Add to layout (replace old setupUI code)
    // auto* centralWidget = new QWidget(mainWindow);
    // auto* layout = new QHBoxLayout(centralWidget);
    // layout->addWidget(modelListWidget, 0);
    // layout->addWidget(viewportWidget, 1);
    // layout->addWidget(propertiesPanel, 0);
    // mainWindow->setCentralWidget(centralWidget);
}

/**
 * Step 5: Replace old button handlers
 * ------------------------------------
 */
void migrateButtonHandlers(Presentation::MainWindowViewModel* viewModel,
                          QPushButton* sliceButton,
                          QProgressBar* progressBar)
{
    // OLD CODE (in MainWindow::setupUI):
    // connect(btnSlice, &QPushButton::clicked, this, &MainWindow::SliceModelviaDLL);
    
    // NEW CODE:
    QObject::connect(sliceButton, &QPushButton::clicked, [viewModel, sliceButton, progressBar]() {
        sliceButton->setEnabled(false);
        progressBar->setVisible(true);
        
        viewModel->sliceModels();
    });
    
    QObject::connect(viewModel, &Presentation::MainWindowViewModel::slicingCompleted,
                    [sliceButton, progressBar]() {
                        sliceButton->setEnabled(true);
                        progressBar->setVisible(false);
                    });
    
    QObject::connect(viewModel, &Presentation::MainWindowViewModel::slicingFailed,
                    [sliceButton, progressBar](const QString& error) {
                        sliceButton->setEnabled(true);
                        progressBar->setVisible(false);
                    });
}

/**
 * COMPLETE MIGRATION EXAMPLE
 * ===========================
 * 
 * Replace MainWindow constructor with this structure:
 */
class RefactoredMainWindow : public QMainWindow {
    Q_OBJECT
    
public:
    RefactoredMainWindow(QWidget* parent = nullptr) : QMainWindow(parent) {
        // 1. Create VTK viewport
        auto* vtkWidget = new QVTKOpenGLNativeWidget(this);
        vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();
        vtkWidget->renderWindow()->AddRenderer(renderer);
        
        // 2. Create domain layer
        m_buildPlate = new Domain::BuildPlate(100.0, 200.0);
        
        // 3. Create infrastructure
        m_stlLoader = new Infrastructure::VtkStlFileLoader();
        m_modelRenderer = new Infrastructure::VtkModelRenderer(renderer);
        m_dllAdapter = new Infrastructure::MarcDllAdapter();
        
        // 4. Create ViewModel
        m_viewModel = new Presentation::MainWindowViewModel(
            m_buildPlate,
            m_stlLoader,
            m_modelRenderer,
            m_dllAdapter,
            this
        );
        
        // 5. Create UI widgets
        auto* modelListWidget = new Presentation::ModelListWidget(m_viewModel, this);
        auto* propertiesPanel = new Presentation::PropertiesPanel(m_viewModel, this);
        
        // 6. Create layout
        auto* centralWidget = new QWidget(this);
        auto* layout = new QHBoxLayout(centralWidget);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(0);
        
        layout->addWidget(modelListWidget, 0);
        layout->addWidget(vtkWidget, 1);
        layout->addWidget(propertiesPanel, 0);
        
        setCentralWidget(centralWidget);
        
        // 7. Connect signals
        connectSignals(modelListWidget, propertiesPanel);
        
        // 8. Apply industrial styling
        applyIndustrialTheme();
    }
    
private:
    // Domain
    Domain::BuildPlate* m_buildPlate;
    
    // Infrastructure
    Infrastructure::VtkStlFileLoader* m_stlLoader;
    Infrastructure::VtkModelRenderer* m_modelRenderer;
    Infrastructure::MarcDllAdapter* m_dllAdapter;
    
    // Presentation
    Presentation::MainWindowViewModel* m_viewModel;
    
    void connectSignals(Presentation::ModelListWidget* modelList,
                       Presentation::PropertiesPanel* properties) {
        // Model selection
        connect(modelList, &Presentation::ModelListWidget::modelSelected,
                properties, &Presentation::PropertiesPanel::onModelSelected);
        
        // Add model
        connect(modelList, &Presentation::ModelListWidget::addModelRequested,
                this, &RefactoredMainWindow::onAddModelRequested);
        
        // Remove model
        connect(modelList, &Presentation::ModelListWidget::removeModelRequested,
                m_viewModel, &Presentation::MainWindowViewModel::removeModel);
        
        // Progress updates
        connect(m_viewModel, &Presentation::MainWindowViewModel::progressUpdate,
                this, &RefactoredMainWindow::onProgressUpdate);
        
        // Errors
        connect(m_viewModel, &Presentation::MainWindowViewModel::errorOccurred,
                this, &RefactoredMainWindow::onError);
    }
    
    void onAddModelRequested() {
        QString fileName = QFileDialog::getOpenFileName(
            this,
            "Open STL File",
            "",
            "STL Files (*.stl);;All Files (*)"
        );
        
        if (!fileName.isEmpty()) {
            m_viewModel->addModel(fileName);
        }
    }
    
    void onProgressUpdate(const QString& msg) {
        statusBar()->showMessage(msg, 3000);
    }
    
    void onError(const QString& error) {
        QMessageBox::critical(this, "Error", error);
    }
    
    void applyIndustrialTheme() {
        setStyleSheet(R"(
            QMainWindow {
                background-color: #2E3440;
            }
            QStatusBar {
                background-color: #3B4252;
                color: #ECEFF4;
            }
        )");
    }
};

/**
 * KEY MIGRATION POINTS
 * ====================
 * 
 * 1. BEFORE: StlViewer owned models and VTK actors together
 *    AFTER: BuildPlate owns model data, VtkModelRenderer owns VTK actors
 * 
 * 2. BEFORE: MainWindow::SliceModelviaDLL() had 200+ lines of inline DLL calls
 *    AFTER: MainWindowViewModel::sliceModels() delegates to MarcDllAdapter
 * 
 * 3. BEFORE: Model transforms stored in vtkActor
 *    AFTER: Model transforms stored in Domain::Model, synced to vtkActor
 * 
 * 4. BEFORE: UI logic mixed with business logic
 *    AFTER: ViewModel mediates, UI only handles display/input
 * 
 * 5. BEFORE: Hard to test, hard to swap VTK for Qt3D
 *    AFTER: Interfaces allow mocking, easy renderer swap
 */
