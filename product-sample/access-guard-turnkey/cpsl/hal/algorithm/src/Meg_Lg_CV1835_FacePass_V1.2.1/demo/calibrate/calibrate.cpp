#include <string>
#include <iostream>
#include <vector>
#include "argparse.h"
#include <opencv2/opencv.hpp>
#include <opencv2/imgcodecs/imgcodecs_c.h>
#include <tool.h>


using namespace std;
using namespace cv;

enum CALIBRATION_STATE {CALIBRATION_NONE,  CALIBRATION_IN_PROGRESS, CALIBRATION_FINISHED};

static int   g_calibration_state = CALIBRATION_NONE;
static float g_left_right_k;
static float g_left_right_b;
static float g_top_bottom_k;
static float g_top_bottom_b;

void resetRgbirCalibration(){
    g_calibration_state = CALIBRATION_NONE;
    g_left_right_k = 0.0;
    g_left_right_b = 0.0;
    g_top_bottom_k = 0.0;
    g_top_bottom_b = 0.0;
}

int doCalibrationVerify(vector<Point2f> &pointbuf_rgb, vector<Point2f> &pointbuf_ir){
    if (pointbuf_rgb.size() != pointbuf_ir.size()) {
        printf("doCalibrationVerify:input size mismatch (%d vs %d)\n", (int)(pointbuf_rgb.size()), (int)(pointbuf_ir.size()));
        return CALIBRATION_IN_PROGRESS;
    }
    if (pointbuf_rgb.size() <= 1) {
        printf("input size should large than 1\n" );
        return CALIBRATION_IN_PROGRESS;
    }

    g_calibration_state = CALIBRATION_IN_PROGRESS;

    vector<float>  x_rgb_calibration;
    vector<float>  y_rgb_calibration;
    vector<float>  x_ir;
    vector<float>  y_ir;
    for(auto &item : pointbuf_rgb) {
        x_rgb_calibration.push_back(item.x * g_left_right_k + g_left_right_b);
        y_rgb_calibration.push_back(item.y * g_top_bottom_k + g_top_bottom_b);
    }
    for(auto &item: pointbuf_ir) {
        x_ir.push_back(item.x);
        y_ir.push_back(item.y);
    }

    float x_deviation_sum = 0.0;
    float y_deviation_sum = 0.0;
    int point_num = x_ir.size();
    for(int i = 0; i < point_num; ++i) {
        x_deviation_sum += pow(x_rgb_calibration[i] - x_ir[i], 2);
        y_deviation_sum += pow(y_rgb_calibration[i] - y_ir[i], 2);
    }
    float x_standard_deviation = sqrt(x_deviation_sum / point_num);
    float y_standard_deviation = sqrt(y_deviation_sum / point_num);

    printf(" x_standard_deviation: %.2f, y_standard_deviation: %.2f\n", x_standard_deviation, y_standard_deviation);

    if (x_standard_deviation < 8.0 && y_standard_deviation < 8.0) {
        g_calibration_state = CALIBRATION_FINISHED;
        printf("calibration state changed:  CALIBRATION_IN_PROGRESS  >>>>  CALIBRATION_FINISHED\n");
    } else {
        g_calibration_state = CALIBRATION_NONE;
        printf("reset calibration to CALIBRATION_NONE >>>>\n");
    }
    return  g_calibration_state;
}

static int  findCornersPoints(Mat &cvimg, Size board_size, vector<Point2f> &pointbuf){

    bool found = findChessboardCorners(cvimg, board_size, pointbuf, CALIB_CB_FAST_CHECK);
    const size_t checkerNum = board_size.width * board_size.height;
    if (!found || (pointbuf.size() < checkerNum)) {
        printf("found Chessboard Corners error, point buffer size:%d !!\n", (int)(pointbuf.size()));
        return -1;
    }

    return 0;
}


/* use ordinary least squares(ols) for linear regression */
void linearRegressionOLS(const std::vector<float>& x,
        const std::vector<float>& y, float &k, float &b){
    if (x.size() != y.size()) {
        printf("linearRegressionOLS:input size mismatch (%d vs %d)\n", (int)(x.size()), (int)(y.size()));
        return;
    }
    if (x.size() <= 1) {
        printf("input size should large than 1\n");
        return;
    }

    float y_mean = 0.0;
    float x_mean = 0.0;
    for (size_t i = 0; i < x.size(); ++i) {
        x_mean += x[i];
        y_mean += y[i];
    }
    x_mean /= static_cast<float>(x.size());
    y_mean /= static_cast<float>(x.size());
    float A_fractions = 0;
    float A_denominator = 0;
    for (size_t i = 0; i < x.size(); ++i) {
        A_fractions += (x[i] * y[i]);
        A_denominator += pow(x[i], 2);
    }
    A_fractions -= (static_cast<float>(x.size()) * x_mean * y_mean);
    A_denominator -= (static_cast<float>(x.size()) * pow(x_mean, 2));
    k = A_fractions / A_denominator;
    b = y_mean - k * x_mean;

}

