#pragma once

#include <opencv2/opencv.hpp>

uint32_t countParticlesInternal(const cv::Mat& image_t);

extern "C" __declspec(dllexport) uint32_t countParticles(uint16_t* data, const uint32_t rows, const uint32_t cols);

extern "C" __declspec(dllexport) uint32_t init();
