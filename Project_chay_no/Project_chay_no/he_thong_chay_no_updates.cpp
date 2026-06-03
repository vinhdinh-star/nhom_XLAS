// Project_chay_no.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <opencv2/opencv.hpp>
#include <vector>

using namespace std;
using namespace cv;

struct FireConfig {
    Scalar lower;
    Scalar upper;
    int minArea;
    int minBrightness;
    double minMotion;
};

FireConfig normalMode = { Scalar(10, 150, 150), Scalar(25, 255, 255), 1000, 180, 5000 };
FireConfig kitchenMode = { Scalar(5, 120, 120), Scalar(35, 255, 255), 3000, 160, 8000 };

// CÁC BIẾN CỜ ĐIỀU KHIỂN HỆ THỐNG (Biến toàn cục phục vụ Mouse Callback)
bool isSystemEnabled = true;       // Trạng thái hệ thống: BẬT (khi không người) / TẮT (khi có người)
bool isAfterShiftMode = false;     // Trạng thái ca làm việc: TRUE (Tan ca) / FALSE (Có người trực)

// Định nghĩa vùng không gian tọa độ vật lý của các nút bấm trên UI
Rect btnToggleSystem(15, 340, 190, 35);
Rect btnToggleShift(15, 390, 190, 35);

// Hàm xử lý sự kiện Click chuột trên Giao diện UI
void onMouseClick(int event, int x, int y, int flags, void* userdata) {
    if (event == EVENT_LBUTTONDOWN) {
        // Kiểm tra nếu click vào Nút 1 (Bật/Tắt hệ thống theo sự xuất hiện của người)
        if (btnToggleSystem.contains(Point(x, y))) {
            isSystemEnabled = !isSystemEnabled;
        }
        // Kiểm tra nếu click vào Nút 2 (Tan ca làm việc / Có người trực)
        if (btnToggleShift.contains(Point(x, y))) {
            isAfterShiftMode = !isAfterShiftMode;
        }
    }
}

