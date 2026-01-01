#ifndef SLMCOMMONS_H
#define SLMCOMMONS_H
#include <QString>
#include <vtkActor.h>
#include <vtkTransform.h>
#include <vtkSmartPointer.h>
struct ModelInfo {
    QString filePath;
    vtkSmartPointer<vtkActor> actor;
    vtkSmartPointer<vtkTransform> transform;  // New!
    double bounds[6];  // Add this if you plan to store bounds persistently
    double best_orientation_angles[3];// stores Roll, Pitch and Yaw, in radians
    double best_build_position[3];// 
};

struct InternalGuiModel
{
    std::string path;
    std::string buildconfig;   //  Build jconfiguration file directory path
    std::string stylesconfig;   //  Build Styles configuration file directory path
    int model_number;
    double xpos, ypos, zpos;
    double roll, pitch, yaw;
   
};  

#endif // STLVIEWER_H