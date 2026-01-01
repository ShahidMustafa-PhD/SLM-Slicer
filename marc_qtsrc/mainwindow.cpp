#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QFileInfo>
#include <QMetaObject>
#include <QtConcurrent/QtConcurrent>
#include <QThread>
#include <QLabel>
#include <QFrame>
#include <QGridLayout>
#include <QIcon>

// ============================================================================
// Constructor and Destructor
// ============================================================================

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , scene(new QGraphicsScene(this))
    , svgItem(nullptr)
{
    // Main container widget
    QWidget *central = new QWidget(this);
    this->setCentralWidget(central);

    // Setup UI components
    this->setupUserInterface(central);
}

MainWindow::~MainWindow()
{
    cleanupGuiDataStructures();
    if (ui) {
        delete ui;
        ui = nullptr;
    }
}

// ============================================================================
// UI Setup Methods
// ============================================================================

void MainWindow::setupUserInterface(QWidget *centralWidget)
{
    if (!centralWidget) {
        centralWidget = new QWidget(this);
        setCentralWidget(centralWidget);
    }

    // Apply global stylesheet for consistency
    centralWidget->setStyleSheet(
        "QMainWindow, QWidget {"
        "  background-color: #FFFFFF;"
        "}"
        "QLabel {"
        "  color: #2C3E50;"
        "}"
        "QTextEdit {"
        "  border: 1px solid #ECF0F1;"
        "  border-radius: 4px;"
        "  padding: 4px;"
        "}"
    );

    // Main vertical layout
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // Create ribbon bar
    createRibbonBar(mainLayout, centralWidget);

    // Horizontal container for sidebars and content
    QWidget *horizontalContainer = new QWidget(centralWidget);
    QHBoxLayout *horizontalLayout = new QHBoxLayout(horizontalContainer);
    horizontalLayout->setContentsMargins(0, 0, 0, 0);
    horizontalLayout->setSpacing(0);

    // Create sidebar and content areas
    createLeftSidebar(centralWidget, horizontalLayout);
    createContentArea(centralWidget, horizontalLayout);
    createRightSidebar(centralWidget, horizontalLayout);

    // Add to main layout
    mainLayout->addWidget(horizontalContainer);

    // Connect signals and apply styles
    connectSignalsAndSlots();
    applyStylesheets();

    // Set window defaults
    this->setWindowTitle("MarcSLM - Professional 3D Model Slicer");
    this->setWindowIcon(QIcon(":/icons/app_icon.png"));
    
    // Enhanced status bar
    statusBar()->setStyleSheet(
        "QStatusBar {"
        "  background-color: #F5F6FA;"
        "  border-top: 1px solid #ECF0F1;"
        "  color: #7F8C8D;"
        "  font-size: 11px;"
        "  padding: 4px 8px;"
        "}"
    );
    statusBar()->showMessage("Ready");
    
    resize(1400, 900);
    setMinimumSize(1200, 800);
}

void MainWindow::createRibbonBar(QVBoxLayout *mainLayout, QWidget *centralWidget)
{
    QWidget *ribbonBar = new QWidget(centralWidget);
    ribbonBar->setStyleSheet(
        "QWidget {"
        "  background-color: #2C3E50;"
        "  border-bottom: 1px solid #34495E;"
        "}"
        "QPushButton {"
        "  padding: 8px 16px;"
        "  font-size: 13px;"
        "  font-weight: 500;"
        "  background-color: #3498DB;"
        "  color: white;"
        "  border: none;"
        "  border-radius: 4px;"
        "  margin: 0px 4px;"
        "}"
        "QPushButton:hover {"
        "  background-color: #2980B9;"
        "}"
        "QPushButton:pressed {"
        "  background-color: #1F618D;"
        "}"
        "QPushButton:disabled {"
        "  background-color: #95A5A6;"
        "  color: #7F8C8D;"
        "}"
    );
    ribbonBar->setFixedHeight(55);

    QHBoxLayout *ribbonLayout = new QHBoxLayout(ribbonBar);
    ribbonLayout->setContentsMargins(12, 8, 12, 8);
    ribbonLayout->setSpacing(8);

    // Logo/Title area
    QLabel *logoLabel = new QLabel("MarcSLM", ribbonBar);
    logoLabel->setStyleSheet(
        "QLabel {"
        "  color: white;"
        "  font-size: 16px;"
        "  font-weight: 600;"
        "  padding: 0px 8px;"
        "}"
    );
    ribbonLayout->addWidget(logoLabel);

    // Separator
    QFrame *separator1 = new QFrame(ribbonBar);
    separator1->setFrameShape(QFrame::VLine);
    separator1->setStyleSheet("QFrame { color: #34495E; }");
    ribbonLayout->addWidget(separator1);

    // File operations section
    btnSlice = new QPushButton("🚀  Slice", ribbonBar);
    btnSlice->setStyleSheet(
        "QPushButton {"
        "  padding: 8px 24px;"
        "  font-size: 13px;"
        "  font-weight: 600;"
        "  background-color: #27AE60;"
        "  color: white;"
        "  border: none;"
        "  border-radius: 4px;"
        "  margin: 0px 4px;"
        "}"
        "QPushButton:hover {"
        "  background-color: #229954;"
        "}"
        "QPushButton:pressed {"
        "  background-color: #1E8449;"
        "}"
        "QPushButton:disabled {"
        "  background-color: #95A5A6;"
        "  color: #7F8C8D;"
        "}"
    );

    btnConfigIni = new QPushButton("📋  Add Config", ribbonBar);
    btnShowHatches = new QPushButton("👁️  Preview", ribbonBar);

    ribbonLayout->addWidget(btnSlice);
    ribbonLayout->addWidget(btnConfigIni);
    ribbonLayout->addWidget(btnShowHatches);

    // Separator
    QFrame *separator2 = new QFrame(ribbonBar);
    separator2->setFrameShape(QFrame::VLine);
    separator2->setStyleSheet("QFrame { color: #34495E; }");
    ribbonLayout->addWidget(separator2);

    // Settings section
    btnSettings = new QPushButton("⚙️  Settings", ribbonBar);
    ribbonLayout->addWidget(btnSettings);

    ribbonLayout->addStretch();

    mainLayout->addWidget(ribbonBar);
}

