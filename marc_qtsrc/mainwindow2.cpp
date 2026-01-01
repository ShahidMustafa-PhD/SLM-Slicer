#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QGraphicsOpacityEffect>


//#define MARC_S_OK        0x00000000
//#define MARC_E_FAIL      0x80004005
//#define MARC_E_INVALID   0x80070057
//#define MARC_E_NOTIMPL   0x80004001

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), scene(new QGraphicsScene(this)), svgItem(nullptr)
{
    // Ensure UI is constructed
    ui = new Ui::MainWindow();
    ui->setupUi(this);

    // Main container widget
    QWidget *central = new QWidget(this);
    this->setCentralWidget(central);

    this->setupUI(central); // Sidebar widget settings
}

MainWindow::~MainWindow()
{
    // Clean up DLL data safely
    clearGuiData();

    // Delete UI last
    delete ui;
}

void MainWindow::toggleSidebar()
{
    if (!leftSidebar || !btnHome || !btnSettings) {
        return; // Guard against nulls
    }
    int startWidth = isSidebarCollapsed ? 50 : 250;
    int endWidth = isSidebarCollapsed ? 250 : 50;

    // Apply a smooth animation with fade effect
    QPropertyAnimation *widthAnimation = new QPropertyAnimation(leftSidebar, "minimumWidth", this);
    widthAnimation->setDuration(300);
    widthAnimation->setStartValue(startWidth);
    widthAnimation->setEndValue(endWidth);
    widthAnimation->setEasingCurve(QEasingCurve::InOutQuad);

    auto *opacityEffect = new QGraphicsOpacityEffect(leftSidebar);
    leftSidebar->setGraphicsEffect(opacityEffect);
    QPropertyAnimation *opacityAnimation = new QPropertyAnimation(opacityEffect, "opacity", this);
    opacityAnimation->setDuration(300);
    opacityAnimation->setStartValue(isSidebarCollapsed ? 0.5 : 1);
    opacityAnimation->setEndValue(isSidebarCollapsed ? 1 : 0.5);

    widthAnimation->start(QAbstractAnimation::DeleteWhenStopped);
    opacityAnimation->start(QAbstractAnimation::DeleteWhenStopped);

    isSidebarCollapsed = !isSidebarCollapsed;

    // Toggle button text when collapsed
    if (isSidebarCollapsed) {
        btnHome->setText("🏠");
        btnSettings->setText("⚙️");
    } else {
        btnHome->setText("🏠 Home");
        btnSettings->setText("⚙️ Settings");
    }
}

void MainWindow::setActiveButton(QPushButton *button)
{
    if (!button) return;
    // Remove active style from previous button
    if (currentActiveButton) {
        currentActiveButton->setStyleSheet("text-align: left; padding: 10px; font-size: 14px; background-color: transparent; color: white; border: none;");
    }
    // Set active style to the current button
    button->setStyleSheet("text-align: left; padding: 10px; font-size: 14px; background-color: #4C566A; color: white; border: none;");
    currentActiveButton = button;
}

