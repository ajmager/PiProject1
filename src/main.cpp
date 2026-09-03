#include <opencv2/opencv.hpp>
#include <iostream>
#include <chrono>
#include <ctime>

std::string getCurrentTimestamp()
{
    auto now = std::chrono::system_clock::now();
    std::time_t nowTime = std::chrono::system_clock::to_time_t(now);
    char buffer[20];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", std::localtime(&nowTime));
    return std::string(buffer);
}

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
    cv::Mat frame, grayFrame, prevGray, diffFrame, threshFrame;

    std::cout << "Live Webcam, press 'Q' to quit." << std::endl;
    cap >> frame;
    cv::cvtColor(frame, prevGray, cv::COLOR_BGR2GRAY);         // convert to grayscale
    cv::GaussianBlur(prevGray, prevGray, cv::Size(21, 21), 0); // apply Gaussian blur

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
        double elapsedTime = std::chrono::duration_cast<std::chrono::seconds>(now - startTime).count();

        if (elapsedTime >= 1.0)
        {
            currentFPS = static_cast<int>(frameCount / elapsedTime);
            frameCount = 0;
            startTime = now;
        }

        // --- MOTION DETECTION LOGIC ---
        cv::cvtColor(frame, grayFrame, cv::COLOR_BGR2GRAY);
        cv::GaussianBlur(grayFrame, grayFrame, cv::Size(21, 21), 0);
        cv::absdiff(prevGray, grayFrame, diffFrame);
        cv::threshold(diffFrame, threshFrame, 25, 255, cv::THRESH_BINARY);
        cv::dilate(threshFrame, threshFrame, cv::Mat(), cv::Point(-1, -1), 2);

        std::vector<std::vector<cv::Point>> contours;
        cv::findContours(threshFrame, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

        bool motionDetected = false;
        for (const auto &contour : contours)
        {
            if (cv::contourArea(contour) > 500) // filter out small movements
            {
                motionDetected = true;
                cv::Rect boundingBox = cv::boundingRect(contour);
                cv::rectangle(frame, boundingBox, cv::Scalar(0, 0, 255), 2);
            }
        }

        if (motionDetected)
        {
            std::cout << "[" << getCurrentTimestamp() << "] Motion detected!" << std::endl;
        }

        prevGray = grayFrame.clone(); // update previous frame for next iteration

        std::string fpsText = "FPS: " + std::to_string(currentFPS);
        cv::putText(frame, fpsText, cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 255, 0), 2);

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