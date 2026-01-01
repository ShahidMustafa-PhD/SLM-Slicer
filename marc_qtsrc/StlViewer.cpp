#include "StlViewer.h"

#include <QVBoxLayout>
#include <QMimeData>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QFileInfo>
#include <QDebug>

#include <vtkSTLReader.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkProperty.h>
#include <vtkCamera.h>


StlViewer::StlViewer(QWidget* parent)
    : QVTKOpenGLNativeWidget(parent)  // ✅ Now valid
{
    setAcceptDrops(true);

    vtkWidget = new QVTKOpenGLNativeWidget(this);
    vtkWidget->setMinimumSize(400, 400);

    auto layout = new QVBoxLayout(this);
    layout->addWidget(vtkWidget);
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);

    renderer = vtkSmartPointer<vtkRenderer>::New();
    renderer->SetBackground(0.95, 0.96, 0.97);  // Light background
    renderer->AutomaticLightCreationOn();

    vtkNew<vtkGenericOpenGLRenderWindow> renderWindow;
    renderWindow->AddRenderer(renderer);
    vtkWidget->setRenderWindow(renderWindow);

    // 🟡 Interactor + Picker + Custom Interactor Style
    vtkRenderWindowInteractor* interactor = vtkWidget->interactor();

    picker = vtkSmartPointer<vtkCellPicker>::New();
    picker->SetTolerance(0.001);  // For precise selection
    interactor->SetPicker(picker);

    // 🟢 Use your CustomInteractorStyle here
    interactorStyle = vtkSmartPointer<CustomInteractorStyle>::New();
    interactorStyle->SetDefaultRenderer(renderer);
    interactor->SetInteractorStyle(interactorStyle);



    //----------------------------------------------------------------------------------------------
    vtkSmartPointer<vtkAxesActor> axes = vtkSmartPointer<vtkAxesActor>::New();
    axes->SetTotalLength(1.0, 1.0, 1.0);
    axes->SetAxisLabels(true);

    // Set font size
    axes->GetXAxisCaptionActor2D()->GetTextActor()->GetTextProperty()->SetFontSize(10);
    axes->GetYAxisCaptionActor2D()->GetTextActor()->GetTextProperty()->SetFontSize(10);
    axes->GetZAxisCaptionActor2D()->GetTextActor()->GetTextProperty()->SetFontSize(10);

    // Set text color to white to avoid yellow tint
    axes->GetXAxisCaptionActor2D()->GetCaptionTextProperty()->SetColor(1, 1, 1);
    axes->GetYAxisCaptionActor2D()->GetCaptionTextProperty()->SetColor(1, 1, 1);
    axes->GetZAxisCaptionActor2D()->GetCaptionTextProperty()->SetColor(1, 1, 1);

    axes->GetXAxisCaptionActor2D()->GetTextActor()->GetTextProperty()->SetColor(1, 1, 1);
    axes->GetYAxisCaptionActor2D()->GetTextActor()->GetTextProperty()->SetColor(1, 1, 1);
    axes->GetZAxisCaptionActor2D()->GetTextActor()->GetTextProperty()->SetColor(1, 1, 1);

    // Set axes line colors (optional)
    axes->GetXAxisShaftProperty()->SetColor(1, 0, 0);
    axes->GetYAxisShaftProperty()->SetColor(0, 1, 0);
    axes->GetZAxisShaftProperty()->SetColor(0, 0, 1);

    // Create the widget as usual
    axesWidget = vtkSmartPointer<vtkOrientationMarkerWidget>::New();
    axesWidget->SetOrientationMarker(axes);
    axesWidget->SetInteractor(vtkWidget->interactor());
    axesWidget->SetViewport(0.75, 0.0, 1.0, 0.25);
    axesWidget->SetEnabled(1);
    axesWidget->InteractiveOff();

    //--------------------------------------------------------------------------------------------------




    renderer->ResetCamera();
    vtkWidget->renderWindow()->Render();
    // Step 5: Add ground plate
    addGroundPlate();  // 🔷 <--- Right here
    // Optional: Background color
    //renderer->SetBackground(0, 0.5, 0.3);  // Dark background

    renderer->SetBackground(0, 0.937, 0);  // Light bluish-gray (#ECEFF4)
    QString stlviewerButtonStyle =
        "QPushButton {"
        " padding: 5px 15px;"
        " font-size: 18px;"
        " background-color: transparent;"
        " color: black;"  // <-- Ensures white text
        " border: none;"
        "}"
        "QPushButton:hover {"
        " background-color: #4C566A;"
        "}";

  

    rotateXBtn = new QPushButton("RotX", this);
    rotateYBtn = new QPushButton("RotY", this);
    rotateZBtn = new QPushButton("RotZ", this);
    arrangeBtn = new QPushButton("Arrange", this);
    pn_toggleButton = new QPushButton("Rot Dir", this);
    add_ModelBtn = new QPushButton("Add Model", this);

    pn_toggleButton->setCheckable(true);  // Makes it a toggle button


    rotateXBtn->setStyleSheet(stlviewerButtonStyle);

    rotateYBtn->setStyleSheet(stlviewerButtonStyle);

    rotateZBtn->setStyleSheet(stlviewerButtonStyle);
    
    add_ModelBtn->setStyleSheet(stlviewerButtonStyle);
    
 

  
    connect(add_ModelBtn, &QPushButton::clicked, this, [this]() {

        QString filePath = QFileDialog::getOpenFileName(this, "Open STL File", "", "STL Files (*.stl)");
        if (!filePath.isEmpty()) {
            addModel(filePath);
        }
    });

    arrangeBtn->setStyleSheet(stlviewerButtonStyle);

   
    // Position in corner using fixed geometry or layout
    arrangeBtn->setGeometry(10, 10, 100, 30); 
    rotateXBtn->setGeometry(10, 50, 100, 30);
    rotateYBtn->setGeometry(10, 90, 100, 30);
    rotateZBtn->setGeometry(10,130, 100, 30);
    pn_toggleButton->setGeometry(10, 170, 100, 30);
    // Position in corner using fixed geometry or layout
    add_ModelBtn->setGeometry(140, 10, 140, 30);  // Position for add_ModelBtn
   
  
 

    // Initially hidden
    rotateXBtn->hide();
    rotateYBtn->hide();
    rotateZBtn->hide();
    arrangeBtn->show();
    pn_toggleButton->hide();
    // Initially hidden
    
 

    connect(rotateXBtn, &QPushButton::clicked, this, [this]() {
        rotateSelectedModel(dir, 0, 0);
        });
    connect(rotateYBtn, &QPushButton::clicked, this, [this]() {
        rotateSelectedModel(0, dir, 0);
        });
    connect(rotateZBtn, &QPushButton::clicked, this, [this]() {
        rotateSelectedModel(0, 0, dir);
        });

    connect(arrangeBtn, &QPushButton::clicked, this, [this]() {
        arrangeModelsOnPlatter();
        });
    auto customStyle = vtkSmartPointer<CustomInteractorStyle>::New();
    customStyle->SetDefaultRenderer(renderer);

    customStyle->SetRemoveActorCallback([this](vtkActor* actor) {
        // Find and remove from model list
        for (int i = 0; i < models.size(); ++i)
        {
            if (models[i].actor == actor)
            {
                renderer->RemoveActor(actor);
                models.remove(i);
                break;
            }
        }

        vtkWidget->renderWindow()->Render();  // Corrected
        });

    interactorStyle = customStyle;  // Store if needed elsewhere

    vtkWidget->renderWindow()->GetInteractor()->SetInteractorStyle(customStyle);  // Corrected

    customStyle->SetShowButtonsCallback([this](bool show) {
        showRotationButtons(show);
        });
    //emit logMessage("- StlViewer set ");
    // In the constructor, after setting other callbacks
    customStyle->SetActorMovedCallback([this](vtkActor* actor, double position[3]) {
        // Find which model corresponds to this actor
        for (int i = 0; i < models.size(); ++i) {
            if (models[i].actor == actor) {
                // Update model's stored position
                models[i].best_build_position[0] = position[0];
                models[i].best_build_position[1] = position[1];
                models[i].best_build_position[2] = position[2];

                // Optionally emit signal about position change
               
                // Update the signal emission to match the existing signal definition
                //emit logMessage(QString("Model %1 moved to position (%2, %3, %4)")
                               // .arg(i)
                                //.arg(position[0])
                                //.arg(position[1])
                               // .arg(position[2]));
                break;
            }
        }
        });



}

