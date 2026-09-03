#include <opencv2/opencv.hpp>
#include <iostream>
#include <chrono>

int main()
{
    // open the default webcam
    cv::VideoCapture cap(0); // Open the default camera

    // check if webcam failed to open
    if (!cap.isOpened())
    { // check if we succeeded
        std::cerr << "Error: Could not open camera." << std::endl;
        return -1;
    }

    int frameCount = 0;
    int currentFPS = 0;
    auto startTime = std::chrono::high_resolution_clock::now();
    auto lastTime = startTime;

    // create a window to display video feed
    const std::string windowName = "Live Webcam";
    cv::namedWindow(windowName, cv::WINDOW_AUTOSIZE);

    // matrix container to hold individual video frames
    cv::Mat frame;

    std::cout << "Live Webcam, press 'Q' to quit." << std::endl;

    while (true)
    {

        // capture frame from webcam
        cap >> frame;

        // check to see if frame is empty
        if (frame.empty())
        {
            std::cerr << "Error: Could not read frame." << std::endl;
            break;
        }

        frameCount++;
        auto now = std::chrono::high_resolution_clock::now();
        double elapsedTime = std::chrono::duration_cast<std::chrono::seconds>(now - startTime).count(); // may have to change lastTime to start

        if (elapsedTime >= 1.0)
        {
            currentFPS = frameCount / elapsedTime;
            frameCount = 0;
            startTime = now;
        }

        std::string fpsText = "FPS: " + std::to_string(currentFPS);
        cv::putText(frame, fpsText, cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 255, 0), 2);

        // display frame
        cv::imshow(windowName, frame); // display the frame in the window
        lastTime = now;

        if (cv::waitKey(30) == 'q')
        { // wait for 'q' key press for 30ms
            std::cout << "Exiting..." << std::endl;
            break;
        }
    }

    cap.release();           // release the webcam
    cv::destroyAllWindows(); // destroy all OpenCV windows

    return 0;
}