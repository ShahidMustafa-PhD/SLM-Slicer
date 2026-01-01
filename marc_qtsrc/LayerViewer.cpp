#include "LayerViewer.h"

LayerViewer::LayerViewer(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle("Layer Viewer");
    resize(1000, 800);
   //ApplyScreenLayout();
    setupUI();
    //applyStyles();
   // QVBoxLayout *mainLayout = new QVBoxLayout(this);
if(0)
{
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    QVBoxLayout *sidebarLayout = new QVBoxLayout();
    sidebarLayout->setAlignment(Qt::AlignTop);
    sidebarLayout->setContentsMargins(5, 5, 5, 5);  // Add margins to the sidebar layout

    // Sidebar widget
    QWidget *sidebarWidget = new QWidget(this);
    sidebarWidget->setStyleSheet("background-color: #2E3440;");  // dark sidebar
    sidebarWidget->setMinimumWidth(90);
    sidebarWidget->setMaximumWidth(120);
    sidebarWidget->setLayout(sidebarLayout);
    
    // Add sidebar widget to main layout
    mainLayout->addWidget(sidebarWidget);

    // Add buttons to the sidebar with styles
    folderButton = new QPushButton("📂 Layers Folder", this);
    folderButton->setStyleSheet("QPushButton {"
        "text-align: left;"
        "padding: 10px;"
        "font-size: 14px;"
        "background-color: transparent;"
        "color: white;"
        "border: none;"
        "}"
        "QPushButton:hover {"
        "background-color: #4C566A;"
        "}");
    sidebarLayout->addWidget(folderButton);

    toggleViewButton = new QPushButton("🔀 Toggle 2D/3D", this);
    toggleViewButton->setStyleSheet("QPushButton {"
        "text-align: left;"
        "padding: 10px;"
        "font-size: 14px;"
        "background-color: transparent;"
        "color: white;"
        "border: none;"
        "}"
        "QPushButton:hover {"
        "background-color: #4C566A;"
        "}");
    sidebarLayout->addWidget(toggleViewButton);

    animateViewButton = new QPushButton("🚀 Animate 3D Stack", this);
    animateViewButton->setStyleSheet("QPushButton {"
        "text-align: left;"
        "padding: 10px;"
        "font-size: 14px;"
        "background-color: transparent;"
        "color: white;"
        "border: none;"
        "}"
        "QPushButton:hover {"
        "background-color: #4C566A;"
        "}");
    sidebarLayout->addWidget(animateViewButton);

    layerLabel = new QLabel("Layer: 1", this);
    layerLabel->setStyleSheet("color: white; font-size: 14px;");
    sidebarLayout->addWidget(layerLabel);

    // Add vertical slider to sidebar
    layerSlider = new QSlider(Qt::Vertical, this);
    layerSlider->setMinimum(1);
    layerSlider->setMaximum(1);
    layerSlider->setValue(1);
    sidebarLayout->addWidget(layerSlider);

    // Graphics View Setup
    scene = new QGraphicsScene(this);
    graphicsView = new QGraphicsView(scene, this);
    graphicsView->setRenderHint(QPainter::Antialiasing);
    graphicsView->setStyleSheet("background: white; border-radius: 10px;");
    mainLayout->addWidget(graphicsView, 1);  // Add the graphics view to the layout

    // Set up connections for buttons
    connect(folderButton, &QPushButton::clicked, this, &LayerViewer::loadFolder);
    connect(layerSlider, &QSlider::valueChanged, this, &LayerViewer::updateLayer);
    connect(toggleViewButton, &QPushButton::clicked, this, &LayerViewer::toggle3DView);
    connect(animateViewButton, &QPushButton::clicked, this, &LayerViewer::animate3DTransition);

    applyStyleSheet();

}
}

void LayerViewer::applyStyleSheet()
{
    this->setStyleSheet(R"(
        QWidget {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                          stop:0 #e0f7fa, stop:1 #ffffff);
        }
        QPushButton {
            background-color: #4fc3f7;
            border: none;
            color: white;
            padding: 10px 20px;
            border-radius: 10px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #039be5;
        }
        QLabel {
            font-size: 16px;
            font-weight: bold;
        }
    QSlider::vertical {
    width: 20px;
    background: #e0f7fa;
        }
    )");
}