int linearRegression(vector<Point2f> &pointbuf_rgb, vector<Point2f> &pointbuf_ir){
    if (pointbuf_rgb.size() != pointbuf_ir.size()) {
        printf("linearRegression:input size mismatch (%d vs %d)\n", (int)(pointbuf_rgb.size()), (int)(pointbuf_ir.size()));
        return CALIBRATION_NONE;
    }
    if (pointbuf_rgb.size() <= 1) {
        printf("input size should large than 1\n");
        return CALIBRATION_NONE;
    }

    vector<float>  x_rgb;
    vector<float>  y_rgb;
    vector<float>  x_ir;
    vector<float>  y_ir;

    for(auto &item : pointbuf_rgb) {
        x_rgb.push_back(item.x);
        y_rgb.push_back(item.y);
    }

    for(auto &item : pointbuf_ir) {
        x_ir.push_back(item.x);
        y_ir.push_back(item.y);
    }

    linearRegressionOLS(x_rgb, x_ir, g_left_right_k, g_left_right_b);
    linearRegressionOLS(y_rgb, y_ir, g_top_bottom_k, g_top_bottom_b);

    g_calibration_state = CALIBRATION_IN_PROGRESS;
    printf("calibration state changed:  CALIBRATION_NONE>>>>CALIBRATION_IN_PROGRESS\n");
    printf("(left_right_k, left_right_b, top_bottom_k, top_bottom_b): (%.2f, %.2f, %.2f, %.2f)\n",
            g_left_right_k, g_left_right_b, g_top_bottom_k, g_top_bottom_b);

    return g_calibration_state;
}


static void help(){
    cout << "\nThis program is calculate the offset between bgr camera and ir camera in facepass sdk\n"
        "Usage: \n"
        "./calibrate  model.yaml -r bgr   -i ./bgr_image_path -a ./ir_path/ -o ./outfolder\n"
        "./calibrate  model.yaml -r yuv420  -i ./bgr_image_path -a ./ir_path/ -o ./outfolder -w width -h height\n\n";
}

