// Used for testing on the command line only

#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/core/utils/logger.hpp>
#include "ParticleCounter.h"

int main()
{
    cv::utils::logging::setLogLevel(cv::utils::logging::LOG_LEVEL_SILENT);
    std::cout << "Hello World!\n";

    // Read the image file 
    cv::Mat image = cv::imread("E:\\Stack000.tif", cv::IMREAD_ANYDEPTH | cv::IMREAD_GRAYSCALE);
    std::cout << "Channels: " << image.channels() << std::endl;
    cv::imshow("Raw data", image*10);

    // Check for failure 
    if (image.empty())
    {
        std::cout << "Image Not Found!!!" << std::endl;
        std::cin.get(); //wait for any key press 
        return -1;
    }
    if (image.type() != CV_16U)
    {
        std::cout << "Incorrect data type" << std::endl;
        std::cin.get(); //wait for any key press 
        return -1;
    }

    // Show our image inside a window. 
    //imshow("plate diagram", image);

    //image.convertTo(image, CV_32F); // Convert to float matrix
    std::cout << "PARTICLES: " << countParticlesInternal(image);

    // Wait for any keystroke in the window 
    cv::waitKey(0);
    return 0;
}
