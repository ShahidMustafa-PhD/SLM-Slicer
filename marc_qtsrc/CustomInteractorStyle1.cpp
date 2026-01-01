#include "CustomInteractorStyle.h"
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkObjectFactory.h>
#include <vtkCamera.h>

vtkStandardNewMacro(CustomInteractorStyle);

CustomInteractorStyle::CustomInteractorStyle()
{
    Picker = vtkSmartPointer<vtkCellPicker>::New();
    Picker->SetTolerance(0.001);

    TimerDuration = 250;  // milliseconds
    ClickTimerId = -1;
    ClickPending = false;
}

void CustomInteractorStyle::SetPicker(vtkSmartPointer<vtkCellPicker> picker)
{
    Picker = picker;
}

void CustomInteractorStyle::OnLeftButtonDown()
{
    int* clickPos = this->GetInteractor()->GetEventPosition();

    if (ClickPending)
    {
        // Double-click detected: cancel single-click timer
        this->GetInteractor()->DestroyTimer(ClickTimerId);
        ClickPending = false;
        ClickTimerId = -1;

        // Handle double-click: deselect actor
        DeselectActor();
        if (ShowButtonsCallback) ShowButtonsCallback(false);
    }
    else
    {
        // Start single-click timer
        ClickPending = true;
        LastClickPosition[0] = clickPos[0];
        LastClickPosition[1] = clickPos[1];
        ClickTimerId = this->GetInteractor()->CreateOneShotTimer(TimerDuration);
    }

    vtkInteractorStyleTrackballCamera::OnLeftButtonDown();
}

void CustomInteractorStyle::OnTimer()
{
    if (ClickPending)
    {
        int* clickPos = LastClickPosition;
        vtkRenderer* renderer = this->GetDefaultRenderer();

        if (!renderer) return;

        Picker->Pick(clickPos[0], clickPos[1], 0, renderer);
        vtkActor* picked = Picker->GetActor();

        if (picked)
        {
            if (PickedActor && PickedActor != picked)
            {
                PickedActor->GetProperty()->SetColor(0, 0.8, 0.8);  // Reset previous actor color
            }

            PickedActor = picked;
            Picker->GetPickPosition(LastPickPosition);

            IsDragging = true;

            // Store original actor position and mouse position
            PickedActor->GetPosition(ActorInitialPosition);
            LastMousePosition[0] = clickPos[0];
            LastMousePosition[1] = clickPos[1];

            PickedActor->GetProperty()->SetColor(1.0, 0.5, 0.0);  // Highlight

            if (ShowButtonsCallback) ShowButtonsCallback(true);
        }
        else
        {
            DeselectActor();
            if (ShowButtonsCallback) ShowButtonsCallback(false);
        }

        ClickPending = false;
        ClickTimerId = -1;
    }

    vtkInteractorStyleTrackballCamera::OnTimer();
}


void CustomInteractorStyle::OnMouseMove()
{
    if (!IsDragging || !PickedActor)
        return;

    int* currentMousePos = this->GetInteractor()->GetEventPosition();

    // Compute mouse delta in screen coordinates
    int dx = currentMousePos[0] - LastMousePosition[0];
    int dy = currentMousePos[1] - LastMousePosition[1];

    vtkRenderer* renderer = this->GetDefaultRenderer();
    if (!renderer) return;

    // Convert screen delta to world delta using focal plane depth
    double world1[4], world2[4];
    double z = LastPickPosition[2];  // Or depth from Picker->GetPickPosition()

    // Convert initial screen point
    renderer->SetDisplayPoint(LastMousePosition[0], LastMousePosition[1], z);
    renderer->DisplayToWorld();
    renderer->GetWorldPoint(world1);

    // Convert new screen point
    renderer->SetDisplayPoint(currentMousePos[0], currentMousePos[1], z);
    renderer->DisplayToWorld();
    renderer->GetWorldPoint(world2);

    if (world1[3] == 0.0 || world2[3] == 0.0)
        return;

    // Normalize homogeneous coordinates
    for (int i = 0; i < 3; ++i) {
        world1[i] /= world1[3];
        world2[i] /= world2[3];
    }

    double deltaWorld[3] = {
        world2[0] - world1[0],
        world2[1] - world1[1],
        world2[2] - world1[2]
    };

    double* currentPos = PickedActor->GetPosition();

    // Optionally clip Z to avoid going below ground
    double newZ = currentPos[2] + deltaWorld[2];
    if (newZ < 0.0)
        deltaWorld[2] = -currentPos[2];

    // Apply the delta
    PickedActor->SetPosition(
        currentPos[0] + deltaWorld[0],
        currentPos[1] + deltaWorld[1],
        currentPos[2] + deltaWorld[2]);

    // Update stored screen position
    LastMousePosition[0] = currentMousePos[0];
    LastMousePosition[1] = currentMousePos[1];

    this->GetInteractor()->GetRenderWindow()->Render();

    vtkInteractorStyleTrackballCamera::OnMouseMove();  // Optional
}


void CustomInteractorStyle::OnLeftButtonUp()
{
    //return;  // disabled
    if (IsDragging)
    {
        IsDragging = false;
        PickedActor = nullptr;
    }

    vtkInteractorStyleTrackballCamera::OnLeftButtonUp();
}

void CustomInteractorStyle::SetRemoveActorCallback(const std::function<void(vtkActor*)>& callback)
{
    RemoveActorCallback = callback;
}

void CustomInteractorStyle::OnKeyPress()
{
    std::string key = this->GetInteractor()->GetKeySym();

    if (key == "Escape")
    {
        DeselectActor();
    }
    else if ((key == "x" || key == "X") && PickedActor)
    {
        if (RemoveActorCallback)
        {
            RemoveActorCallback(PickedActor); // Tell viewer to remove actor
            PickedActor = nullptr;
            RotationAxis = 0;
            IsDragging = false;
            this->GetInteractor()->GetRenderWindow()->Render();
        }
    }
    else if (PickedActor)
    {
        // Apply color change only if an actor is picked
        PickedActor->GetProperty()->SetColor(0, 1.0, 0.0);
    }

    vtkInteractorStyleTrackballCamera::OnKeyPress();
}

void CustomInteractorStyle::DeselectActor()
{
    if (PickedActor)
    {
        // Reset to neutral color on deselect
        PickedActor->GetProperty()->SetColor(0, 0.9, 0.1);

        PickedActor = nullptr;
        RotationAxis = 0;
        IsDragging = false;

        this->GetInteractor()->GetRenderWindow()->Render();
    }
}