// Hàm vẽ Giao diện điều khiển Dashboard OSD nâng cao
void drawAdvancedUI(Mat& canvas, bool hasPerson, bool fire, bool smoke, bool fireTooClose) {
    // 1. Vẽ dải nền Dashboard mờ phía bên trái (Rộng 220px)
    Mat roi = canvas(Rect(0, 0, 220, canvas.rows));
    Mat color(roi.size(), CV_8UC3, Scalar(35, 35, 35));
    double alpha = 0.85;
    addWeighted(color, alpha, roi, 1.0 - alpha, 0.0, roi);

    // Vẽ đường biên ngăn cách vùng Dashboard và vùng Video
    line(canvas, Point(220, 0), Point(220, canvas.rows), Scalar(100, 100, 100), 1);

    // 2. Tiêu đề Phần mềm
    putText(canvas, "BANG DIEU KHIEN v2.0", Point(15, 25), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(255, 255, 255), 2);
    line(canvas, Point(10, 35), Point(210, 35), Scalar(150, 150, 150), 1);

    // 3. Hiển thị logic trạng thái Mode tự động theo người
    putText(canvas, "TRANG THAI BOI CANH:", Point(15, 60), FONT_HERSHEY_SIMPLEX, 0.4, Scalar(200, 200, 200), 1);
    if (hasPerson) {
        putText(canvas, "CHE DO BEP", Point(15, 80), FONT_HERSHEY_SIMPLEX, 0.6, Scalar(0, 255, 255), 2);
    }
    else {
        putText(canvas, "CHE DO NOMARL", Point(15, 80), FONT_HERSHEY_SIMPLEX, 0.6, Scalar(255, 255, 255), 2);
    }

    // 4. Khối đèn LED hiển thị trạng thái cảm biến vật lý
    int startY = 130;
    int spacing = 35;

    // LED Con người
    circle(canvas, Point(25, startY), 8, hasPerson ? Scalar(255, 0, 0) : Scalar(70, 70, 70), -1);
    //PERSON: INSIDE" : "PERSON: NONE
    putText(canvas, hasPerson ? "NGUOI: BEN TRONG" : "NGUOI: KHONG CO", Point(45, startY + 5), FONT_HERSHEY_SIMPLEX, 0.45, Scalar(255, 255, 255), 1);

    // LED Khói
    circle(canvas, Point(25, startY + spacing), 8, smoke ? Scalar(180, 180, 180) : Scalar(70, 70, 70), -1);
    putText(canvas, smoke ? "KHOI: CANH BAO!" : "KHOI: KHONG CO KHOI", Point(45, startY + spacing + 5), FONT_HERSHEY_SIMPLEX, 0.45, Scalar(255, 255, 255), 1);

    // LED Lửa
    Scalar fireLed = Scalar(70, 70, 70);
    if (fireTooClose) fireLed = Scalar(0, 0, 255);
    else if (fire) fireLed = Scalar(0, 145, 255);
    circle(canvas, Point(25, startY + 2 * spacing), 8, fireLed, -1);
    //"FIRE: DETECTED" : "FIRE: SAFE"
    putText(canvas, fire ? "FIRE: PHAT HIEN CHAY" : "FIRE: KHONG CO CHAY", Point(45, startY + 2 * spacing + 5), FONT_HERSHEY_SIMPLEX, 0.45, Scalar(255, 255, 255), 1);

    // 5. VẼ CÁC NÚT LỆNH ĐIỀU KHIỂN TƯƠNG TÁC CHUỘT
    line(canvas, Point(10, 310), Point(210, 310), Scalar(150, 150, 150), 1);
    //INTERACTIVE COMMANDS:
    putText(canvas, "LENH TUONG TAC:", Point(15, 325), FONT_HERSHEY_SIMPLEX, 0.4, Scalar(200, 200, 200), 1);

    // Nút lệnh 1: Tự động Tắt khi có người / Bật khi không người (Ghi đè thủ công qua nút)
    Scalar btn1Color = isSystemEnabled ? Scalar(0, 180, 0) : Scalar(0, 0, 180);
    rectangle(canvas, btnToggleSystem, btn1Color, -1); // Vẽ nút đặc màu
    rectangle(canvas, btnToggleSystem, Scalar(255, 255, 255), 1); // Vẽ viền nút
    string btn1Text = isSystemEnabled ? "SYS: BAT (TU DONG)" : "SYS: CUONG BUOC TAT"; //SYS: ON (AUTO)" : "SYS: FORCE OFF
    putText(canvas, btn1Text, Point(25, 362), FONT_HERSHEY_SIMPLEX, 0.45, Scalar(255, 255, 255), 1, LINE_AA);

    // Nút lệnh 2: Bật báo cháy khi tan ca / Tắt khi có người trực
    Scalar btn2Color = isAfterShiftMode ? Scalar(0, 0, 200) : Scalar(150, 100, 0);
    rectangle(canvas, btnToggleShift, btn2Color, -1);
    rectangle(canvas, btnToggleShift, Scalar(255, 255, 255), 1);
    string btn2Text = isAfterShiftMode ? "CHE ĐO: SAU CA LAM VIEC" : "CHE DO: DANG LAM NHIEM VU";
    putText(canvas, btn2Text, Point(25, 412), FONT_HERSHEY_SIMPLEX, 0.42, Scalar(255, 255, 255), 1, LINE_AA);

    // 6. Khối thông báo trạng thái Tổng hợp dưới đáy UI
    line(canvas, Point(10, canvas.rows - 45), Point(210, canvas.rows - 45), Scalar(150, 150, 150), 1);
    if (!isSystemEnabled) {
        //SYSTEM DISABLED BY USER
        putText(canvas, "HE THONG ĐA BI NGUOI DUNG VO HIEU HOA", Point(15, canvas.rows - 20), FONT_HERSHEY_SIMPLEX, 0.42, Scalar(0, 0, 255), 1);
    }
    else if (isAfterShiftMode) {
        //HIGH SECURITY ACTIVE
        putText(canvas, "BAO MAT CAO HOAT ĐONG", Point(15, canvas.rows - 20), FONT_HERSHEY_SIMPLEX, 0.42, Scalar(0, 255, 255), 1);
    }
    else {
        //MONITORING SYSTEM RUNNING
        putText(canvas, "HE THONG GIAM SAT DANG HOAT DONG", Point(15, canvas.rows - 20), FONT_HERSHEY_SIMPLEX, 0.41, Scalar(0, 255, 0), 1);
    }
}

// Hàm Nhận diện ngọn lửa
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

            Mat roiHSV = hsv(safeBox); // SỬA LỖI: Lấy kênh HSV để đo độ sáng chính xác
            Scalar meanVal = mean(roiHSV);

            Mat motionROI = grayDiff(safeBox);
            double motion = sum(motionROI)[0];

            if (meanVal[2] > cfg.minBrightness && motion > cfg.minMotion) {
                fireDetected = true;
                rectangle(frame, box, Scalar(0, 0, 255), 2); // Vẽ hộp bao ngọn lửa
            }
        }
    }
    return fireDetected;
}

// Hàm nhận diện con người bằng học máy HOG + SVM
bool detectPerson(Mat& frame, HOGDescriptor& hog) {
    vector<Rect> people;
    hog.detectMultiScale(frame, people);

    for (auto p : people) {
        rectangle(frame, p, Scalar(255, 0, 0), 2); // Vẽ hộp bao người màu xanh lam
    }
    return !people.empty();
}

// Hàm nhận diện khói
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