void MainWindow::setupUI(QWidget *central)
{
    if (!central) {
        central = new QWidget(this);
        setCentralWidget(central);
    }
    // 1. Central widget
    setCentralWidget(central);

    // 2. Main vertical layout (ribbon + content)
    QVBoxLayout *mainLayout = new QVBoxLayout(central);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // 3. Ribbon bar
    QWidget *ribbonBar = new QWidget(central);
    ribbonBar->setStyleSheet("background-color: #1B2A49;");
    ribbonBar->setFixedHeight(50);

    QFont emojiFont;
    emojiFont.setFamily("Segoe UI Emoji");
    ribbonBar->setFont(emojiFont);

    QHBoxLayout *ribbonLayout = new QHBoxLayout(ribbonBar);
    ribbonLayout->setContentsMargins(10, 0, 10, 0);
    ribbonLayout->setSpacing(20);
    btnMenu = new QPushButton("☰", ribbonBar);
    btnMenu->setFixedSize(40, 40);
    btnMenu->setStyleSheet("font-size: 18px; background-color: #4C566A; border: none; color: white;");

    // Add widgets to leftSidebarLayout if needed
    btnSettings = new QPushButton("📑 Settings", ribbonBar);
    btnConfigini = new QPushButton("📂 Add Configs", ribbonBar);
   // QPushButton *btnConfigjson = new QPushButton("📂 Add Styles", ribbonBar);
    QPushButton *btnSlice = new QPushButton("🚀 Slice", ribbonBar);
    btnSvgimages = new QPushButton("▶️ Show Hatches", ribbonBar);

    QString ribbonButtonStyle =
        "QPushButton {"
        " padding: 5px 15px;"
        " font-size: 14px;"
        " background-color: transparent;"
        " color: white;"
        " border: none;"
        "}"
        "QPushButton:hover {"
        " background-color: #4C566A;"
        "}";

    btnSettings->setStyleSheet(ribbonButtonStyle);
    btnConfigini->setStyleSheet(ribbonButtonStyle);
    btnSlice->setStyleSheet(ribbonButtonStyle);
    btnSvgimages->setStyleSheet(ribbonButtonStyle);
   // btnConfigjson->setStyleSheet(ribbonButtonStyle);

    ribbonLayout->addWidget(btnMenu, 0, Qt::AlignLeft);
    ribbonLayout->addWidget(btnSettings);
    ribbonLayout->addWidget(btnConfigini);
    //ribbonLayout->addWidget(btnConfigjson);
    ribbonLayout->addWidget(btnSlice);
    ribbonLayout->addWidget(btnSvgimages);
    ribbonLayout->addStretch();

    contentArea = new QWidget(central);
    contentArea->setStyleSheet("background-color: #FFFFFF;");
    contentArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    this->viewerLayout = new QVBoxLayout(contentArea);
    this->viewerLayout->setContentsMargins(0, 0, 0, 0);

    this->m_stlViewer = new StlViewer(this);
    this->m_stlViewer->setBackgroundColor(0.95, 0.95, 0.95);
    this->viewerLayout->addWidget(this->m_stlViewer);

    // Set initial height to half of contentArea after the window is shown
    QTimer::singleShot(0, this, [this]() {
        if (contentArea && m_stlViewer) {
            int half = contentArea->height() / 2;
            m_stlViewer->setFixedHeight(half);
            // Remove the fixed height after a short delay to allow resizing
            QTimer::singleShot(100, m_stlViewer, [this]() {
                m_stlViewer->setMinimumHeight(0);
                m_stlViewer->setMaximumHeight(QWIDGETSIZE_MAX);
                m_stlViewer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
            });
        }
    });

    leftSidebar = new QWidget(central);
    leftSidebar->setStyleSheet("background-color: #1B2A49;");
    leftSidebar->setMinimumWidth(50);
    leftSidebar->setMaximumWidth(50);
    QVBoxLayout *leftSidebarLayout = new QVBoxLayout(leftSidebar);

    QWidget *rightSidebar = new QWidget(central);
    rightSidebar->setStyleSheet("background-color: #1B2A49;");
    rightSidebar->setMinimumWidth(150);
    rightSidebar->setMaximumWidth(200);

    QVBoxLayout *rightSidebarLayout = new QVBoxLayout(rightSidebar);
    rightSidebarLayout->setContentsMargins(5, 5, 5, 5);

    textEdit = new QTextEdit(rightSidebar);
    textEdit->setStyleSheet("font-size: 14px; color: white; background-color: transparent; border: none;");
    textEdit->setReadOnly(true);
    rightSidebarLayout->addWidget(textEdit);

    QWidget *horizontalContainer = new QWidget(central);
    QHBoxLayout *horizontalLayout = new QHBoxLayout(horizontalContainer);
    horizontalLayout->setContentsMargins(0, 0, 0, 0);
    horizontalLayout->setSpacing(0);

    horizontalLayout->addWidget(leftSidebar, 0);
    horizontalLayout->addWidget(contentArea, 1);
    horizontalLayout->addWidget(rightSidebar, 0);

    mainLayout->addWidget(ribbonBar);
    mainLayout->addWidget(horizontalContainer);

    progressBar = new QProgressBar(leftSidebar);
    progressBar->setOrientation(Qt::Vertical);
    progressBar->setMinimumWidth(10);
    progressBar->setRange(0, 0);
    progressBar->setVisible(false);
    progressBar->setTextVisible(false);
    progressBar->setStyleSheet(
        "QProgressBar {"
        " border: 1px solid #B0BEC5;"
        " background-color: #ECEFF4;"
        " border-radius: 5px;"
        " }"
        "QProgressBar::chunk {"
        " background-color: #5E81AC;"
        " border-radius: 5px;"
        " margin: 1px;"
        " }"
    );

    leftSidebarLayout->addWidget(progressBar);
    leftSidebar->setLayout(leftSidebarLayout);

    textEdit->setText("");
    textEdit->append("-Generates Slice files for observation and PBF-SLM Machine.");
    textEdit->append("----------------------");
    textEdit->append("");

    connect(btnSettings , &QPushButton::clicked, this, &MainWindow::AddConfigurationData);
    connect(btnConfigini, &QPushButton::clicked, this, &MainWindow::LoadbuildConfigurations);
    connect(btnSvgimages, &QPushButton::clicked, this, &MainWindow::on_openLayerViewer_clicked);
    //connect(btnConfigjson,&QPushButton::clicked,this,&MainWindow::LoadBuildStyle);

    connect(m_stlViewer, &StlViewer::logMessage, this, &MainWindow::appendLog);

    connect(btnSlice, &QPushButton::clicked, this, [this, btnSlice]() {
        btnSlice->setEnabled(false);
        progressBar->setVisible(true);

        // Use QThread for better compatibility with older Qt/MSVC versions
        QThread* workerThread = QThread::create([this]() {
            this->SliceModelviaDLL();
        });
        
        // Re-enable button when thread finishes
        connect(workerThread, &QThread::finished, this, [this, btnSlice, workerThread]() {
            btnSlice->setEnabled(true);
            progressBar->setVisible(false);
            workerThread->deleteLater(); // Clean up thread
        });
        
        // Start the background thread
        workerThread->start();
    });

    statusBar()->showMessage("© 2025 Marc. All rights reserved.");
    resize(800, 600);
}

