    #include "CustomInteractorStyle.h"
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkObjectFactory.h>
#include <vtkCamera.h>
#include <vtkMath.h>
#include <vtkSmartPointer.h>

vtkStandardNewMacro(CustomInteractorStyle);

CustomInteractorStyle::CustomInteractorStyle()
{
    Picker = vtkSmartPointer<vtkCellPicker>::New();
    Picker->SetTolerance(0.001);

    TimerDuration = 250;  // milliseconds
    ClickTimerId = -1;
    ClickPending = false;

    // Initialize drag state
    IsDragging = false;
    DragPlaneNormal[0] = 0.0;
    DragPlaneNormal[1] = 0.0;
    DragPlaneNormal[2] = 1.0;  // Default to XY plane
    DragPlanePoint[0] = 0.0;
    DragPlanePoint[1] = 0.0;
    DragPlanePoint[2] = 0.0;  // Default at origin
    DragOffset[0] = 0.0;
    DragOffset[1] = 0.0;
    DragOffset[2] = 0.0;
}

// New method for smooth drag calculation
void CustomInteractorStyle::MoveActorToMousePosition(int x, int y)
{
    // Only proceed if dragging and actor still valid
    vtkSmartPointer<vtkActor> actorSmart = pickedActorRaw();
    if (!IsDragging || !actorSmart) return;

    vtkRenderer* renderer = this->GetDefaultRenderer();
    if (!renderer) return;

    vtkRenderWindowInteractor* interactor = this->GetInteractor();
    if (!interactor) return;

    vtkCamera* camera = renderer->GetActiveCamera();
    if (!camera) return;

    // Convert mouse position to world ray
    double cameraPos[3], rayDirection[3], rayOrigin[3];
    camera->GetPosition(cameraPos);

    double displayPos[3] = { static_cast<double>(x), static_cast<double>(y), 0.0 };
    double worldPos[4];
    renderer->SetDisplayPoint(displayPos);
    renderer->DisplayToWorld();
    renderer->GetWorldPoint(worldPos);

    // Calculate ray direction
    rayOrigin[0] = cameraPos[0];
    rayOrigin[1] = cameraPos[1];
    rayOrigin[2] = cameraPos[2];
    rayDirection[0] = worldPos[0] - cameraPos[0];
    rayDirection[1] = worldPos[1] - cameraPos[1];
    rayDirection[2] = worldPos[2] - cameraPos[2];
    vtkMath::Normalize(rayDirection);

    // Intersect with XY plane at actor's Z
    double denominator = vtkMath::Dot(rayDirection, DragPlaneNormal);
    if (fabs(denominator) < 1e-6) return; // avoid division by zero

    double vectorToPlane[3] = { DragPlanePoint[0] - rayOrigin[0], DragPlanePoint[1] - rayOrigin[1], DragPlanePoint[2] - rayOrigin[2] };
    double t = vtkMath::Dot(DragPlaneNormal, vectorToPlane) / denominator;
    if (t < 0) return; // intersection behind camera

    double intersectionPoint[3] = { rayOrigin[0] + t * rayDirection[0], rayOrigin[1] + t * rayDirection[1], rayOrigin[2] + t * rayDirection[2] };

    double newPosition[3] = { intersectionPoint[0] - DragOffset[0], intersectionPoint[1] - DragOffset[1], DragPlanePoint[2] };

    // Update actor position and render
    actorSmart->SetPosition(newPosition);
    if (interactor->GetRenderWindow())
        interactor->GetRenderWindow()->Render();
}

void CustomInteractorStyle::SetPicker(vtkSmartPointer<vtkCellPicker> picker)
{
    Picker = picker;
}

void CustomInteractorStyle::OnLeftButtonDown()
{
    vtkRenderWindowInteractor* interactor = this->GetInteractor();
    if (!interactor) return;

    int* clickPos = interactor->GetEventPosition();

    if (ClickPending)
    {
        // Double-click detected: cancel single-click timer
        if (ClickTimerId != -1) interactor->DestroyTimer(ClickTimerId);
        ClickPending = false;
        ClickTimerId = -1;

        // Handle double-click: deselect actor
        DeselectActor();
        if (ShowButtonsCallback) ShowButtonsCallback(false);
    }
    else
    {
        vtkRenderer* renderer = this->GetDefaultRenderer();
        if (renderer && Picker)
        {
            Picker->Pick(clickPos[0], clickPos[1], 0, renderer);
            vtkActor* picked = Picker->GetActor();

            // If user clicked the currently picked actor, start dragging
            if (picked && picked == pickedActorRaw()) {
                IsDragging = true;
                LastMousePosition[0] = clickPos[0];
                LastMousePosition[1] = clickPos[1];

                vtkSmartPointer<vtkActor> actorSmart = pickedActorRaw();
                if (actorSmart) {
                    actorSmart->GetPosition(ActorInitialPosition);
                    // Setup drag plane at actor Z
                    DragPlaneNormal[0] = 0.0; DragPlaneNormal[1] = 0.0; DragPlaneNormal[2] = 1.0;
                    DragPlanePoint[0] = 0.0; DragPlanePoint[1] = 0.0; DragPlanePoint[2] = ActorInitialPosition[2];
                    if (Picker) Picker->GetPickPosition(LastPickPosition);
                    DragOffset[0] = LastPickPosition[0] - ActorInitialPosition[0];
                    DragOffset[1] = LastPickPosition[1] - ActorInitialPosition[1];
                    DragOffset[2] = 0.0;
                }
            }
        }

        // Start single-click timer for selection
        ClickPending = true;
        LastClickPosition[0] = clickPos[0];
        LastClickPosition[1] = clickPos[1];
        ClickTimerId = interactor->CreateOneShotTimer(TimerDuration);
    }

    // Call base class for camera interaction behavior
    vtkInteractorStyleTrackballCamera::OnLeftButtonDown();
}

