#include "FireDetector.h"

// Định nghĩa giá trị cho các biến toàn cục
FireConfig normalMode = { Scalar(10, 150, 150), Scalar(25, 255, 255), 1000, 180, 5000 };
FireConfig kitchenMode = { Scalar(5, 120, 120), Scalar(35, 255, 255), 3000, 160, 8000 };

bool isSystemEnabled = true;
bool isAfterShiftMode = false;

Rect btnToggleSystem(15, 340, 190, 35);
Rect btnToggleShift(15, 390, 190, 35);

void onMouseClick(int event, int x, int y, int flags, void* userdata) {
    if (event == EVENT_LBUTTONDOWN) {
        if (btnToggleSystem.contains(Point(x, y))) {
            isSystemEnabled = !isSystemEnabled;
        }
        if (btnToggleShift.contains(Point(x, y))) {
            isAfterShiftMode = !isAfterShiftMode;
        }
    }
}

void drawAdvancedUI(Mat& canvas, bool hasPerson, bool fire, bool smoke, bool fireTooClose) {
    Mat roi = canvas(Rect(0, 0, 220, canvas.rows));
    Mat color(roi.size(), CV_8UC3, Scalar(35, 35, 35));
    double alpha = 0.85;
    addWeighted(color, alpha, roi, 1.0 - alpha, 0.0, roi);

    line(canvas, Point(220, 0), Point(220, canvas.rows), Scalar(100, 100, 100), 1);
    putText(canvas, "CONTROL PANEL v2.0", Point(15, 25), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(255, 255, 255), 2);
    line(canvas, Point(10, 35), Point(210, 35), Scalar(150, 150, 150), 1);

    putText(canvas, "TRANG THAI BOI CANH:", Point(15, 60), FONT_HERSHEY_SIMPLEX, 0.4, Scalar(200, 200, 200), 1);
    if (hasPerson) {
        putText(canvas, "CHE DO BEP", Point(15, 80), FONT_HERSHEY_SIMPLEX, 0.6, Scalar(0, 255, 255), 2);
    }
    else {
        putText(canvas, "CHE DO NORMAL", Point(15, 80), FONT_HERSHEY_SIMPLEX, 0.6, Scalar(255, 255, 255), 2);
    }

    int startY = 130;
    int spacing = 35;

    circle(canvas, Point(25, startY), 8, hasPerson ? Scalar(255, 0, 0) : Scalar(70, 70, 70), -1);
    putText(canvas, hasPerson ? "PERSON: INSIDE" : "PERSON: NONE", Point(45, startY + 5), FONT_HERSHEY_SIMPLEX, 0.45, Scalar(255, 255, 255), 1);

    circle(canvas, Point(25, startY + spacing), 8, smoke ? Scalar(180, 180, 180) : Scalar(70, 70, 70), -1);
    putText(canvas, smoke ? "SMOKE: ALERT!" : "SMOKE: CLEAR", Point(45, startY + spacing + 5), FONT_HERSHEY_SIMPLEX, 0.45, Scalar(255, 255, 255), 1);

    Scalar fireLed = Scalar(70, 70, 70);
    if (fireTooClose) fireLed = Scalar(0, 0, 255);
    else if (fire) fireLed = Scalar(0, 145, 255);
    circle(canvas, Point(25, startY + 2 * spacing), 8, fireLed, -1);
    putText(canvas, fire ? "FIRE: DETECTED" : "FIRE: SAFE", Point(45, startY + 2 * spacing + 5), FONT_HERSHEY_SIMPLEX, 0.45, Scalar(255, 255, 255), 1);

    line(canvas, Point(10, 310), Point(210, 310), Scalar(150, 150, 150), 1);
    putText(canvas, "INTERACTIVE COMMANDS:", Point(15, 325), FONT_HERSHEY_SIMPLEX, 0.4, Scalar(200, 200, 200), 1);

    Scalar btn1Color = isSystemEnabled ? Scalar(0, 180, 0) : Scalar(0, 0, 180);
    rectangle(canvas, btnToggleSystem, btn1Color, -1);
    rectangle(canvas, btnToggleSystem, Scalar(255, 255, 255), 1);
    string btn1Text = isSystemEnabled ? "SYS: ON (AUTO)" : "SYS: FORCE OFF";
    putText(canvas, btn1Text, Point(25, 362), FONT_HERSHEY_SIMPLEX, 0.45, Scalar(255, 255, 255), 1, LINE_AA);

    Scalar btn2Color = isAfterShiftMode ? Scalar(0, 0, 200) : Scalar(150, 100, 0);
    rectangle(canvas, btnToggleShift, btn2Color, -1);
    rectangle(canvas, btnToggleShift, Scalar(255, 255, 255), 1);
    string btn2Text = isAfterShiftMode ? "MODE: AFTER SHIFT" : "MODE: ON DUTY";
    putText(canvas, btn2Text, Point(25, 412), FONT_HERSHEY_SIMPLEX, 0.42, Scalar(255, 255, 255), 1, LINE_AA);

    line(canvas, Point(10, canvas.rows - 45), Point(210, canvas.rows - 45), Scalar(150, 150, 150), 1);
    if (!isSystemEnabled) {
        putText(canvas, "SYSTEM DISABLED BY USER", Point(15, canvas.rows - 20), FONT_HERSHEY_SIMPLEX, 0.42, Scalar(0, 0, 255), 1);
    }
    else if (isAfterShiftMode) {
        putText(canvas, "HIGH SECURITY ACTIVE", Point(15, canvas.rows - 20), FONT_HERSHEY_SIMPLEX, 0.42, Scalar(0, 255, 255), 1);
    }
    else {
        putText(canvas, "MONITORING SYSTEM RUNNING", Point(15, canvas.rows - 20), FONT_HERSHEY_SIMPLEX, 0.41, Scalar(0, 255, 0), 1);
    }
}

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

            double ratio = area / (frame.rows * frame.cols);
            if (ratio > 0.15) {
                fireTooClose = true;
            }

            Mat roiHSV = hsv(safeBox);
            Scalar meanVal = mean(roiHSV);

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

bool detectPerson(Mat& frame, HOGDescriptor& hog) {
    vector<Rect> people;
    hog.detectMultiScale(frame, people);
    for (auto p : people) {
        rectangle(frame, p, Scalar(255, 0, 0), 2);
    }
    return !people.empty();
}

bool detectSmoke(Mat& frame) {
    Mat hsv, mask;
    cvtColor(frame, hsv, COLOR_BGR2HSV);

    Scalar lower(0, 0, 80);
    Scalar upper(180, 60, 200);

    inRange(hsv, lower, upper, mask);

    int whitePixels = countNonZero(mask);
    double ratio = (double)whitePixels / (frame.rows * frame.cols);

    return ratio > 0.1;
}