// Project_phcn.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "FireDetector.h"

int main() {
    VideoCapture cap(0);
    if (!cap.isOpened()) {
        cout << "Khong mo duoc camera!" << endl;
        return -1;
    }

    HOGDescriptor hog;
    hog.setSVMDetector(HOGDescriptor::getDefaultPeopleDetector());

    Mat frame, prevFrame;
    string windowName = "HE THONG GIAM SAT CHAY NO - NHOM_XLAS";
    namedWindow(windowName, WINDOW_AUTOSIZE);

    // Dang ky su kien chuot
    setMouseCallback(windowName, onMouseClick, NULL);

    while (true) {
        cap >> frame;
        if (frame.empty()) break;

        resize(frame, frame, Size(640, 480));

        if (prevFrame.empty()) {
            prevFrame = frame.clone();
            continue;
        }

        bool hasPerson = detectPerson(frame, hog);

        // Logic tu dong bat/tat theo nguoi
        if (hasPerson && isSystemEnabled) {
            isSystemEnabled = false;
        }
        else if (!hasPerson && !isSystemEnabled && !isAfterShiftMode) {
            isSystemEnabled = true;
        }

        FireConfig currentMode = hasPerson ? kitchenMode : normalMode;

        bool fireTooClose = false;
        bool fire = false;
        bool smoke = false;

        if (isSystemEnabled || isAfterShiftMode) {
            fire = detectFireMode(frame, prevFrame, currentMode, fireTooClose);
            smoke = detectSmoke(frame);
        }

        // Logic canh bao chuong hu coi / hien thi text
        if (fireTooClose) {
            rectangle(frame, Rect(0, 0, frame.cols, frame.rows), Scalar(0, 0, 255), 10);
            putText(frame, "NGUY HIEM! LUA QUA GAN!", Point(240, 50), FONT_HERSHEY_SIMPLEX, 0.7, Scalar(0, 0, 255), 2);
            system("beep");
        }
        else if (isAfterShiftMode) {
            if (fire) {
                putText(frame, "TAN CA: CHAY NGUY HIEM!", Point(240, 50), FONT_HERSHEY_SIMPLEX, 0.7, Scalar(0, 0, 255), 2);
                system("beep");
            }
            else if (smoke) {
                putText(frame, "TAN CA: CANH BAO KHOI!", Point(240, 50), FONT_HERSHEY_SIMPLEX, 0.7, Scalar(0, 0, 255), 2);
                system("beep");
            }
        }
        else if (isSystemEnabled) {
            if (fire) {
                putText(frame, "PHAT HIEN CHAY!", Point(240, 50), FONT_HERSHEY_SIMPLEX, 0.7, Scalar(0, 0, 255), 2);
                system("beep");
            }
            else if (smoke) {
                putText(frame, "PHAT HIEN KHOI!", Point(240, 110), FONT_HERSHEY_SIMPLEX, 0.7, Scalar(200, 200, 200), 2);
            }
        }
        else {
            if (fire) {
                putText(frame, "NHAT KY: GIAM SAT HOA HOAN...", Point(240, 50), FONT_HERSHEY_SIMPLEX, 0.7, Scalar(0, 255, 255), 2);
            }
            if (smoke) {
                putText(frame, "NHAT KY: GIAM SAT KHOI...", Point(240, 110), FONT_HERSHEY_SIMPLEX, 0.7, Scalar(200, 200, 200), 2);
            }
        }

        // Ve UI va hien thi
        drawAdvancedUI(frame, hasPerson, fire, smoke, fireTooClose);
        imshow(windowName, frame);

        prevFrame = frame.clone();

        if (waitKey(30) == 27) break;
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
