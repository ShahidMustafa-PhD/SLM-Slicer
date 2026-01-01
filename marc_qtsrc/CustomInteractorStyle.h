#ifndef CUSTOMINTERACTORSTYLE_H
#define CUSTOMINTERACTORSTYLE_H

#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkSmartPointer.h>
#include <vtkWeakPointer.h>
#include <vtkActor.h>
#include <vtkCellPicker.h>
#include <vtkProperty.h>
#include <vtkCommand.h>
#include <functional>

class CustomInteractorStyle : public vtkInteractorStyleTrackballCamera
{
public:
    static CustomInteractorStyle* New();
    vtkTypeMacro(CustomInteractorStyle, vtkInteractorStyleTrackballCamera);

    CustomInteractorStyle();

    void OnLeftButtonDown() override;
    void OnMouseMove() override;
    void OnLeftButtonUp() override;
    void OnTimer() override;

    void SetPicker(vtkSmartPointer<vtkCellPicker> picker);
    void SetRotationAxis(int axis) { RotationAxis = axis; }
    void SetRemoveActorCallback(const std::function<void(vtkActor*)>& callback);
    void OnKeyPress();
    void DeselectActor();
   

    std::function<void(bool)> ShowButtonsCallback;
    void SetShowButtonsCallback(std::function<void(bool)> cb) {
        ShowButtonsCallback = std::move(cb);
    }

    vtkActor* GetPickedActor() const { return pickedActorRaw(); }
    // Add to class definition:
    std::function<void(vtkActor*, double[3])> ActorMovedCallback;

    // Add setter method:
    void SetActorMovedCallback(const std::function<void(vtkActor*, double[3])>& callback) {
        ActorMovedCallback = callback;
    }

private:
    vtkSmartPointer<vtkCellPicker> Picker;
    vtkWeakPointer<vtkActor> PickedActor; // use weak pointer to avoid owning lifetime here

    bool IsDragging = false;
    double LastPickPosition[3];
    int RotationAxis = 0; // 0: none, 1: X, 2: Y, 3: Z
    int LastMousePosition[2];

    vtkSmartPointer<vtkActor> XHandle;
    vtkSmartPointer<vtkActor> YHandle;
    vtkSmartPointer<vtkActor> ZHandle;
    vtkSmartPointer<vtkAssembly> RotationHandles;
    std::function<void(vtkActor*)> RemoveActorCallback;

    //int LeftClickTimerId = -1;
    int ClickTimerId = -1;
    bool ClickPending = false;
    int LastClickPosition[2] = { -1, -1 };
    double ActorInitialPosition[3];
    double DragPlaneNormal[3];
    double DragPlanePoint[3];
    double DragOffset[3];

    // And declare the new method
    void MoveActorToMousePosition(int x, int y);

    // Helper to return raw pointer safely
    vtkActor* pickedActorRaw() const { return PickedActor.GetPointer(); }
};

#endif // CUSTOMINTERACTORSTYLE_H
