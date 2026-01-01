#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QtSvg/QSvgRenderer>
#include <QtSvgWidgets/QGraphicsSvgItem>
#include <QFileDialog>
#include <QSlider>
#include <QTextEdit>
#include <QMessageBox>
#include <QProgressBar>
#include <QToolButton>
#include <QElapsedTimer>

#include <filesystem>
#include <vector>
#include <cstring>

#include "LayerViewer.h"
#include "MarcAPIInterface.h"
#include "ConfigDialog.h"
#include "MarcUtils.hpp"
#include "StlViewer.h"
#include "StlLoader.h"
#include "OrientationOptimizerInterface.h"
#include "OrientationOptimizer.h"
#include "toDllFromDll.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    // UI interaction handlers
    void onToggleSidebarClicked();
    void onSettingsButtonClicked();
    void onAddConfigsButtonClicked();
    void onShowHatchesButtonClicked();
    void onSliceButtonClicked();

    // File operations
    void onLoadSTLModelsRequested();
    void onLoadBuildConfigurationRequested();
    void onLoadBuildStyleRequested();
    void onLayerViewerRequested();

    // Slicing operations
    void performSliceViaDLL();
    void performSliceViaDLLLegacy();

    // Model visualization
    void displaySTLModel();
    
    // Logging
    void appendLogMessage(const QString& message);

private:
    // ==================== UI Components ====================
    Ui::MainWindow *ui = nullptr;
    
    // Ribbon bar buttons
    QPushButton *btnMenu = nullptr;
    QPushButton *btnHome = nullptr;
    QPushButton *btnSettings = nullptr;
    QPushButton *btnConfigIni = nullptr;
    QPushButton *btnSlice = nullptr;        // ADDED: Slice button member variable
    QPushButton *btnShowHatches = nullptr;
    
    // Sidebars
    QWidget *leftSidebar = nullptr;
    QWidget *contentArea = nullptr;
    
    // Main viewer and controls
    StlViewer *m_stlViewer = nullptr;
    QVBoxLayout *viewerLayout = nullptr;
    QTextEdit *textEdit = nullptr;
    QProgressBar *progressBar = nullptr;
    
    // Dialogs
    ConfigDialog *configDialog = nullptr;
    
    // Graphics scene (for SVG layers)
    QGraphicsScene *scene = nullptr;
    QGraphicsSvgItem *svgItem = nullptr;
    QSlider *layerSlider = nullptr;
    
    // ==================== State Management ====================
    bool isSidebarCollapsed = false;
    QPushButton *currentActiveButton = nullptr;
    
    // ==================== File Paths ====================
    std::vector<std::filesystem::path> modelFilePaths;
    std::filesystem::path buildConfigFilePath;
    std::filesystem::path buildStylesFilePath;
    
    // ==================== DLL Data Management ====================
    GuiDataArray m_guiDataArrayForDll;
    std::vector<InternalGuiModel> m_internalGuiDataArray;
    
    // Model tracking
    uint32_t modelCount = 0;
    uint32_t configFileCount = 0;

    // ==================== UI Setup Methods ====================
    void setupUserInterface(QWidget *centralWidget);
    void createRibbonBar(QVBoxLayout *mainLayout, QWidget *centralWidget);
    void createLeftSidebar(QWidget *centralWidget, QHBoxLayout *horizontalLayout);
    void createContentArea(QWidget *centralWidget, QHBoxLayout *horizontalLayout);
    void createRightSidebar(QWidget *centralWidget, QHBoxLayout *horizontalLayout);
    void connectSignalsAndSlots();
    void applyStylesheets();

    // ==================== Sidebar Management ====================
    void activateButton(QPushButton *button);
    void animateSidebarToggle();

    // ==================== File Operations ====================
    bool validateFilePaths(const std::vector<std::filesystem::path>& paths);
    void printFilePathsToLog(const std::vector<std::filesystem::path>& paths);

    // ==================== SVG Layer Viewer ====================
    void loadSvgIntoScene(const QString& filePath);
    void onLayerSliderValueChanged(int layerNumber);
    void loadLayerIntoScene(QGraphicsScene *scene, int layerNumber);
    void displayLayerViewerWindow();

    // ==================== Model Display ====================
    void loadAndDisplayModels();

    // ==================== Slicing Operations ====================
    bool prepareModelsForDLL();
    void executeSlicingOperation(MarcHandle handle);

    // ==================== DLL Data Conversion ====================
    void sendDataToDLLFormat();
    void receiveDataFromDLLFormat();
    void cleanupGuiDataStructures();
    void printGuiDataArrayDebugInfo();
    void printInternalModelArrayDebugInfo();

    // ==================== Utilities ====================
    void orientationOptimization();
};

#endif // MAINWINDOW_H