void LayerViewer::loadFolder()
{
    QString dir = QFileDialog::getExistingDirectory(this, "Select Folder Containing SVG Layers");
    if (!dir.isEmpty()) {
        folderPath = dir;
        QDir directory(folderPath);
        svgFiles = directory.entryList(QStringList() << "*.svg", QDir::Files);
        svgFiles.sort();

        int layerCount = svgFiles.count();
        if (layerCount >= 0) {
            layerSlider->setMaximum(layerCount);
            layerSlider->setValue(0);
            is3DView = false;
            toggleViewButton->setText("🔀 Toggle 2D/3D");
            show2D();
        }
    }
}

void LayerViewer::updateLayer(int value)
{
    layerLabel->setText(QString("Layer: %1").arg(value));
    if (!is3DView) {
        show2D();
    }
}

void LayerViewer::toggle3DView()
{
    is3DView = !is3DView;
    if (is3DView) {
        toggleViewButton->setText("🔀 3D to 2D");
        show3D();
    } else {
        toggleViewButton->setText("🔀 2D to 3D");
        show2D();
    }
}

void LayerViewer::show2D()
{
    clearScene();
    int layerNumber = layerSlider->value();
    loadSVG(layerNumber);
    graphicsView->fitInView(scene->sceneRect(), Qt::KeepAspectRatio);
}

void LayerViewer::show3D()
{
   clearScene();
    
    if (folderPath.isEmpty())
    return;

    QDir directory(folderPath);
      svgFiles = directory.entryList(QStringList() << "*.svg", QDir::Files);
      svgFiles.sort();

    int loadedItemCount = 0;
    int totalItemCount = svgFiles.size(); // Set total number of SVGs

    SvgLoaderWorker* worker = new SvgLoaderWorker(folderPath, svgFiles);
    QThread* loaderThread = new QThread;

    worker->moveToThread(loaderThread);

    connect(loaderThread, &QThread::started, worker, &SvgLoaderWorker::process);
    connect(worker, &SvgLoaderWorker::svgItemLoaded, this, &LayerViewer::handleSvgItemLoaded);
    connect(worker, &SvgLoaderWorker::finished, loaderThread, &QThread::quit);
    connect(worker, &SvgLoaderWorker::finished, worker, &SvgLoaderWorker::deleteLater);
    connect(loaderThread, &QThread::finished, loaderThread, &QThread::deleteLater);

    loaderThread->start();

    // Show loading message and cancel button
    //loadingLabel->show();
    //cancelButton->show(); 

    /*
    double zSpacing = 10.0;

    for (int i = 0; i < svgFiles.size(); ++i) {
        QString filePath = folderPath + "/" + svgFiles[i];
        QGraphicsSvgItem *svgItem = new QGraphicsSvgItem(filePath);
        svgItem->setZValue(i * zSpacing);
        svgItem->setPos(0, 0);  // Centered
        scene->addItem(svgItem);
    }

    graphicsView->fitInView(scene->itemsBoundingRect(), Qt::KeepAspectRatio);
    */
}

void LayerViewer::animate3DTransition()
{    QMessageBox::information(this, "Information", "This is not implemented Yet!  In the future, it will show layers animation in 3D view.");
    return;
    clearScene();
    QDir directory(folderPath);
    //QStringList 
    //svgFiles = directory.entryList(QStringList() << "*.svg", QDir::Files);
    //svgFiles.sort();

    double zSpacing = 10.0;
    QParallelAnimationGroup *group = new QParallelAnimationGroup(this);

    for (int i = 0; i < svgFiles.size(); ++i) {
        QString filePath = folderPath + "/" + svgFiles[i];
        QGraphicsSvgItem *svgItem = new QGraphicsSvgItem(filePath);
        svgItem->setZValue(0);
        svgItem->setPos(0, 0);
        scene->addItem(svgItem);

        QPropertyAnimation *anim = new QPropertyAnimation(svgItem, "zValue");
        anim->setDuration(1000);
        anim->setStartValue(0);
        anim->setEndValue(i * zSpacing);
        anim->setEasingCurve(QEasingCurve::OutCubic);

        group->addAnimation(anim);
    }

    group->start(QAbstractAnimation::DeleteWhenStopped);
    graphicsView->fitInView(scene->itemsBoundingRect(), Qt::KeepAspectRatio);
}

