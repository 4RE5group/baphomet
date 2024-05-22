#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <vector>
#include <filesystem>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>
#include <cmath>

using namespace std;

// Constants
const int WIDTH = 800;
const int HEIGHT = 600;

// Define a simple 3D point class
class Vec3 {
public:
    float x, y, z;

    Vec3(float x, float y, float z) : x(x), y(y), z(z) {}

    Vec3 operator+(const Vec3& other) const {
        return Vec3(x + other.x, y + other.y, z + other.z);
    }

    Vec3 operator-(const Vec3& other) const {
        return Vec3(x - other.x, y - other.y, z - other.z);
    }

    Vec3 operator*(float scalar) const {
        return Vec3(x * scalar, y * scalar, z * scalar);
    }

    float dot(const Vec3& other) const {
        return x * other.x + y * other.y + z * other.z;
    }

    Vec3 cross(const Vec3& other) const {
        return Vec3(y * other.z - z * other.y, z * other.x - x * other.z, x * other.y - y * other.x);
    }

    float length() const {
        return std::sqrt(x * x + y * y + z * z);
    }

    Vec3 normalized() const {
        float len = length();
        return Vec3(x / len, y / len, z / len);
    }
};

// Define a simple sphere class
class Sphere {
public:
    Vec3 center;
    float radius;
    cv::Vec3b color;

    Sphere(const Vec3& center, float radius, const cv::Vec3b& color) : center(center), radius(radius), color(color) {}

    // Ray-sphere intersection test
    bool intersect(const Vec3& origin, const Vec3& direction, float& t) const {
        Vec3 oc = origin - center;
        float a = direction.dot(direction);
        float b = 2.0 * oc.dot(direction);
        float c = oc.dot(oc) - radius * radius;
        float discriminant = b * b - 4 * a * c;
        if (discriminant < 0) return false;
        float sqrt_discriminant = std::sqrt(discriminant);
        float t1 = (-b - sqrt_discriminant) / (2.0 * a);
        float t2 = (-b + sqrt_discriminant) / (2.0 * a);
        if (t1 > 0) {
            t = t1;
            return true;
        }
        if (t2 > 0) {
            t = t2;
            return true;
        }
        return false;
    }
};

// Define a simple plane class
class Plane {
public:
    Vec3 normal;
    float distance;
    cv::Vec3b color;

    Plane(const Vec3& normal, float distance, const cv::Vec3b& color) : normal(normal), distance(distance), color(color) {}

    // Ray-plane intersection test
    bool intersect(const Vec3& origin, const Vec3& direction, float& t) const {
        float denom = normal.dot(direction);
        if (std::abs(denom) > 1e-6) {
            t = (distance - normal.dot(origin)) / denom;
            return t >= 0;
        }
        return false;
    }
};

// Trace function
cv::Vec3b trace(const Vec3& origin, const Vec3& direction, const std::vector<Sphere>& spheres, const std::vector<Plane>& planes, int depth) {
    if (depth <= 0) return cv::Vec3b(0, 0, 0);

    float min_t = std::numeric_limits<float>::max();
    cv::Vec3b color(0, 0, 0);

    // Check intersections with spheres
    for (const auto& sphere : spheres) {
        float t;
        if (sphere.intersect(origin, direction, t) && t < min_t) {
            min_t = t;
            color = sphere.color;
        }
    }

    // Check intersections with planes
    for (const auto& plane : planes) {
        float t;
        if (plane.intersect(origin, direction, t) && t < min_t) {
            min_t = t;
            color = plane.color;
        }
    }

    // If no intersection, return background color
    if (min_t == std::numeric_limits<float>::max()) {
        return cv::Vec3b(135, 206, 235); // Sky blue
    }

    // Compute intersection point
    Vec3 intersection_point = origin + direction * min_t;

    // Compute shading
    Vec3 light_dir = Vec3(0.5, 0.5, -1).normalized(); // Light direction
    float intensity = light_dir.dot(Vec3(0, 0, 1)); // Lambertian shading

    // Apply shading
    color *= intensity;

    // Recursive reflection
    Vec3 reflect_dir = direction - normal * 2 * normal.dot(direction); // Reflect direction
    cv::Vec3b reflected_color = trace(intersection_point, reflect_dir, spheres, planes, depth - 1);

    // Mix reflected color with object color
    float reflection_intensity = 0.3;
    color = color * (1 - reflection_intensity) + reflected_color * reflection_intensity;

    return color;
}