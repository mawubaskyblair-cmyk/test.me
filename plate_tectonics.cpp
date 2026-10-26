#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <iomanip>
#include <memory>
#include <algorithm>

const double PI = 3.14159265358979323846;
const double EARTH_RADIUS_KM = 6371.0;

struct Vector3D {
    double x, y, z;

    Vector3D(double x_ = 0, double y_ = 0, double z_ = 0) : x(x_), y(y_), z(z_) {}

    Vector3D operator+(const Vector3D& other) const {
        return Vector3D(x + other.x, y + other.y, z + other.z);
    }

    Vector3D operator-(const Vector3D& other) const {
        return Vector3D(x - other.x, y - other.y, z - other.z);
    }

    Vector3D operator*(double scalar) const {
        return Vector3D(x * scalar, y * scalar, z * scalar);
    }

    double magnitude() const {
        return std::sqrt(x * x + y * y + z * z);
    }

    Vector3D normalized() const {
        double mag = magnitude();
        if (mag == 0) return Vector3D(0,0,0);
        return Vector3D(x / mag, y / mag, z / mag);
    }

    static double dot(const Vector3D& a, const Vector3D& b) {
        return a.x * b.x + a.y * b.y + a.z * b.z;
    }

    static Vector3D cross(const Vector3D& a, const Vector3D& b) {
        return Vector3D(
            a.y * b.z - a.z * b.y,
            a.z * b.x - a.x * b.z,
            a.x * b.y - a.y * b.x
        );
    }
};

enum class BoundaryType {
    DIVERGENT,
    CONVERGENT,
    TRANSFORM,
    UNKNOWN
};

class TectonicPlate {
private:
    std::string name;
    double area_sq_km;
    Vector3D euler_pole; // Euler pole rotation vector
    double angular_velocity_rad_yr; // Rotation rate in radians per million years

public:
    TectonicPlate(std::string name, double area, Vector3D pole, double ang_vel)
        : name(name), area_sq_km(area), euler_pole(pole.normalized()), angular_velocity_rad_yr(ang_vel) {}

    std::string getName() const { return name; }
    double getArea() const { return area_sq_km; }

    Vector3D getVelocityAtPoint(double lat, double lon) const {
        // Convert Lat/Lon to 3D Cartesian Position on Earth surface
        double phi = lat * PI / 180.0;
        double theta = lon * PI / 180.0;

        Vector3D pos(
            EARTH_RADIUS_KM * std::cos(phi) * std::cos(theta),
            EARTH_RADIUS_KM * std::cos(phi) * std::sin(theta),
            EARTH_RADIUS_KM * std::sin(phi)
        );

        Vector3D omega = euler_pole * angular_velocity_rad_yr;
        Vector3D velocity = Vector3D::cross(omega, pos); // v = w x r (mm/yr scale)
        return velocity;
    }
};

class PlateBoundary {
private:
    std::string name;
    std::shared_ptr<TectonicPlate> plateA;
    std::shared_ptr<TectonicPlate> plateB;
    double latitude;
    double longitude;

public:
    PlateBoundary(std::string name, std::shared_ptr<TectonicPlate> a, std::shared_ptr<TectonicPlate> b, double lat, double lon)
        : name(name), plateA(a), plateB(b), latitude(lat), longitude(lon) {}

    BoundaryType classifyBoundary() const {
        Vector3D vA = plateA->getVelocityAtPoint(latitude, longitude);
        Vector3D vB = plateB->getVelocityAtPoint(latitude, longitude);
        Vector3D relativeVelocity = vA - vB;

        // Radial vector pointing out from center of Earth
        double phi = latitude * PI / 180.0;
        double theta = longitude * PI / 180.0;
        Vector3D normal(std::cos(phi) * std::cos(theta), std::cos(phi) * std::sin(theta), std::sin(phi));

        double dotProd = Vector3D::dot(relativeVelocity, normal);
        double mag = relativeVelocity.magnitude();

        if (mag < 0.1) return BoundaryType::UNKNOWN;

        // Determine tangential drift direction vs distance change
        if (dotProd > 0.05) return BoundaryType::DIVERGENT;
        if (dotProd < -0.05) return BoundaryType::CONVERGENT;
        return BoundaryType::TRANSFORM;
    }

    void printAnalysis() const {
        Vector3D vA = plateA->getVelocityAtPoint(latitude, longitude);
        Vector3D vB = plateB->getVelocityAtPoint(latitude, longitude);
        Vector3D rel = vA - vB;
        double relativeSpeed = rel.magnitude() * 100.0; // convert to mm/yr scale

        BoundaryType type = classifyBoundary();
        std::string typeStr;
        switch(type) {
            case BoundaryType::DIVERGENT: typeStr = "Divergent (Rifting)"; break;
            case BoundaryType::CONVERGENT: typeStr = "Convergent (Subduction/Collision)"; break;
            case BoundaryType::TRANSFORM: typeStr = "Transform (Strike-Slip)"; break;
            default: typeStr = "Complex/Stable"; break;
        }

        std::cout << std::left << std::setw(25) << name
                  << std::setw(20) << (plateA->getName() + "-" + plateB->getName())
                  << std::setw(15) << relativeSpeed
                  << std::setw(30) << typeStr << "\n";
    }
};

class SimulationEnvironment {
private:
    std::vector<std::shared_ptr<TectonicPlate>> plates;
    std::vector<PlateBoundary> boundaries;

public:
    void setupDefaultEarthModel() {
        auto pacific = std::make_shared<TectonicPlate>("Pacific", 103300000.0, Vector3D(-61.0, 84.0, 0.0), 0.00000087);
        auto northAmerican = std::make_shared<TectonicPlate>("North American", 75900000.0, Vector3D(48.0, -78.0, 0.0), 0.00000024);
        auto eurasian = std::make_shared<TectonicPlate>("Eurasian", 67800000.0, Vector3D(61.0, -85.0, 0.0), 0.00000021);
        auto indoAustralian = std::make_shared<TectonicPlate>("Indo-Australian", 58900000.0, Vector3D(60.0, 2.0, 0.0), 0.00000067);

        plates.push_back(pacific);
        plates.push_back(northAmerican);
        plates.push_back(eurasian);
        plates.push_back(indoAustralian);

        boundaries.emplace_back("San Andreas Fault", pacific, northAmerican, 36.0, -120.0);
        boundaries.emplace_back("Aleutian Trench", pacific, northAmerican, 52.0, -175.0);
        boundaries.emplace_back("Himalayan Front", indoAustralian, eurasian, 28.0, 85.0);
        boundaries.emplace_back("Mid-Atlantic Ridge", northAmerican, eurasian, 45.0, -28.0);
    }

    void run() {
        std::cout << "===================================================================================\n";
        std::cout << "               EARTH TECTONIC PLATE DRIFT & BOUNDARY ANALYZER                      \n";
        std::cout << "===================================================================================\n";
        std::cout << std::left << std::setw(25) << "Boundary Name"
                  << std::setw(20) << "Plates Involved"
                  << std::setw(15) << "Drift (mm/yr)"
                  << std::setw(30) << "Interaction Type" << "\n";
        std::cout << "-----------------------------------------------------------------------------------\n";

        for (const auto& boundary : boundaries) {
            boundary.printAnalysis();
        }
        std::cout << "-----------------------------------------------------------------------------------\n";
    }
};

int main() {
    SimulationEnvironment sim;
    sim.setupDefaultEarthModel();
    sim.run();
    return 0;
}