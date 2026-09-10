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
    // auto lastTime = startTime;

    // create a window to display video feed
    const std::string windowName = "Live Webcam";
    cv::namedWindow(windowName, cv::WINDOW_AUTOSIZE);

    // matrix container to hold individual video frames
    cv::Mat frame, grayFrame, fgMask, threshFrame;
    // cv::Mat prevGray, diffFrame; // for absdiff motion detection

    // new MOG2 background subtractor object
    cv::Ptr<cv::BackgroundSubtractor> pBackSub = cv::createBackgroundSubtractorMOG2(500, 20, true);

    std::cout << "Live Webcam V_MOG2, press 'Q' to quit." << std::endl;

    /*
    ###################################################################################
                                    absdiff logic
    ###################################################################################
    cap >> frame;
    cv::cvtColor(frame, prevGray, cv::COLOR_BGR2GRAY);         // convert to grayscale
    cv::GaussianBlur(prevGray, prevGray, cv::Size(21, 21), 0); // apply Gaussian blur
    ###################################################################################
    */

    // --- Persistance & Cooldown Tracking ---
    int consectutiveMotionFrames = 0;
    const int motionThreshold = 3; // number of consecutive frames with motion to trigger an event
    auto lastMotionTime = std::chrono::high_resolution_clock::now();
    const double cooldownPeriod = 3.0; // seconds

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

        // --- MOG2 MOTION DETECTION LOGIC ---
        cv::cvtColor(frame, grayFrame, cv::COLOR_BGR2GRAY);          // convert to grayscale
        cv::GaussianBlur(grayFrame, grayFrame, cv::Size(21, 21), 0); // apply Gaussian blur to reduce noise and improve motion detection
        pBackSub->apply(grayFrame, fgMask, 0.01);                    // apply background subtraction

        cv::threshold(fgMask, threshFrame, 200, 255, cv::THRESH_BINARY);
        cv::dilate(threshFrame, threshFrame, cv::Mat(), cv::Point(-1, -1), 4);
        cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(15, 15));
        cv::morphologyEx(threshFrame, threshFrame, cv::MORPH_CLOSE, kernel);

        /*
        // --- ABS Diff MOTION DETECTION LOGIC ---
        cv::cvtColor(frame, grayFrame, cv::COLOR_BGR2GRAY); // convert to grayscale
        cv::GaussianBlur(grayFrame, grayFrame, cv::Size(21, 21), 0); // apply Gaussian blur to reduce noise and improve motion detection
        cv::absdiff(prevGray, grayFrame, diffFrame);


        cv::threshold(diffFrame, threshFrame, 25, 255, cv::THRESH_BINARY);
        cv::dilate(threshFrame, threshFrame, cv::Mat(), cv::Point(-1, -1), 2);
        */

        std::vector<std::vector<cv::Point>> contours;
        cv::findContours(threshFrame, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

        bool motionDetected = false;
        cv::Rect combinedBoundingBox;
        bool firstBox = true;

        for (const auto &contour : contours)
        {
            if (cv::contourArea(contour) > 500) // filter out small movements
            {
                motionDetected = true;
                cv::Rect boundingBox = cv::boundingRect(contour);
                // cv::rectangle(frame, boundingBox, cv::Scalar(0, 0, 255), 2);
                if (firstBox)
                {
                    combinedBoundingBox = boundingBox;
                    firstBox = false;
                }
                else
                {
                    combinedBoundingBox |= boundingBox; // combine bounding boxes
                }
            }
        }

        if (motionDetected)
        {
            // std::cout << "[" << getCurrentTimestamp() << "] Motion detected!" << std::endl;
            consectutiveMotionFrames++;
        }
        else
        {
            consectutiveMotionFrames = 0;
        }

        bool isMotion = consectutiveMotionFrames >= motionThreshold;

        if (isMotion)
        {
            cv::rectangle(frame, combinedBoundingBox, cv::Scalar(0, 0, 255), 2);

            double timeSinceLastMotion = std::chrono::duration<double>(now - lastMotionTime).count();
            if (timeSinceLastMotion >= cooldownPeriod)
            {
                std::cout << "[" << getCurrentTimestamp() << "] Motion detected!" << std::endl;
                lastMotionTime = now;
            }
        }
        // prevGray = grayFrame.clone(); // update previous frame for next iteration --- absdiff logic

        std::string fpsText = "FPS: " + std::to_string(currentFPS);
        cv::putText(frame, fpsText, cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 255, 0), 2);

        // display frame
        cv::imshow(windowName, frame); // display the frame in the window
        // lastTime = now;

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