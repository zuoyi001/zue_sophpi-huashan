//
// Created by megvii on 19-6-24.
//

#ifndef FACEARC_HISI_TOOL_H
#define FACEARC_HISI_TOOL_H


#include<iostream>
#include<fstream>
#include<string>
#include<vector>
#include<map>
#include<chrono>

#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

#include "face_sdk.h"
#include <opencv2/imgcodecs/imgcodecs.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include "config.h"


using namespace std;

#define MIN_FACE 10

namespace cw{


    class Path{
        public:
            static string get_panrent_dir(string path){
                path = normalize_path(path);
                size_t pos = path.rfind("/");
                if( pos == string::npos ){
                    return path;
                }
                return path.substr(0, pos);
            }

            static string get_name(string path){
                path = normalize_path(path);
                size_t pos = path.rfind("/");
                if( pos == string::npos ){
                    return path;
                }
                return path.substr(pos + 1, path.size());
            }

            static string get_basename(string path){
                string name = get_name(path);
                size_t pos = name.rfind(".");
                if( pos == string::npos ){
                    return name;
                }
                return name.substr(0, pos);
            }

            static string get_basename_path(string path){
                size_t pos = path.rfind(".");
                if( pos == string::npos ){
                    return path;
                }
                return path.substr(0, pos);
            }

            static string get_extname(string path){
                string name = get_name(path);
                size_t pos = name.rfind(".");
                if( pos == string::npos ){
                    return "";
                }
                return name.substr(pos, name.size());
            }

            static string join(string path, string name){
                return normalize_path(path) + "/" + name;
            }

            static string normalize_path(const string &path){
                string ret = path;
                if(ret[ret.size()-1] == '/'){
                    ret = ret.substr(0, ret.size()-1);
                }
                return ret;
            }
    };

    class File{
        public:
            File(string path){
                m_path = path;
            }

            string path(){
                return m_path;
            }

            bool is_dir(){
                struct stat path_stat;
                stat(m_path.c_str(), &path_stat);
                return S_ISDIR(path_stat.st_mode);
            }

            bool exists(){
                return access( m_path.c_str(), F_OK ) != -1;
            }

            bool make_dir(){
                return mkdir(m_path.c_str(), 0777) == 0;
            }

            vector<string> list_file(){
                vector<string> ret;
                auto d = opendir(m_path.c_str());
                if(d){
                    for(auto dir = readdir(d); dir != NULL; dir = readdir(d)){
                        ret.push_back( Path::join(m_path, dir->d_name) );
                    }
                    closedir(d);
                }
                return ret;
            }

            vector<string> list_file_by_ext(vector<string> exts){
                vector<string> ret;
                for(auto &ext : exts){
                    ext = to_uppercase(ext);
                }
                for(auto &p: list_file()){
                    string ext_name = to_uppercase(Path::get_extname(p));
                    if (find(exts.begin(), exts.end(),ext_name) != exts.end()){
                        ret.push_back(p);
                    }
                }
                return ret;
            }

            vector<string> list_file_by_ext_recursive(vector<string> exts){
                vector<string> ret;
                for(auto &ext : exts){
                    ext = to_uppercase(ext);
                }
                for(auto &path : list_file() ){
                    File file(path);
                    if( file.is_dir() ){
                        string basename = Path::get_name(path);
                        if( basename == "." || basename == ".." ){
                            continue;
                        }
                        auto list = file.list_file_by_ext_recursive(exts);
                        ret.insert(ret.end(), list.begin(), list.end());
                        continue;
                    }
                    string ext_name = to_uppercase(Path::get_extname(path));
                    if( find(exts.begin(), exts.end(), ext_name) != exts.end() ){
                        ret.push_back(path);
                    }
                }

                return ret;
            }

        private:
            string m_path;

            string to_uppercase(string str){
                for_each(str.begin(), str.end(), [](char &c){ c = ::toupper(c); } );
                return str;
            }

    };