QStringList StlViewer::getLoadedModelNames() const
{
    QStringList names;
    for (const auto& model : models) {
        names << QFileInfo(model.filePath).fileName();
    }
    return names;
}

void StlViewer::setModelColor(int index, double r, double g, double b)
{
    if (index >= 0 && index < models.size()) {
        models[index].actor->GetProperty()->SetColor(r, g, b);
        vtkWidget->renderWindow()->Render();
    }
}

void StlViewer::removeModel(int index)
{
    if (index >= 0 && index < models.size()) {
        // If the interactor style currently holds this actor as picked, deselect it first
        if (interactorStyle) {
            vtkActor* picked = interactorStyle->GetPickedActor();
            if (picked && picked == models[index].actor) {
                interactorStyle->DeselectActor();
            }
        }

        renderer->RemoveActor(models[index].actor);
        models.remove(index);
        vtkWidget->renderWindow()->Render();
    }
    
}
void StlViewer::clearBuildPlate()
{
    // Remove models in reverse order to avoid index shifting issues
    for (int i = models.size() - 1; i >= 0; --i)
    {
        // reuse removeModel which now safely deselects picked actor
        removeModel(i);
    }
}

QVector<int> StlViewer::getdeletedmodels()
{
    return  this->deletedmodels;
}

void StlViewer::setBackgroundColor(double r, double g, double b)
{
    if (renderer) {
        renderer->SetBackground(r, g, b);
        vtkWidget->renderWindow()->Render();
    }
}

