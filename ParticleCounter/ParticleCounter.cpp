//

#include "ParticleCounter.h"

#include <opencv2/opencv.hpp>
#include <vector>
#include <algorithm>
#include <numeric>
#include <ctype.h>  //uint16_t
#include <iostream>
#include <fstream>


// Standard deviation of a matrix
double stdDev(const cv::Mat& mat) {
    cv::Scalar mean, stddev;
    cv::meanStdDev(mat, mean, stddev);
    return stddev[0];
}


// LabVIEW entry point, requiring conversion of data to cv::Mat
uint32_t countParticles(uint16_t* data, const uint32_t rows, const uint32_t cols)
{
    cv::Mat input(rows, cols, CV_16U, data);
    return countParticlesInternal(input);  // .clone());
}



// Counts the number of intensity maxima above an automatically calculated threshold.
// Designed to exactly reproduce a very old algorithm in SPARTAN.
// Translated from SPARTAN's movieBG and getPeaks functions.
uint32_t countParticlesInternal(const cv::Mat& image_t)
{
    const int den = 6;  //downsampling factor
    const int partition = static_cast<int>(std::floor(0.167 * den * den)); // Index of sorted pixel to use

    //std::ofstream log;
    //log.open("C:\\temp\\ParticleCounter.txt", std::ios::out | std::ios::trunc);

    cv::Size szField = image_t.size();
    cv::Size tempSize(szField.width / den, szField.height / den);
    cv::Mat temp = cv::Mat::zeros(image_t.rows/den, image_t.cols/den, CV_16U);
    
    // Divide image into den-x-den squares and find 16% lowest value in each
    for (int i = 0; i < temp.rows; ++i)
    {
        for (int j = 0; j < temp.cols; ++j)
        {
            //Extract square from original image
            cv::Rect roi = cv::Rect( den*j, den*i, den, den);
            cv::Mat window = image_t(roi);

            //Find the partition value
            cv::Mat sorted;
            window.copyTo(sorted);
            sorted.reshape(1, 1);  //makes a row vector
            cv::sort(sorted, sorted, cv::SORT_ASCENDING);
            temp.at<uint16_t>(i,j) = sorted.at<uint16_t>(partition);
        }
    }

    //log << "Downsample.\n";

    // Resize the temp image back to the original size and subtract
    cv::Mat resizedTemp;
    cv::resize(temp, resizedTemp, szField, 0, 0, cv::INTER_CUBIC);
    cv::Mat subtractedImage = image_t - resizedTemp;

    //log << "Interpolate and subtract.\n";

    // Calculate threshold for selecting particles above baseline
    cv::Mat sortedImage;
    subtractedImage.copyTo(sortedImage);
    sortedImage = sortedImage.reshape(1, 1);
    cv::sort(sortedImage, sortedImage, cv::SORT_ASCENDING);
    int newLength = sortedImage.total() * 0.75;
    cv::Mat truncated = sortedImage(cv::Range::all(), cv::Range(0,newLength));
    float threshold = 12 * stdDev(truncated);
    //std::cout << "THRESHOLD: " << threshold << std::endl;

    //log << "THRESHOLD: " << threshold << std::endl;

    // Count spots above threshold.
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_CROSS, cv::Size(3, 3));
    cv::Mat maxima;
    cv::dilate(subtractedImage, maxima, kernel);
    maxima = (maxima == subtractedImage) & (subtractedImage > threshold);

    //log << "imregionalmax.\n";

    return cv::countNonZero(maxima);
}


// Dummy function to force OpenCV startup on command
extern "C" __declspec(dllexport) uint32_t init()
{
    cv::Mat temp = cv::Mat::zeros(100, 100, CV_16U);
    return temp.at<uint16_t>(1, 1);
}