    class FileIO{
        public:
            template <typename T>
                static bool readFile(string path, vector<T> &data){
                    ifstream ifs(path, ios::binary | ios::ate);
                    if(ifs.fail()){
                        return false;
                    }

                    size_t size = ifs.tellg();

                    data.resize((size + sizeof(T) - 1) / sizeof(T));
                    ifs.seekg(ios::beg);
                    ifs.read((char *) data.data(), size);

                    if(ifs.fail()){
                        return false;
                    }

                    return true;
                }

            template <typename T>
                static bool writeFile(string path, const vector<T> &data){

                    ofstream ofs(path, ios::binary);
                    if(ofs.fail()){
                        return false;
                    }

                    ofs.write((char *) data.data(), data.size() * sizeof(T));

                    if(ofs.fail()){
                        return false;
                    }

                    return true;
                }

            static bool readString(string path, string &output){
                ifstream ifs(path);
                if(ifs.fail()){
                    return false;
                }

                output.clear();
                for(char c; ifs.get(c); )
                    output += c;

                return true;
            }

            static bool readLines(string path, vector<string> &lines){
                ifstream ifs(path);

                if(ifs.fail()){
                    return false;
                }

                lines.clear();
                for(string line; getline(ifs, line); ){
                    lines.push_back(line);
                }

                if(ifs.fail()){
                    return false;
                }

                return true;
            }

    };

    struct ProfilerInfo{
        ProfilerInfo():total(0.0),count(0){}
        double total;
        uint32_t count;
    };

    class ScopedProfiler{
        public:
            typedef chrono::high_resolution_clock Clock;
            //typedef chrono::milliseconds Unit;
            typedef chrono::nanoseconds Unit;

            ScopedProfiler(string tag, uint32_t count = 1):m_count(count),m_tag(tag),m_profiler_info(nullptr){
                m_start = Clock::now();
            }

            ScopedProfiler(string tag, ProfilerInfo *pi, uint32_t count = 0):m_count(count),m_tag(tag),m_profiler_info(pi){
                if(m_profiler_info) m_profiler_info->count++;
                m_start = Clock::now();
            }

            ~ScopedProfiler(){
                chrono::duration<double> elapsed = Clock::now() - m_start;
                Unit u = chrono::duration_cast<Unit>(elapsed);
                if(m_profiler_info) m_profiler_info->total += (double) u.count() / 1000000.0;
                if(m_count != 0) printf("=== [%s] count: %u avgtime: %lf ms\n", m_tag.c_str(), m_count, (double) u.count() / m_count / 1000000.0 );
            }
        private:
            uint32_t m_count;
            string m_tag;
            ProfilerInfo *m_profiler_info;
            chrono::time_point<chrono::system_clock> m_start;
    };


    class ProfilerFactory{
        public:
            static ProfilerFactory *get(){
                return &s_instance;
            }

            ~ProfilerFactory(){
                for(auto &iter : m_info_maps){
                    printf("=== [ProfilerFactory] [%s] count: %u avgtime: %lf ms\n", iter.first.c_str(), iter.second.count , iter.second.total / iter.second.count);
                }
            }

            ScopedProfiler make(string tag){
                return ScopedProfiler(tag, &m_info_maps[tag], 1);
            }

        private:
            static ProfilerFactory s_instance;
            map<string,ProfilerInfo> m_info_maps;
    };

    ProfilerFactory ProfilerFactory::s_instance;

}

class ImageUtil {
    public:
        static vector<char> readFile(string path) {
            vector<char> ret;
            ifstream ifs(path, ios::ate | ios::binary);
            size_t size = ifs.tellg();
            ifs.seekg(0, ios::beg);
            ret.resize(size);
            if (!ifs.read(ret.data(), size)) {
                printf("read %s failed\n", path.c_str());
                return vector<char>();
            }
            //printf("read %s %u byte\n", path.c_str(), size);
            return ret;
        }

        static cv::Mat readYuvByte(string path, int w, int h) {
            auto data = readFile(path);
            cv::Mat ret(h + h / 2, w, CV_8UC1, data.data());
            cv::cvtColor(ret, ret, cv::COLOR_YUV2BGR_IYUV);
            return ret;
        }