void StlViewer::dragEnterEvent(QDragEnterEvent* event)
{
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void StlViewer::dropEvent(QDropEvent* event)
{
    const QList<QUrl> urls = event->mimeData()->urls();
    for (const QUrl& url : urls) {
        QString filePath = url.toLocalFile();
        if (QFileInfo(filePath).suffix().toLower() == "stl") {
            addModel(filePath);
        }
    }
}

void StlViewer::arrangeModelsOnPlatter()
{
    const double zpos = 20.0;
    const double maxX = 130.0;
    const double maxY = 130.0;
    const double edgeSpacing = 5.0; // Minimum distance between model edges

    // Calculate grid origin (top-left corner)
    const double originX = -maxX / 2.0;
    const double originY =  maxY / 2.0;

    emit logMessage(QString("Arranging %1 models in grid pattern...").arg(models.size()));

    struct ModelBox {
        int index;
        double width, depth, height;
        double bboxMin[3];
        double bboxMax[3];
        double center[3];
        double roll, pitch, yaw;
        vtkSmartPointer<vtkTransform> rotationOnly;
    };
    QVector<ModelBox> boxes;

    // 1. For each model, reset, rotate, and get the new bounding box
    for (int i = 0; i < models.size(); ++i) {
        ModelInfo& model = models[i];
        model.actor->SetUserTransform(nullptr);
        vtkWidget->renderWindow()->Render();

        // Get original bounds and center
        double ob[6];
        model.actor->GetBounds(ob);
        double center[3] = {
            (ob[0] + ob[1]) / 2.0,
            (ob[2] + ob[3]) / 2.0,
            (ob[4] + ob[5]) / 2.0
        };

        // Build rotation transform about center
        double* angles = model.best_orientation_angles;
        vtkSmartPointer<vtkTransform> rot = vtkSmartPointer<vtkTransform>::New();
        rot->PostMultiply();
        rot->Translate(-center[0], -center[1], -center[2]);
        rot->RotateX(vtkMath::DegreesFromRadians(angles[0]));
        rot->RotateY(vtkMath::DegreesFromRadians(angles[1]));
        rot->RotateZ(vtkMath::DegreesFromRadians(angles[2]));
        rot->Translate(center[0], center[1], center[2]);

        model.actor->SetUserTransform(rot);
        vtkWidget->renderWindow()->Render();

        // Get rotated bounding box
        double rb[6];
        model.actor->GetBounds(rb);

        boxes.append({
            i,
            rb[1] - rb[0], // width
            rb[3] - rb[2], // depth
            rb[5] - rb[4], // height
            {rb[0], rb[2], rb[4]}, // bbox min (x, y, z)
            {rb[1], rb[3], rb[5]}, // bbox max (x, y, z)
            {center[0], center[1], center[2]},
            angles[0], angles[1], angles[2],
            rot
            });
    }

    // 2. Arrange models on the grid (left-to-right, top-to-bottom, 5mm edge spacing)
    double cursorX = originX + edgeSpacing;
    double cursorY = originY - edgeSpacing;
    double rowHeight = 0.0;

    for (const ModelBox& box : boxes) {
        // If model doesn't fit in current row, move to next row
        if (cursorX + box.width > originX + maxX) {
            cursorX = originX + edgeSpacing;
            cursorY -= (rowHeight + edgeSpacing);
            rowHeight = 0.0;
        }

        // If we've run out of vertical space
        if (cursorY - box.depth < -originY + edgeSpacing) {
            emit logMessage("Warning: Not enough space to place all models.");
            break;
        }

        ModelInfo& model = models[box.index];

        // 3. Build the final transform: rotation (about original center), then translation so bbox min is at (cursorX, cursorY)
        vtkSmartPointer<vtkTransform> t = vtkSmartPointer<vtkTransform>::New();
        t->PostMultiply();
        t->Concatenate(box.rotationOnly);
        // After rotation, move so that the bbox min is at (cursorX, cursorY, zpos)
        t->Translate(cursorX - box.bboxMin[0], cursorY - box.bboxMin[1]-(build_plate_radius/(2)*0.70), zpos - box.bboxMin[2]);  // just an compensation along y axis(build_plate_radius/(2)*0.70)

        model.actor->SetUserTransform(t);
        model.transform = t;

        // Store the new position (we'll use the bbox min as the reference)
        model.best_build_position[0] = cursorX;
        model.best_build_position[1] = cursorY -(build_plate_radius / (2) * 0.70);
        model.best_build_position[2] = zpos;

        // Store updated bounds
        model.actor->GetBounds(model.bounds);

        // Update cursor for next model
        cursorX += box.width + edgeSpacing;
        rowHeight = std::max(rowHeight, box.depth);
    }

    vtkWidget->renderWindow()->Render();
    emit logMessage("Models arranged in grid pattern complete.");
}

void StlViewer::onOrientationOptimizationFinished()
{
    //emit logMessage("-Orientation optimizations done!");
    //optimizationWatcher.disconnect();  // Clean up connection
    //optimizationNeeded = false; // Reset the optimization flag
    finalizeModelArrangement();        // Continue with arranging
   // optimizationNeeded = false;
}

void StlViewer::finalizeModelArrangement()
{
    const double zpos = 10.0;

    // Iterate through all models
    for (int i = 0; i < models.size(); ++i) {
        ModelInfo& model = models[i];
        model.actor->SetUserTransform(nullptr);
        vtkWidget->renderWindow()->Render();
        double* angles = model.best_orientation_angles;
        double* pos = model.best_build_position;
        vtkSmartPointer<vtkTransform> t = vtkSmartPointer<vtkTransform>::New();
        t->PostMultiply();
        double ob[6];
        model.actor->GetBounds(ob);
        double cx = (ob[0] + ob[1]) / 2.0, cy = (ob[2] + ob[3]) / 2.0, cz = (ob[4] + ob[5]) / 2.0;
        t->Translate(-cx, -cy, -cz);
        t->RotateX(vtkMath::DegreesFromRadians(angles[0]));
        t->RotateY(vtkMath::DegreesFromRadians(angles[1]));
        t->RotateZ(vtkMath::DegreesFromRadians(angles[2]));
        t->Translate(cx, cy, cz);
        // Translate to grid position
      
        t->Translate(pos[0], pos[1], zpos);
        model.actor->SetUserTransform(t);
        model.transform = t;
    }

    vtkWidget->renderWindow()->Render();
}

void StlViewer::addModel(const QString& stlFilePath)
{
    if (!QFileInfo::exists(stlFilePath))
        return;

    auto reader = vtkSmartPointer<vtkSTLReader>::New();
    reader->SetFileName(stlFilePath.toUtf8().constData());
    reader->Update();

    auto mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputConnection(reader->GetOutputPort());

    auto actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);
    actor->GetProperty()->SetColor(0, 0.9, 0.1); // Default gray
    //this->setModelColor(index++, 0, 0.9, 0.1);
       // Change background color to light gray
    this->setBackgroundColor(0.95, 0.95, 0.95);

    // 🔷 Assign a transform for button-based rotation
    auto transform = vtkSmartPointer<vtkTransform>::New();
    transform->PostMultiply();  // Enable stacking transformations
    actor->SetUserTransform(transform);
    
    // 🔷 Add to scene and model list
    renderer->AddActor(actor);
    models.append({ stlFilePath, actor, transform });

    renderer->ResetCamera();
    vtkWidget->renderWindow()->Render();
}



