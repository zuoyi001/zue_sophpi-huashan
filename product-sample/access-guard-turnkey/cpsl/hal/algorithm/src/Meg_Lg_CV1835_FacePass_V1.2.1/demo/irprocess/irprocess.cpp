//
// Created by megvii on 19-6-26.
//

#include "tool.h"
#include "argparse.h"

void irProcess(const pair<string, string>& imagePair, const string& outFolder,
        const string& runOption, const int width, const int height){
    const string& bgrPath = imagePair.first;
    const string& irPath = imagePair.second;
    printf("[FACEPASS_TEST] Processing : %s , %s\n", bgrPath.c_str(), irPath.c_str());

    cv::Mat bgrImg;
    vector<char> bgrData;
    Image bgrImage = {0};
    int bgrLength = 128;
    std::vector<FaceHandle> bgrHandleArr(bgrLength, nullptr);
    FaceRetCode retCode = RET_OK;
    if(runOption == "bgr"){
        bgrImg = cv::imread(bgrPath, cv::IMREAD_COLOR);
        bgrImage = {bgrImg.data, nullptr, -1, bgrImg.cols, bgrImg.rows, bgrImg.cols * 3, bgrImg.rows, Format_BGR888};
        retCode = detect(&bgrImage, 0.75, MIN_FACE, bgrHandleArr.data(), &bgrLength);
        bgrHandleArr.resize(bgrLength);
    }
    else if(runOption == "yuv420"){
        bgrData = ImageUtil::readFile(bgrPath);
        bgrImage = {static_cast<void*>(bgrData.data()), nullptr, -1, width, height, width, height, Format_YUV420P_YU12};
        retCode = detect((const Image* const)&bgrImage, 0.75, MIN_FACE, bgrHandleArr.data(), &bgrLength);
        bgrHandleArr.resize(bgrLength);
        // to show result
        cv::Mat yuv420(height + height / 2, width, CV_8UC1, bgrData.data());
        cv::cvtColor(yuv420, bgrImg, cv::COLOR_YUV2BGR_IYUV);
    }

    if(RET_NO_FACE == retCode){
        printf("[FACEPASS_TEST] detect no faces in %s!\n", bgrPath.c_str());
        return;
    }
    else if(RET_OK != retCode){
        printf("[FACEPASS_TEST] error occurred during detect bgrImage!\n");
        return;
    }

    cv::Mat irImg;
    vector<char> irData;
    Image irImage = {0};
    int irLength = 128;
    std::vector<FaceHandle> irHandleArr(irLength, nullptr);

    if(runOption == "bgr"){
        irImg = cv::imread(irPath, cv::IMREAD_COLOR);
        irImage = {irImg.data, nullptr, -1, irImg.cols, irImg.rows, irImg.cols * 3, irImg.rows, Format_BGR888};
        retCode = detect((const Image* const)&irImage, 0.75, MIN_FACE, irHandleArr.data(), &irLength);
        irHandleArr.resize(irLength);
    }
    else if(runOption == "yuv420"){
        irData  = ImageUtil::readFile(irPath);
        irImage = {static_cast<void*>(irData.data()), nullptr, -1, width, height, width, height, Format_YUV420P_YU12};
        retCode = detect((const Image* const)&irImage, 0.75, MIN_FACE, irHandleArr.data(), &irLength);
        irHandleArr.resize(irLength);
        cv::Mat ir(height + height / 2, width, CV_8UC1, irData.data());
        cv::cvtColor(ir, irImg, cv::COLOR_YUV2BGR_IYUV);
    }
    if(RET_NO_FACE == retCode){
        printf("[FACEPASS_TEST] detect no faces in %s!\n", irPath.c_str());
    }
    if(RET_OK != retCode){
        printf("[FACEPASS_TEST] error occurred during detect irImage!\n");
    }

    printf("[FACEPASS_TEST] bgr handle num:%d\t ir handle num:%d\n", bgrLength, irLength);

#if 1 // livenessTest
    vector<int> corrIrIndexes(bgrLength, irLength);
    {
        auto sp = cw::ProfilerFactory::get()->make("BGR_IR_match");
        BGR_IR_match(bgrHandleArr.data(), bgrLength, irHandleArr.data(), irLength, corrIrIndexes.data());
    }

    for(int i = 0; i < bgrLength; ++i){
        float score;
        const cv::Scalar green(0, 255, 0);
        const cv::Scalar red(0, 0, 255);
        FaceRect rect;
        if(corrIrIndexes[i] == irLength){
            printf("[FACEPASS_TEST] The bgrIndex=%d face handle is attack, can not pass the ir filter process!!\n", i);
            getFaceRect(bgrHandleArr[i], &rect);
            ImageUtil::drawBox(bgrImg, rect, i, red);
        }
        else{
            FaceRetCode retCode;
            {
                auto sp = cw::ProfilerFactory::get()->make("getLiveness_bgrir");
                retCode = getLiveness_bgrir(&bgrImage, bgrHandleArr[i], &irImage, irHandleArr[corrIrIndexes[i]], &score);

            }
            if(RET_OK == retCode){
                printf("[FACEPASS_TEST] bgrIndex=%d--score=%f--irIndex=%d\n", i, score, corrIrIndexes[i]);
                getFaceRect(bgrHandleArr[i], &rect);
                ImageUtil::drawScore(bgrImg, cv::Point(rect.left, rect.top), score, green);
                getFaceRect(irHandleArr[corrIrIndexes[i]], &rect);
                ImageUtil::drawScore(irImg, cv::Point(rect.left, rect.top), score, green);
            }
            else{
                printf("[FACEPASS_TEST] can not detect liveness!\n");
            }
            getFaceRect(bgrHandleArr[i], &rect);
            ImageUtil::drawBox(bgrImg, rect, i, green);
            getFaceRect(irHandleArr[corrIrIndexes[i]], &rect);
            ImageUtil::drawBox(irImg, rect, corrIrIndexes[i], green);
        }
    }


#else //irFilterTest
    vector<char> flags(bgrLength);
    ir_filter(bgrHandles, bgrLength, irHandles, irLength, flags.data());

    const cv::Scalar green(0, 255, 0);
    const cv::Scalar red(0, 0,  255);
    FaceRect rect;
    for(size_t i = 0; i < flags.size(); ++i){
        getFaceRect(bgrHandles[i], &rect);
        if(!flags[i]){
            printf("[FACEPASS_TEST] bgrIndex=%d is filtered out\n",i);
            ImageUtil::drawBox(bgrImage, rect, i, red);
        }
        else{
            ImageUtil::drawBox(bgrImage, rect, i, green);
        }
    }

    for(int i = 0; i < irLength; ++i){
        getFaceRect(irHandles[i], &rect);
        ImageUtil::drawBox(irImage, rect, i, green);
    }

#endif
    for(int i = 0; i < bgrLength; ++i){
        releaseFace(bgrHandleArr[i]);
    }
    for(int i = 0; i < irLength; ++i){
        releaseFace(irHandleArr[i]);
    }

    const string bgrResult = cw::Path::join(outFolder, cw::Path::get_basename(bgrPath) + ".jpeg");
    cv::imwrite(bgrResult, bgrImg);
    const string irResult = cw::Path::join(outFolder, cw::Path::get_basename(irPath) + ".jpeg");
    cv::imwrite(irResult, irImg);
}