        static cv::Mat normalizeImage(const cv::Mat& image, int w, int h) {
            int iw = image.cols;
            int ih = image.rows;
            float aspet_ratio = (float)iw / ih;
            if (aspet_ratio > (float)w / h) {
                iw = w;
                ih = w / aspet_ratio;
            } else {
                ih = h;
                iw = h * aspet_ratio;
            }
            printf("iw:%d,ih:%d\n", iw, ih);
            cv::Mat image_resized = cv::Mat::zeros(ih, iw, image.type());
            cv::resize(image, image_resized, image_resized.size());
            cv::Mat ret = cv::Mat::zeros(h, w, image.type());
            int pw = (w - iw) / 2;
            int ph = (h - ih) / 2;
            image_resized.copyTo(ret(cv::Rect(pw, ph, iw, ih)));
            return ret;
        }

        static void drawBox(cv::Mat& img, const FaceRect& rect, const int id, const cv::Scalar& color){

            cv::rectangle(img, cv::Point(rect.left, rect.top), cv::Point(rect.right, rect.bottom), color, 2);

            const string text = std::to_string(id);
            const int fontFace = cv::FONT_HERSHEY_SIMPLEX;
            const double fontScale = 1;
            const int thickness = 2;

            int baseline=0;
            cv::Size textSize = cv::getTextSize(text, fontFace,
                    fontScale, thickness, &baseline);
            baseline += thickness;

            // center the text
            cv::Point textOrg(rect.left, rect.top+textSize.height);

            // draw the box
            cv::rectangle(img, textOrg - cv::Point(0, textSize.height),
                    textOrg + cv::Point(textSize.width, baseline),
                    color, cv::FILLED);

            // then put the text itself
            cv::putText(img, text, textOrg, fontFace, fontScale,
                    cv::Scalar::all(255), thickness, 8);
            //cv::Scalar::all(255), thickness, 8, true);

        }

        static void drawScore(cv::Mat& img, const cv::Point& leftTop, const float score, const cv::Scalar& color){

            const string text = "live:" + std::to_string(score);
            const int fontFace = cv::FONT_HERSHEY_SIMPLEX;
            const double fontScale = 1;
            const int thickness = 2;

            int baseline=0;
            cv::Size textSize = cv::getTextSize(text, fontFace,
                    fontScale, thickness, &baseline);
            baseline += thickness;

            // center the text
            cv::Point textOrg = leftTop - cv::Point(0, textSize.height);

            cv::putText(img, text, textOrg, fontFace, fontScale,
                    color, thickness, 8);
        }
        static void drawTrackResult(const FaceHandle* const faceHandles, const int length, cv::Mat& image, const int frameId){

            cv::putText(image, to_string(frameId).c_str() ,cv::Point(50, 50), cv::FONT_HERSHEY_PLAIN, 3.0, cv::Scalar(0, 255, 255), 2 );
            for(int i = 0; i < length; ++i){
                FaceRect rect;
                getFaceRect(faceHandles[i], &rect);
                int trackId;
                getTrackId(faceHandles[i], &trackId);
                cv::rectangle(image, cv::Point(rect.left, rect.top), cv::Point(rect.right, rect.bottom), cv::Scalar(0, 255, 255), 2);
                cv::putText(image, to_string(trackId).c_str() ,cv::Point(rect.left, rect.top), cv::FONT_HERSHEY_PLAIN, 2, cv::Scalar(0, 255, 0), 2 );
            }
        }
};


