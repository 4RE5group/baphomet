// #include <opencv2/opencv.hpp>
#include "opencv2/core.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include <iostream>
#include <vector>
#include <cmath>
#include <stdlib.h>
#include "baphomet.h"
#include <X11/Xlib.h>
#include <X11/extensions/XTest.h>

using namespace std;
using namespace cv;

const string window_name = "BAPHOMET :: 1.0 - 4re5 group";
const int WIDTH = 800;
const int HEIGHT = 600;
const int LOW_WIDTH = 600;  // Lower width for rendering  -> upscaled later
const int LOW_HEIGHT = 400; // Lower height for rendering -> upscaled later
const float focal_length = 1.0;
const float delta = 1.0f;
const Vec3b BORDER_COLOR(0, 0, 0);


// init camera
Baphomet::Camera camera(Baphomet::Point(0, 0, -10));

void setMouseCursorPos(int x, int y) {
    Display* display = XOpenDisplay(nullptr);
    XWarpPointer(display, None, DefaultRootWindow(display), 0, 0, 0, 0, WIDTH / 2, HEIGHT / 2);

    // if (!display) return;
    // XWarpPointer(display, None, DefaultRootWindow(display), 0, 0, 0, 0, x, y);
    XCloseDisplay(display);
}

bool isCursorLocked = false;            // Lock state
cv::Point2i centerPoint(WIDTH / 2, HEIGHT / 2); // Center of the screen
bool firstMove = true;                  // To initialize previous mouse position
void mouseCallback(int event, int x, int y, int, void*) {
    if (!isCursorLocked) return;

    static cv::Point2i prevMousePos(centerPoint.x, centerPoint.y);

    if (firstMove) {
        prevMousePos = cv::Point2i(x, y);
        firstMove = false;
    }

    // Calculate relative movement
    int dx = x - prevMousePos.x;
    int dy = y - prevMousePos.y;

    // Update camera angles based on mouse movement
    camera.direction_horizontal += dx * 0.1f; // Adjust horizontal sensitivity
    camera.direction_vertical -= dy * 0.1f;   // Adjust vertical sensitivity

    // Clamp vertical rotation to avoid flipping
    camera.direction_vertical = std::clamp(camera.direction_vertical, static_cast<int>(-90), static_cast<int>(90));


    // Reset mouse position
    prevMousePos = cv::Point2i(x, y);
    cv::setMouseCallback(window_name, mouseCallback);
    setMouseCursorPos(centerPoint.x, centerPoint.y);
}


