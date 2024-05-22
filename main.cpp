#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>
#include <cmath>
#include <chrono>
#include <thread>
#include <iostream>
// sudo apt install libx11-dev

using namespace std;
using namespace std::this_thread;     // sleep_for, sleep_until
using namespace std::chrono_literals; // ns, us, ms, s, h, etc.
using std::chrono::system_clock;

// Constants
const string window_name = "Graphic Engine";
const int WIDTH = 800;
const int HEIGHT = 600;


class Point {
public:
    float x, y, z;
    Point(float x, float y, float z) : x(x), y(y), z(z) {}
    Point() : x(0), y(0), z(0) {}
};

class Vector3D {
public:
    Point from;
    Point to;

    Vector3D(Point a, Point b) : from(a), to(b) {}

    Point direction() const {
        return Point(to.x - from.x, to.y - from.y, to.z - from.z);
    }
};
class Camera {
public:
    Point position;
    float direction_horizontal=0; // 0° = direction x; 180=-x     90 and 270=z and -z 
    float direction_vertical=0;

    Camera(Point a) : position(a) {}

    void moveForward(float value) {
        position.x += value;
    }
    void moveSide(float value) {
        position.z += value;
    }
    void moveUpward(float value) {
        position.y += value;
    }
    void updateDirection(float vertical_delta, float horizontal_delta) {
        direction_vertical += vertical_delta;
        direction_horizontal += horizontal_delta;
    }
};
class Rect {
public:
    Point x1, x2, x3, x4;
    Rect(Point x1, Point x2,  Point x3,  Point x4) : x1(x1), x2(x2), x3(x3), x4(x4) {}
    Rect() {}

    Point getScreenTopLeftCorner(Camera camera) {
        // calc shortest ray between a corner
        float dist_x1 = sqrt(pow(camera.position.x-x1.x, 2)+pow(camera.position.y-x1.y, 2)+pow(camera.position.z-x1.z, 2));
        float dist_x2 = sqrt(pow(camera.position.x-x2.x, 2)+pow(camera.position.y-x2.y, 2)+pow(camera.position.z-x2.z, 2));
        float dist_x3 = sqrt(pow(camera.position.x-x3.x, 2)+pow(camera.position.y-x3.y, 2)+pow(camera.position.z-x3.z, 2));
        float dist_x4 = sqrt(pow(camera.position.x-x4.x, 2)+pow(camera.position.y-x4.y, 2)+pow(camera.position.z-x4.z, 2));
        if(dist_x1 < dist_x2) {
            if(dist_x1 < dist_x3) {
                if(dist_x1 < dist_x4) {
                    return x1;
                } else {
                    return x4;
                }
            } else {
                if(dist_x3 < dist_x4) {
                    return x3;
                } else {
                    return x4;
                }
            }
        } else {
            if(dist_x2 < dist_x3) {
                if(dist_x2 < dist_x4) {
                    return x2;
                } else {
                    return x4;
                }
            } else {
                if(dist_x3 < dist_x4) {
                    return x3;
                } else {
                    return x4;
                }                  
            }
        }
    }
};
class Sphere {
public:
    Point center;
    float radius;
    Rect rect;

    Sphere(Point center, float radius) : center(center), radius(radius) {
        rect.x1 = Point(center.x-radius, center.y+radius, center.z-radius);
        rect.x2 = Point(center.x+radius, center.y+radius, center.z-radius);
        rect.x3 = Point(center.x-radius, center.y+radius, center.z+radius);
        rect.x4 = Point(center.x-radius, center.y-radius, center.z-radius);
    }



