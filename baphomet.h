#ifndef BAPHOMET_H
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>
#include <cmath>
#include <chrono>
#include <thread>
#include <mutex>
#include <vector>
#include <sstream>
#include <iomanip>
#include <omp.h>

const float PI = 3.14159265f;

using namespace cv;
using namespace std;

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>
#include <stdexcept>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <limits>
#include <cstring>

using namespace std;



string remove_space(string input) {
    input.erase(remove(input.begin(), input.end(), '	'), input.end());
    input.erase(remove(input.begin(), input.end(), ' '), input.end());
    return input;
}
string splitintwo(const string& str, char delimiter, bool beforeDelimiter = true) {
    size_t pos = str.find(delimiter);
    if (pos != string::npos) {
        if (beforeDelimiter) {
            return str.substr(0, pos);
        } else {
            return str.substr(pos + 1);
        }
    }
    return str; // Return an empty string if delimiter is not found
}
char* delete_crlf(const char* input) {
    size_t input_length = strlen(input);
    char* result = (char*)malloc(input_length + 1); // Allocate memory for the result
    if (result == NULL) {
        return NULL; // Memory allocation failed
    }

    size_t j = 0; // Index for the result buffer
    for (size_t i = 0; i < input_length; ++i) {
        // Look for "\r\n" and skip them
        if (input[i] == '\r' && i + 1 < input_length && input[i + 1] == '\n') {
            ++i; // Skip the '\n' too
            continue;
        }

        // Copy character to the result buffer
        result[j++] = input[i];
    }
    result[j] = '\0'; // Null-terminate the result string

    return result;
}
string trim(const string& str) {
    // Find the first non-space character
    size_t start = str.find_first_not_of(" \t\n\r");

    // If the string is all spaces, return an empty string
    if (start == string::npos) return "";

    // Find the last non-space character
    size_t end = str.find_last_not_of(" \t\n\r");

    // Return the trimmed substring
    return str.substr(start, end - start + 1);
}

char* getSetting(char* name, char* path)
{
    char* output="0";

    ifstream file(path); // Open the file
    if (!file.is_open()) {
        printf("Can't open  settings file\n");
        return "0";
    }

    string line;
    string name2;
    while (getline(file, line)) { 
        name2=delete_crlf(splitintwo(line, ':', true).c_str()); // Get setting name
        if(remove_space(name2) == (string)name)
        {
            name2=splitintwo(line, ':', false);
            name2=trim(name2);
            output=delete_crlf(name2.c_str()); // Get setting value
        }
    }

    file.close(); // Close the file
    return output;
}


namespace Baphomet {
    class Baphomet {
        public:
        static string banner;
    };

    string Baphomet::banner = 
" .S_SSSs     .S_SSSs     .S_sSSs     .S    S.     sSSs_sSSs     .S_SsS_S.     sSSs  sdSS_SSSSSSbs  \n"\
".SS~SSSSS   .SS~SSSSS   .SS~YS%%b   .SS    SS.   d%%SP~YS%%b   .SS~S*S~SS.   d%%SP  YSSS~S%SSSSSP  \n"\
"S%S   SSSS  S%S   SSSS  S%S   `S%b  S%S    S%S  d%S'     `S%b  S%S `Y' S%S  d%S'         S%S       \n"\
"S%S    S%S  S%S    S%S  S%S    S%S  S%S    S%S  S%S       S%S  S%S     S%S  S%S          S%S       \n"\
"S%S SSSS%P  S%S SSSS%S  S%S    d*S  S%S SSSS%S  S&S       S&S  S%S     S%S  S&S          S&S       \n"\
"S&S  SSSY   S&S  SSS%S  S&S   .S*S  S&S  SSS&S  S&S       S&S  S&S     S&S  S&S_Ss       S&S       \n"\
"S&S    S&S  S&S    S&S  S&S_sdSSS   S&S    S&S  S&S       S&S  S&S     S&S  S&S~SP       S&S       \n"\
"S&S    S&S  S&S    S&S  S&S~YSSY    S&S    S&S  S&S       S&S  S&S     S&S  S&S          S&S       \n"\
"S*S    S&S  S*S    S&S  S*S         S*S    S*S  S*b       d*S  S*S     S*S  S*b          S*S       \n"\
"S*S    S*S  S*S    S*S  S*S         S*S    S*S  S*S.     .S*S  S*S     S*S  S*S.         S*S       \n"\
"S*S SSSSP   S*S    S*S  S*S         S*S    S*S   SSSbs_sdSSS   S*S     S*S   SSSbs       S*S       \n"\
"S*S  SSY    SSS    S*S  S*S         SSS    S*S    YSSP~YSSY    SSS     S*S    YSSP       S*S       \n"\
"SP                 SP   SP                 SP                          SP                SP        \n"\
"Y                  Y    Y                  Y                           Y                 Y         \n"\
"                                                                                                   ";


