// Project_chay_no.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

struct FireConfig {
    Scalar lower;
    Scalar upper;
    int minArea;
    int minBrightness;
    double minMotion;
};

FireConfig normalMode = {
    Scalar(10,150,150),
    Scalar(25,255,255),
    1000,
    180,
    5000
};

FireConfig kitchenMode = {
    Scalar(5,120,120),
    Scalar(35,255,255),
    3000,
    160,
    8000
};

// hàm detect cháy
bool detectFireMode(Mat& frame, Mat& prevFrame, FireConfig cfg, bool& fireTooClose) {
    Mat hsv, mask, diff, grayDiff;

    fireTooClose = false;

    absdiff(prevFrame, frame, diff);
    cvtColor(diff, grayDiff, COLOR_BGR2GRAY);
    threshold(grayDiff, grayDiff, 25, 255, THRESH_BINARY);

    cvtColor(frame, hsv, COLOR_BGR2HSV);
    inRange(hsv, cfg.lower, cfg.upper, mask);

    erode(mask, mask, Mat(), Point(-1, -1), 2);
    dilate(mask, mask, Mat(), Point(-1, -1), 2);

    vector<vector<Point>> contours;
    findContours(mask, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

    bool fireDetected = false;

    for (auto c : contours) {
        double area = contourArea(c);

        if (area > cfg.minArea) {
            Rect box = boundingRect(c);
            Rect safeBox = box & Rect(0, 0, frame.cols, frame.rows);

            // 🔥 TÍNH TỶ LỆ DIỆN TÍCH
            double ratio = area / (frame.rows * frame.cols);

            if (ratio > 0.15) {
                fireTooClose = true;
            }

            Mat roi = frame(safeBox);
            Scalar meanVal = mean(roi);

            Mat motionROI = grayDiff(safeBox);
            double motion = sum(motionROI)[0];

            if (meanVal[2] > cfg.minBrightness && motion > cfg.minMotion) {
                fireDetected = true;
                rectangle(frame, box, Scalar(0, 0, 255), 2);
            }
        }
    }

    return fireDetected;
}

//Hàm detect người (ON/OFF system)
bool detectPerson(Mat& frame, HOGDescriptor& hog) {
    vector<Rect> people;
    hog.detectMultiScale(frame, people);

    for (auto p : people) {
        rectangle(frame, p, Scalar(255, 0, 0), 2);
    }

    return !people.empty();
}

// phát hiện khói
bool detectSmoke(Mat& frame) {
    Mat hsv, mask;
    cvtColor(frame, hsv, COLOR_BGR2HSV);

    // khói: S thấp, V trung bình
    Scalar lower(0, 0, 80);
    Scalar upper(180, 60, 200);

    inRange(hsv, lower, upper, mask);

    int whitePixels = countNonZero(mask);
    double ratio = (double)whitePixels / (frame.rows * frame.cols);

    return ratio > 0.1; // 10% ảnh là khói
}


//main
int main() {
    VideoCapture cap(0);

    if (!cap.isOpened()) {
        cout << "Không mở được camera!" << endl;
        return -1;
    }

    HOGDescriptor hog;
    hog.setSVMDetector(HOGDescriptor::getDefaultPeopleDetector());

    Mat frame, prevFrame;

    while (true) {
        cap >> frame;
        if (frame.empty()) break;

        if (prevFrame.empty()) {
            prevFrame = frame.clone();
        }

        bool hasPerson = detectPerson(frame, hog);

        FireConfig currentMode;

        // 👉 Logic chuyển mode
        if (hasPerson) {
            currentMode = kitchenMode;
            putText(frame, "KITCHEN MODE", Point(50, 80),
                FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 255, 255), 2);
        }
        else {
            currentMode = normalMode;
            putText(frame, "NORMAL MODE", Point(50, 80),
                FONT_HERSHEY_SIMPLEX, 1, Scalar(255, 255, 255), 2);
        }

        bool fireTooClose = false;

        bool fire = detectFireMode(frame, prevFrame, currentMode, fireTooClose);

        // 🔥 ƯU TIÊN CAO NHẤT
        if (fireTooClose) {
             // 🔴 viền đỏ toàn màn hình
             rectangle(frame, Rect(0, 0, frame.cols, frame.rows),
                 Scalar(0, 0, 255), 10);
            
            putText(frame, "DANGER! FIRE TOO CLOSE!", Point(50, 50),
                FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 0, 255), 3);
            system("beep"); // 🔊 phát âm thanh
        }
        else if (hasPerson) {
            putText(frame, "SYSTEM OFF (PERSON)", Point(50, 50),
                FONT_HERSHEY_SIMPLEX, 1, Scalar(255, 0, 0), 2);
        }
        else {
            if (fire) {
                putText(frame, "FIRE DETECTED!", Point(50, 50),
                    FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 0, 255), 2);
                system("beep"); // 🔊 phát âm thanh
            }
            else {
                putText(frame, "NO FIRE", Point(50, 50),
                    FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 255, 0), 2);
            }
    bool smoke = detectSmoke(frame);

        if (smoke) {
           putText(frame, "SMOKE DETECTED!", Point(50, 120),
               FONT_HERSHEY_SIMPLEX, 1, Scalar(200, 200, 200), 2);
            }   
        }
    }
    cap.release();
    destroyAllWindows();

    return 0;
}
// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