void MainWindow::LoadstlModels()
{
    // Open file dialog for multiple STL file selection
    QStringList fileNames = QFileDialog::getOpenFileNames(
        this,
        tr("Open STL Models"),
        "",
        tr("STL Files (*.stl);;All Files (*)")
    );

    if (fileNames.isEmpty()) {
        textEdit->append("-No STL models selected.");
        return;
    }
//--------
// Somewhere in your MainWindow constructor or slot




    // Clear old model paths if needed
    //model_paths.clear();
    //modCount = 0;

    for (const QString& fileName : fileNames) {
        QFileInfo fileInfo(fileName);
        textEdit->append("-Selected STL Model: " + fileInfo.fileName());

        model_paths.emplace_back(fileName.toStdString());
        modCount++;

        if (modCount >= 12) {
            textEdit->append("-Maximum of 12 models supported. Ignoring extra files.");
           break;
        }
    }

    textEdit->append("-Number of STL Models: " + QString::number(modCount));

    this->ShowStlModel();
}
void MainWindow::loadSvgIntoView(const QString& filePath)
{
    // Reuse member scene to avoid leak
    if (!scene) {
        scene = new QGraphicsScene(this);
    } else {
        scene->clear();
    }

    auto *item = new QGraphicsSvgItem(filePath);
    item->setFlags(QGraphicsItem::ItemClipsToShape);
    item->setCacheMode(QGraphicsItem::NoCache);
    item->setZValue(0);

    scene->addItem(item);
}