void MainWindow::createLeftSidebar(QWidget *centralWidget, QHBoxLayout *horizontalLayout)
{
    leftSidebar = new QWidget(centralWidget);
    leftSidebar->setStyleSheet(
        "QWidget {"
        "  background-color: #F5F6FA;"
        "  border-right: 1px solid #ECF0F1;"
        "}"
        "QLabel {"
        "  color: #2C3E50;"
        "  font-weight: 600;"
        "  font-size: 12px;"
        "  padding: 8px 12px;"
        "}"
        "QProgressBar {"
        "  border: 1px solid #BDC3C7;"
        "  background-color: #ECF0F1;"
        "  border-radius: 4px;"
        "  text-align: center;"
        "  color: #2C3E50;"
        "}"
        "QProgressBar::chunk {"
        "  background-color: #3498DB;"
        "  border-radius: 3px;"
        "}"
    );
    leftSidebar->setMinimumWidth(280);
    leftSidebar->setMaximumWidth(350);

    QVBoxLayout *leftSidebarLayout = new QVBoxLayout(leftSidebar);
    leftSidebarLayout->setContentsMargins(12, 12, 12, 12);
    leftSidebarLayout->setSpacing(12);

    // Models section header
    QLabel *modelsLabel = new QLabel("MODELS", leftSidebar);
    modelsLabel->setStyleSheet(
        "QLabel {"
        "  color: #2C3E50;"
        "  font-weight: 700;"
        "  font-size: 11px;"
        "  letter-spacing: 1px;"
        "  padding: 8px 0px;"
        "}"
    );
    leftSidebarLayout->addWidget(modelsLabel);

    // Model list area (placeholder for future expansion)
    QWidget *modelListContainer = new QWidget(leftSidebar);
    modelListContainer->setStyleSheet(
        "QWidget {"
        "  background-color: white;"
        "  border: 1px solid #ECF0F1;"
        "  border-radius: 4px;"
        "}"
    );
    QVBoxLayout *modelListLayout = new QVBoxLayout(modelListContainer);
    modelListLayout->setContentsMargins(8, 8, 8, 8);
    modelListLayout->setSpacing(0);

    QLabel *emptyModelsLabel = new QLabel("No models loaded", modelListContainer);
    emptyModelsLabel->setStyleSheet(
        "QLabel {"
        "  color: #95A5A6;"
        "  font-size: 12px;"
        "  padding: 40px 8px;"
        "  text-align: center;"
        "}"
    );
    emptyModelsLabel->setAlignment(Qt::AlignCenter);
    modelListLayout->addWidget(emptyModelsLabel);
    modelListLayout->addStretch();

    modelListContainer->setLayout(modelListLayout);
    leftSidebarLayout->addWidget(modelListContainer, 1);

    // Progress section
    QLabel *statusLabel = new QLabel("OPERATION STATUS", leftSidebar);
    statusLabel->setStyleSheet(
        "QLabel {"
        "  color: #2C3E50;"
        "  font-weight: 700;"
        "  font-size: 11px;"
        "  letter-spacing: 1px;"
        "  padding: 8px 0px;"
        "}"
    );
    leftSidebarLayout->addWidget(statusLabel);

    progressBar = new QProgressBar(leftSidebar);
    progressBar->setOrientation(Qt::Horizontal);
    progressBar->setMinimumHeight(24);
    progressBar->setRange(0, 0);  // Indeterminate mode
    progressBar->setVisible(false);
    progressBar->setTextVisible(true);
    progressBar->setStyleSheet(
        "QProgressBar {"
        "  border: 1px solid #BDC3C7;"
        "  background-color: #ECF0F1;"
        "  border-radius: 4px;"
        "  text-align: center;"
        "  color: #2C3E50;"
        "  font-size: 11px;"
        "  font-weight: 500;"
        "  padding: 2px;"
        "}"
        "QProgressBar::chunk {"
        "  background-color: qlineargradient(x1:0, y1:0, x2:1, y2:0,"
        "    stop:0 #3498DB, stop:1 #2980B9);"
        "  border-radius: 3px;"
        "}"
    );
    leftSidebarLayout->addWidget(progressBar);

    // Statistics section
    QLabel *statsLabel = new QLabel("STATISTICS", leftSidebar);
    statsLabel->setStyleSheet(
        "QLabel {"
        "  color: #2C3E50;"
        "  font-weight: 700;"
        "  font-size: 11px;"
        "  letter-spacing: 1px;"
        "  padding: 8px 0px;"
        "}"
    );
    leftSidebarLayout->addWidget(statsLabel);

    QWidget *statsContainer = new QWidget(leftSidebar);
    statsContainer->setStyleSheet(
        "QWidget {"
        "  background-color: white;"
        "  border: 1px solid #ECF0F1;"
        "  border-radius: 4px;"
        "}"
    );
    QGridLayout *statsLayout = new QGridLayout(statsContainer);
    statsLayout->setContentsMargins(8, 8, 8, 8);
    statsLayout->setSpacing(8);

    // Model count
    QLabel *countLbl = new QLabel("Models:", statsContainer);
    countLbl->setStyleSheet("QLabel { color: #7F8C8D; font-size: 11px; }");
    QLabel *countVal = new QLabel("0", statsContainer);
    countVal->setStyleSheet("QLabel { color: #2C3E50; font-weight: 600; font-size: 12px; }");
    statsLayout->addWidget(countLbl, 0, 0);
    statsLayout->addWidget(countVal, 0, 1, Qt::AlignRight);

    // Config status
    QLabel *configLbl = new QLabel("Config:", statsContainer);
    configLbl->setStyleSheet("QLabel { color: #7F8C8D; font-size: 11px; }");
    QLabel *configVal = new QLabel("Not Set", statsContainer);
    configVal->setStyleSheet("QLabel { color: #E74C3C; font-weight: 600; font-size: 11px; }");
    statsLayout->addWidget(configLbl, 1, 0);
    statsLayout->addWidget(configVal, 1, 1, Qt::AlignRight);

    statsContainer->setLayout(statsLayout);
    leftSidebarLayout->addWidget(statsContainer);

    leftSidebarLayout->addStretch();
    leftSidebar->setLayout(leftSidebarLayout);

    horizontalLayout->addWidget(leftSidebar, 0);
}