class FacePassProWrapper{
    public:
        FacePassProWrapper(const string &path,  const string &image_type, int width = 0, int height = 0)
        {
            FaceRetCode state=RET_OK;

            if(image_type == "bgr")
            {
                img = cv::imread(path.c_str());
                image = {img.data, nullptr, -1, img.cols, img.rows, img.cols * 3, img.rows, Format_BGR888};
                //printf("[FACEPASS_TEST] image width = %d, image_height = %d\n", image.cols, image.rows);
                state = detect((const Image* const)&image, 0.75, MIN_FACE, faceHandleArr.data(), &face_num);
                faceHandleArr.resize(face_num);
            }
            else if(image_type == "yuv420")
            {
                vector<char> data = ImageUtil::readFile(path);
                img = cv::Mat(height*3/2, width, CV_8UC1, static_cast<void*>(data.data())).clone();
                image = {img.data, nullptr, -1, width, height, width, height, Format_YUV420P_YU12};
                state = detect((const Image* const)&image, 0.75, MIN_FACE, faceHandleArr.data(), &face_num);
                faceHandleArr.resize(face_num);
            }
            else if(image_type == "binary")
            {
                vector<char> data = ImageUtil::readFile(path);
                //printf("[FACEPASS_TEST] data.size = %d\n", data.size());
                img = cv::Mat(height, width, CV_8UC3, data.data()).clone();
                image = {img.data, nullptr, -1, img.cols, img.rows, img.cols * 3, img.rows, Format_BGR888};
                state = detect((const Image* const)&image, 0.75, MIN_FACE, faceHandleArr.data(), &face_num);
                faceHandleArr.resize(face_num);
            }
            if(RET_NO_FACE == state){
                printf("[FACEPASS_TEST]  detect no faces in %s!\n", path.c_str());
            }
            else if(RET_OK != state){
                printf("[FACEPASS_TEST] error occurred during detect!\n");
            }

            printf("[FACEPASS_TEST] detect faces number is %d\n", face_num);
        }

        ~FacePassProWrapper(){
            for(int i = 0; i < face_num; ++i){
                releaseFace(faceHandleArr[i]);
            }
        }

        vector<char> extract_max_face(){

            int index = -1;
            int max_area = 0;

            if(face_num <= 0){
                printf("[FACEPASS_TEST] no faces\n");
            }

            for(int i = 0; i < face_num; i++){
                FaceRect rect;
                getFaceRect(faceHandleArr[i], &rect);
                int area = (rect.right - rect.left) * (rect.bottom - rect.top);
                if( area > max_area ){
                    max_area = area;
                    index = i;
                }
            }
            vector<char> ret;
            if(index != -1){
                char *feature;
                int feature_len;
                extract((const Image* const)&image, faceHandleArr[index], &feature, &feature_len);
                ret.resize(feature_len);
                memcpy(ret.data(), feature, feature_len * sizeof(char));
                releaseFeature(feature);
                feature = nullptr;

            }


            return ret;
        }

    private:
        int face_num = 128;/*max number of detected faces*/
        char *feature = nullptr;
        int feature_len = 0;
        cv::Mat img;
        Image image = {0};
        std::vector<FaceHandle> faceHandleArr = std::vector<FaceHandle>(128, nullptr);
};