    class Point2d {
    public:
        float x, y;
        Point2d(float x, float y): x(x), y(y) {}
    };

    class Point {
    public:
        float x, y, z;

        Point(float x = 0, float y = 0, float z = 0) : x(x), y(y), z(z) {}

        // Method to calculate the length of the vector
        float length() const {
            return sqrt(x * x + y * y + z * z);
        }

        // Overload the subtraction operator to calculate the difference between two points
        Point operator-(const Point& other) const {
            return Point(x - other.x, y - other.y, z - other.z);
        }

        // Dot product method
        float dot(const Point& other) const {
            return x * other.x + y * other.y + z * other.z;
        }

        // Normalize the vector
        Point normalize() const {
            float len = length();
            return (len > 0) ? Point(x / len, y / len, z / len) : Point(0, 0, 0);
        }
    };
    class Ray {
    public:
        Point origin, direction;
        Ray(Point o, Point d) : origin(o), direction(d) {}
    };

    float dotProduct(const Point& a, const Point& b) {
        return a.x * b.x + a.y * b.y + a.z * b.z;
    }

    Point operator*(float scalar, const Point& point) {
        return Point(scalar * point.x, scalar * point.y, scalar * point.z);
    }
    Point operator*(const Point& point, float scalar) {
        return Point(scalar * point.x, scalar * point.y, scalar * point.z);
    }
    
    Point operator+(const Point& p1, const Point& p2) {
        return Point(p1.x + p2.x, p1.y + p2.y, p1.z + p2.z);
    }

    

    class Object {
    public:
        Point center; // Center point of the object
        Vec3b color;  // Color of the object

        Object(Point center, Vec3b col) : center(center), color(col) {}
        virtual ~Object() {} // Virtual destructor

        virtual bool rayIntersect(const Ray& ray, float& t) = 0;
    };

    class Scene {
        public:
        std::vector<Object*> objects;
        void addObject(Object* obj) {
            objects.push_back(obj);
        }
        Scene() {}
        ~Scene() {
            for (auto obj : objects) {
                delete obj;
            }
        }
    };

    class Obj {
    public:
        struct Vertex {
            float x, y, z;
        };

        struct Face {
            int v[3]; // Indices of vertices
        };

        std::vector<Vertex> vertices;
        std::vector<Face> faces;

        // Load OBJ file
        bool load(const std::string& filename) {
            std::ifstream file(filename);
            if (!file.is_open()) {
                std::cerr << "Could not open the file: " << filename << std::endl;
                return false;
            }

            std::string line;
            while (std::getline(file, line)) {
                std::istringstream iss(line);
                std::string prefix;
                iss >> prefix;

                if (prefix == "v") {
                    Vertex vertex;
                    iss >> vertex.x >> vertex.y >> vertex.z;
                    vertices.push_back(vertex);
                } else if (prefix == "f") {
                    Face face;
                    for (int i = 0; i < 3; ++i) {
                        std::string vertexData;
                        iss >> vertexData;
                        size_t pos = vertexData.find('/');
                        face.v[i] = std::stoi(vertexData.substr(0, pos)) - 1; // OBJ indices are 1-based
                    }
                    faces.push_back(face);
                }
            }

            return true;
        }