void MainWindow::createContentArea(QWidget *centralWidget, QHBoxLayout *horizontalLayout)
{
    contentArea = new QWidget(centralWidget);
    contentArea->setStyleSheet(
        "QWidget {"
        "  background-color: #FFFFFF;"
        "  border: none;"
        "}"
    );
    contentArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QVBoxLayout *contentLayout = new QVBoxLayout(contentArea);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(0);

    // Viewport header
    QWidget *viewportHeader = new QWidget(contentArea);
    viewportHeader->setStyleSheet(
        "QWidget {"
        "  background-color: #F5F6FA;"
        "  border-bottom: 1px solid #ECF0F1;"
        "}"
        "QLabel {"
        "  color: #2C3E50;"
        "  font-weight: 600;"
        "}"
    );
    viewportHeader->setFixedHeight(45);

    QHBoxLayout *headerLayout = new QHBoxLayout(viewportHeader);
    headerLayout->setContentsMargins(16, 0, 16, 0);
    headerLayout->setSpacing(12);

    QLabel *viewportIcon = new QLabel("🖼️", viewportHeader);
    viewportIcon->setStyleSheet("QLabel { font-size: 14px; }");
    viewportIcon->setFixedWidth(24);

    QLabel *viewportLabel = new QLabel("3D VIEWPORT", viewportHeader);
    viewportLabel->setStyleSheet(
        "QLabel {"
        "  color: #2C3E50;"
        "  font-weight: 600;"
        "  font-size: 12px;"
        "}"
    );

    headerLayout->addWidget(viewportIcon);
    headerLayout->addWidget(viewportLabel);
    
    // View mode buttons (placeholder for future expansion)
    QWidget *viewModeContainer = new QWidget(viewportHeader);
    QHBoxLayout *viewModeLayout = new QHBoxLayout(viewModeContainer);
    viewModeLayout->setContentsMargins(0, 0, 0, 0);
    viewModeLayout->setSpacing(4);

    QPushButton *perspectiveBtn = new QPushButton("P", viewModeContainer);
    perspectiveBtn->setFixedSize(28, 28);
    perspectiveBtn->setStyleSheet(
        "QPushButton {"
        "  background-color: #3498DB;"
        "  color: white;"
        "  border: none;"
        "  border-radius: 3px;"
        "  font-weight: 600;"
        "  font-size: 11px;"
        "}"
        "QPushButton:hover {"
        "  background-color: #2980B9;"
        "}"
    );
    perspectiveBtn->setToolTip("Perspective View");
    viewModeLayout->addWidget(perspectiveBtn);

    QPushButton *topBtn = new QPushButton("T", viewModeContainer);
    topBtn->setFixedSize(28, 28);
    topBtn->setStyleSheet(
        "QPushButton {"
        "  background-color: #ECF0F1;"
        "  color: #2C3E50;"
        "  border: 1px solid #BDC3C7;"
        "  border-radius: 3px;"
        "  font-weight: 600;"
        "  font-size: 11px;"
        "}"
        "QPushButton:hover {"
        "  background-color: #D5DBDB;"
        "}"
    );
    topBtn->setToolTip("Top View");
    viewModeLayout->addWidget(topBtn);

    viewModeContainer->setLayout(viewModeLayout);
    headerLayout->addWidget(viewModeContainer);

    headerLayout->addStretch();

    viewportHeader->setLayout(headerLayout);
    contentLayout->addWidget(viewportHeader);

    // Viewport area
    QWidget *viewportContainer = new QWidget(contentArea);
    viewportContainer->setStyleSheet(
        "QWidget {"
        "  background-color: #FFFFFF;"
        "  border: none;"
        "}"
    );

    viewerLayout = new QVBoxLayout(viewportContainer);
    viewerLayout->setContentsMargins(0, 0, 0, 0);
    viewerLayout->setSpacing(0);

    // Create STL viewer with border
    m_stlViewer = new StlViewer(this);
    m_stlViewer->setBackgroundColor(0.95, 0.95, 0.95);
    m_stlViewer->setStyleSheet(
        "QWidget {"
        "  border: none;"
        "  background-color: #FFFFFF;"
        "}"
    );

    viewerLayout->addWidget(m_stlViewer);
    viewportContainer->setLayout(viewerLayout);
    contentLayout->addWidget(viewportContainer, 1);

    // Status bar at bottom of viewport
    QWidget *statusBar = new QWidget(contentArea);
    statusBar->setStyleSheet(
        "QWidget {"
        "  background-color: #F5F6FA;"
        "  border-top: 1px solid #ECF0F1;"
        "}"
        "QLabel {"
        "  color: #7F8C8D;"
        "  font-size: 11px;"
        "}"
    );
    statusBar->setFixedHeight(28);

    QHBoxLayout *statusLayout = new QHBoxLayout(statusBar);
    statusLayout->setContentsMargins(16, 0, 16, 0);
    statusLayout->setSpacing(12);

    QLabel *viewInfoLabel = new QLabel("Ready | Press SPACE to reset view | Scroll to zoom", statusBar);
    viewInfoLabel->setStyleSheet("QLabel { color: #7F8C8D; font-size: 11px; }");
    statusLayout->addWidget(viewInfoLabel);
    statusLayout->addStretch();

    QLabel *coordLabel = new QLabel("X: 0.00 | Y: 0.00 | Z: 0.00", statusBar);
    coordLabel->setStyleSheet(
        "QLabel {"
        "  color: #2C3E50;"
        "  font-size: 11px;"
        "  font-family: 'Consolas', monospace;"
        "  padding: 0px 8px;"
        "}"
    );
    statusLayout->addWidget(coordLabel);

    statusBar->setLayout(statusLayout);
    contentLayout->addWidget(statusBar);

    contentArea->setLayout(contentLayout);
    horizontalLayout->addWidget(contentArea, 1);
}

