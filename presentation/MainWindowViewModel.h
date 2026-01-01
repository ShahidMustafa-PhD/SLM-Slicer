#ifndef MAINWINDOWVIEWMODEL_H
#define MAINWINDOWVIEWMODEL_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <memory>
#include <QThread>

// Forward declarations to avoid circular dependencies
namespace MarcSLM {
    namespace Domain {
        class BuildPlate;
        class Model;
        struct Transform;
    }
    namespace Application {
        class AddModelUseCase;
        class IStlFileLoader;
        class IModelRenderer;
    }
    namespace Infrastructure {
        class MarcDllAdapter;
    }
}

namespace MarcSLM {
namespace Presentation {

/**
 * @brief ViewModel for MainWindow
 * 
 * Mediates between the UI (MainWindow) and the business logic (use cases).
 */
class MainWindowViewModel : public QObject {
    Q_OBJECT
    
public:
    explicit MainWindowViewModel(
        std::shared_ptr<Domain::BuildPlate> buildPlate,
        std::shared_ptr<Application::IStlFileLoader> stlLoader,
        std::shared_ptr<Application::IModelRenderer> renderer,
        std::shared_ptr<Infrastructure::MarcDllAdapter> dllAdapter,
        QObject* parent = nullptr
    );
    
    ~MainWindowViewModel();
    
    // Model management
    void addModel(const QString& filePath);
    void removeModel(int modelId);
    void clearBuildPlate();
    
    // Transform operations
    void updateModelTransform(int modelId, const Domain::Transform& transform);
    Domain::Transform getModelTransform(int modelId) const;
    
    // Queries
    QStringList getModelNames() const;
    int getModelCount() const;
    bool hasModels() const;
    
    // Configuration
    void setConfigPath(const QString& configPath);
    QString getConfigPath() const;
    
    // Slicing workflow
    void sliceModels();
    
signals:
    void modelAdded(int modelId, const QString& fileName);
    void modelRemoved(int modelId);
    void transformChanged(int modelId);
    void buildPlateCleared();
    
    void progressUpdate(const QString& message);
    void errorOccurred(const QString& errorMessage);
    void slicingStarted();
    void slicingCompleted();
    void slicingFailed(const QString& errorMessage);
    
private:
    std::shared_ptr<Domain::BuildPlate> m_buildPlate;
    std::shared_ptr<Application::IStlFileLoader> m_stlLoader;
    std::shared_ptr<Application::IModelRenderer> m_renderer;
    std::shared_ptr<Infrastructure::MarcDllAdapter> m_dllAdapter;
    
    std::unique_ptr<Application::AddModelUseCase> m_addModelUseCase;
    
    QString m_configPath;
    QString m_stylesPath;
    
    // Mapping from domain model ID to VTK actor ID
    std::map<int, int> m_modelToActorMap;
    
    void onProgressUpdate(const std::string& message);
};

} // namespace Presentation
} // namespace MarcSLM

#endif // MAINWINDOWVIEWMODEL_H