void LayerViewer::loadSVG(int layerNumber)
{
    if (folderPath.isEmpty())
        return;

    QString filePath = QString("%1/Layer%2.svg").arg(folderPath).arg(layerNumber);
    QFile file(filePath);
    if (!file.exists()) {
        return;
    }

    // Clear the previous scene before loading a new one
    scene->clear();

    // Add the new SVG to the scene
    QGraphicsSvgItem *svgItem = new QGraphicsSvgItem(filePath);
    scene->addItem(svgItem);

    // Optionally adjust the view to fit the SVG (this ensures the image is fully visible after loading)
    graphicsView->fitInView(scene->sceneRect(), Qt::KeepAspectRatio);
}

void LayerViewer::clearScene()
{
    scene->clear();
}

void LayerViewer::chooseDirectory()
{
    QSettings settings("ShahidMustafa", "MarcSLM");  // <<< Unique names

    // Load previous directory or default to home
    QString lastDir = settings.value("lastDirectory", QDir::homePath()).toString();

    QString dir = QFileDialog::getExistingDirectory(this, "Select Directory", lastDir);
    if (!dir.isEmpty())
    {
        folderPath = dir;
        settings.setValue("lastDirectory", dir); // <<< Save the new directory
        updateLayers();
    }
}

void LayerViewer::updateLayers()
{
    if (folderPath.isEmpty())
        return;

    // Clear existing file list
    svgFiles.clear();

    QDir dir(folderPath);
    QStringList filters;
    filters << "Layer*.svg";  // Only files like Layer1.svg, Layer2.svg
    svgFiles = dir.entryList(filters, QDir::Files, QDir::Name);

    if (!svgFiles.isEmpty()) {
        layerSlider->setRange(0, svgFiles.size()-1); // Update the slider range
        layerSlider->setValue(0);                  // Reset to first layer
        //loadLayer(0);                         // Load first layer immediately
    }
}

