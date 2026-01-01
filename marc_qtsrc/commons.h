struct ModelInfo {
    QString filePath;
    vtkSmartPointer<vtkActor> actor;
    vtkSmartPointer<vtkTransform> transform;  // New!
    double bounds[6];  // Add this if you plan to store bounds persistently
};