void StlViewer::addGroundPlate()
{
    // Create a circular plate (disk) centered at origin in XY plane
    auto disk = vtkSmartPointer<vtkDiskSource>::New();
    disk->SetInnerRadius(0.0);
    disk->SetOuterRadius(build_plate_radius);  // 200 mm diameter = 100 mm radius
    disk->SetCircumferentialResolution(64);
    disk->Update();

    auto mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputConnection(disk->GetOutputPort());

    auto actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);
    actor->GetProperty()->SetColor(0.8, 0.8, 0.8);  // Light yellowish
    actor->GetProperty()->SetRepresentationToSurface();

    actor->SetPosition(0.0, 0.0, 0.0);  // 🟢 Set precisely at world origin
    actor->PickableOff();              // 🔒 Not interactable
    renderer->AddActor(actor);

    // Add grid lines on top of disk, slightly above z=0 to avoid z-fighting
    const int gridSpacing = 10;
    const int gridSize = static_cast<int>(build_plate_radius*0.70);
    for (int i = -gridSize; i <= gridSize; i += gridSpacing)
    {
        auto lineX = vtkSmartPointer<vtkLineSource>::New();
        lineX->SetPoint1(i, -gridSize, 0.01);
        lineX->SetPoint2(i, gridSize, 0.01);

        auto lineY = vtkSmartPointer<vtkLineSource>::New();
        lineY->SetPoint1(-gridSize, i, 0.01);
        lineY->SetPoint2(gridSize, i, 0.01);

        auto mapperX = vtkSmartPointer<vtkPolyDataMapper>::New();
        mapperX->SetInputConnection(lineX->GetOutputPort());

        auto mapperY = vtkSmartPointer<vtkPolyDataMapper>::New();
        mapperY->SetInputConnection(lineY->GetOutputPort());

        auto actorX = vtkSmartPointer<vtkActor>::New();
        actorX->SetMapper(mapperX);
        actorX->GetProperty()->SetColor(0.3, 0.3, 0.3);
        actorX->GetProperty()->SetLineWidth(1.0);
        actorX->PickableOff();

        auto actorY = vtkSmartPointer<vtkActor>::New();
        actorY->SetMapper(mapperY);
        actorY->GetProperty()->SetColor(0.3, 0.3, 0.3);
        actorY->GetProperty()->SetLineWidth(1.0);
        actorY->PickableOff();

        renderer->AddActor(actorX);
        renderer->AddActor(actorY);
    }
}