void LayerViewer::ApplyScreenLayout()
{// Create main horizontal layout
    
    if (1) {
        QHBoxLayout *mainLayout = new QHBoxLayout(this);
        
        // Create the sidebar layout
        QVBoxLayout *sidebarLayout = new QVBoxLayout();
        sidebarLayout->setAlignment(Qt::AlignTop);
        sidebarLayout->setContentsMargins(5, 5, 5, 5);  // Add margins to the sidebar layout
    
        // Sidebar widget
        QWidget *sidebarWidget = new QWidget(this);
        sidebarWidget->setStyleSheet("background-color: #2E3440;");  // Dark sidebar
        sidebarWidget->setMinimumWidth(90);
        sidebarWidget->setMaximumWidth(120);
        sidebarWidget->setLayout(sidebarLayout);
        
        // Add sidebar widget to the main layout (on the left side)
        mainLayout->addWidget(sidebarWidget);
    
        // Add buttons to the sidebar with styles
        folderButton = new QPushButton("📂 Layers Folder", this);
        folderButton->setStyleSheet("QPushButton {"
            "text-align: left;"
            "padding: 10px;"
            "font-size: 14px;"
            "background-color: transparent;"
            "color: white;"
            "border: none;}"
            "QPushButton:hover {"
            "background-color: #4C566A;}"
        );
        sidebarLayout->addWidget(folderButton);
    
        toggleViewButton = new QPushButton("🔀 Toggle 2D/3D", this);
        toggleViewButton->setStyleSheet("QPushButton {"
            "text-align: left;"
            "padding: 10px;"
            "font-size: 14px;"
            "background-color: transparent;"
            "color: white;"
            "border: none;}"
            "QPushButton:hover {"
            "background-color: #4C566A;}"
        );
        sidebarLayout->addWidget(toggleViewButton);
    
        animateViewButton = new QPushButton("🚀 Animate 3D Stack", this);
        animateViewButton->setStyleSheet("QPushButton {"
            "text-align: left;"
            "padding: 10px;"
            "font-size: 14px;"
            "background-color: transparent;"
            "color: white;"
            "border: none;}"
            "QPushButton:hover {"
            "background-color: #4C566A;}"
        );
        sidebarLayout->addWidget(animateViewButton);
    
        layerLabel = new QLabel("Layer: 1", this);
        layerLabel->setStyleSheet("color: white; font-size: 14px;");
        sidebarLayout->addWidget(layerLabel);
    
        // Graphics View Setup
        scene = new QGraphicsScene(this);
        graphicsView = new QGraphicsView(scene, this);
        graphicsView->setRenderHint(QPainter::Antialiasing);
        graphicsView->setStyleSheet("background: white; border-radius: 10px;");
        mainLayout->addWidget(graphicsView, 1);  // Add the graphics view to the layout
    
        // Create the right side widget for the slider
        QWidget *rightSideWidget = new QWidget(this);
        QVBoxLayout *rightSideLayout = new QVBoxLayout(rightSideWidget);
        rightSideWidget->setStyleSheet("background-color: #2E3440;");  // Same background color as sidebar
        rightSideWidget->setMinimumWidth(50);  // Set a minimum width for the right-side widget
        rightSideWidget->setMaximumWidth(150); // Set a maximum width for the right-side widget
        
        // Add the vertical slider to the right-side layout
        layerSlider = new QSlider(Qt::Vertical, this);
        layerSlider->setMinimum(0);
        layerSlider->setMaximum(1);
        layerSlider->setValue(0);
        rightSideLayout->addWidget(layerSlider);
        
        // Add the right side widget to the main layout (on the right side)
        mainLayout->addWidget(rightSideWidget);  // Add the right-side widget containing the slider
    
        // Set up connections for buttons
        connect(folderButton, &QPushButton::clicked, this, &LayerViewer::loadFolder);
        connect(layerSlider, &QSlider::valueChanged, this, &LayerViewer::updateLayer);
        connect(toggleViewButton, &QPushButton::clicked, this, &LayerViewer::toggle3DView);
        connect(animateViewButton, &QPushButton::clicked, this, &LayerViewer::animate3DTransition);
    
        applyStyleSheet();
    }
    
    
    
}

void LayerViewer::setupZoomSlider()
{
    // Create a slider for zoom control
    zoomSlider = new QSlider(Qt::Horizontal, this);
    zoomSlider->setRange(1, 200);  // Zoom range (1% to 200%)
    zoomSlider->setValue(100);  // Set initial value to 100% (no zoom)
    zoomSlider->setStyleSheet("QSlider {"
                              "height: 15px;"
                              "border-radius: 5px;"
                              "background: #4C566A;"
                              "border: 1px solid #2E3440;"
                              "}"
                              "QSlider::handle:horizontal {"
                              "background: #88C0D0;"
                              "border-radius: 5px;"
                              "width: 15px;"
                              "height: 15px;"
                              "border: 1px solid #2E3440;"
                              "}");
    // Add the slider to the layout (right-side of the window)
    //rightSideLayout->addWidget(zoomSlider);

    // Connect the slider's value change to the zoom function
    //connect(zoomSlider, &QSlider::valueChanged, this, &LayerViewer::zoomImage);
}
void LayerViewer::zoomImage(int value)
{
    // Calculate the zoom scale based on the slider value
    qreal scaleFactor = value / 100.0;  // 100% corresponds to a scale factor of 1

    // Set the scale on the QGraphicsView (this will zoom in or out the scene)
    graphicsView->resetTransform();
    graphicsView->scale(scaleFactor, scaleFactor);  // Zoom the scene based on the slider value
}