        // Draw the object (pseudo-code)
        void draw(cv::Mat& image) {
            for (const auto& face : faces) {
                // Get vertices for the face
                const Vertex& v1 = vertices[face.v[0]];
                const Vertex& v2 = vertices[face.v[1]];
                const Vertex& v3 = vertices[face.v[2]];

                // Convert 3D vertices to 2D screen coordinates (pseudo-code)
                // You will need to implement a proper projection based on your camera
                cv::Point2f p1(v1.x, v1.y); // Replace with actual projection
                cv::Point2f p2(v2.x, v2.y); // Replace with actual projection
                cv::Point2f p3(v3.x, v3.y); // Replace with actual projection

                // Draw the triangle (pseudo-code)
                cv::line(image, p1, p2, cv::Scalar(255, 255, 255));
                cv::line(image, p2, p3, cv::Scalar(255, 255, 255));
                cv::line(image, p3, p1, cv::Scalar(255, 255, 255));
            }
        }

        // Ray casting method (pseudo-code)
        bool rayIntersect(const Ray& ray, float& t) {
            bool hit = false;
            for (const auto& face : faces) {
                // Implement ray-triangle intersection test
                // If intersection occurs, update t and set hit to true
            }
            return hit;
        }
    };
    class Cube : public Object {
    public:
        float c;
        float minX;
        float maxX;
        float minY;
        float maxY;
        float minZ;
        float maxZ;

        Cube(Point center, float c, Vec3b col) : Object(center, col), c(c) {
            minX = center.x-(c/2);
            maxX = center.x+(c/2);
            minY = center.y-(c/2);
            maxY = center.y+(c/2);
            minZ = center.z-(c/2);
            maxZ = center.z+(c/2);
        }

        
    
        bool rayIntersect(const Ray& ray, float& t) {
            float tmin = (center.x - c / 2 - ray.origin.x) / ray.direction.x;
            float tmax = (center.x + c / 2 - ray.origin.x) / ray.direction.x;
            
            if (tmin > tmax) std::swap(tmin, tmax);

            float tymin = (center.y - c / 2 - ray.origin.y) / ray.direction.y;
            float tymax = (center.y + c / 2 - ray.origin.y) / ray.direction.y;

            if (tymin > tymax) std::swap(tymin, tymax);

            if ((tmin > tymax) || (tymin > tmax))
                return false;

            if (tymin > tmin)
                tmin = tymin;

            if (tymax < tmax)
                tmax = tymax;

            float tzmin = (center.z - c / 2 - ray.origin.z) / ray.direction.z;
            float tzmax = (center.z + c / 2 - ray.origin.z) / ray.direction.z;

            if (tzmin > tzmax) std::swap(tzmin, tzmax);

            if ((tmin > tzmax) || (tzmin > tmax))
                return false;

            t = tmin;
            return true;
        }
    };
    class Skybox : public Object {
    public:
        float size;
        cv::Mat texture; // Skybox texture
        Skybox(Point center, Vec3b col, float size, const std::string& texturePath) : Object(center, col), size(size) {
            // Load the texture
            texture = cv::imread(texturePath);
            if (texture.empty()) {
                throw std::runtime_error("Failed to load skybox texture: " + texturePath);
            }
        }

        bool rayIntersect(const Ray& ray, float& t)  {
            // Skybox doesn't block rays, always return false for intersections
            return false;
        }

        cv::Vec3b getColor(const Ray& ray) const {
            // Map ray direction to texture coordinates
            float u = 0.5f + (std::atan2(ray.direction.z, ray.direction.x) / (2.0f * M_PI));
            float v = 0.5f - (std::asin(ray.direction.y) / M_PI);

            int x = static_cast<int>(u * texture.cols) % texture.cols;
            int y = static_cast<int>(v * texture.rows) % texture.rows;

            return texture.at<cv::Vec3b>(y, x);
        }
    };
    class Sphere : public Object {
    public:
        float radius;
        Sphere(Point c, float r, Vec3b col) : Object(center, col), radius(r) {}
    