int initSDK(const string &configPath){
    FaceModels models;
    memset(&models, 0 , sizeof(FaceModels));

    Config::setParameterFile ( configPath );

    string anchor_path        = Config::get<string> ("anchor_path");
    string detect_model       = Config::get<string> ("detect_model");
    string postfilter_model   = Config::get<string> ("postfilter_model");
    string poseblur_model     = Config::get<string> ("poseblur_model");
    string refine_model       = Config::get<string> ("refine_model");

    string liveness_bgr_model     = Config::get<string> ("liveness_bgr_model");
    string liveness_ir_model     = Config::get<string> ("liveness_ir_model");
    string liveness_bgrir_model     = Config::get<string> ("liveness_bgrir_model");

    string age_gender_model   = Config::get<string> ("age_gender_model");
    string rc_model           = Config::get<string> ("rc_model");
    string occl_model           = Config::get<string> ("occl_model");
    string stn_model_path     = Config::get<string> ("stn_model");
    string feature_model      = Config::get<string> ("feature_model");
    string group_model_path   = Config::get<string> ("group_model_path");


    models.anchor_path        = anchor_path.c_str();
    models.detect_model       = detect_model.c_str();
    models.postfilter_model   = postfilter_model.c_str();
    models.pose_blur_model    = poseblur_model.c_str();
    models.refine_model       = refine_model.c_str();

    models.liveness_bgr_model      = liveness_bgr_model.c_str();
    models.liveness_ir_model      = liveness_ir_model.c_str();
    models.liveness_bgrir_model      = liveness_bgrir_model.c_str();

    models.age_gender_model   = age_gender_model.c_str();
    models.rc_model           = rc_model.c_str();
    models.occl_model           = occl_model.c_str();
    models.stn_model          = stn_model_path.c_str();
    models.feature_model      = feature_model.c_str();
    models.group_model_path   = group_model_path.c_str();

    auto detect_threshold       = Config::get<float> ("detect_threshold" );
    auto postfilter_threshold   = Config::get<float> ("postfilter_threshold");

    printf("[FACEPASS_TEST] using anchor_path: %s\n", models.anchor_path);
    printf("[FACEPASS_TEST] using detect_model: %s\n", models.detect_model);
    printf("[FACEPASS_TEST] using postfilter_model: %s\n", models.postfilter_model);
    printf("[FACEPASS_TEST] using pose_blur_model: %s\n", models.pose_blur_model);
    printf("[FACEPASS_TEST] using refine_model: %s\n", models.refine_model);

    printf("[FACEPASS_TEST] using liveness_bgr_model: %s\n", models.liveness_bgr_model);
    printf("[FACEPASS_TEST] using liveness_ir_model: %s\n", models.liveness_ir_model);
    printf("[FACEPASS_TEST] using liveness_bgrir_model: %s\n", models.liveness_bgrir_model);

    printf("[FACEPASS_TEST] using age_gender_model: %s\n", models.age_gender_model);
    printf("[FACEPASS_TEST] using rc_model: %s\n", models.rc_model);
    printf("[FACEPASS_TEST] using occl_model: %s\n", models.occl_model);
    printf("[FACEPASS_TEST] using stn_model: %s\n", models.stn_model);
    printf("[FACEPASS_TEST] using feature_model: %s\n", models.feature_model);
    printf("[FACEPASS_TEST] using group_model_path: %s\n", models.group_model_path);

    printf("[FACEPASS_TEST] using detect param: detect_threshold:%f, postfilter_threshold:%f\n", detect_threshold, postfilter_threshold);
    const char *sdk_info = getVersion();
    printf("[FACEPASS_TEST] sdk version is %s\n", sdk_info);

    auto ret = ::init(models);
    if (ret != RET_OK) {
        printf("[FACEPASS_TEST] error: init return %d\n", ret);
        return 1;
    }
    set_detect_config(detect_threshold, postfilter_threshold);

    //    set_match_config(0.991393, 16.6103,  1.07092, -45.5217);
    set_match_config(1.00758 ,-52.66727, 1.00936, 12.62161, 0.4);

    return 0;
};

std::vector<std::pair<std::string, std::vector<char>>> getFeatureWithName(const char* path, const std::string& image_type, const int width, const int height){
    vector<pair<string, vector<char>>> vFeatures;
    auto file = cw::File(path);

    if(file.is_dir()){
        vector<string>imagePaths;

        if(image_type == "bgr"){
            imagePaths = file.list_file_by_ext({".jpg", ".bmp", ".png"});
        }
        else if(image_type == "yuv420"){
            imagePaths = file.list_file_by_ext({".yuv"});
        }
        else if(image_type == "binary"){
            imagePaths = file.list_file_by_ext({".bmp"}); //define by usr
        }
        printf("[FACEPASS_TEST] %d images in %s\n", imagePaths.size(), path);

        for (auto &item : imagePaths){
            printf("[FACEPASS_TEST] image path : %s\n", item.c_str());

            auto feature = FacePassProWrapper(item, image_type, width, height).extract_max_face();
            if(!feature.empty()){
                vFeatures.push_back(make_pair<string, vector<char>>(cw::Path::get_basename(item), move(feature)));
            }
        }

    }
    else{
        auto feature = FacePassProWrapper(path, image_type, width, height).extract_max_face();
        if(!feature.empty()){
            vFeatures.push_back(make_pair<string, vector<char>>(cw::Path::get_basename(path), move(feature)));
        }
    }

    return vFeatures;
}

#endif //FACEARC_HISI_TOOL_H