void LayerViewer::setupUI()
{
   
    //-----------------------------------------------------------------//
 
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    
    // Create the sidebar layout
    QVBoxLayout *sidebarLayout = new QVBoxLayout();
    sidebarLayout->setAlignment(Qt::AlignTop);
    sidebarLayout->setContentsMargins(5, 5, 5, 5);  // Add margins to the sidebar layout
    
    // Sidebar widget
    QWidget *sidebarWidget = new QWidget(this);
    sidebarWidget->setStyleSheet("background-color: #2E3440;");  // Dark sidebar
    sidebarWidget->setMinimumWidth(90);
    sidebarWidget->setMaximumWidth(150);
    sidebarWidget->setLayout(sidebarLayout);
    
    // Add sidebar widget to the main layout (on the left side)
    mainLayout->addWidget(sidebarWidget);
    
    // Add buttons to the sidebar with styles
    folderButton = new QPushButton("📂 Layers Folder", this);
    folderButton->setStyleSheet("QPushButton {"
        "text-align: left;"
        "padding: 10px;"
        "font-size: 14px;"
        "background-color: transparent;"
        "color: white;"
        "border: none;}"
        "QPushButton:hover {"
        "background-color: #4C566A;}");

    sidebarLayout->addWidget(folderButton);

    toggleViewButton = new QPushButton("🔀 2D to 3D", this);
    toggleViewButton->setStyleSheet("QPushButton {"
        "text-align: left;"
        "padding: 10px;"
        "font-size: 14px;"
        "background-color: transparent;"
        "color: white;"
        "border: none;}"
        "QPushButton:hover {"
        "background-color: #4C566A;}");

    sidebarLayout->addWidget(toggleViewButton);

    animateViewButton = new QPushButton("🚀 Animate 3D Stack", this);
    animateViewButton->setStyleSheet("QPushButton {"
        "text-align: left;"
        "padding: 10px;"
        "font-size: 14px;"
        "background-color: transparent;"
        "color: white;"
        "border: none;}"
        "QPushButton:hover {"
        "background-color: #4C566A;}");

    sidebarLayout->addWidget(animateViewButton);

    layerLabel = new QLabel("Layer: 1", this);
    layerLabel->setStyleSheet("color: white; font-size: 14px;");
    sidebarLayout->addWidget(layerLabel);

    // Graphics View Setup
    scene = new QGraphicsScene(this);
    graphicsView = new QGraphicsView(scene, this);
    graphicsView->setRenderHint(QPainter::Antialiasing);
    graphicsView->setStyleSheet("background: white; border-radius: 10px;");
   
//--------------------For Simulated 3DView----------------------------------------------//
graphicsView->setRenderHint(QPainter::SmoothPixmapTransform);
// Enable the 3D-like effect by adjusting the transformation matrix (camera angle)


//-------------------------------------------------------------------------------------//



    mainLayout->addWidget(graphicsView, 1);  // Add the graphics view to the layout

    // Create the right side widget for the sliders
    QWidget *rightSideWidget = new QWidget(this);
    QVBoxLayout *rightSideLayout = new QVBoxLayout(rightSideWidget);
    rightSideWidget->setStyleSheet("background-color: #2E3440;");  // Same background color as sidebar
    rightSideWidget->setMinimumWidth(50);  // Set a minimum width for the right-side widget
    rightSideWidget->setMaximumWidth(100); // Set a maximum width for the right-side widget

    // Create a horizontal layout to place sliders side by side
    QHBoxLayout *slidersLayout = new QHBoxLayout();

    // Layer Slider Label
    QLabel *layerSliderLabel = new QLabel("Layer", this);
    layerSliderLabel->setStyleSheet("color: white; font-size: 14px;");
    
    // Add Layer Slider
    layerSlider = new QSlider(Qt::Vertical, this);
    layerSlider->setMinimum(0);
    layerSlider->setMaximum(1);
    layerSlider->setValue(0);
    layerSlider->setStyleSheet("QSlider {"
        "height: 100px;"
        "border-radius: 5px;"
        "background: #4C566A;"
        "border: 1px solid #2E3440;"
        "}"
        "QSlider::handle:vertical {"
        "background: #88C0D0;"
        "border-radius: 5px;"
        "width: 15px;"
        "height: 15px;"
        "border: 1px solid #2E3440;"
        "}");

    // Add Zoom Slider Label
    QLabel *zoomSliderLabel = new QLabel("Zoom", this);
    zoomSliderLabel->setStyleSheet("color: white; font-size: 14px;");

    // Add Zoom Slider
    zoomSlider = new QSlider(Qt::Vertical, this);
    zoomSlider->setMinimum(10);  // Min zoom (10%)
    zoomSlider->setMaximum(1000); // Max zoom (500%)
    zoomSlider->setValue(100);   // Default zoom (100%)
    zoomSlider->setStyleSheet("QSlider {"
        "height: 100px;"
        "border-radius: 5px;"
        "background: #4C566A;"
        "border: 1px solid #2E3440;"
        "}"
        "QSlider::handle:vertical {"
        "background: #81A1C1;"
        "border-radius: 5px;"
        "width: 15px;"
        "height: 15px;"
        "border: 1px solid #2E3440;"
        "}");

    // Place the sliders in the layout
    slidersLayout->addWidget(layerSliderLabel);
    slidersLayout->addWidget(layerSlider);
    slidersLayout->addWidget(zoomSliderLabel);
    slidersLayout->addWidget(zoomSlider);

    rightSideLayout->addLayout(slidersLayout);

    // Add the right side widget containing sliders to the main layout
    mainLayout->addWidget(rightSideWidget);

    // Add "+" and "-" labels for the Zoom Slider
    QLabel *zoomInLabel = new QLabel("+", this);
    zoomInLabel->setAlignment(Qt::AlignCenter);
    zoomInLabel->setStyleSheet("color: white; font-size: 16px;");
    QLabel *zoomOutLabel = new QLabel("-", this);
    zoomOutLabel->setAlignment(Qt::AlignCenter);
    zoomOutLabel->setStyleSheet("color: white; font-size: 16px;");

    // Place the "+" and "-" labels above and below the zoom slider
    rightSideLayout->addWidget(zoomInLabel);
    rightSideLayout->addWidget(zoomSlider);
    rightSideLayout->addWidget(zoomOutLabel);

    // Set up connections for buttons and sliders
    connect(folderButton, &QPushButton::clicked, this, &LayerViewer::loadFolder);
    connect(layerSlider, &QSlider::valueChanged, this, &LayerViewer::updateLayer);
    connect(toggleViewButton, &QPushButton::clicked, this, &LayerViewer::toggle3DView);
    connect(animateViewButton, &QPushButton::clicked, this, &LayerViewer::animate3DTransition);
    connect(zoomSlider, &QSlider::valueChanged, this, &LayerViewer::zoomImage);  // Connect zoom slider

    applyStyleSheet();
   
}
// In LayerViewer.cpp
void LayerViewer::handleSvgItemLoaded(QGraphicsSvgItem* item)
{
    if (!scene || !item) 
        return;  // Always good to safety-check

    // Add the loaded SVG item to the scene
    scene->addItem(item);
    graphicsView->fitInView(scene->itemsBoundingRect(), Qt::KeepAspectRatio);
    // Increment the loaded items counter
    //loadedItemCount++;

    // Optionally update a progress indicator here (if you have one)

    // Check if all SVGs have been loaded
    //if (loadedItemCount >= totalItemCount) 
    //{
        // Adjust the view to fit all items
        //graphicsView->fitInView(scene->itemsBoundingRect(), Qt::KeepAspectRatio);

        // Hide the loading message and cancel button
        //if (loadingLabel)
            //loadingLabel->hide();
        
        //if (cancelButton)
           // cancelButton->hide();
    //}
}

void LayerViewer::cancelLoading()
{
    if (loaderThread) {
        loaderThread->requestInterruption(); // Ask thread to stop
    }
}

void LayerViewer::onSvgLayerLoaded()
{ // Once the layer is loaded, fit the view and hide the loading label
    graphicsView->fitInView(scene->sceneRect(), Qt::KeepAspectRatio);
    loadingLabel->hide();  // Hide the loading message when done
}