void MainWindow::createRightSidebar(QWidget *centralWidget, QHBoxLayout *horizontalLayout)
{
    QWidget *rightSidebar = new QWidget(centralWidget);
    rightSidebar->setStyleSheet(
        "QWidget {"
        "  background-color: #F5F6FA;"
        "  border-left: 1px solid #ECF0F1;"
        "}"
        "QLabel {"
        "  color: #2C3E50;"
        "  font-weight: 700;"
        "  font-size: 11px;"
        "  letter-spacing: 1px;"
        "}"
    );
    rightSidebar->setMinimumWidth(300);
    rightSidebar->setMaximumWidth(450);

    QVBoxLayout *rightSidebarLayout = new QVBoxLayout(rightSidebar);
    rightSidebarLayout->setContentsMargins(12, 12, 12, 12);
    rightSidebarLayout->setSpacing(12);

    // Console header with icon
    QHBoxLayout *consoleHeaderLayout = new QHBoxLayout();
    consoleHeaderLayout->setContentsMargins(0, 0, 0, 0);
    consoleHeaderLayout->setSpacing(8);

    QLabel *consoleIcon = new QLabel("📋", rightSidebar);
    consoleIcon->setStyleSheet("QLabel { font-size: 14px; }");
    consoleIcon->setFixedWidth(24);

    QLabel *consoleLabel = new QLabel("OPERATION LOG", rightSidebar);
    consoleLabel->setStyleSheet(
        "QLabel {"
        "  color: #2C3E50;"
        "  font-weight: 700;"
        "  font-size: 11px;"
        "  letter-spacing: 1px;"
        "}"
    );

    consoleHeaderLayout->addWidget(consoleIcon);
    consoleHeaderLayout->addWidget(consoleLabel);
    consoleHeaderLayout->addStretch();

    rightSidebarLayout->addLayout(consoleHeaderLayout);

    // Console/Log area
    QWidget *consoleContainer = new QWidget(rightSidebar);
    consoleContainer->setStyleSheet(
        "QWidget {"
        "  background-color: #1E1E1E;"
        "  border: 1px solid #ECF0F1;"
        "  border-radius: 4px;"
        "}"
    );
    QVBoxLayout *consoleLayout = new QVBoxLayout(consoleContainer);
    consoleLayout->setContentsMargins(0, 0, 0, 0);
    consoleLayout->setSpacing(0);

    // Log text edit
    textEdit = new QTextEdit(consoleContainer);
    textEdit->setReadOnly(true);
    textEdit->setStyleSheet(
        "QTextEdit {"
        "  background-color: #1E1E1E;"
        "  color: #D4D4D4;"
        "  border: none;"
        "  font-family: 'Consolas', 'Monaco', 'Courier New', monospace;"
        "  font-size: 11px;"
        "  padding: 8px;"
        "  margin: 0px;"
        "}"
        "QScrollBar:vertical {"
        "  width: 10px;"
        "  background-color: #252526;"
        "  border: none;"
        "}"
        "QScrollBar::handle:vertical {"
        "  background-color: #464647;"
        "  border-radius: 5px;"
        "  min-height: 20px;"
        "}"
        "QScrollBar::handle:vertical:hover {"
        "  background-color: #5A5A5A;"
        "}"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {"
        "  height: 0px;"
        "}"
    );

    // Set welcome message
    textEdit->setText("");
    textEdit->append("<span style='color: #4EC9B0;'>█</span> <span style='color: #9CDCFE;'>MarcSLM Console Ready</span>");
    textEdit->append("<span style='color: #858585;'>──────────────────────────────</span>");
    textEdit->append("");

    consoleLayout->addWidget(textEdit);
    consoleContainer->setLayout(consoleLayout);
    rightSidebarLayout->addWidget(consoleContainer, 1);

    // Command buttons section
    QLabel *actionsLabel = new QLabel("QUICK ACTIONS", rightSidebar);
    actionsLabel->setStyleSheet(
        "QLabel {"
        "  color: #2C3E50;"
        "  font-weight: 700;"
        "  font-size: 11px;"
        "  letter-spacing: 1px;"
        "  padding: 8px 0px;"
        "}"
    );
    rightSidebarLayout->addWidget(actionsLabel);

    QWidget *actionsContainer = new QWidget(rightSidebar);
    actionsContainer->setStyleSheet(
        "QWidget {"
        "  background-color: white;"
        "  border: 1px solid #ECF0F1;"
        "  border-radius: 4px;"
        "}"
    );
    QVBoxLayout *actionsLayout = new QVBoxLayout(actionsContainer);
    actionsLayout->setContentsMargins(8, 8, 8, 8);
    actionsLayout->setSpacing(6);

    QPushButton *clearLogBtn = new QPushButton("Clear Log", actionsContainer);
    clearLogBtn->setStyleSheet(
        "QPushButton {"
        "  padding: 6px 12px;"
        "  font-size: 11px;"
        "  background-color: #ECF0F1;"
        "  color: #2C3E50;"
        "  border: 1px solid #BDC3C7;"
        "  border-radius: 3px;"
        "}"
        "QPushButton:hover {"
        "  background-color: #D5DBDB;"
        "}"
        "QPushButton:pressed {"
        "  background-color: #BDC3C7;"
        "}"
    );
    connect(clearLogBtn, &QPushButton::clicked, textEdit, &QTextEdit::clear);

    actionsLayout->addWidget(clearLogBtn);
    actionsContainer->setLayout(actionsLayout);
    rightSidebarLayout->addWidget(actionsContainer);

    rightSidebar->setLayout(rightSidebarLayout);
    horizontalLayout->addWidget(rightSidebar, 0);
}

void MainWindow::connectSignalsAndSlots()
{
    // Connect ribbon bar buttons
    connect(btnMenu, &QPushButton::clicked, this, &MainWindow::onToggleSidebarClicked);
    connect(btnSettings, &QPushButton::clicked, this, &MainWindow::onSettingsButtonClicked);
    connect(btnConfigIni, &QPushButton::clicked, this, &MainWindow::onAddConfigsButtonClicked);
    connect(btnShowHatches, &QPushButton::clicked, this, &MainWindow::onShowHatchesButtonClicked);

    // Connect slice button - NOW PROPERLY ACCESSIBLE
    if (btnSlice) {
        connect(btnSlice, &QPushButton::clicked, this, &MainWindow::onSliceButtonClicked);
    }

    // STL viewer signals
    if (m_stlViewer) {
        connect(m_stlViewer, &StlViewer::logMessage, this, &MainWindow::appendLogMessage);
    }
}

void MainWindow::applyStylesheets()
{
    // Apply consistent styling to all dialogs and windows
    this->setStyleSheet(
        "QMainWindow {"
        "  background-color: #FFFFFF;"
        "}"
        "QMessageBox, QFileDialog {"
        "  background-color: #FFFFFF;"
        "}"
        "QMessageBox QLabel, QFileDialog QLabel {"
        "  color: #2C3E50;"
        "}"
        "QPushButton {"
        "  padding: 6px 12px;"
        "  border-radius: 4px;"
        "  font-weight: 500;"
        "}"
    );
}

// ============================================================================
// Sidebar and Button Management
// ============================================================================

void MainWindow::onToggleSidebarClicked()
{
    animateSidebarToggle();
}

void MainWindow::animateSidebarToggle()
{
    if (!leftSidebar || !btnSettings) {
        return;
    }

    int startWidth = isSidebarCollapsed ? 50 : 250;
    int endWidth = isSidebarCollapsed ? 250 : 50;

    // Width animation
    QPropertyAnimation *widthAnimation = new QPropertyAnimation(leftSidebar, "minimumWidth", this);
    widthAnimation->setDuration(300);
    widthAnimation->setStartValue(startWidth);
    widthAnimation->setEndValue(endWidth);
    widthAnimation->setEasingCurve(QEasingCurve::InOutQuad);

    // Opacity animation
    auto *opacityEffect = new QGraphicsOpacityEffect(leftSidebar);
    leftSidebar->setGraphicsEffect(opacityEffect);
    QPropertyAnimation *opacityAnimation = new QPropertyAnimation(opacityEffect, "opacity", this);
    opacityAnimation->setDuration(300);
    opacityAnimation->setStartValue(isSidebarCollapsed ? 0.5 : 1.0);
    opacityAnimation->setEndValue(isSidebarCollapsed ? 1.0 : 0.5);

    widthAnimation->start(QAbstractAnimation::DeleteWhenStopped);
    opacityAnimation->start(QAbstractAnimation::DeleteWhenStopped);

    // Toggle state
    isSidebarCollapsed = !isSidebarCollapsed;

    // Update button text
    if (isSidebarCollapsed) {
        btnSettings->setText("⚙️");
    } else {
        btnSettings->setText("⚙️ Settings");
    }
}

void MainWindow::activateButton(QPushButton *button)
{
    if (!button) {
        return;
    }

    // Deactivate previous button
    if (currentActiveButton) {
        currentActiveButton->setStyleSheet(
            "text-align: left; padding: 10px; font-size: 14px; "
            "background-color: transparent; color: white; border: none;"
        );
    }

    // Activate current button
    button->setStyleSheet(
        "text-align: left; padding: 10px; font-size: 14px; "
        "background-color: #4C566A; color: white; border: none;"
    );
    currentActiveButton = button;
}

// ============================================================================
// Button Click Handlers
// ============================================================================

void MainWindow::onAddConfigsButtonClicked()
{
    onLoadBuildConfigurationRequested();
}

void MainWindow::onShowHatchesButtonClicked()
{
    onLayerViewerRequested();
}

void MainWindow::onSliceButtonClicked()
{
    if (!btnSlice) {
        return;
    }

    btnSlice->setEnabled(false);
    if (progressBar) {
        progressBar->setVisible(true);
    }

    // Run slicing in background thread using QThread
    QThread *workerThread = QThread::create([this]() {
        this->performSliceViaDLL();
    });

    // Connect thread finished signal for cleanup
    connect(workerThread, &QThread::finished, this, [this, workerThread]() {
        if (btnSlice) {
            btnSlice->setEnabled(true);
        }
        if (progressBar) {
            progressBar->setVisible(false);
        }
        workerThread->deleteLater();
    });

    // Start the worker thread
    workerThread->start();
}

void MainWindow::onSettingsButtonClicked()
{
    if (!configDialog) {
        configDialog = new ConfigDialog(this);
    }
    configDialog->show();
}

// ============================================================================
// File Operations and Dialog Handlers
// ============================================================================

void MainWindow::onLoadSTLModelsRequested()
{
    QStringList fileNames = QFileDialog::getOpenFileNames(
        this,
        tr("Open STL Models"),
        "",
        tr("STL Files (*.stl);;All Files (*)")
    );

    if (fileNames.isEmpty()) {
        appendLogMessage("-No STL models selected.");
        return;
    }

    for (const QString &fileName : fileNames) {
        QFileInfo fileInfo(fileName);
        appendLogMessage("-Selected STL Model: " + fileInfo.fileName());

        modelFilePaths.emplace_back(fileName.toStdString());
        modelCount++;

        if (modelCount >= 12) {
            appendLogMessage("-Maximum of 12 models supported. Ignoring extra files.");
            break;
        }
    }

    appendLogMessage("-Number of STL Models: " + QString::number(modelCount));
    displaySTLModel();
}

void MainWindow::onLoadBuildConfigurationRequested()
{
    QString fileName = QFileDialog::getOpenFileName(
        this,
        tr("Select Build Config JSON"),
        "",
        tr("JSON Files (*.json);;All Files (*)")
    );

    if (fileName.isEmpty()) {
        appendLogMessage("-No JSON configuration file selected.");
        return;
    }

    QFileInfo fileInfo(fileName);
    appendLogMessage("-Selected Config File: " + fileInfo.fileName());
    buildConfigFilePath = std::filesystem::path(fileName.toStdString());
    buildStylesFilePath = std::filesystem::path(fileName.toStdString());  // Mask styles path
}

void MainWindow::onLoadBuildStyleRequested()
{
    QString fileName = QFileDialog::getOpenFileName(
        this,
        tr("Select Build Style JSON"),
        "",
        tr("JSON Files (*.json);;All Files (*)")
    );

    if (fileName.isEmpty()) {
        appendLogMessage("-No JSON configuration file selected.");
        return;
    }

    QFileInfo fileInfo(fileName);
    appendLogMessage("-Selected Config File: " + fileInfo.fileName());
    buildStylesFilePath = std::filesystem::path(fileName.toStdString());
}

void MainWindow::onLayerViewerRequested()
{
    LayerViewer *viewer = new LayerViewer(this);
    viewer->setAttribute(Qt::WA_DeleteOnClose);
    viewer->setWindowFlag(Qt::Window);
    viewer->show();
}

// ============================================================================
// File Path Validation
// ============================================================================

bool MainWindow::validateFilePaths(const std::vector<std::filesystem::path> &paths)
{
    if (paths.empty()) {
        return false;
    }

    for (const auto &path : paths) {
        if (path.empty()) {
            return false;
        }

        if (!std::filesystem::exists(path)) {
            appendLogMessage("-Error: File does not exist: " + QString::fromStdString(path.string()));
            return false;
        }

        if (!std::filesystem::is_regular_file(path)) {
            appendLogMessage("-Error: Path is not a regular file: " + QString::fromStdString(path.string()));
            return false;
        }
    }

    return true;
}

void MainWindow::printFilePathsToLog(const std::vector<std::filesystem::path> &paths)
{
    for (const auto &path : paths) {
        QString qPath = QString::fromStdString(path.string());
        appendLogMessage(qPath);
    }
}

// ============================================================================
// SVG Layer Viewer
// ============================================================================

void MainWindow::loadSvgIntoScene(const QString &filePath)
{
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

void MainWindow::onLayerSliderValueChanged(int layerNumber)
{
    // This slot is connected to a slider in the layer viewer
    // Implementation in displayLayerViewerWindow
}

void MainWindow::loadLayerIntoScene(QGraphicsScene *scene, int layerNumber)
{
    if (!scene) {
        return;
    }

    // Remove old item
    if (svgItem) {
        scene->removeItem(svgItem);
        delete svgItem;
        svgItem = nullptr;
    }

    // Construct file path
    QString filePath = QString(":/images/Layer%1.svg").arg(layerNumber);

    // Verify file exists
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists()) {
        QMessageBox::warning(this, "File Not Found", "The selected layer file does not exist.");
        return;
    }

    // Load SVG item
    svgItem = new QGraphicsSvgItem(filePath);
    svgItem->setFlags(QGraphicsItem::ItemClipsToShape);
    svgItem->setCacheMode(QGraphicsItem::NoCache);
    svgItem->setZValue(0);

    scene->addItem(svgItem);
    scene->setSceneRect(svgItem->boundingRect());
}

void MainWindow::displayLayerViewerWindow()
{
    QWidget *layerWindow = new QWidget();
    layerWindow->setWindowTitle("SVG Layer Viewer");
    layerWindow->resize(800, 600);

    QVBoxLayout *mainLayout = new QVBoxLayout(layerWindow);

    // Create graphics view and scene
    QGraphicsScene *layerScene = new QGraphicsScene(layerWindow);
    QGraphicsView *view = new QGraphicsView(layerScene, layerWindow);
    mainLayout->addWidget(view);

    // Create slider for layer selection
    layerSlider = new QSlider(Qt::Horizontal, layerWindow);
    layerSlider->setRange(1, 500);
    layerSlider->setValue(1);
    mainLayout->addWidget(layerSlider);

    // Connect slider to layer loading
    connect(layerSlider, &QSlider::valueChanged, this, [this, layerScene](int value) {
        loadLayerIntoScene(layerScene, value);
    });

    // Load initial layer
    loadLayerIntoScene(layerScene, layerSlider->value());

    // Show window
    layerWindow->show();
}

// ============================================================================
// Model Display
// ============================================================================

void MainWindow::displaySTLModel()
{
    if (modelFilePaths.empty()) {
        QMessageBox::warning(this, "Error", "No model path specified.");
        return;
    }

    if (m_stlViewer) {
        QVector<int> deletedModels = m_stlViewer->getdeletedmodels();
        m_stlViewer->clearBuildPlate();
    }

    // Arrange models on platter
    if (m_stlViewer) {
        m_stlViewer->arrangeModelsOnPlatter();
        if (viewerLayout) {
            viewerLayout->addWidget(m_stlViewer);
        }
        m_stlViewer->setBackgroundColor(0.95, 0.95, 0.95);
    }
}

void MainWindow::appendLogMessage(const QString &message)
{
    if (textEdit) {
        textEdit->append(message);
    }
}

void MainWindow::orientationOptimization()
{
    // TODO: Implement orientation optimization
    // This is a placeholder for future orientation optimization logic
}

// ============================================================================
// Slicing Operations via DLL
// ============================================================================

void MainWindow::performSliceViaDLL()
{
    // Log start
    QMetaObject::invokeMethod(this, [this]() {
        if (textEdit) {
            textEdit->clear();
            textEdit->append("-Starting slicing operation...");
        }
    }, Qt::QueuedConnection);

    // Prepare models
    if (!prepareModelsForDLL()) {
        QMetaObject::invokeMethod(this, [this]() {
            if (textEdit) {
                textEdit->append("-Preparation failed: invalid models/configs");
            }
        }, Qt::QueuedConnection);
        return;
    }

    // Validate data
    if (m_guiDataArrayForDll.count == 0 || m_guiDataArrayForDll.models == nullptr) {
        QMetaObject::invokeMethod(this, [this]() {
            if (textEdit) {
                textEdit->append("-No models to slice");
            }
        }, Qt::QueuedConnection);
        return;
    }

    // Create MARC API handle
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
        executeSlicingOperation(handle);
    }
    catch (const std::exception &e) {
        QMetaObject::invokeMethod(this, [this, e]() {
            if (textEdit) {
                textEdit->append(QString("-Exception occurred: %1").arg(e.what()));
            }
        }, Qt::QueuedConnection);
    }
    catch (...) {
        QMetaObject::invokeMethod(this, [this]() {
            if (textEdit) {
                textEdit->append("-Unknown error occurred during slicing operation.");
            }
        }, Qt::QueuedConnection);
    }

    // Cleanup
    destroy_marc_api(handle);
}

