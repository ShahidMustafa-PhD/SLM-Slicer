#include "MainWindowViewModel.h"
#include "../core/domain/BuildPlate.h"
#include "../core/domain/Model.h"
#include "../core/application/usecases/AddModelUseCase.h"
#include "../infrastructure/MarcDllAdapter.h"

#include <QFileInfo>
#include <algorithm>
#include <QMetaObject>

namespace MarcSLM {
namespace Presentation {

MainWindowViewModel::MainWindowViewModel(
    std::shared_ptr<Domain::BuildPlate> buildPlate,
    std::shared_ptr<Application::IStlFileLoader> stlLoader,
    std::shared_ptr<Application::IModelRenderer> renderer,
    std::shared_ptr<Infrastructure::MarcDllAdapter> dllAdapter,
    QObject* parent
)
    : QObject(parent)
    , m_buildPlate(std::move(buildPlate))
    , m_stlLoader(std::move(stlLoader))
    , m_renderer(std::move(renderer))
    , m_dllAdapter(std::move(dllAdapter))
{
    // Validate dependencies
    if (!m_buildPlate || !m_stlLoader || !m_renderer || !m_dllAdapter) {
        qCritical() << "MainWindowViewModel: Null dependency detected!";
        qCritical() << "  buildPlate:" << (m_buildPlate != nullptr);
        qCritical() << "  stlLoader:" << (m_stlLoader != nullptr);
        qCritical() << "  renderer:" << (m_renderer != nullptr);
        qCritical() << "  dllAdapter:" << (m_dllAdapter != nullptr);
        // Continue without creating use case - errors will be handled in addModel
        return;
    }
    
    // Create use case
    m_addModelUseCase = std::make_unique<Application::AddModelUseCase>(
        m_stlLoader,
        m_buildPlate,
        m_renderer
    );
    
    // Set progress callback
    m_addModelUseCase->setProgressCallback(
        [this](const std::string& msg) { this->onProgressUpdate(msg); }
    );
}

MainWindowViewModel::~MainWindowViewModel() {
}

void MainWindowViewModel::addModel(const QString& filePath) {
    // Check if use case is initialized
    if (!m_addModelUseCase) {
        qCritical() << "AddModelUseCase not initialized - dependencies missing";
        emit errorOccurred("Model loading system not initialized. Please check application setup.");
        return;
    }
    
    std::string path = filePath.toStdString();
    
    emit progressUpdate("Loading model: " + QFileInfo(filePath).fileName());
    
    try {
        // Execute use case
        auto result = m_addModelUseCase->execute(path);
        
        if (result.isError()) {
            emit errorOccurred(QString::fromStdString(result.errorMessage()));
            return;
        }
        
        // Find the newly added model
        auto models = m_buildPlate->getAllModels();
        if (!models.empty()) {
            auto lastModel = models.back();
            if (lastModel) {
                emit modelAdded(lastModel->id(), QFileInfo(filePath).fileName());
            }
        }
    } catch (const std::exception& e) {
        emit errorOccurred(QString("Exception during model loading: %1").arg(e.what()));
    } catch (...) {
        emit errorOccurred("Unknown error during model loading");
    }
}

void MainWindowViewModel::removeModel(int modelId) {
    // Remove from renderer if mapped
    auto it = m_modelToActorMap.find(modelId);
    if (it != m_modelToActorMap.end() && m_renderer) {
        m_renderer->removeModel(it->second);
        m_modelToActorMap.erase(it);
    }
    
    // Remove from build plate
    if (m_buildPlate->removeModel(modelId)) {
        emit modelRemoved(modelId);
    }
}

void MainWindowViewModel::clearBuildPlate() {
    if (m_renderer) {
        m_renderer->clearScene();
    }
    
    if (m_buildPlate) {
        m_buildPlate->clear();
    }
    m_modelToActorMap.clear();
    
    emit buildPlateCleared();
}

void MainWindowViewModel::updateModelTransform(int modelId, const Domain::Transform& transform) {
    auto modelPtr = m_buildPlate->getModel(modelId);
    if (!modelPtr) {
        return;
    }
    
    modelPtr->setTransform(transform);
    
    // Update renderer
    auto it = m_modelToActorMap.find(modelId);
    if (it != m_modelToActorMap.end() && m_renderer) {
        m_renderer->updateModelTransform(it->second, transform);
        m_renderer->render();
    }
    
    emit transformChanged(modelId);
}

Domain::Transform MainWindowViewModel::getModelTransform(int modelId) const {
    auto modelPtr = m_buildPlate->getModel(modelId);
    if (modelPtr) {
        return modelPtr->transform();
    }
    return Domain::Transform();
}

QStringList MainWindowViewModel::getModelNames() const {
    QStringList names;
    auto models = m_buildPlate->getAllModels();
    
    for (const auto& modelPtr : models) {
        if (modelPtr) {
            QFileInfo fileInfo(QString::fromStdString(modelPtr->filePath()));
            names.append(fileInfo.fileName());
        }
    }
    
    return names;
}

int MainWindowViewModel::getModelCount() const {
    return m_buildPlate->modelCount();
}

bool MainWindowViewModel::hasModels() const {
    return m_buildPlate->modelCount() > 0;
}

void MainWindowViewModel::setConfigPath(const QString& configPath) {
    m_configPath = configPath;
    m_stylesPath = configPath; // For now, use same path
}

QString MainWindowViewModel::getConfigPath() const {
    return m_configPath;
}

void MainWindowViewModel::sliceModels() {
    if (!hasModels()) {
        emit errorOccurred("No models loaded. Please add models before slicing.");
        return;
    }
    
    if (m_configPath.isEmpty()) {
        emit errorOccurred("No configuration file selected. Please select a config JSON file.");
        return;
    }
    
    emit slicingStarted();
    emit progressUpdate("Initializing slicer...");
    
    // Initialize DLL
    auto result = m_dllAdapter->initialize(
        m_buildPlate->radius(),
        m_buildPlate->height(),
        5.0  // spacing
    );
    
    if (result.isError()) {
        emit slicingFailed(QString::fromStdString(result.errorMessage()));
        return;
    }
    
    emit progressUpdate("Preparing models...");
    
    // Send models to DLL - convert to const pointers
    auto models = m_buildPlate->getAllModels();
    std::vector<const Domain::Model*> constModels;
    constModels.reserve(models.size());
    for (const auto& ptr : models) {
        constModels.push_back(ptr.get());
    }
    result = m_dllAdapter->setModels(constModels);
    
    if (result.isError()) {
        emit slicingFailed(QString::fromStdString(result.errorMessage()));
        m_dllAdapter->cleanup();
        return;
    }
    
    emit progressUpdate("Loading configuration...");
    
    // Set config
    result = m_dllAdapter->setConfig(m_configPath.toStdString());
    
    if (result.isError()) {
        emit slicingFailed(QString::fromStdString(result.errorMessage()));
        m_dllAdapter->cleanup();
        return;
    }
    
    emit progressUpdate("Updating models...");
    
    // Update model
    result = m_dllAdapter->updateModel();
    
    if (result.isError()) {
        emit slicingFailed(QString::fromStdString(result.errorMessage()));
        m_dllAdapter->cleanup();
        return;
    }
    
    emit progressUpdate("Exporting slice file...");
    
    // Export
    result = m_dllAdapter->exportSlmFile();
    
    if (result.isError()) {
        emit slicingFailed(QString::fromStdString(result.errorMessage()));
        m_dllAdapter->cleanup();
        return;
    }
    
    // Cleanup
    m_dllAdapter->cleanup();
    
    emit progressUpdate("Slicing completed successfully!");
    emit slicingCompleted();
}

void MainWindowViewModel::onProgressUpdate(const std::string& message) {
    emit progressUpdate(QString::fromStdString(message));
}

} // namespace Presentation
} // namespace MarcSLM