void MainWindow::onLayerChanged(int value)
{
    // Load the corresponding SVG layer based on the slider value
    //loadLayer(value);
}
void MainWindow::loadLayer(QGraphicsScene *scene, int layerNumber)
{
    // Delete the existing SVG item if it exists
    if (svgItem) {
        scene->removeItem(svgItem);
        delete svgItem;
    }

    // Construct the filename for the selected layer
    QString filePath = QString(":/images/Layer%1.svg").arg(layerNumber);

    // Check if the file exists (optional)
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists()) {
        // If the file doesn't exist, show a warning
        QMessageBox::warning(this, "File Not Found", "The selected layer file does not exist.");
        return;
    }

    // Load the SVG item and add it to the scene
    svgItem = new QGraphicsSvgItem(filePath);
    svgItem->setFlags(QGraphicsItem::ItemClipsToShape);
    svgItem->setCacheMode(QGraphicsItem::NoCache);
    svgItem->setZValue(0);

    scene->addItem(svgItem);
    scene->setSceneRect(svgItem->boundingRect());
}
void MainWindow::ShowLayers()
{
    // Create a new independent window
    QWidget *layerWindow = new QWidget();
    layerWindow->setWindowTitle("SVG Layer Viewer");
    layerWindow->resize(800, 600);

    QVBoxLayout *mainLayout = new QVBoxLayout(layerWindow);

    // Create a new scene and view
    QGraphicsScene *scene = new QGraphicsScene(layerWindow);
    QGraphicsView *view = new QGraphicsView(scene, layerWindow);
    mainLayout->addWidget(view);

    // Create a slider for layer selection
    QSlider *slider = new QSlider(Qt::Horizontal, layerWindow);
    slider->setRange(1, 500);
    slider->setValue(1);
    mainLayout->addWidget(slider);

    // Connect the slider's valueChanged signal to a lambda
    connect(slider, &QSlider::valueChanged, this, [=](int value) {
        loadLayer(scene, value);  // Pass the scene to loadLayer
    });

    // Load the first layer
    loadLayer(scene, slider->value());

    // Show the new window
    layerWindow->show();
}
// In your button handler:
void MainWindow::on_openLayerViewer_clicked()
{
    // Give parent to manage lifetime; still top-level window
    LayerViewer *viewer = new LayerViewer(this);
    viewer->setAttribute(Qt::WA_DeleteOnClose);
    viewer->setWindowFlag(Qt::Window);
    viewer->show();
}
void MainWindow::LoadbuildConfigurations()
{
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Select Build Config JSON"),
        "",
        tr("JSON Files (*.json);;All Files (*)"));

    if (fileName.isEmpty())
    {
        this->textEdit->append("-No json  configuration file selected.");
        return;
    }


    QFileInfo fileInfo(fileName);
    this->textEdit->append("-Selected Config File: " + fileInfo.fileName());
    build_config_path_json = std::filesystem::path(fileName.toStdString());
    build_styles_path_json = std::filesystem::path(fileName.toStdString());// just masking it. styles are no where used in this programe. nor in DLL
    // Just to pass the file existance tests
    return;

 
}
void MainWindow::SliceModelviaDLL() {
    // Ensure this runs worker-safe and avoids use-after-free
    QMetaObject::invokeMethod(this, [this]() {
        if (textEdit) {
            textEdit->clear();
            textEdit->append("-Starting slicing operation...");
        }
    }, Qt::QueuedConnection);

    // Prepare models and configs
    if (!prepareModelForDLL()) {
        QMetaObject::invokeMethod(this, [this]() {
            if (textEdit) textEdit->append("-Preparation failed: invalid models/configs");
        }, Qt::QueuedConnection);
        return;
    }
    // Guard: ensure we actually have models to send
    if (m_guiDataArrayforDll.count == 0 || m_guiDataArrayforDll.models == nullptr) {
        QMetaObject::invokeMethod(this, [this]() {
            if (textEdit) textEdit->append("-No models to slice");
        }, Qt::QueuedConnection);
        return;
    }

    MarcHandle handle = create_marc_api(200.0f, 200.0f, 5.0f);
    if (!handle) {
        QMetaObject::invokeMethod(this, [this]() {
            if (textEdit) {
                textEdit->append("-Failed to create MARC API handle.");
            }
            QMessageBox::critical(this, "API Initialization Failed",
                                  "Failed to initialize the MARC API. Please check your installation.");
        }, Qt::QueuedConnection);
        return;
    }

    try {
        // 1) Send models
        if (textEdit) {
            textEdit->append("-size of DataArray: " + QString::number(m_guiDataArrayforDll.count));
        }
        MarcErrorCode err = set_models(handle, m_guiDataArrayforDll);
        QMetaObject::invokeMethod(this, [this, err]() {
            if (textEdit) textEdit->append(err == MARC_S_OK ? "-Models loading succeeded" : "-Models loading failed");
        }, Qt::QueuedConnection);
        if (err != MARC_S_OK) {
            destroy_marc_api(handle);
            return;
        }

        // 2) Load styles JSON (required before update/export)
        const std::string stylesPath = build_styles_path_json.string();
        if (stylesPath.empty()) {
            QMetaObject::invokeMethod(this, [this]() {
                if (textEdit) textEdit->append("-Styles path empty");
            }, Qt::QueuedConnection);
            destroy_marc_api(handle);
            return;
        }
        err = set_config_json(handle, stylesPath.c_str());
        QMetaObject::invokeMethod(this, [this, err]() {
            if (textEdit) textEdit->append(err == MARC_S_OK ? "-Config json loading succeeded" : "-Config json loading failed");
        }, Qt::QueuedConnection);
        if (err != MARC_S_OK) {
            destroy_marc_api(handle);
            return;
        }

        // 3) Update model with loaded configs/styles
        err = update_model(handle);
        QMetaObject::invokeMethod(this, [this, err]() {
            if (textEdit) textEdit->append(err == MARC_S_OK ? "-Model updating succeeded" : "-Model updating failed");
        }, Qt::QueuedConnection);
        if (err != MARC_S_OK) {
            destroy_marc_api(handle);
            return;
        }
        
        // 4) Export SLM
        err = export_slm_file(handle);
        QMetaObject::invokeMethod(this, [this, err]() {
            if (textEdit) textEdit->append(err == MARC_S_OK ? "-Model sliced and exported successfully" : "-Model slicing failed");
            if (textEdit) textEdit->append("-Operation completed.");
        }, Qt::QueuedConnection);

        // 5) Cleanup local state (after export)
        clearGuiData();
        model_paths.clear();
        cofCount = 0;
        modCount = 0;
    }
    catch (const std::exception& e) {
        QMetaObject::invokeMethod(this, [this, e]() {
            if (textEdit) textEdit->append(QString("-Exception occurred: %1").arg(e.what()));
        }, Qt::QueuedConnection);
    }
    catch (...) {
        QMetaObject::invokeMethod(this, [this]() {
            if (textEdit) textEdit->append("-Unknown error occurred during slicing operation.");
        }, Qt::QueuedConnection);
    }

    destroy_marc_api(handle);
}