int main(int argc, char* argv[]) {
    char* mapPath;
    if(argc >= 2) {
        mapPath = argv[1];
    } else {
        cout << "[x] Usage: " << argv[0] << " <your map path>" << endl;
        return 1;
    }


    cv::Mat image(HEIGHT, WIDTH, CV_8UC3);
    cv::Mat lowResImage(LOW_HEIGHT, LOW_WIDTH, CV_8UC3);
    cv::resize(lowResImage, image, cv::Size(WIDTH, HEIGHT));



    // Baphomet::loadMap(mapPath, &camera);

    Baphomet::Scene scene;
    // scene.addObject(new Baphomet::Plane(Baphomet::Point(-5, 0, 0), Baphomet::Point(-5, 0, 1), Vec3b(255, 0, 0))); // Left Wall
    // scene.addObject(new Baphomet::Plane(Baphomet::Point(5, 0, 0), Baphomet::Point(5, 0, 1), Vec3b(255, 0, 0))); // Right Wall
    // // Create the floor (horizontal plane)
    // scene.addObject(new Baphomet::Plane(Baphomet::Point(-5, -1, 0), Baphomet::Point(5, -1, 0), Vec3b(200, 200, 200))); // Floor
    // // Create the ceiling (horizontal plane)
    // scene.addObject(new Baphomet::Plane(Baphomet::Point(-5, 1, 0), Baphomet::Point(5, 1, 0), Vec3b(200, 200, 200))); // Ceiling
    
    Baphomet::Skybox* skybox = nullptr;

    try {
        skybox = new Baphomet::Skybox(Baphomet::Point(0.0f, 0.0f, 0.0f), Vec3b(255, 255, 255), 100.0f, "./assets/skybox.jpg");
    } catch (const std::exception& e) {
        std::cerr << "Error loading skybox: " << e.what() << std::endl;
        return 1;
    }

    scene.addObject(new Baphomet::Cube(Baphomet::Point(0.0f, 1.0f, 5.0f), 2.0f, Vec3b(255, 0, 0))); // Red Cube
    scene.addObject(new Baphomet::Sphere(Baphomet::Point(2, 2, 5), 1.0f, Vec3b(0, 255, 0))); // Green Sphere
    scene.addObject(new Baphomet::Plane(Baphomet::Point(-5.0f, 0, -5.0f), Baphomet::Point(5.0f, 0.0f, 0.0f), Baphomet::Point(0, 0, 5.0f), Vec3b(255, 255, 255)));
    
    scene.addObject(new Baphomet::Sphere(Baphomet::Point(5.0f, 5.0f, 5.0f), 1.0f, Vec3b(255, 0, 0)));
    Baphomet::Light light(Baphomet::Point(5.0f, 5.0f, 5.0f), Vec3b(255, 0, 255)); // White light
    
    bool debug = false;

    system("clear");
    cout << Baphomet::Baphomet::banner << endl;
    cout << window_name << endl;
    cout << "   >> loaded '" << mapPath << "'" << endl;

    int fov_horizontal = 100;
    int fov_vertical = 100;

    
    float u=0;
    float v=0;
    while(true) {
        if (isCursorLocked) {
            setMouseCursorPos(centerPoint.x, centerPoint.y);
        }
        #pragma omp parallel for
        for (int y = 0; y < LOW_HEIGHT; ++y) {
            for (int x = 0; x < LOW_WIDTH; ++x) {
                u = (x - LOW_WIDTH / 2.0f) / LOW_WIDTH;
                v = (y - LOW_HEIGHT / 2.0f) / LOW_HEIGHT;

                float h_rad = (camera.direction_horizontal + u * fov_horizontal) * M_PI / 180.0f;
                float v_rad = (camera.direction_vertical + v * fov_vertical) * M_PI / 180.0f;

                Baphomet::Point direction(
                    cos(v_rad) * sin(h_rad),
                    sin(v_rad),
                    cos(v_rad) * cos(h_rad)
                );

                direction = Baphomet::normalize(direction);
                Baphomet::Ray ray(camera.position, direction);

                Vec3b pixelColor(0, 0, 0);
                float t_min = numeric_limits<float>::max();
                const Baphomet::Object* hitObject = nullptr;
                bool hit = false;

                for (const auto& object : scene.objects) {
                    float t = 0;
                    if (object->rayIntersect(ray, t) && t < t_min) {
                        hit = true;
                        t_min = t;
                        hitObject = object;
                        Baphomet::Point intersectionPoint = ray.origin + t * ray.direction;

                        // Shadow check
                        if (isInShadow(intersectionPoint, light, scene, hitObject)) {
                            pixelColor = pixelColor * 0.5; // Dim color to simulate shadow
                        } else {
                            pixelColor = object->color; // Object color if lit
                        }
                    }
                }

                if (hit) {
                    
                } else {
                    if (skybox) {
                        // Use skybox color when no object is hit
                        pixelColor = skybox->getColor(ray);
                    }
                }
                lowResImage.at<Vec3b>(Point(x, LOW_HEIGHT - 1 - y)) = pixelColor;
                
            }
        }
        cv::resize(lowResImage, image, cv::Size(WIDTH, HEIGHT));
        if (debug) {
            int debug_menu_x = WIDTH - 200;
            cv::putText(image, "x, y, z", cv::Point(debug_menu_x, 10), cv::FONT_HERSHEY_COMPLEX, 0.5, cv::Scalar(255, 255, 255), 1);
            cv::putText(image, Baphomet::toStringWithPrecision(camera.position.x, 2) + ", " + Baphomet::toStringWithPrecision(camera.position.y, 2) + ", " + Baphomet::toStringWithPrecision(camera.position.z, 2), cv::Point(debug_menu_x, 30), cv::FONT_HERSHEY_COMPLEX, 0.5, cv::Scalar(255, 255, 255), 1);
            cv::putText(image, "rot: x, y", cv::Point(debug_menu_x, 50), cv::FONT_HERSHEY_COMPLEX, 0.5, cv::Scalar(255, 255, 255), 1);
            cv::putText(image, Baphomet::toStringWithPrecision(camera.direction_horizontal, 2) + ", " + Baphomet::toStringWithPrecision(camera.direction_vertical, 2), cv::Point(debug_menu_x, 70), cv::FONT_HERSHEY_COMPLEX, 0.5, cv::Scalar(255, 255, 255), 1);
            Baphomet::drawOrientationArrows(image, camera);
        }

        
        imshow(window_name, image);
        // init callback for movements
        // setMouseCallback(window_name, mouseCallback);
        
        int key = cv::waitKey(1);
        // printf("%d", key);
        if (key != -1) {
            if (key == 27) {
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
            } else if (key == 'h') {
                debug = !debug;
            } else if (key == 'l') {
                cout << "Enter x,y,z of light" << endl;
                light.position.x-=1;
            } else if(key == 'm') {
                isCursorLocked = !isCursorLocked;
                firstMove = true; // Reset for smooth transition
                cv::setMouseCallback(window_name, isCursorLocked ? mouseCallback : nullptr);
                cout << (isCursorLocked ? "Cursor Locked" : "Cursor Unlocked") << endl;
            } else if (key == 'r') {
                camera.position = Baphomet::Point(0, 0, -10);
                camera.direction_horizontal = 0;
                camera.direction_vertical = 0;
            } 
            switch (key) {
                case 32: // space
                    camera.moveUpward(1);
                    break;
                case 82: 
                    camera.updateDirection(0, delta);
                    break;
                case 84: 
                    camera.updateDirection(0, -delta);
                    break;
                case 81: 
                    camera.updateDirection(-delta, 0);
                    break;
                case 83: 
                    camera.updateDirection(delta, 0);
                    break;
                default:
                    break;
            }
        }
    }

    return 0;
}