void CustomInteractorStyle::OnTimer()
{
    // Handle single-click selection when timer fires
    if (!ClickPending) {
        vtkInteractorStyleTrackballCamera::OnTimer();
        return;
    }

    ClickPending = false;
    ClickTimerId = -1;

    vtkRenderer* renderer = this->GetDefaultRenderer();
    if (!renderer) {
        vtkInteractorStyleTrackballCamera::OnTimer();
        return;
    }

    if (!Picker) {
        vtkInteractorStyleTrackballCamera::OnTimer();
        return;
    }

    Picker->Pick(LastClickPosition[0], LastClickPosition[1], 0, renderer);
    vtkActor* picked = Picker->GetActor();

    if (picked)
    {
        // Reset previous actor color if different
        vtkSmartPointer<vtkActor> previousSmart = pickedActorRaw();
        if (previousSmart && previousSmart != picked) {
            auto prop = previousSmart->GetProperty();
            if (prop) prop->SetColor(0, 0.8, 0.8);
        }

        // Store via weak pointer to avoid ownership, but hold a temporary smart pointer while we mutate properties
        vtkSmartPointer<vtkActor> currentSmart = picked;
        PickedActor = picked;
        Picker->GetPickPosition(LastPickPosition);

        if (currentSmart) {
            currentSmart->GetPosition(ActorInitialPosition);
            LastMousePosition[0] = LastClickPosition[0];
            LastMousePosition[1] = LastClickPosition[1];
            auto prop = currentSmart->GetProperty();
            if (prop) prop->SetColor(1.0, 0.5, 0.0);
            if (ShowButtonsCallback) ShowButtonsCallback(true);
        }
    }
    else
    {
        // If nothing picked, deselect safely
        DeselectActor();
        if (ShowButtonsCallback) ShowButtonsCallback(false);
    }

    vtkInteractorStyleTrackballCamera::OnTimer();
}

void CustomInteractorStyle::OnMouseMove()
{
    vtkSmartPointer<vtkActor> actorSmart = pickedActorRaw();
    if (IsDragging && actorSmart) {
        int* mousePos = this->GetInteractor()->GetEventPosition();
        MoveActorToMousePosition(mousePos[0], mousePos[1]);
    }
    else {
        vtkInteractorStyleTrackballCamera::OnMouseMove();
    }
}

void CustomInteractorStyle::OnLeftButtonUp()
{
    vtkSmartPointer<vtkActor> actorSmart = pickedActorRaw();
    if (IsDragging && actorSmart)
    {
        IsDragging = false;

        double finalPosition[3];
        actorSmart->GetPosition(finalPosition);

        if (ActorMovedCallback) {
            // Clear weak reference before calling user callback (callback may remove actor)
            PickedActor = nullptr;
            ActorMovedCallback(actorSmart, finalPosition);
        }
    }
    vtkInteractorStyleTrackballCamera::OnLeftButtonUp();
}

void CustomInteractorStyle::SetRemoveActorCallback(const std::function<void(vtkActor*)>& callback)
{
    RemoveActorCallback = callback;
}

void CustomInteractorStyle::OnKeyPress()
{
    vtkRenderWindowInteractor* interactor = this->GetInteractor();
    if (!interactor) return;

    std::string key = interactor->GetKeySym();

    if (key == "Escape")
    {
        DeselectActor();
    }
    else if ((key == "x" || key == "X"))
    {
        vtkSmartPointer<vtkActor> actorSmart = pickedActorRaw();
        if (actorSmart && RemoveActorCallback)
        {
            // Clear weak reference before calling remove callback to avoid use-after-free in this object
            PickedActor = nullptr;
            RemoveActorCallback(actorSmart);
            RotationAxis = 0;
            IsDragging = false;
            if (interactor->GetRenderWindow()) interactor->GetRenderWindow()->Render();
        }
    }
    else
    {
        vtkSmartPointer<vtkActor> actorSmart = pickedActorRaw();
        if (actorSmart) {
            auto prop = actorSmart->GetProperty();
            if (prop) prop->SetColor(0, 1.0, 0.0);
        }
    }

    vtkInteractorStyleTrackballCamera::OnKeyPress();
}

void CustomInteractorStyle::DeselectActor()
{
    vtkSmartPointer<vtkActor> actorSmart = pickedActorRaw();
    if (actorSmart)
    {
        auto prop = actorSmart->GetProperty();
        if (prop) prop->SetColor(0, 0.9, 0.1);

        // Clear weak pointer before any callback or external changes
        PickedActor = nullptr;
        RotationAxis = 0;
        IsDragging = false;
        if (this->GetInteractor() && this->GetInteractor()->GetRenderWindow())
            this->GetInteractor()->GetRenderWindow()->Render();
    }
}