void MainWindow::executeSlicingOperation(MarcHandle handle)
{
    // Step 1: Send models to API
    MarcErrorCode err = set_models(handle, m_guiDataArrayForDll);
    QMetaObject::invokeMethod(this, [this, err]() {
        if (textEdit) {
            textEdit->append(err == MARC_S_OK ? "-Models loading succeeded" : "-Models loading failed");
        }
    }, Qt::QueuedConnection);

    if (err != MARC_S_OK) {
        return;
    }

    // Step 2: Load configuration JSON
    const std::string stylesPath = buildStylesFilePath.string();
    if (!stylesPath.empty()) {
        err = set_config_json(handle, stylesPath.c_str());
        QMetaObject::invokeMethod(this, [this, err]() {
            if (textEdit) {
                textEdit->append(err == MARC_S_OK ? "-Config JSON loading succeeded" 
                                                   : "-Config JSON loading failed");
            }
        }, Qt::QueuedConnection);

        if (err != MARC_S_OK) {
            return;
        }
    }

    // Step 3: Update model
    err = update_model(handle);
    QMetaObject::invokeMethod(this, [this, err]() {
        if (textEdit) {
            textEdit->append(err == MARC_S_OK ? "-Model updating succeeded" : "-Model updating failed");
        }
    }, Qt::QueuedConnection);

    if (err != MARC_S_OK) {
        return;
    }

    // Step 4: Export SLM file
    err = export_slm_file(handle);
    QMetaObject::invokeMethod(this, [this, err]() {
        if (textEdit) {
            textEdit->append(err == MARC_S_OK ? "-Model sliced and exported successfully" 
                                              : "-Model slicing failed");
            textEdit->append("-Operation completed.");
        }
    }, Qt::QueuedConnection);

    // Step 5: Cleanup local state
    cleanupGuiDataStructures();
    modelFilePaths.clear();
    configFileCount = 0;
    modelCount = 0;
}

