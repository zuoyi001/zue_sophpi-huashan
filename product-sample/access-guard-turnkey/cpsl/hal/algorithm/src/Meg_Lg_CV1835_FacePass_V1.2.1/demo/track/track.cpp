//
// Created by megvii on 19-6-26.
//

#include "tool.h"
#include "argparse.h"

void printTrackId(const FaceHandle* const faceHandles, const int length, const int frameId);

void trackProcessForImages(const std::vector<std::string>& image_paths, const string &outputFolder){
    size_t frameId = 0;
    for (auto imagePath : image_paths){
        printf("[FACEPASS_TEST] process image path : %s\n", imagePath.c_str());
        cv::Mat img = cv::imread(imagePath);
        if(img.empty()){
            printf("[FACEPASS_TEST] load image error!!\n");
            assert(0);
        }

        int nCount = 128;/*max number of detected faces*/
        Image image = {img.data, nullptr, -1, img.cols, img.rows, img.cols * 3, img.rows, Format_BGR888};
        std::vector<FaceHandle> faceHandleArr(nCount, nullptr);
        FaceRetCode retCode = RET_OK;

        retCode = detectAndTrack(&image, 0.75, MIN_FACE, faceHandleArr.data(), &nCount);
        faceHandleArr.resize(nCount);
        if(RET_NO_FACE == retCode){
            printf("[FACEPASS_TEST] detect no faces in %s!\n", imagePath.c_str());
        }
        else if(RET_OK != retCode){
            printf("[FACEPASS_TEST] error occurred during track!\n");
        }
        printf("[FACEPASS_TEST] detect faces number is %d\n", nCount);
        printTrackId(faceHandleArr.data(), nCount, frameId);

        ImageUtil::drawTrackResult(faceHandleArr.data(), nCount, img, frameId);
        for(int i = 0; i < nCount; ++i){
            releaseFace(faceHandleArr[i]);
        }

        ++frameId;
        const string result = cw::Path::join(outputFolder, cw::Path::get_basename(imagePath) + ".jpg");
        cv::imwrite(result, img);
    }
}


void trackProcessForVideo(const string &video_path, const string &outputFolder, const int width, const int height){
    std::ifstream is (video_path.c_str(), std::ifstream::binary);
    if(is){
        printf("[FACEPASS_TEST] read video path %s\n", video_path.c_str());
    }
    else{
        printf("[FACEPASS_TEST] cannot read video path %s\n", video_path.c_str());
        return ;
    }

    size_t image_size = (width * height * 3)>>1;
    std::vector<unsigned char> yuv_buffer(image_size);
    const size_t numberWidth = 5;
    size_t frameId = 0;
    while(is.good()){
        is.read((char*)(yuv_buffer.data()), image_size);
        int nCount = 128;/*max number of detected faces*/
        std::vector<FaceHandle> faceHandleArr(nCount, nullptr);
        Image image = {static_cast<void*>(yuv_buffer.data()), nullptr, -1, width, height, width, height, Format_YUV420P_YU12};

        FaceRetCode retCode = detectAndTrack((const Image* const)&image, 0.75, MIN_FACE, faceHandleArr.data(), &nCount);
        faceHandleArr.resize(nCount);
        if(RET_NO_FACE == retCode){
            printf("[FACEPASS_TEST] detect no faces in frame %d!\n", frameId);
        }
        printf("[FACEPASS_TEST] detect faces number is %d\n", nCount);
        printTrackId(faceHandleArr.data(), nCount, frameId);

        cv::Mat yuv420(height + (height>>1), width, CV_8U, yuv_buffer.data());
        cv::Mat img;
        cv::cvtColor(yuv420, img, cv::COLOR_YUV2BGR_IYUV);
        ImageUtil::drawTrackResult(faceHandleArr.data(), nCount, img, frameId);
        for(int i = 0; i < nCount; ++i){
            releaseFace(faceHandleArr[i]);
        }

        const string indexString = std::string(numberWidth-std::to_string(frameId).size(), '0').append( std::to_string(frameId));
        const string result = cw::Path::join(outputFolder, indexString + ".jpg");
        cv::imwrite(result, img);
        ++frameId;

        /*if(stream.eof()){
          is.seekg (0, is.beg);
          }*/
    }
}

void printTrackId(const FaceHandle* const faceHandles, const int length, const int frameId){
    printf("%05d(%d): ", frameId, length);
    int trackId;
    for(int i=0;i<length; ++i){
        getTrackId(faceHandles[i], &trackId);
        printf("%d, ", trackId);
    }
    printf("\n");
}

static void help(){
    cout << "\nThis program demostrates track function in facepass sdk,\n"
        " input is bgr frame images(e.g. 0001.jpg, 0002.jpg, ...)  or yuv420 video file\n"
        "Usage: \n"
        "./track  model.yaml -r bgr -i ./input_folder/  -o ./out_folder/\n"
        "./track  model.yaml -r yuv420 -i ./video_path  -o ./out_folder/ -w width -h height\n\n";

}

int main(int argc, char *argv[]){

    help();

    ArgumentParser parser("FACEPASS_TEST IR PROCESS");
    parser.add_positional_arg("model_config", "1", "the path of the model.yaml");
    parser.add_keyword_arg("-r", "--run_option", "", true, "", "bgr means bgr frame images, yuv420 is video.yuv");
    parser.add_keyword_arg("-i", "--input_path", "", true, "", "the folder for input images");
    parser.add_keyword_arg("-o", "--output_path", "", true, "", "the folder for write image");
    parser.add_keyword_arg("-w", "--width", "", false, "1080", "the width of input image");
    parser.add_keyword_arg("-h", "--height", "", false, "1920", "the height of input image");

    ArgumentResult args = parser.parse_args(argc, argv);

    const auto modelConfig  =  args.args["model_config"];
    const auto run_option   =  args.args["run_option"];
    const auto inputFolder  =  args.args["input_path"];
    const auto outFolder    =  args.args["output_path"];
    const auto width = std::stoi(args.args["width"]);
    const auto height = std::stoi(args.args["height"]);

    if(initSDK(modelConfig) != 0){
        printf("[FACEPASS_TEST] sdk init error! \n");
        release();
        return 1;
    }

    if(run_option == "bgr"){
        cw::File image_folder(inputFolder);
        if(image_folder.is_dir() && image_folder.exists()){
            auto image_paths = cw::File(image_folder).list_file_by_ext_recursive(
                    {".PNG", ".JPG", ".JPEG", ".BMP"});
            printf("[FACEPASS_TEST] %d images in input_path %s\n", image_paths.size(), inputFolder.c_str());
            std::sort(image_paths.begin(), image_paths.end());
            trackProcessForImages(image_paths, outFolder);
        }
        else{
            printf("[FACEPASS_TEST] %s is not dir or cannot access!\n", inputFolder.c_str());
        }
    }
    else if(run_option == "yuv420"){
        trackProcessForVideo(inputFolder, outFolder, width, height);
    }
    else{
        printf("-r input params is illegal\n");
    }

    ::release();

    return 0;
}

