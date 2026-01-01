#include "ModelListWidget.h"
#include "MainWindowViewModel.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QMimeData>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QUrl>

namespace MarcSLM {
namespace Presentation {

ModelListWidget::ModelListWidget(MainWindowViewModel* viewModel, QWidget* parent)
    : QWidget(parent)
    , m_viewModel(viewModel)
{
    setupUI();
    applyIndustrialStyle();
    
    // Enable drag and drop
    setAcceptDrops(true);
    
    // Connect to view model signals if available
    if (m_viewModel) {
        connect(m_viewModel, &MainWindowViewModel::modelAdded,
                this, &ModelListWidget::onModelAdded);
        connect(m_viewModel, &MainWindowViewModel::modelRemoved,
                this, &ModelListWidget::onModelRemoved);
        connect(m_viewModel, &MainWindowViewModel::buildPlateCleared,
                this, &ModelListWidget::onBuildPlateCleared);
    }
}

void ModelListWidget::setupUI() {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(10, 10, 10, 10);
    layout->setSpacing(10);
    
    // Header
    auto* headerLabel = new QLabel("Build Plate", this);
    QFont headerFont = headerLabel->font();
    headerFont.setPointSize(12);
    headerFont.setBold(true);
    headerLabel->setFont(headerFont);
    layout->addWidget(headerLabel);
    
    // Model list
    m_listWidget = new QListWidget(this);
    m_listWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    m_listWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    
    // Enable drag and drop on list widget
    m_listWidget->setAcceptDrops(true);
    m_listWidget->setDragEnabled(false);  // We don't want to drag items out
    
    layout->addWidget(m_listWidget);
    
    // Connect signals
    connect(m_listWidget, &QListWidget::itemClicked,
            this, &ModelListWidget::onListItemClicked);
    connect(m_listWidget, &QListWidget::customContextMenuRequested,
            this, &ModelListWidget::showContextMenu);
    
    // Buttons row
    auto* buttonLayout = new QHBoxLayout();
    
    m_addButton = new QPushButton("Add", this);
    m_removeButton = new QPushButton("Remove", this);
    m_clearButton = new QPushButton("Clear All", this);

    m_addButton->setToolTip("Add STL models");
    m_removeButton->setToolTip("Remove selected model");
    m_clearButton->setToolTip("Clear all models from build plate");
    
    m_removeButton->setEnabled(false);
    m_clearButton->setEnabled(false);
    
    buttonLayout->addWidget(m_addButton);
    buttonLayout->addWidget(m_removeButton);
    buttonLayout->addWidget(m_clearButton);
    
    layout->addLayout(buttonLayout);
    
    // Build volume info
    m_buildVolumeLabel = new QLabel("Models: 0 | Volume: 0%", this);
    m_buildVolumeLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(m_buildVolumeLabel);
    
    // Drag and drop hint
    auto* dropHintLabel = new QLabel("Drag & drop STL files here", this);
    dropHintLabel->setAlignment(Qt::AlignCenter);
    dropHintLabel->setStyleSheet("color: #81A1C1; font-size: 11px; font-style: italic;");
    layout->addWidget(dropHintLabel);
    
    // Connect button signals
    connect(m_addButton, &QPushButton::clicked,
            this, &ModelListWidget::onAddClicked);
    connect(m_removeButton, &QPushButton::clicked,
            this, &ModelListWidget::onRemoveClicked);
    connect(m_clearButton, &QPushButton::clicked, [this]() {
        if (!m_viewModel) {
            QMessageBox::warning(this, "Error", "Application not fully initialized.");
            return;
        }
        if (QMessageBox::question(this, "Clear Build Plate",
                                  "Remove all models from build plate?",
                                  QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
            m_viewModel->clearBuildPlate();
        }
    });
}

void ModelListWidget::applyIndustrialStyle() {
    // Industrial color scheme
    QString style = R"(
        QWidget {
            background-color: #2E3440;
            color: #ECEFF4;
            font-family: 'Segoe UI', sans-serif;
        }
        
        QLabel {
            color: #ECEFF4;
        }
        
        QListWidget {
            background-color: #3B4252;
            border: 1px solid #4C566A;
            border-radius: 4px;
            padding: 5px;
        }
        
        QListWidget::item {
            padding: 8px;
            border-radius: 3px;
        }
        
        QListWidget::item:selected {
            background-color: #5E81AC;
            color: white;
        }
        
        QListWidget::item:hover {
            background-color: #434C5E;
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

void ModelListWidget::onModelAdded(int modelId, const QString& fileName) {
    auto* item = new QListWidgetItem(fileName);
    item->setData(Qt::UserRole, modelId);
    m_listWidget->addItem(item);
    
    m_removeButton->setEnabled(true);
    m_clearButton->setEnabled(true);
    
    updateBuildVolumeInfo();
}

void ModelListWidget::onModelRemoved(int modelId) {
    for (int i = 0; i < m_listWidget->count(); ++i) {
        auto* item = m_listWidget->item(i);
        if (item->data(Qt::UserRole).toInt() == modelId) {
            delete m_listWidget->takeItem(i);
            break;
        }
    }
    
    if (m_listWidget->count() == 0) {
        m_removeButton->setEnabled(false);
        m_clearButton->setEnabled(false);
    }
    
    updateBuildVolumeInfo();
}

void ModelListWidget::onBuildPlateCleared() {
    m_listWidget->clear();
    m_removeButton->setEnabled(false);
    m_clearButton->setEnabled(false);
    updateBuildVolumeInfo();
}

void ModelListWidget::onAddClicked() {
    // Open file dialog here in the widget, emit per-file request
    QStringList fileNames = QFileDialog::getOpenFileNames(
        this,
        tr("Open STL Models"),
        "",
        tr("STL Files (*.stl);;All Files (*)")
    );

    if (fileNames.isEmpty()) {
        return;
    }

    // Check if we have a valid ViewModel before emitting
    if (!m_viewModel) {
        QMessageBox::warning(this, "Error", "Application not fully initialized.");
        return;
    }

    for (const QString& fileName : fileNames) {
        emit addModelRequested(fileName);
    }
}

void ModelListWidget::onRemoveClicked() {
    auto* currentItem = m_listWidget->currentItem();
    if (currentItem) {
        int modelId = currentItem->data(Qt::UserRole).toInt();
        emit removeModelRequested(modelId);
    }
}

void ModelListWidget::onListItemClicked(QListWidgetItem* item) {
    if (item) {
        int modelId = item->data(Qt::UserRole).toInt();
        emit modelSelected(modelId);
    }
}

void ModelListWidget::showContextMenu(const QPoint& pos) {
    auto* item = m_listWidget->itemAt(pos);
    if (!item) {
        return;
    }
    
    int modelId = item->data(Qt::UserRole).toInt();
    
    QMenu menu(this);
    
    QAction* removeAction = menu.addAction("Remove");
    QAction* focusAction = menu.addAction("Focus Camera");
    
    QAction* selected = menu.exec(m_listWidget->mapToGlobal(pos));
    
    if (selected == removeAction) {
        emit removeModelRequested(modelId);
    }
    // TODO: Implement focus camera
}

void ModelListWidget::updateBuildVolumeInfo() {
    int modelCount = m_viewModel->getModelCount();
    // TODO: Get actual volume percentage from BuildPlate
    m_buildVolumeLabel->setText(
        QString("Models: %1 | Volume: --%%").arg(modelCount)
    );
}

void ModelListWidget::dragEnterEvent(QDragEnterEvent* event)
{
    // Accept drag if it contains file URLs
    if (event->mimeData()->hasUrls()) {
        // Check if at least one URL is an STL file
        bool hasStl = false;
        for (const QUrl& url : event->mimeData()->urls()) {
            if (url.toLocalFile().endsWith(".stl", Qt::CaseInsensitive)) {
                hasStl = true;
                break;
            }
        }
        
        if (hasStl) {
            event->acceptProposedAction();
            m_listWidget->setStyleSheet(m_listWidget->styleSheet() + " QListWidget { border: 2px dashed #5E81AC; }");
        }
    }
}

void ModelListWidget::dragLeaveEvent(QDragLeaveEvent* event)
{
    // Reset style when drag leaves
    applyIndustrialStyle();
}

void ModelListWidget::dropEvent(QDropEvent* event)
{
    // Reset style
    applyIndustrialStyle();
    
    if (event->mimeData()->hasUrls()) {
        for (const QUrl& url : event->mimeData()->urls()) {
            QString filePath = url.toLocalFile();
            if (filePath.endsWith(".stl", Qt::CaseInsensitive)) {
                emit addModelRequested(filePath);
            }
        }
        event->acceptProposedAction();
    }
}

} // namespace Presentation
} // namespace MarcSLM