void MainWindow::performSliceViaDLLLegacy()
{
    // Legacy implementation - kept for backward compatibility
    if (!validateFilePaths(modelFilePaths)) {
        QMetaObject::invokeMethod(this, [this]() {
            QMessageBox::information(this, "Invalid Paths", "Ensure all model/config paths are valid.");
        }, Qt::QueuedConnection);
        return;
    }

    QMetaObject::invokeMethod(this, [this]() {
        if (textEdit) {
            textEdit->clear();
        }
    }, Qt::QueuedConnection);

    MarcHandle handle = create_marc_api(200.0f, 200.0f, 5.0f);
    if (!handle) {
        QMetaObject::invokeMethod(this, [this]() {
            if (textEdit) {
                textEdit->append("-Failed to create MARC API handle.");
            }
        }, Qt::QueuedConnection);
        return;
    }

    try {
        // Convert data
        m_guiDataArrayForDll = toDllFromDll::toDll(m_internalGuiDataArray);

        // Execute operation
        MarcErrorCode err = set_models(handle, m_guiDataArrayForDll);
        QMetaObject::invokeMethod(this, [this, err]() {
            if (textEdit) {
                textEdit->append(err == MARC_S_OK ? "-Models loading succeeded" : "-Models loading failed");
            }
        }, Qt::QueuedConnection);

        if (err != MARC_S_OK) {
            return;
        }

        // Load configuration
        err = set_config_json(handle, buildStylesFilePath.string().c_str());
        QMetaObject::invokeMethod(this, [this, err]() {
            if (textEdit) {
                textEdit->append(err == MARC_S_OK ? "-Config JSON loading succeeded" 
                                                   : "-Config JSON loading failed");
            }
        }, Qt::QueuedConnection);

        if (err != MARC_S_OK) {
            return;
        }

        // Update and export
        err = update_model(handle);
        QMetaObject::invokeMethod(this, [this, err]() {
            if (textEdit) {
                textEdit->append(err == MARC_S_OK ? "-Model updating succeeded" : "-Model updating failed");
            }
        }, Qt::QueuedConnection);

        if (err != MARC_S_OK) {
            return;
        }

        err = export_slm_file(handle);
        QMetaObject::invokeMethod(this, [this, err]() {
            if (textEdit) {
                textEdit->append(err == MARC_S_OK ? "-Model sliced and exported successfully" 
                                                   : "-Model slicing failed");
            }
        }, Qt::QueuedConnection);

        // Cleanup
        modelFilePaths.clear();
        configFileCount = 0;
        modelCount = 0;
    }
    catch (const std::exception &e) {
        QMetaObject::invokeMethod(this, [this, e]() {
            if (textEdit) {
                textEdit->append(QString("-Exception occurred: %1").arg(e.what()));
            }
        }, Qt::QueuedConnection);
    }
    catch (...) {
        QMetaObject::invokeMethod(this, [this]() {
            if (textEdit) {
                textEdit->append("-Unknown error occurred.");
            }
        }, Qt::QueuedConnection);
    }

    destroy_marc_api(handle);
}