int main() {
    VideoCapture cap(0);
    if (!cap.isOpened()) {
        cout << "Không mở được camera!" << endl;
        return -1;
    }

    HOGDescriptor hog;
    hog.setSVMDetector(HOGDescriptor::getDefaultPeopleDetector());

    Mat frame, prevFrame;

    string windowName = "He Thong Giam Sat Chay No(Fire and Explosion Monitoring System) - NHOM_XLAS_PHCN";
    namedWindow(windowName, WINDOW_AUTOSIZE);

    // ĐĂNG KÝ SỰ KIỆN CHUỘT CHO CỬA SỔ UI CHÍNH
    setMouseCallback(windowName, onMouseClick, NULL);

    while (true) {
        cap >> frame;
        if (frame.empty()) break;

        resize(frame, frame, Size(640, 480));

        if (prevFrame.empty()) {
            prevFrame = frame.clone();
            continue;
        }

        // Bước 1: Nhận diện con người để phân tích ngữ cảnh tự động
        bool hasPerson = detectPerson(frame, hog);

        // LOGIC TỰ ĐỘNG CHUYỂN ĐỔI CHẾ ĐỘ KHI CÓ NGƯỜI / KHÔNG CÓ NGƯỜI
        // (Nếu không bị cưỡng bách TẮT hệ thống bằng nút nhấn)
        if (hasPerson && isSystemEnabled) {
            isSystemEnabled = false; // Tự động TẮT hệ thống báo cháy thông thường khi phát hiện có người
        }
        else if (!hasPerson && !isSystemEnabled && !isAfterShiftMode) {
            isSystemEnabled = true;  // Tự động BẬT lại hệ thống khi con người rời khỏi vùng quét
        }

        FireConfig currentMode = hasPerson ? kitchenMode : normalMode;

        bool fireTooClose = false;
        bool fire = false;
        bool smoke = false;

        // CHỈ THỰC THI QUÉT HOẢ HOẠN NẾU HỆ THỐNG ĐANG Ở TRẠNG THÁI BẬT HOẶC ĐÃ TAN CA
        if (isSystemEnabled || isAfterShiftMode) {
            fire = detectFireMode(frame, prevFrame, currentMode, fireTooClose);
            smoke = detectSmoke(frame);
        }

        // HỆ THỐNG PHÂN TẦNG ƯU TIÊN VÀ XỬ LÝ SỰ KIỆN ĐIỀU KHIỂN NÚT BẤM
        if (fireTooClose) {
            // 🔴 ƯU TIÊN CAO NHẤT: Lửa quá lớn/gần -> Báo động khẩn cấp không phụ thuộc vào trạng thái nút bấm
            rectangle(frame, Rect(0, 0, frame.cols, frame.rows), Scalar(0, 0, 255), 10);
            //DANGER! FIRE TOO CLOSE
            putText(frame, "NGUY HIEM! LUA QUA GAN!", Point(240, 50), FONT_HERSHEY_SIMPLEX, 0.7, Scalar(0, 0, 255), 2);
            system("beep");
        }
        else if (isAfterShiftMode) {
            // 🟡 LOGIC TAN CA: Hệ thống ở mức bảo mật cao nhất, có lửa hoặc khói là lập tức rú còi
            if (fire) {
                putText(frame, "TAN CA: CHAY NGUY HIEM!", Point(240, 50), FONT_HERSHEY_SIMPLEX, 0.7, Scalar(0, 0, 255), 2);
                system("beep");
            }
            else if (smoke) {
                putText(frame, "TAN CA: CANH BAO KHOI!", Point(240, 50), FONT_HERSHEY_SIMPLEX, 0.7, Scalar(0, 0, 255), 2);
                system("beep"); // Tan ca không người trực -> Khói cũng phát chuông rú báo động phần cứng
            }
        }
        else if (isSystemEnabled) {
            // 🟢 LOGIC KHI BẬT HỆ THỐNG (VẮNG NGƯỜI/TRẠNG THÁI THƯỜNG)
            if (fire) {
                putText(frame, "PHAT HIEN CHAY!", Point(240, 50), FONT_HERSHEY_SIMPLEX, 0.7, Scalar(0, 0, 255), 2);
                system("beep");
            }
            else if (smoke) {
                putText(frame, "PHAT HIEN KHOI!", Point(240, 110), FONT_HERSHEY_SIMPLEX, 0.7, Scalar(200, 200, 200), 2);
            }
        }
        else {
            // 🔵 CÓ NGƯỜI TRỰC HOẶC HỆ THỐNG TẮT: Chỉ hiển thị text nhắc nhở cảnh báo sớm trực quan, KHÔNG rú còi phiền hà
            //NHẬT KÝ: LOG: FIRE MONITORING - LOG: SMOKE MONITORING
            if (fire) {
                putText(frame, "NHAT KY: GIAM SAT HOA HOAN...", Point(240, 50), FONT_HERSHEY_SIMPLEX, 0.7, Scalar(0, 255, 255), 2);
            }
            if (smoke) {
                putText(frame, "NHAT KY: GIAM SAT KHOI...", Point(240, 110), FONT_HERSHEY_SIMPLEX, 0.7, Scalar(200, 200, 200), 2);
            }
        }

        // GỌI HÀM VẼ TOÀN BỘ GIAO DIỆN NÚT BẤM VÀ ĐÈN LED LÊN FRAME CHÍNH
        drawAdvancedUI(frame, hasPerson, fire, smoke, fireTooClose);

        // HIỂN THỊ CỬA SỔ ĐỒ HỌA TƯƠNG TÁC
        imshow(windowName, frame);

        prevFrame = frame.clone(); // Cập nhật khung hình nền vi sai thời gian

        // Nhấn phím ESC (Mã ASCII 27) để thoát chương trình an toàn
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