void MainWindow::SliceModelviaDLL_old() {
    if (!validate_paths(model_paths) ) {
        QMetaObject::invokeMethod(this, [this]() {
            QMessageBox::information(this, "Invalid Paths", "Ensure all model/config paths are valid.");
        }, Qt::QueuedConnection);
        return;
    }

    QMetaObject::invokeMethod(this, [=]() {
        textEdit->clear();
    }, Qt::QueuedConnection);

    MarcHandle handle = create_marc_api(200.0f, 200.0f, 5.0f);
    if (!handle) {
        QMetaObject::invokeMethod(this, [=]() {
            textEdit->append("-Failed to create MARC API handle.");
        }, Qt::QueuedConnection);
        return;
    }

    clearGuiData();

    // Convert from internal format to DLL format
    m_guiDataArrayforDll = toDllFromDll::toDll(this->m_internalguiDataArray);     
    //const char* models_[] = { "femure.stl", "bridge2.stl" };
    //GuiDataArray models;
    // Allocate the size based on available model paths
    //size_t newSize = model_paths.size();
	//resetGuiDataArray(gui_modelArray, newSize);  // clear the previous array and allocate new size

    
  //  models=getMarcPathArray(1); // 1 for models


    MarcErrorCode err = set_models(handle, m_guiDataArrayforDll);
    QMetaObject::invokeMethod(this, [=]() {
        textEdit->append(err == MARC_S_OK ? "-Models loading succeeded" 
                                          : "-Models loading failed");
    }, Qt::QueuedConnection);
    if (err != MARC_S_OK) {
        destroy_marc_api(handle);
        return;
    }

    //const char* configs_[] = { "femure.ini", "bridge2.ini" };
   // MarcPathArray configs ;
    //configs=getMarcPathArray(0); // 0 for ini 
   //err = set_configs(handle, configs);
    QMetaObject::invokeMethod(this, [=]() {
        textEdit->append(err == MARC_S_OK ? "-Config loading succeeded" 
                                          : QString("-Config loading failed"));
    }, Qt::QueuedConnection);
    if (err != MARC_S_OK) {
        destroy_marc_api(handle);
        return;
    }
  
  // Clear for next model selections and resetting
    model_paths.clear();
    //config_paths.clear();
    cofCount = 0;
    modCount = 0;
//-----------------------------------------
//const char* filePath = build_styles_path_json.string().c_str();
err = set_config_json(handle, build_styles_path_json.string().c_str());
    QMetaObject::invokeMethod(this, [=]() {
        textEdit->append(err == MARC_S_OK ? "-Config json loading succeeded" 
                                          : QString("-Config json loading failed"));
    }, Qt::QueuedConnection);
    if (err != MARC_S_OK) {
        destroy_marc_api(handle);
        return;
    }
//-----------------------------------------
    err = update_model(handle);
    QMetaObject::invokeMethod(this, [=]() {
        textEdit->append(err == MARC_S_OK ? "-Model updating succeeded" 
                                          : QString("-Model updating failed:"));
    }, Qt::QueuedConnection);
    if (err != MARC_S_OK) {
        destroy_marc_api(handle);
        return;
    }

    err = export_slm_file(handle);
    QMetaObject::invokeMethod(this, [=]() {
        textEdit->append(err == MARC_S_OK ? "-Model sliced and exported successfully." 
                                          : QString("-Model slicing failed"));
    }, Qt::QueuedConnection);

    destroy_marc_api(handle);
    return;
}
void MainWindow::SliceModel() {
    // Validate paths on the GUI thread
    if (!validate_paths(model_paths)) {
        QMetaObject::invokeMethod(this, [this]() {
            QMessageBox::information(this, "Invalid Paths", "Ensure all model paths are valid.");
        }, Qt::QueuedConnection);
        return;
    }
    if (!validate_paths(config_paths)) {
        QMetaObject::invokeMethod(this, [this]() {
            QMessageBox::information(this, "Invalid Paths", "Ensure all config paths are valid.");
        }, Qt::QueuedConnection);
        return;
    }



    QMetaObject::invokeMethod(this, [=]() {
        
                textEdit->clear();
    }, Qt::QueuedConnection);
    // Print paths to textEdit in the main thread once
    //QMetaObject::invokeMethod(this, [this]() {
       // printModelPathsToTextEdit(model_paths);
      //  printModelPathsToTextEdit(config_paths);
    //}, Qt::QueuedConnection);

    // Initialize the manager
    Marc::MarcAPIManager manager(200.0f, 200.0f);
    MarcErrorCode result;

    // Perform model loading and processing
    result = manager.setModels(model_paths);
    QMetaObject::invokeMethod(this, [=]() {
        textEdit->append(result == MARC_S_OK ? "Models loading succeeded" : "Models loading failed");
    }, Qt::QueuedConnection);
    if (result != MARC_S_OK) return;

    result = manager.setConfigs(config_paths);
    QMetaObject::invokeMethod(this, [=]() {
        textEdit->append(result == MARC_S_OK ? "Config loading succeeded" : "Config loading failed");
    }, Qt::QueuedConnection);
    if (result != MARC_S_OK) return;

   // clear the previous loaded models and configs
    model_paths.clear();
    config_paths.clear();

    result = manager.updateModel();
    QMetaObject::invokeMethod(this, [=]() {
        textEdit->append(result == MARC_S_OK ? "Model updating succeeded" : "Model updating failed");
    }, Qt::QueuedConnection);
    if (result != MARC_S_OK) return;

    result = manager.exportSlmFile();
    QMetaObject::invokeMethod(this, [=]() {
        if (result == MARC_S_OK) {
            textEdit->append("Model sliced and exported successfully.");
        } else {
            textEdit->append("Model slicing failed.");
        }
    }, Qt::QueuedConnection);
   
    
return;
}
void MainWindow::AddConfigurationData()
{ //this->textEdit->append("Adding configurations data will be implemented in future releases");
    //QMessageBox::information(this, "In Progress", "Adding configurations data will be implemented in future releases.");

    
    if (!configDialog)
        configDialog = new ConfigDialog(this);
        configDialog->show();  // or exec() if you want a modal dialog
    //configDialog->exec();  // Blocks until dialog is closed
   // return;
}
bool MainWindow::validate_paths(const std::vector<std::filesystem::path>& paths)
{
    // Check if the vector is empty
    if (paths.empty()) {
        return false;
    }

    // Check each path to ensure it's valid and the file exists
    for (const auto& path : paths) {
        // Check if path is empty
        if (path.empty()) {
            return false;
        }

        // Check if file exists on the filesystem
        if (!std::filesystem::exists(path)) {
            // Log the missing file for debugging purposes
            QMetaObject::invokeMethod(this, [this, path]() {
                textEdit->append("-Error: File does not exist: " +
                    QString::fromStdString(path.string()));
                }, Qt::QueuedConnection);
            return false;
        }

        // Additional check: Ensure it's a regular file and not a directory
        if (!std::filesystem::is_regular_file(path)) {
            QMetaObject::invokeMethod(this, [this, path]() {
                textEdit->append("-Error: Path is not a regular file: " +
                    QString::fromStdString(path.string()));
                }, Qt::QueuedConnection);
            return false;
        }
    }

    // All paths are valid and files exist
    return true;
}
void MainWindow::printModelPathsToTextEdit(const std::vector<std::filesystem::path>& model_paths)
{
    // Clear the QTextEdit before printing new paths
    //this->textEdit->clear();

    // Append the paths to the QTextEdit
    for (const auto& path : model_paths) {
        QString qPath = QString::fromStdString(path.string());
        this->textEdit->append(qPath);
    }
}
void MainWindow::LoadBuildStyle()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Select Build Style JSON"),
                                                    "",
                                                    tr("JSON Files (*.json);;All Files (*)"));

    if (fileName.isEmpty())
        {this->textEdit->append("-No json  configuration file selected.");
            return;
        }
       
    
    QFileInfo fileInfo(fileName);
    this->textEdit->append("-Selected Config File: " + fileInfo.fileName());
    build_styles_path_json = std::filesystem::path(fileName.toStdString());
    return;
   
}
void MainWindow::ShowStlModel()
{

    if (model_paths.empty()) {
        QMessageBox::warning(this, "Error", "No model path specified.");
        return;
    }

        if (this->m_stlViewer) {
            QVector<int> deletedModels = this->m_stlViewer->getdeletedmodels();
            this->m_stlViewer->clearBuildPlate();

        }
       
        // Create a new viewer with the updated STL path
        QString stlPath = QString::fromStdString(model_paths.back().string());
        //this->m_stlViewer = new StlViewer(this->contentArea);

        
        // Call the test emit function
        //m_stlViewer->testEmitLog();
        int index = 0;
        QVector<QVector<double>> optimized_transforms;
       // for (const auto& path : model_paths) {
            //std::cout << "Model path: " << path.string() << std::endl;
           // stlPath = QString::fromStdString(path.string());
           // this->m_stlViewer->addModel(stlPath);
            //this->m_stlViewer->setModelColor(index, 0, 0.9, 0.1);
            //index++;
             //}
        //------------------------------------------------ optimize here
       
        
        
        
        
      // QElapsedTimer timer;
       //timer.start();

      
      // OrientationOptimizerInterface optimizerInterface;
       //optimizerInterface.setModels(this->m_stlViewer->getModelsPointer());
       //optimizerInterface.setWeights(1, 0, 0, 0);
       //optimized_transforms=optimizerInterface.startOptimization(45);


      // qint64 elapsed = timer.elapsed(); // in milliseconds
       // this->textEdit->append("- Time(ms) to Optimize the orientation: " + QString::number(elapsed));
        //------------------------------------------------

        this->m_stlViewer->arrangeModelsOnPlatter();
        this->viewerLayout->addWidget(this->m_stlViewer);
        
      

        // Change background color to light gray
        this->m_stlViewer->setBackgroundColor(0.95, 0.95, 0.95);
  
}
void MainWindow::OrientationOptimizer()
{
   // OrientationOptimizer* optimizerInterface = new OrientationOptimizer();



}
void MainWindow::appendLog(const QString& msg)
{
    this->textEdit->append(msg);
}
//_______________________________
// Example usage - converting from internal to DLL format
void MainWindow::sendDataToDll() {
    // Clear previous DLL data to prevent memory leaks
    clearGuiData();

    // Convert from internal format to DLL format
    m_guiDataArrayforDll = toDllFromDll::toDll(m_internalguiDataArray);

    // Now use m_guiDataArrayforDll with DLL functions
    // e.g., set_models(handle, m_guiDataArrayforDll);
}