// ============================================================================
// Model Preparation and DLL Data Conversion
// ============================================================================

bool MainWindow::prepareModelsForDLL()
{
    // Validate build configuration file
    if (buildConfigFilePath.empty() || !std::filesystem::exists(buildConfigFilePath)) {
        QMetaObject::invokeMethod(this, [this]() {
            QMessageBox::warning(this, "Invalid Config File",
                                 "Please select a valid build configuration JSON file.");
        }, Qt::QueuedConnection);
        return false;
    }

    QMetaObject::invokeMethod(this, [this]() {
        if (textEdit) {
            textEdit->append("-File Path Validated!");
        }
    }, Qt::QueuedConnection);

    // Get models from STL viewer
    m_internalGuiDataArray.clear();
    m_internalGuiDataArray = m_stlViewer ? m_stlViewer->getModels() 
                                         : std::vector<InternalGuiModel>{};

    // Assign config paths to each model
    if (!m_internalGuiDataArray.empty()) {
        for (auto &model : m_internalGuiDataArray) {
            model.buildconfig = buildConfigFilePath.string();
            model.stylesconfig = buildStylesFilePath.string();
        }
    }

    // Validate we have models
    if (m_internalGuiDataArray.empty()) {
        QMetaObject::invokeMethod(this, [this]() {
            QMessageBox::warning(this, "No Valid Models",
                                 "No valid models available to process.");
        }, Qt::QueuedConnection);
        return false;
    }

    printInternalModelArrayDebugInfo();

    // Convert to DLL format
    try {
        m_guiDataArrayForDll = toDllFromDll::toDll(m_internalGuiDataArray);

        // Verify conversion was successful
        if (!m_guiDataArrayForDll.models || m_guiDataArrayForDll.count == 0) {
            QMetaObject::invokeMethod(this, [this]() {
                if (textEdit) {
                    textEdit->append("-Error: Invalid model data structure.");
                }
            }, Qt::QueuedConnection);
            return false;
        }

        // Validate each model
        for (size_t i = 0; i < m_guiDataArrayForDll.count; i++) {
            const GuiData &model = m_guiDataArrayForDll.models[i];

            // Check model path
            if (model.path[0] == '\0') {
                QMetaObject::invokeMethod(this, [this, i]() {
                    if (textEdit) {
                        textEdit->append(QString("-Error: Model %1 has empty path.").arg(i));
                    }
                }, Qt::QueuedConnection);
                return false;
            }

            std::string modelPath = model.path;
            if (!std::filesystem::exists(modelPath)) {
                QMetaObject::invokeMethod(this, [this, modelPath]() {
                    if (textEdit) {
                        textEdit->append(QString("-Error: Model file does not exist: %1")
                            .arg(QString::fromStdString(modelPath)));
                    }
                }, Qt::QueuedConnection);
                return false;
            }

            // Check build config
            if (model.buildconfig[0] != '\0') {
                std::string configPath = model.buildconfig;
                if (!std::filesystem::exists(configPath)) {
                    QMetaObject::invokeMethod(this, [this, configPath]() {
                        if (textEdit) {
                            textEdit->append(QString("-Error: Build config file does not exist: %1")
                                .arg(QString::fromStdString(configPath)));
                        }
                    }, Qt::QueuedConnection);
                    return false;
                }
            }

            // Check styles config
            if (model.stylesconfig[0] != '\0') {
                std::string stylesPath = model.stylesconfig;
                if (!std::filesystem::exists(stylesPath)) {
                    QMetaObject::invokeMethod(this, [this, stylesPath]() {
                        if (textEdit) {
                            textEdit->append(QString("-Error: Styles config file does not exist: %1")
                                .arg(QString::fromStdString(stylesPath)));
                        }
                    }, Qt::QueuedConnection);
                    return false;
                }
            }
        }

        // Log success
        QMetaObject::invokeMethod(this, [this]() {
            if (textEdit) {
                textEdit->append(QString("-Processing %1 models...").arg(m_guiDataArrayForDll.count));
            }
        }, Qt::QueuedConnection);

        printGuiDataArrayDebugInfo();
        return true;
    }
    catch (const std::exception &e) {
        QMetaObject::invokeMethod(this, [this, e]() {
            if (textEdit) {
                textEdit->append(QString("-Exception occurred: %1").arg(e.what()));
            }
        }, Qt::QueuedConnection);
        return false;
    }
    catch (...) {
        QMetaObject::invokeMethod(this, [this]() {
            if (textEdit) {
                textEdit->append("-Unknown error occurred during model preparation.");
            }
        }, Qt::QueuedConnection);
        return false;
    }
}