void StlViewer::showRotationButtons(bool show)
{
    rotateXBtn->setVisible(show);
    rotateYBtn->setVisible(show);
    rotateZBtn->setVisible(show);
    pn_toggleButton->setVisible(show);
    //emit logMessage("- you clicked : " + QString::number(5));
}

void StlViewer::rotateSelectedModel(double xDeg, double yDeg, double zDeg)
{
    vtkActor* actor = interactorStyle->GetPickedActor();

    if (!actor) {
        emit logMessage("-No model selected.");
        return;
    }

    // Toggle rotation direction if needed
    if (this->pn_toggleButton->isChecked()) {
        xDeg = -xDeg;
        yDeg = -yDeg;
        zDeg = -zDeg;
    }

    for (auto& model : models)
    {
        if (model.actor == actor)
        {
            // Ensure transform is initialized
            if (!model.transform)
                model.transform = vtkSmartPointer<vtkTransform>::New();

            // Get current actor transform
            vtkSmartPointer<vtkTransform> currentTransform = vtkSmartPointer<vtkTransform>::New();
            if (actor->GetUserTransform())
                currentTransform->DeepCopy(actor->GetUserTransform());

            model.transform->DeepCopy(currentTransform);
            model.transform->PostMultiply();  // Important for transform order

            // Get the bounds of the actor to compute its center
            double bounds[6];
            actor->GetBounds(bounds);
            double center[3] = {
                (bounds[0] + bounds[1]) / 2.0,
                (bounds[2] + bounds[3]) / 2.0,
                (bounds[4] + bounds[5]) / 2.0
            };

            // Apply rotation about the center
            model.transform->Translate(-center[0], -center[1], -center[2]);
            model.transform->RotateX(xDeg);
            model.transform->RotateY(yDeg);
            model.transform->RotateZ(zDeg);
            model.transform->Translate(center[0], center[1], center[2]);

            model.transform->Modified();  // Mark as modified
            actor->SetUserTransform(model.transform);

            // Update the model's orientation angles (store in radians)
            model.best_orientation_angles[0] += vtkMath::RadiansFromDegrees(xDeg); // Roll (X)
            model.best_orientation_angles[1] += vtkMath::RadiansFromDegrees(yDeg); // Pitch (Y)
            model.best_orientation_angles[2] += vtkMath::RadiansFromDegrees(zDeg); // Yaw (Z)

            // Force render to update bounds
            vtkWidget->renderWindow()->Render();

            // Update bounds after transform is applied
            actor->GetBounds(bounds);
            std::copy(bounds, bounds + 6, model.bounds);

            // Optional: visual feedback
            model.actor->GetProperty()->SetColor(0.0, 0.9, 0.1);

            break;
        }
    }

    vtkWidget->update();                // Qt refresh
    vtkWidget->renderWindow()->Render(); // VTK redraw
}