vector<pair<string, string>> getImagePairs(const string& bgrPath, const string& irPath,
        const string& runOption){
    vector<pair<string, string>> pairAarry;
    cw::File file(bgrPath);
    if( file.is_dir() ){
        vector<string> exts;
        if(runOption == "bgr"){
            exts =  {".jpg", ".bmp", ".png"};
        }
        else if(runOption == "yuv420"){
            exts =  {".yuv"};
        }
        for (auto &bgrFile : file.list_file_by_ext_recursive(exts)){
            auto bgrName = cw::Path::get_name(bgrFile);
            const auto irName = bgrName.replace(bgrName.length() -7, 3, "ir");
            const auto irFile = cw::Path::join(irPath, irName);
            pairAarry.emplace_back(make_pair(bgrFile, irFile));
        }
    }
    else{
        pairAarry.emplace_back(make_pair(bgrPath, irPath));
    }

    return pairAarry;
}

static void help(){

    cout << "\nThis program demostrates BGR_IR_match and getliveness_ir interface on facepass\n"
        "input is bgr or yuv420 image path or folder, outfolder for write image; do BGR_IR_match, then getliveness_ir, get scores\n"
        "yuv420 image format, need setting width and height, default 1080*1920\n"
        "Usage: \n"
        "./irprocess  model.yaml -r bgr -i ./bgrpath -a ./irpath/ -o ./outfolder\n"
        "./irprocess model.yaml -r yuv420 -i ./bgrth -a ./irpath/ -o ./outfolder -w width -h height\n\n";
}

int main(int argc, char *argv[]){

    help();

    ArgumentParser parser("FACEPASS_DV300_TEST IR PROCESS");
    parser.add_positional_arg("model_config", "1", "the path of the model.yaml");
    parser.add_keyword_arg("-r", "--run_option", "", true, "", "bgr means bgr image type, yuv420 stands for yuv input");
    parser.add_keyword_arg("-i", "--input_path", "", true, "", "the path of visible input image");
    parser.add_keyword_arg("-a", "--auxi_path", "", true, "", "the path of ir input image, same type and size with visible image");
    parser.add_keyword_arg("-o", "--output_path", "", true, "", "the folder for write image");
    parser.add_keyword_arg("-w", "--width", "", false, "1080", "the width of input image");
    parser.add_keyword_arg("-h", "--height", "", false, "1920", "the height of input image");

    ArgumentResult args = parser.parse_args(argc, argv);

    const auto modelConfig =  args.args["model_config"];
    const auto runOption   =  args.args["run_option"];
    const auto bgrPath     =  args.args["input_path"];
    const auto irPath      =  args.args["auxi_path"];
    const auto outFolder   =  args.args["output_path"];
    const auto width = std::stoi(args.args["width"]);
    const auto height = std::stoi(args.args["height"]);
    if(runOption != "bgr" && runOption != "yuv420"){
        printf("[FACEPASS_TEST] invalid runOption! \n");
        return -1;
    }

    setLogLevel(LOG_LEVEL_INFO);
    if(initSDK(modelConfig) != 0){
        printf("[FACEPASS_TEST] sdk init error!\n");
        release();
        return 1;
    };
    const vector<pair<string, string>> bgrIr = getImagePairs(bgrPath, irPath, runOption);
    for( auto &pair : bgrIr){
        irProcess(pair, outFolder, runOption, width, height);
    }

    ::release();

    return 0;
}
