#ifndef TRANSFORM_H
#define TRANSFORM_H

#include <cmath>

namespace MarcSLM {
namespace Domain {

/**
 * @brief Represents position and orientation of a 3D model
 * 
 * Position uses Cartesian coordinates (X, Y, Z) in millimeters.
 * Orientation uses Euler angles (roll, pitch, yaw) in degrees.
 * 
 * Convention:
 * - Roll: Rotation about X-axis
 * - Pitch: Rotation about Y-axis  
 * - Yaw: Rotation about Z-axis
 */
struct Transform {
    // Position in millimeters
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
    
    // Orientation in degrees
    double roll = 0.0;   // Rotation about X-axis
    double pitch = 0.0;  // Rotation about Y-axis
    double yaw = 0.0;    // Rotation about Z-axis
    
    Transform() = default;
    
    Transform(double px, double py, double pz, 
              double r = 0.0, double p = 0.0, double y = 0.0)
        : x(px), y(py), z(pz), roll(r), pitch(p), yaw(y) {}
    
    // Equality comparison
    bool operator==(const Transform& other) const {
        constexpr double epsilon = 1e-6;
        return std::abs(x - other.x) < epsilon &&
               std::abs(y - other.y) < epsilon &&
               std::abs(z - other.z) < epsilon &&
               std::abs(roll - other.roll) < epsilon &&
               std::abs(pitch - other.pitch) < epsilon &&
               std::abs(yaw - other.yaw) < epsilon;
    }
    
    bool operator!=(const Transform& other) const {
        return !(*this == other);
    }
};

} // namespace Domain
} // namespace MarcSLM

#endif // TRANSFORM_H
