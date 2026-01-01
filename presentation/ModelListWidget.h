#ifndef MODELLISTWIDGET_H
#define MODELLISTWIDGET_H

#include <QWidget>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QMenu>

class QDragEnterEvent;
class QDropEvent;
class QDragLeaveEvent;

namespace MarcSLM {
namespace Presentation {

class MainWindowViewModel;

/**
 * @brief Industrial-grade model list widget for the left panel
 * 
 * Features:
 * - Displays loaded models in a list
 * - Shows model name and status (selected, visible)
 * - Context menu for operations (remove, duplicate, focus)
 * - Add/Remove buttons
 * - Build volume usage indicator
 * - Drag and drop support for STL files
 */
class ModelListWidget : public QWidget {
    Q_OBJECT
    
public:
    explicit ModelListWidget(MainWindowViewModel* viewModel, QWidget* parent = nullptr);
    
public slots:
    void onModelAdded(int modelId, const QString& fileName);
    void onModelRemoved(int modelId);
    void onBuildPlateCleared();
    
signals:
    void modelSelected(int modelId);
    void addModelRequested();
    void addModelRequested(const QString& filePath);  // For drag-and-drop
    void removeModelRequested(int modelId);
    
protected:
    // Drag and drop events
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragLeaveEvent(QDragLeaveEvent* event) override;
    void dropEvent(QDropEvent* event) override;
    
private slots:
    void onAddClicked();
    void onRemoveClicked();
    void onListItemClicked(QListWidgetItem* item);
    void showContextMenu(const QPoint& pos);
    
private:
    MainWindowViewModel* m_viewModel;
    
    QListWidget* m_listWidget;
    QPushButton* m_addButton;
    QPushButton* m_removeButton;
    QPushButton* m_clearButton;
    QLabel* m_buildVolumeLabel;
    
    void setupUI();
    void applyIndustrialStyle();
    void updateBuildVolumeInfo();
};

} // namespace Presentation
} // namespace MarcSLM

#endif // MODELLISTWIDGET_H