// Example usage - converting from DLL to internal format
void MainWindow::receiveDataFromDll() {
    // Assuming m_guiDataArrayforDll was updated by a DLL function
    // e.g., m_guiDataArrayforDll = get_models(handle);

    // Convert from DLL format to internal format
    m_internalguiDataArray = toDllFromDll::fromDll(m_guiDataArrayforDll);

    // Free DLL data after processing
    toDllFromDll::freeGuiDataArray(m_guiDataArrayforDll);
}

// Clear DLL data to prevent memory leaks
void MainWindow::clearGuiData() {
    // Only attempt to free if models pointer is valid and count is non-zero
    if (m_guiDataArrayforDll.models != nullptr && m_guiDataArrayforDll.count > 0) {
        // Log the freeing operation for debugging
        QMetaObject::invokeMethod(this, [this]() {
            textEdit->append("-Freeing model array with " +
                QString::number(m_guiDataArrayforDll.count) + " models");
        }, Qt::QueuedConnection);

        delete[] m_guiDataArrayforDll.models;

        // Reset to safe state to prevent double-free issues
        m_guiDataArrayforDll.models = nullptr;
        m_guiDataArrayforDll.count = 0;
    }
    else {
        // Initialize to safe empty state
        m_guiDataArrayforDll.models = nullptr;
        m_guiDataArrayforDll.count = 0;
    }
}