void MainWindow::sendDataToDLLFormat()
{
    cleanupGuiDataStructures();
    m_guiDataArrayForDll = toDllFromDll::toDll(m_internalGuiDataArray);
}

void MainWindow::receiveDataFromDLLFormat()
{
    m_internalGuiDataArray = toDllFromDll::fromDll(m_guiDataArrayForDll);
    toDllFromDll::freeGuiDataArray(m_guiDataArrayForDll);
}

void MainWindow::cleanupGuiDataStructures()
{
    if (m_guiDataArrayForDll.models != nullptr && m_guiDataArrayForDll.count > 0) {
        QMetaObject::invokeMethod(this, [this]() {
            if (textEdit) {
                textEdit->append("-Freeing model array with " +
                    QString::number(m_guiDataArrayForDll.count) + " models");
            }
        }, Qt::QueuedConnection);

        delete[] m_guiDataArrayForDll.models;
        m_guiDataArrayForDll.models = nullptr;
        m_guiDataArrayForDll.count = 0;
    }
    else {
        m_guiDataArrayForDll.models = nullptr;
        m_guiDataArrayForDll.count = 0;
    }
}

// ============================================================================
// Debug and Logging
// ============================================================================

void MainWindow::printGuiDataArrayDebugInfo()
{
    qDebug() << "-------- GUI Data Array Contents --------";

    if (!m_guiDataArrayForDll.models || m_guiDataArrayForDll.count == 0) {
        qDebug() << "-No models in data array";
        qDebug() << "------------------------------------";
        return;
    }

    qDebug() << "-Total models:" << m_guiDataArrayForDll.count;

    for (size_t i = 0; i < m_guiDataArrayForDll.count; i++) {
        const GuiData &model = m_guiDataArrayForDll.models[i];

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
        } else {
            qDebug() << "  Build config: <none>";
        }

        if (model.stylesconfig[0] != '\0') {
            qDebug() << "  Styles config:" << model.stylesconfig;
        } else {
            qDebug() << "  Styles config: <none>";
        }
    }

    qDebug() << "\n------------------------------------";
}

void MainWindow::printInternalModelArrayDebugInfo()
{
    qDebug() << "-------- Internal Model Array Debug Output --------";

    if (m_internalGuiDataArray.empty()) {
        qDebug() << "No models in internal array";
        qDebug() << "------------------------------------";
        return;
    }

    qDebug() << "Total internal models:" << m_internalGuiDataArray.size();

    for (size_t i = 0; i < m_internalGuiDataArray.size(); i++) {
        const InternalGuiModel &model = m_internalGuiDataArray[i];

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
        } else {
            qDebug() << "  Build config: <none>";
        }

        if (!model.stylesconfig.empty()) {
            qDebug() << "  Styles config:" << QString::fromStdString(model.stylesconfig);
        } else {
            qDebug() << "  Styles config: <none>";
        }
    }

    qDebug() << "------------------------------------";
}