        bool rayIntersect(const Ray& ray, float& t) {
            Point oc = ray.origin - center;
            float a = dotProduct(ray.direction, ray.direction);
            float b = 2.0f * dotProduct(oc, ray.direction);
            float c = dotProduct(oc, oc) - radius * radius;
            float discriminant = b * b - 4 * a * c;

            if (discriminant < 0) {
                return false; // No intersection
            } else {
                t = (-b - sqrt(discriminant)) / (2.0f * a); // Get the nearest intersection
                return t >= 0; // Ensure the intersection is in front of the ray
            }
        }
    };
    class Plane : public Object { // Ensure Rectangle inherits from Object
    public:
        Point corner; // One corner of the rectangle
        Point widthVector; // Vector representing the width
        Point heightVector; // Vector representing the height

        Plane(const Point& corner, const Point& widthVector, const Point& heightVector, const Vec3b& color) 
        : Object(corner, color), corner(corner), widthVector(widthVector), heightVector(heightVector) 
        {}

        bool rayIntersect(const Ray& ray, float& t) {
            // Calculate the normal of the rectangle using the cross product of the width and height vectors
            Point normal = crossProduct(widthVector, heightVector);
            float denominator = normal.x * ray.direction.x + normal.y * ray.direction.y + normal.z * ray.direction.z;

            // Check if the ray is parallel to the rectangle
            if (fabs(denominator) > 1e-6) {
                // Calculate the vector from the ray origin to the rectangle corner
                Point p0l0 = corner - ray.origin;

                // Calculate t using the plane equation
                t = (p0l0.x * normal.x + p0l0.y * normal.y + p0l0.z * normal.z) / denominator;

                // Check if the intersection point is in front of the ray
                if (t >= 0) {
                    // Calculate the intersection point
                    Point intersectionPoint = ray.origin + ray.direction * t;

                    // Check if the intersection point is within the rectangle bounds
                    Point toIntersection = intersectionPoint - corner;

                    // Project the intersection point onto the width and height vectors
                    float widthProjection = dotProduct(toIntersection, widthVector) / dotProduct(widthVector, widthVector);
                    float heightProjection = dotProduct(toIntersection, heightVector) / dotProduct(heightVector, heightVector);

                    // Check if the projections are within the bounds of the rectangle
                    return (widthProjection >= 0 && widthProjection <= widthVector.length() &&
                            heightProjection >= 0 && heightProjection <= heightVector.length());
                }
            }
            return false;
        }

        // Helper functions for vector operations
        static Point crossProduct(const Point& a, const Point& b) {
            return Point(
                a.y * b.z - a.z * b.y,
                a.z * b.x - a.x * b.z,
                a.x * b.y - a.y * b.x
            );
        }

        static float dotProduct(const Point& a, const Point& b) {
            return a.x * b.x + a.y * b.y + a.z * b.z;
        }
    };

    class Light {
    public:
        Point position;
        Vec3b color; // RGB color of the light

        Light(Point pos, Vec3b col) : position(pos), color(col) {}
    };

    // to represent direction of length 1
    Point normalize(const Point& p) {
        float length = sqrt(p.x * p.x + p.y * p.y + p.z * p.z);
        
        if (length != 0) {
            return Point(p.x / length, p.y / length, p.z / length);
        } else {
            return Point(0, 0, 0);
        }
    }

    bool isInShadow(const Point& intersectionPoint, const Light& light, const Scene& scene, const Object* hitObject) {
        Point lightDir = normalize(light.position - intersectionPoint);
        Ray shadowRay(intersectionPoint + 1e-4 * lightDir, lightDir); // Offset to avoid self-intersection
        float t_min = numeric_limits<float>::max();

        for (const auto& object : scene.objects) {
            float t = 0;
            if (object != hitObject && object->rayIntersect(shadowRay, t)) {
                // If we find any object between the intersection point and the light source
                if (t > 0.0f && t < t_min) {
                    return true; // Intersection means the point is in shadow
                }
            }
        }
        return false; // No blocking objects found
    }


