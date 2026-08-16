#include <opencv2/opencv.hpp>
#include <iostream>

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

        // display frame
        cv::imshow(windowName, frame); // display the frame in the window

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