#ifndef LAYERVIEWER_H
#define LAYERVIEWER_H

#include <QWidget>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsSvgItem>
#include <QSlider>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFileDialog>
#include <QDir>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QSettings>
#include "SvgLoaderWorker.h"
#include <QThread>
#include <QMessageBox>
// layer viewer widget
class LayerViewer : public QWidget
{
    Q_OBJECT

public:
    explicit LayerViewer(QWidget *parent = nullptr);

private slots:
    void loadFolder();
    void updateLayer(int value);
    void toggle3DView();
    void animate3DTransition();
    void applyStyleSheet();
    void chooseDirectory();
    void zoomImage(int value);
    void handleSvgItemLoaded(QGraphicsSvgItem* item);

private:
    void loadSVG(int layerNumber);
    void show2D();
    void show3D();
    void clearScene();
    void updateLayers();
    void setupUI();
    void ApplyScreenLayout();
    void setupZoomSlider();
    void cancelLoading();
    void onSvgLayerLoaded();
   

    QGraphicsView *graphicsView;
    QGraphicsScene *scene;
    QSlider *layerSlider;
    QPushButton *folderButton;
    QPushButton *toggleViewButton;
    QPushButton *animateViewButton;
    QPushButton *btnHome;
    QPushButton *btnSettings;
    QPushButton *btnStlModels;
    QPushButton *btnConfigini;
    QPushButton *btnSvgimages;
    QLabel *layerLabel;
    QWidget *sidebar;
    QPushButton *btnMenu;

    QString folderPath;
    bool is3DView = false;
    QStringList svgFiles;
    QSlider *zoomSlider;  // Declare zoomSlider here
    //QString directoryPath;  // <- This is where the folder path is stored
    int loadedItemCount = 0;   // Number of SVG items loaded
    int totalItemCount = 0;    // Total number of SVG items to load
    QThread* loaderThread = nullptr;  // Pointer to the loader thread
    QLabel* loadingLabel;
};

#endif // LAYERVIEWER_H