bool MainWindow::prepareModelForDLL()
{   // Start with a header
   
   /*QMetaObject::invokeMethod(this, [this]() {
       
        textEdit->append("-------- Preparing  models data --------");
        }, Qt::QueuedConnection);
    // STEP 1: Validate model paths
    if (!validate_paths(model_paths)) {
        QMetaObject::invokeMethod(this, [this]() {
            QMessageBox::warning(this, "Invalid Paths", "Please select valid STL model files.");
            }, Qt::QueuedConnection);
        return false;
    }*/

  
    // STEP 2: Validate config files exist
    if (build_config_path_json.empty() || !std::filesystem::exists(build_config_path_json)) {
        QMetaObject::invokeMethod(this, [this]() {
            QMessageBox::warning(this, "Invalid Config file", "Please select a valid build configuration JSON file.");
            }, Qt::QueuedConnection);
        return false;
    }

    //if (build_styles_path_json.empty() || !std::filesystem::exists(build_styles_path_json)) {
       // QMetaObject::invokeMethod(this, [this]() {
          //  QMessageBox::warning(this, "Invalid Styles", "Please select a valid Build styles JSON file.");
         //  }, Qt::QueuedConnection);
      //  return false;
    //}

    QMetaObject::invokeMethod(this, [this]() {
        if (textEdit) textEdit->append("-File Path Validated!");
    }, Qt::QueuedConnection);
   
    m_internalguiDataArray.clear(); // Clear any existing models before adding new ones
    // STEP 3: Convert model paths to InternalGuiModel array if needed
    m_internalguiDataArray = this->m_stlViewer ? this->m_stlViewer->getModels() : std::vector<InternalGuiModel>{};
    if (!m_internalguiDataArray.empty()) {
        for (auto &model : m_internalguiDataArray) {
            model.buildconfig = this->build_config_path_json.string();
            model.stylesconfig = this->build_styles_path_json.string();
        }
    }
 
  
    if (m_internalguiDataArray.empty()) {
        QMetaObject::invokeMethod(this, [this]() {
            QMessageBox::warning(this, "No Valid Models", "No valid models available to process.");
        }, Qt::QueuedConnection);
        return false;
    }
    debugInternalModelArray();
    //return true;
    // STEP 4: Clean up previous DLL data and prepare model data
 // clearGuiData();
  QMetaObject::invokeMethod(this, [this]() {

      textEdit->append("-End Prepare models ");
      }, Qt::QueuedConnection);
    try {
        // Convert from internal format to DLL format
        m_guiDataArrayforDll = toDllFromDll::toDll(m_internalguiDataArray);

        // STEP 5: Verify models array structure
        if (!m_guiDataArrayforDll.models || m_guiDataArrayforDll.count == 0) {
            QMetaObject::invokeMethod(this, [this]() {
                if (textEdit) textEdit->append("-Error: Invalid model data structure.");
            }, Qt::QueuedConnection);
            return false;
        }

        // Verify each individual model path exists and is valid
        for (size_t i = 0; i < m_guiDataArrayforDll.count; i++) {
            // Check if path field is valid
            if (m_guiDataArrayforDll.models[i].path[0] == '\0') {
                QMetaObject::invokeMethod(this, [this, i]() {
                    if (textEdit) textEdit->append(QString("-Error: Model %1 has empty path.").arg(i));
                }, Qt::QueuedConnection);
                return false;
            }

            // Check if the configured paths actually exist
            std::string modelPath = m_guiDataArrayforDll.models[i].path;
            if (!std::filesystem::exists(modelPath)) {
                QMetaObject::invokeMethod(this, [this, modelPath]() {
                    if (textEdit) textEdit->append(QString("-Error: Model file does not exist: %1")
                        .arg(QString::fromStdString(modelPath)));
                }, Qt::QueuedConnection);
                return false;
            }

            // If model has a specific buildconfig, check it
            if (m_guiDataArrayforDll.models[i].buildconfig[0] != '\0') {
                std::string configPath = m_guiDataArrayforDll.models[i].buildconfig;
                if (!std::filesystem::exists(configPath)) {
                    QMetaObject::invokeMethod(this, [this, configPath]() {
                        if (textEdit) textEdit->append(QString("-Error: Build config file does not exist: %1")
                            .arg(QString::fromStdString(configPath)));
                    }, Qt::QueuedConnection);
                    return false;
                }
            }

            // If model has a specific stylesconfig, check it
            if (m_guiDataArrayforDll.models[i].stylesconfig[0] != '\0') {
                std::string stylesPath = m_guiDataArrayforDll.models[i].stylesconfig;
                if (!std::filesystem::exists(stylesPath)) {
                    QMetaObject::invokeMethod(this, [this, stylesPath]() {
                        if (textEdit) textEdit->append(QString("-Error: Styles config file does not exist: %1")
                            .arg(QString::fromStdString(stylesPath)));
                    }, Qt::QueuedConnection);
                    return false;
                }
            }
        }

        // Log information about models being processed
        QMetaObject::invokeMethod(this, [this]() {
            if (textEdit) textEdit->append(QString("-Processing %1 models...").arg(m_guiDataArrayforDll.count));
        }, Qt::QueuedConnection);

        printGuiDataArray();
        return true; // Successfully prepared data
    }
    catch (const std::exception& e) {
        QMetaObject::invokeMethod(this, [this, e]() {
            if (textEdit) textEdit->append(QString("-Exception occurred: %1").arg(e.what()));
        }, Qt::QueuedConnection);
        return false;
    }
    catch (...) {
        QMetaObject::invokeMethod(this, [this]() {
            if (textEdit) textEdit->append("-Unknown error occurred during model preparation.");
        }, Qt::QueuedConnection);
        return false;
    }
}
void MainWindow::printGuiDataArray()
{
    // Start with a header
    qDebug() << "-------- GUI Data Array Contents --------";

    if (!m_guiDataArrayforDll.models || m_guiDataArrayforDll.count == 0) {
        qDebug() << "-No models in data array";
        qDebug() << "------------------------------------";
        return;
    }

    qDebug() << "-Total models:" << m_guiDataArrayforDll.count;

    // Print details for each model
    for (size_t i = 0; i < m_guiDataArrayforDll.count; i++) {
        const GuiData& model = m_guiDataArrayforDll.models[i];

        qDebug() << "\n-Model #" << (i + 1) << "(" << model.number << "):";
        qDebug() << "  Path:" << model.path;
        qDebug().nospace() << "  Position: X=" << qSetRealNumberPrecision(2) << model.xpos
            << ", Y=" << model.ypos
            << ", Z=" << model.zpos;
        qDebug().nospace() << "  Rotation: Roll=" << qSetRealNumberPrecision(2) << model.roll
            << "°, Pitch=" << model.pitch
            << "°, Yaw=" << model.yaw << "°";

        if (model.buildconfig[0] != '\0') {
            qDebug() << "  Build config:" << model.buildconfig;
        }
        else {
            qDebug() << "  Build config: <none>";
        }

        if (model.stylesconfig[0] != '\0') {
            qDebug() << "  Styles config:" << model.stylesconfig;
        }
        else {
            qDebug() << "  Styles config: <none>";
        }
    }

    qDebug() << "\n------------------------------------";
}

