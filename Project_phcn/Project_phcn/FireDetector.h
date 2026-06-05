#pragma once
#ifndef FIRE_DETECTOR_H
#define FIRE_DETECTOR_H

#include <iostream>
#include <opencv2/opencv.hpp>
#include <vector>

using namespace std;
using namespace cv;

// ??nh ngh?a c?u hình c?u trúc d? li?u
struct FireConfig {
    Scalar lower;
    Scalar upper;
    int minArea;
    int minBrightness;
    double minMotion;
};

// Khai báo các bi?n c?u hình (extern ?? các file khác có th? dùng chung)
extern FireConfig normalMode;
extern FireConfig kitchenMode;

extern bool isSystemEnabled;
extern bool isAfterShiftMode;

extern Rect btnToggleSystem;
extern Rect btnToggleShift;

// Khai báo các hàm x? lý
void onMouseClick(int event, int x, int y, int flags, void* userdata);
void drawAdvancedUI(Mat& canvas, bool hasPerson, bool fire, bool smoke, bool fireTooClose);
bool detectFireMode(Mat& frame, Mat& prevFrame, FireConfig cfg, bool& fireTooClose);
bool detectPerson(Mat& frame, HOGDescriptor& hog);
bool detectSmoke(Mat& frame);

#endif // FIRE_DETECTOR_H