int main(int argc, char *argv[]){
    help();
    ArgumentParser parser("FACEPASS_DV300_TEST CALIBRATE PROCESS");
    parser.add_positional_arg("model_config", "1", "the path of the model.yaml");
    parser.add_keyword_arg("-r", "--run_option", "", true, "", "bgr means bgr image type, yuv420 stands for yuv input");
    parser.add_keyword_arg("-i", "--input_path", "", true, "", "the path of visible input image");
    parser.add_keyword_arg("-a", "--auxi_path", "", true, "", "the path of ir input image, same type and size with visible image");
    parser.add_keyword_arg("-o", "--output_path", "", true, "", "the folder for write image");
    parser.add_keyword_arg("-w", "--width", "", false, "1080", "the width of input image");
    parser.add_keyword_arg("-h", "--height", "", false, "1920", "the height of input image");

    ArgumentResult args = parser.parse_args(argc, argv);


    const auto modelConfig = args.args["model_config"];
    const auto runOption   =  args.args["run_option"];
    const auto bgr_image_path =  args.args["input_path"];
    const auto ir_image_path   =  args.args["auxi_path"];
    const auto outFolder   =  args.args["output_path"];
    const auto width = std::stoi(args.args["width"]);
    const auto height = std::stoi(args.args["height"]);

    cv::Mat bgrImg;
    cv::Mat irImg;

    if(runOption == "bgr"){
        bgrImg = cv::imread(bgr_image_path, CV_LOAD_IMAGE_COLOR);
        irImg = cv::imread(ir_image_path, CV_LOAD_IMAGE_COLOR);
    }
    else if(runOption == "yuv420"){
        vector<char> bgr_data = ImageUtil::readFile(bgr_image_path);
        vector<char> ir_data = ImageUtil::readFile(ir_image_path);

        cv::Mat bgr_yuv(height*3/2, width, CV_8UC1, bgr_data.data());
        cv::cvtColor(bgr_yuv, bgrImg, cv::COLOR_YUV2BGR_IYUV);

        cv::Mat ir_yuv(height*3/2, width, CV_8UC1, ir_data.data());
        cv::cvtColor(ir_yuv, irImg, cv::COLOR_YUV2BGR_IYUV);
    }
    else{
        printf("-r input param is illegal\n");
        return -1;
    }
    if(bgrImg.empty()){
        printf("bgr_image is empty!\n");
        return -1;
    }
    if(irImg.empty()){
        printf("ir_image is empty!\n");
        return -1;
    }
    vector<Point2f> points_rgb;
    vector<Point2f> points_ir;
    Size board_size;
    board_size.width = 9;
    board_size.height = 6;
    resetRgbirCalibration();

    findCornersPoints(bgrImg, board_size, points_rgb);
    printf("bgrImg corner size = %d\n", points_rgb.size());

    findCornersPoints(irImg, board_size, points_ir);
    printf("irImg corner size = %d\n", points_ir.size());


    if (g_calibration_state == CALIBRATION_NONE) {
        g_calibration_state = linearRegression(points_rgb, points_ir);
    }

    if (g_calibration_state == CALIBRATION_IN_PROGRESS) {
        g_calibration_state = doCalibrationVerify(points_rgb, points_ir);
    }

    initSDK(modelConfig);
    set_match_config(g_left_right_k, g_left_right_b, g_top_bottom_k, g_top_bottom_b, 0.4);

    /*set bgr and ir image info*/
    Image bgrImage = {0};
    Image irImage = {0};
    bgrImage = {bgrImg.data, nullptr, -1, bgrImg.cols, bgrImg.rows, bgrImg.cols * 3, bgrImg.rows, Format_BGR888};
    irImage = {irImg.data, nullptr, -1, irImg.cols, irImg.rows, irImg.cols * 3, irImg.rows, Format_BGR888};


    int bgrLength = 128;
    int irLength = 128;
    std::vector<FaceHandle> bgrHandleArr(bgrLength, nullptr);
    detect(&bgrImage, 0.75, MIN_FACE, bgrHandleArr.data(), &bgrLength);
    bgrHandleArr.resize(bgrLength);
    std::vector<FaceHandle> irHandleArr(irLength, nullptr);
    detect(&irImage, 0.75, MIN_FACE, irHandleArr.data(), &irLength);
    irHandleArr.resize(irLength);

    printf("[FACEPASS_TEST]  bgr handle num:%d\t ir handle num:%d\n",bgrLength,irLength);

    // ir_filter test
    std::vector<char> flags(bgrLength);
    ir_filter(bgrHandleArr.data(), bgrLength, irHandleArr.data(), irLength, flags.data());
    printf("[FACEPASS_TEST] flags[i]=1 pass!!!\n");

    for(size_t i = 0; i < flags.size(); ++i){
        printf("[FACEPASS_TEST] bgr face flags[%d]==%d(0 indicate reject, 1 stand for pass)\n",i, flags[i]);
    }

    //liveness_ir test
    vector<int> corrIrIndexes(bgrLength, irLength);
    BGR_IR_match(bgrHandleArr.data(), bgrLength, irHandleArr.data(), irLength, corrIrIndexes.data());
    float score;
    const cv::Scalar green(0, 255, 0);
    const cv::Scalar red(0, 0, 255);
    for(int i = 0; i < bgrLength; ++i){
        FaceRect rect;
        getFaceRect(bgrHandleArr[i], &rect);
        if(corrIrIndexes[i] == irLength){
            ImageUtil::drawBox(bgrImg, rect, i, red);
            printf("[FACEPASS_TEST] This bgrIndex=%d face handle is attack, can not pass BGR_IR_match!!\n", i);
        }
        else{
            {
                auto sp = cw::ProfilerFactory::get()->make("getLiveness_bgrir");
                getLiveness_bgrir(&bgrImage, bgrHandleArr[i], &irImage, irHandleArr[corrIrIndexes[i]], &score);
            }
            ImageUtil::drawBox(bgrImg, rect, i, green);

            getFaceRect(irHandleArr[corrIrIndexes[i]], &rect);
            ImageUtil::drawBox(irImg, rect, corrIrIndexes[i], green);

            printf("[FACEPASS_TEST] bgrIndex=%d--score=%f--irIndex=%d\n", i, score, corrIrIndexes[i]);
        }
    }
    for(int i = 0; i < bgrLength; ++i){
        releaseFace(bgrHandleArr[i]);
    }
    for(int i = 0; i < bgrLength; ++i){
        releaseFace(irHandleArr[i]);
    }

    const string bgrResult = cw::Path::join(outFolder, cw::Path::get_basename(bgr_image_path) + ".jpg");
    cv::imwrite(bgrResult, bgrImg);
    const string irResult = cw::Path::join(outFolder, cw::Path::get_basename(ir_image_path) + ".jpg");
    cv::imwrite(irResult, irImg);

    return 0;
}