void MainWindow::debugInternalModelArray()
{
    qDebug() << "-------- Internal Model Array Debug Output --------";

    if (m_internalguiDataArray.empty()) {
        qDebug() << "No models in internal array";
        qDebug() << "------------------------------------";
        return;
    }

    qDebug() << "Total internal models:" << m_internalguiDataArray.size();

    // Print details for each model
    for (size_t i = 0; i < m_internalguiDataArray.size(); i++) {
        const InternalGuiModel& model = m_internalguiDataArray[i];

        qDebug() << "\nInternal Model #" << (i + 1) << "(" << model.model_number << "):";
        qDebug() << "  Path:" << QString::fromStdString(model.path);
        qDebug().nospace() << "  Position: X=" << qSetRealNumberPrecision(2) << model.xpos
            << ", Y=" << model.ypos
            << ", Z=" << model.zpos;
        qDebug().nospace() << "  Rotation: Roll=" << qSetRealNumberPrecision(2) << model.roll
            << "°, Pitch=" << model.pitch
            << "°, Yaw=" << model.yaw << "°";

        if (!model.buildconfig.empty()) {
            qDebug() << "  Build config:" << QString::fromStdString(model.buildconfig);
        }
        else {
            qDebug() << "  Build config: <none>";
        }

        if (!model.stylesconfig.empty()) {
            qDebug() << "  Styles config:" << QString::fromStdString(model.stylesconfig);
        }
        else {
            qDebug() << "  Styles config: <none>";
        }
    }

    qDebug() << "------------------------------------";
}