    class Camera {
        public:
        Point position;
        Point forward; // Forward direction vector
        Point right; // Right direction vector
        Point up; // Up direction vector
        
        Camera(Point pos) : position(pos) {
            updateDirection(0, 0);
        }

        Point direction = Point(0.0f, 0.0f, 1.0f);
        int direction_horizontal = 0;
        int direction_vertical = 0;

        void moveForward(float distance) {
            // Assuming you have a forward vector that represents the direction the camera is facing
            position = position + (forward * distance); // Move in the forward direction
        }

        Point cross(const Point& a, const Point& b) const {
            return Point(
                a.y * b.z - a.z * b.y,
                a.z * b.x - a.x * b.z,
                a.x * b.y - a.y * b.x
            );
        }

        void moveSide(float distance) {
            // Assuming you have a right vector that represents the right direction of the camera
            position = position + (right * distance); // Move in the right direction
        }
        void moveUpward(float value) {
            position.y += value;
        }
        void updateDirection(float horizontalChange, float verticalChange) {
            direction_horizontal += horizontalChange;
            direction_vertical += verticalChange;

            // Calculate forward vector based on yaw and pitch
            forward.x = cos(direction_vertical * M_PI / 180.0f) * sin(direction_horizontal * M_PI / 180.0f);
            forward.y = sin(direction_vertical * M_PI / 180.0f);
            forward.z = cos(direction_vertical * M_PI / 180.0f) * cos(direction_horizontal * M_PI / 180.0f);
            forward = forward.normalize(); // Normalize the forward vector

            // Calculate right vector (perpendicular to forward and up)
            // Assuming up vector is (0, 1, 0)
            Point up(0, 1, 0);
            right = cross(forward, up).normalize(); // Normalize the right vecto
        }

        
    };
    void drawOrientationArrows(cv::Mat& image, const Camera& camera) {
        // Define arrow lengths
        const int arrowLength = 50;

        // Calculate arrow directions based on camera orientation
        Point xArrow = camera.position + camera.direction * arrowLength; // Red
        Point yArrow = camera.position + Point(0, 1, 0) * arrowLength; // Green
        Point zArrow = camera.position + Point(0, 0, 1) * arrowLength; // Blue

        // Draw arrows
        cv::arrowedLine(image, cv::Point(camera.position.x, camera.position.y), 
                        cv::Point(20+xArrow.x, 20+xArrow.y), cv::Scalar(0, 0, 255), 2); // Red
        cv::arrowedLine(image, cv::Point(camera.position.x, camera.position.y), 
                        cv::Point(20+yArrow.x, 20+yArrow.y), cv::Scalar(0, 255, 0), 2); // Green
        cv::arrowedLine(image, cv::Point(camera.position.x, camera.position.y), 
                        cv::Point(20+zArrow.x, 20+zArrow.y), cv::Scalar(255, 0, 0), 2); // Blue
    }

    


    string toStringWithPrecision(float value, int precision) {
        ostringstream out;
        out << fixed << setprecision(precision) << value;
        return out.str();
    }


    void loadMap(char* path, Camera* camera) {
        cout << "[~] Loading '" << path << "'" << endl;
        float camX, camY, camZ, horDir, verDir = 0;

        try {
            camX = stof(getSetting("startX", path));
            camY = stof(getSetting("startY", path));
            camZ = stof(getSetting("startZ", path));

            horDir = stof(getSetting("startRotH", path));
            horDir = stof(getSetting("startRotH", path));
        } catch(exception e) {
            cout << "[x] Error on map, invalid value format" << e.what() << endl;
        }

        camera = new Camera(Point(camX, camY, camZ));
        camera->direction_horizontal = horDir;
        camera->direction_vertical = verDir;

        cout << endl << "MAP '" << path << "' loaded successfully!" << endl; 
        
    }
}


#endif // !BAPHOMET_H