    bool intersects(const Vector3D& ray, float& t) const {
        Point dir = ray.direction();
        Point oc = Point(ray.from.x - center.x, ray.from.y - center.y, ray.from.z - center.z);

        float a = dir.x * dir.x + dir.y * dir.y + dir.z * dir.z;
        float b = 2.0 * (oc.x * dir.x + oc.y * dir.y + oc.z * dir.z);
        float c = oc.x * oc.x + oc.y * oc.y + oc.z * oc.z - radius * radius;
        float discriminant = b * b - 4 * a * c;

        if (discriminant < 0) {
            return false;
        } else {
            t = (-b - sqrt(discriminant)) / (2.0 * a);
            return true;
        }
    }
};

int main() {
    const float delta = 0.1f; // Adjust this value for sensitivity
    const int render_distance = 100;


    // init texture test
    cv::Mat frame = cv::imread("./assets/texture.jpg", cv::IMREAD_ANYDEPTH);

    // Be sure that the image is loaded
    if (frame.empty())
    {
        printf("No image loaded\n");
        return -1;
    }

    // Be sure that the image is 16bpp and single channel
    // if (frame.type() != CV_16U || frame.channels() != 1)
    // {
    //     printf("Wrong image depth or channels\n");
    //     return -1;
    // }


    
    // Initialize camera
    Camera camera(Point(0, 0, -10));

    vector<Sphere> objects;
    objects.emplace_back(Point(0, 0, 10), 2);

    cv::Mat image(HEIGHT, WIDTH, CV_8UC3);

    int a, b;

    while (true) {

        // Render
        int r, g, b;
        for (int y = 0; y < HEIGHT; ++y) {
            for (int x = 0; x < WIDTH; ++x) {
                Point endPoint(camera.position.x - WIDTH / 2 + x, camera.position.y - HEIGHT / 2 + y, camera.position.z + 1000);
                Vector3D ray(camera.position, endPoint);

                Rect textureRect;
                bool hit = false;
                int distance=5;
                for (const Sphere& object : objects) {
                    float t;
                    if (object.intersects(ray, t)) {
                        distance=sqrt(pow(camera.position.x-object.center.x, 2)+pow(camera.position.y-object.center.y, 2)+pow(camera.position.z-object.center.z, 2));
                        textureRect = object.rect;
                        hit = true;
                        break;
                    }
                }

                if (hit) {
                    // shadow system
                    Point startCorner = textureRect.getScreenTopLeftCorner(camera);
                    cv::Vec3b pixel = frame.at<cv::Vec3b>(y-startCorner.y, x-startCorner.x);
                    float precentage = 1-float(distance)/float(render_distance);
                    if(precentage <= 1 && precentage >= 0) {
                        r = trunc(int(pixel[0])*precentage);
                        g = trunc(int(pixel[1])*precentage);
                        b = trunc(int(pixel[2])*precentage);
                        image.at<cv::Vec3b>(cv::Point(x, y)) = cv::Vec3b(r, g, b);; // Black for sphere
                    }
                    else {
                        image.at<cv::Vec3b>(cv::Point(x, y)) = frame.at<cv::Vec3b>(cv::Point(x,y));
                    }
                } else {
                    image.at<cv::Vec3b>(cv::Point(x, y)) = cv::Vec3b(252, 100, 64); // Sky blue for background
                }
            }
        }

        cv::imshow(window_name, image);
        int key = cv::waitKey(1);
        if (key != -1) {
            // Check for specific keys
            if (key == 27) {  // ESC key
                printf("Finished\n");
                break;
            } else if (key == 'w') {
                camera.moveForward(1);
            } else if (key == 's') {
                camera.moveForward(-1);
            } else if (key == 'a') {
                camera.moveSide(-1);
            } else if (key == 'd') {
                camera.moveSide(1);
            }
            switch (key) {
                case 82: // Up arrow key
                    camera.updateDirection(-delta, 0);
                    break;
                case 84: // Down arrow key
                    camera.updateDirection(delta, 0);
                    break;
                case 81: // Left arrow key
                    camera.updateDirection(0, -delta);
                    break;
                case 83: // Right arrow key
                    camera.updateDirection(0, delta);
                    break;
                default:
                    break;
            }
        }
    }

    return 0;
}