void StlViewer::OrientationOptimizerfnc()
{
    // Use smart pointer to ensure automatic cleanup of optimizer
    std::unique_ptr<OrientationOptimizer> optimizer = std::make_unique<OrientationOptimizer>(this);
    connect(optimizer.get(), &OrientationOptimizer::logMessage,
        this, &StlViewer::logMessage);

    // Guard against empty model list
    if (models.isEmpty()) {
        emit logMessage("No models to optimize.");
        return;
    }

    // Iterate safely over models by reference
    for (ModelInfo& model : this->models) {
        // best_angles will hold the result for each model, freshly created
        //QVector<double> best_angles;
        emit logMessage("-Inside optimation loop!");
        // If your function can fail, consider try-catch or error checking here
        optimizer->optimizeModelOrientation(model, 135);

       // model.best_orientation_angles[0] = best_angles[0];
        //model.best_orientation_angles[1] = best_angles[1];
        //model.best_orientation_angles[1] = best_angles[1];
        // Use or store best_angles here safely
        // For example, you could store it back into model or emit a signal
    }
}

std::vector<InternalGuiModel> StlViewer::getModels() const
{
    std::vector<InternalGuiModel> internalModels;
    for (const ModelInfo& model : this->models) {
        InternalGuiModel internalModel;
        internalModel.path = model.filePath.toStdString();
        internalModel.xpos = model.best_build_position[0];
        internalModel.ypos = model.best_build_position[1];
        internalModel.zpos = model.best_build_position[2];
       
         
        internalModel.roll = model.best_orientation_angles[0];
        internalModel.pitch = model.best_orientation_angles[1];
        internalModel.yaw = model.best_orientation_angles[2];
        internalModels.push_back(internalModel);
    }
    return internalModels;
}