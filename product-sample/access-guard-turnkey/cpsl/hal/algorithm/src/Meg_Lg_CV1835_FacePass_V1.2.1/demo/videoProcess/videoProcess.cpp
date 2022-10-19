//
// Created by megvii on 19-6-26.
//

#include "tool.h"
#include "argparse.h"
#include "shared_quene.h"
#include <thread>
#include <atomic>
#include <numeric>
#include <fstream>
#include "sqlite3.h"

class FaceGallery{
    public:
        FaceGallery(){
            createFaceGroup(&faceGroupHandle);
        };
        FaceGallery(const FaceGallery&) = delete;
        FaceGallery& operator=(FaceGallery const&)= delete;
        // Move constructor.
        FaceGallery(FaceGallery&& other) noexcept
        {
            faceGroupHandle = other.faceGroupHandle;
            other.faceGroupHandle = nullptr;
            name_map = std::move(other.name_map);
            feature_map = std::move(other.feature_map);
            available_id = other.available_id;
            other.available_id = 0;
        }
        // Move assignment operator.
        FaceGallery& operator=(FaceGallery&& other) noexcept
        {
            if (this != &other)
            {
                // Free the existing resource.
                if(faceGroupHandle){
                    releaseFaceGroup(faceGroupHandle);
                }
                //std::swap(s, other.faceHandles);
                faceGroupHandle = other.faceGroupHandle;
                other.faceGroupHandle = nullptr;
                name_map = std::move(other.name_map);
                feature_map = std::move(other.feature_map);
                available_id = other.available_id;
                other.available_id = 0;
                //std::swap(length, other.length);
            }
            return *this;
        }

        ~FaceGallery(){
            if(faceGroupHandle){
                releaseFaceGroup(faceGroupHandle);
            }
        };
        FaceRetCode insert(const std::string& name, std::vector<char>& feat){
            //TODO find unused id in group if available_id overflow
            const FaceRetCode retCode = insertFaceGroup(faceGroupHandle, feat.data(), feat.size(), available_id);
            if(retCode == RET_OK){
                name_map[available_id] = name;
                feature_map[available_id] = feat;
                ++available_id;
            }
            else{
                printf("[FACEPASS_TEST] cannot insert %s\n", name.c_str());
            }
            return retCode;
        }
        FaceRetCode search(const char* feat, const int feature_length, std::string& name, float& score){
            uint64_t faceId;
            const FaceRetCode retCode = identifyFromFaceGroup(faceGroupHandle, feat, feature_length, &score, &faceId);
            if(retCode == RET_OK){
                name = name_map[faceId];
            }
            else{
                printf("[FACEPASS_TEST] can not query from face group!\n");
            }
            return retCode;
        }
        int storage(const std::string& db_name) {
            sqlite3 *db;
            int rc;

            rc = sqlite3_open(db_name.c_str(), &db);

            if( rc ) {
                fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
                return 1;
            } else {
                fprintf(stderr, "Opened database successfully\n");
            }
            std::string query = "CREATE TABLE IF NOT EXISTS baseGroup (faceId INTEGER PRIMARY KEY NOT NULL, Name TEXT, feature BLOB);";
            rc = sqlite3_exec(db, query.c_str(), 0, 0, nullptr);
            if( rc ) {
                fprintf(stderr, "sqlite - create table statement: %s\n", sqlite3_errmsg(db));
                return 1;
            }
            query = "INSERT INTO baseGroup (faceId, name, feature) VALUES (?,?,?);";

            sqlite3_stmt *stmt;
            sqlite3_prepare_v2(db, query.c_str(), query.size(), &stmt, nullptr);

            // Iterate over an unordered_map using range based for loop
            for (const std::pair<uint64_t, std::string>& face : name_map){
                const uint64_t faceId = face.first;
                const std::string& name = face.second;
                const std::vector<char>& feature = feature_map[faceId];

                rc = sqlite3_bind_int(stmt, 1, faceId);
                if( rc ) {
                    fprintf(stderr, "sqlite - bind prepared int statement: %s\n", sqlite3_errmsg(db));
                    return 1;
                }
                rc = sqlite3_bind_text(stmt, 2, name.c_str(), name.size(), SQLITE_STATIC);
                if( rc ) {
                    fprintf(stderr, "sqlite - bind prepared text statement: %s\n", sqlite3_errmsg(db));
                    return 1;
                }
                rc = sqlite3_bind_blob(stmt, 3, feature.data(), feature.size(),
                        SQLITE_TRANSIENT);
                if(rc) {
                    fprintf(stderr, "sqlite - bind prepared blob statement: %s\n", sqlite3_errmsg(db));
                    return 1;
                }
                rc = sqlite3_step(stmt);
                if( SQLITE_DONE != rc && SQLITE_OK != rc) {
                    fprintf(stderr, "sqlite - bind step statement: %s\n", sqlite3_errmsg(db));
                    return 1;
                }
                sqlite3_reset(stmt);
            }
            sqlite3_finalize(stmt);

            sqlite3_close(db);
            return 0;
        }
    private:
        std::unordered_map<uint64_t, string> name_map;
        std::unordered_map<uint64_t, std::vector<char>> feature_map;
        FaceGroupHandle faceGroupHandle=nullptr;
        uint64_t available_id = 0;
};

class FrameImage{
    public:
        FrameImage(size_t imageSize){
            image.vir_addr = (void *)malloc(imageSize);
        }

        ~FrameImage(){
            if(image.vir_addr){
                free(image.vir_addr);
                image.vir_addr = nullptr;
            }
        }

        Image image = {0};
};

typedef enum{
    Track_Reject = 0,
    Track_Doing,
    Track_Pass,
    Track_Strange,
}Track_State;

struct Track_data{
    FaceHandle faceHandle;
    int trackId;
    Track_data(){
        faceHandle = nullptr;
        trackId = 0;
    }
    std::shared_ptr<FrameImage> image;
};
using TrackBuffer = shared_queue<std::shared_ptr<Track_data>>;

int get_face_size(FaceRect& face);


std::vector<int> sortBySize(const FaceHandle faceHandleArr[], const int count);

bool check_update_track_list(list<pair<int, Track_State>> &track_list, int trackId);

FaceGallery create_face_gallery(const std::string& basePath, const std::string& runOption, const int width, const int height) {
    auto base_features = getFeatureWithName(basePath.c_str(), runOption, width, height);
    printf("[FACEPASS_TEST] base size : %d\n", base_features.size());
    FaceGallery face_gallery;
    for(size_t i = 0; i < base_features.size(); ++i){
        const FaceRetCode retCode = face_gallery.insert(base_features[i].first, base_features[i].second);
        if(RET_OK != retCode){
            break;
        }
    }
    return std::move(face_gallery);
}


void seekOptimum(FaceHandle faceHandleArr[], const int count, list<pair<int, Track_State>>& track_list, std::mutex& track_list_mutex, TrackBuffer& g_track_buffer, std::shared_ptr<FrameImage> &frameImage){
    std::vector<int> sizeDescIdx = sortBySize(faceHandleArr, count);
    size_t i = 0;
    for(;i<sizeDescIdx.size();++i){
        /// query track state
        int track_id;
        getTrackId(faceHandleArr[sizeDescIdx[i]], &track_id);
        bool need_process = false;
        {
            std::lock_guard<std::mutex> guard(track_list_mutex);
            need_process = check_update_track_list(track_list, track_id);
            list<pair<int, Track_State>>::iterator track_iterator;
            for(track_iterator = track_list.begin(); track_iterator != track_list.end(); ++track_iterator){
                printf("trackId->state:%d-->%d||", track_iterator->first, track_iterator->second);
            }
            printf("\n");
        }
        if(need_process == false){
            releaseFace(faceHandleArr[sizeDescIdx[i]]);
            faceHandleArr[sizeDescIdx[i]] = nullptr;
            continue;
        }

        FacePoseBlur pose_blur;
        Image *image = &(frameImage.get()->image);
        FaceRetCode retCode = getPoseBlurAttribute((const Image* const)image, faceHandleArr[sizeDescIdx[i]], &pose_blur);
        if(RET_OK == retCode){
            printf("[FACEPASS_TEST] getPoseBlurAttribute:roll=%f--yaw=%f--pitch=%f--blur=%f\n", pose_blur.roll, pose_blur.yaw, pose_blur.pitch, pose_blur.blur);
        }
        else{
            printf("[FACEPASS_TEST] can not getPoseBlurAttribute!\n");
        }

        if (std::abs(pose_blur.yaw) < 45 && std::abs(pose_blur.pitch) < 45 && pose_blur.blur < 0.8) {
            /// push face
            auto track_data = make_shared<Track_data>();
            track_data->faceHandle = faceHandleArr[sizeDescIdx[i]];
            track_data->trackId = track_id;
            track_data->image = frameImage;
            g_track_buffer.producer(track_data);
            printf("g_track_buffer length = %d\n", g_track_buffer.size());
            ++i;//jump release
            break;
        }
        else {
            releaseFace(faceHandleArr[sizeDescIdx[i]]);
            faceHandleArr[sizeDescIdx[i]] = nullptr;
            printf("[FACEPASS_TEST] face can not pass poseblur!!\n");
        }
    }
    for(;i<sizeDescIdx.size();++i){
        releaseFace(faceHandleArr[sizeDescIdx[i]]);
        faceHandleArr[sizeDescIdx[i]] = nullptr;
    }
}

std::vector<int> sortBySize(const FaceHandle faceHandleArr[], const int count){
    std::vector<int> index(count);
    std::iota(index.begin(), index.end(), 0);
    std::vector<int> face_size(count);
    for(int i=0;i<count;++i){
        FaceRect face_rect;
        getFaceRect(faceHandleArr[i], &face_rect);
        face_size[i] = get_face_size(face_rect);
    }
    std::stable_sort(index.begin(), index.end(),
            [&face_size](size_t i1, size_t i2) {
            return face_size[i1] > face_size[i2];});
    return index;
}

int get_face_size(FaceRect& face) {
    return (face.right - face.left) * (face.bottom - face.top);
}

bool check_update_track_list(list<pair<int, Track_State>> &track_list, int trackId){
    if(track_list.empty()){
        track_list.push_front(make_pair(trackId, Track_Doing));
        return true;
    }

    for(auto item: track_list){
        if(item.first == trackId){
            if( item.second == Track_Reject) {
                printf("track_id %d don not pass, need do again!!\n", trackId);
                return true;
            }
            else if(item.second == Track_Strange){
                printf("track_id %d recognize score is too low, is a stranger, need not do !!\n", trackId);
                return false;
            }
            else{
                printf("track_id %d is doing or pass, need not do !!\n", trackId);
                return false;
            }

        }
    }

    if(track_list.size() == 10){
        track_list.pop_back();
    }

    printf("this track_id  is a new track, need do !!\n");
    track_list.push_front(make_pair(trackId, Track_Doing));
    return true;
}

void update_track_states(list<pair<int, Track_State>> &track_list, int trackId, Track_State track_state){
    for(auto &item : track_list){
        if(item.first == trackId){
            item.second = track_state;
            return;
        }
    }
}

FaceRetCode extractAndSearch(const Image* const image, FaceGallery& face_gallery, const FaceHandle& face, string& name, float& score){
    char *feature;
    int feature_length;
    FaceRetCode retCode;
    {
        auto sp = cw::ProfilerFactory::get()->make("extract + identify");
        retCode = extract(image, face, &feature, &feature_length);
        if(RET_OK != retCode){
            printf("[FACEPASS_TEST] can not extract feature!\n");
        }
        retCode = face_gallery.search(feature, feature_length, name, score);
    }
    retCode = releaseFeature(feature);
    feature = nullptr;
    return retCode;
}

static void help(){
    cout << "\nThis program demostrates video processing in facepass sdk \n"
        "runOption stand for register image type\n"
        "Usage: \n"
        "./videoProcess model.yaml -r bgr -i ./video.yuv -b ./base_path/  -o ./out_folder/\n"
        "./videoProcess model.yaml -r yuv420 -i ./video.yuv -b ./base_path/  -o ./out_folder/ -w width -h height\n";
}

int main(int argc, char *argv[]){

    help();

    ArgumentParser parser("FACEPASS_TEST IR PROCESS");
    parser.add_positional_arg("model_config", "1", "the path of the model.yaml");
    parser.add_keyword_arg("-r", "--run_option", "", true, "", "register image type");
    parser.add_keyword_arg("-i", "--input_path", "", true, "", "the folder of images or video file path");
    parser.add_keyword_arg("-b", "--base_group", "", true, "", "the folder of base group");
    parser.add_keyword_arg("-o", "--output_path", "", true, "", "the folder for write image");
    parser.add_keyword_arg("-w", "--width", "", false, "1080", "the width of input image");
    parser.add_keyword_arg("-h", "--height", "", false, "1920", "the height of input image");

    ArgumentResult args = parser.parse_args(argc, argv);

    const auto modelConfig =  args.args["model_config"];
    const auto runOption   =  args.args["run_option"];
    const auto inputPath   =  args.args["input_path"];
    const auto basePath    =  args.args["base_group"];
    const auto outputFolder    =  args.args["output_path"];
    const auto width = std::stoi(args.args["width"]);
    const auto height = std::stoi(args.args["height"]);


    if(initSDK(modelConfig) != 0){
        printf("[FACEPASS_TEST] sdk init error! \n");
        release();
        return 1;
    };

    FaceGallery face_gallery = create_face_gallery(basePath, runOption, width, height);
    face_gallery.storage(cw::Path::join(outputFolder, "featureDB.db"));

    std::ifstream is (inputPath.c_str(), std::ifstream::binary);
    if(is){
        printf("[FACEPASS_TEST] read video path %s\n", inputPath.c_str());
    }
    else{
        printf("[FACEPASS_TEST] cannot read video path %s\n", inputPath.c_str());
        return 1;
    }

    size_t image_size = (width * height * 3)>>1;
    std::vector<unsigned char> yuv_buffer(image_size);

    volatile bool b_detect = true;
    TrackBuffer g_track_buffer;
    std::mutex track_list_mutex;
    list<pair<int, Track_State>> track_list;

    auto detect_thread = [&]{
        std::atomic<size_t>frameId(0);
        while(is.good() && frameId<1000){
            is.read((char*)(yuv_buffer.data()), image_size);
            printf("[FACEPASS_TEST] Frame Id : %d\n", frameId.load());

            /// ignore read cost
            auto sp = cw::ProfilerFactory::get()->make("detect + process");
            int count = 128;
            std::vector<FaceHandle> faceHandleArr(count, nullptr);
            FaceRetCode retCode;
            auto frameImage = std::make_shared<FrameImage>(image_size);

            memcpy(frameImage->image.vir_addr, yuv_buffer.data(), image_size);
            frameImage->image.width    = width;
            frameImage->image.height   = height;
            frameImage->image.wstride    = width;
            frameImage->image.hstride   = height;
            frameImage->image.pixel_format = Format_YUV420P_YU12;
            Image *image = &(frameImage.get()->image);
            retCode = detectAndTrack(image, 0.75, MIN_FACE, faceHandleArr.data(), &count);
            faceHandleArr.resize(count);
            if(RET_NO_FACE == retCode){
                printf("[FACEPASS_TEST] detect no faces in frame %d!\n", frameId.load());
            }
            else if (RET_OK == retCode) {
                /// save result will be slow
                cv::Mat mat_yuv(height + (height>>1), width, CV_8U, yuv_buffer.data());
                  cv::Mat img;
                  cv::cvtColor(mat_yuv, img, cv::COLOR_YUV2BGR_IYUV);
                  ImageUtil::drawTrackResult(faceHandleArr.data(), count, img, frameId.load());
                  const size_t numberWidth = 5;
                  const string indexString = std::string(numberWidth-std::to_string(frameId).size(), '0').append( std::to_string(frameId));
                  const string result = cw::Path::join(outputFolder, indexString + ".jpg");
                  cv::imwrite(result, img);

                printf("[FACEPASS_TEST] detect face count = %d\n", count);
                seekOptimum(faceHandleArr.data(), count, track_list, track_list_mutex, g_track_buffer, frameImage);
            }
            else {
                printf("[FACEPASS_TEST] error occurred during detect!\n");
                b_detect = false;
                break;
            }
            ++frameId;
            /*if(stream.eof()){
              is.seekg (0, is.beg);
              }*/
        }
        b_detect = false;
        printf("[FACEPASS_TEST] detect thread exited\n");
        g_track_buffer.set_runout();
        g_track_buffer.invoke_all();

    };


    auto recognize_thread = [&] {
        while(b_detect){
            auto track_data = g_track_buffer.consume();
            if(track_data == nullptr){
                break;
            }

            std::string face_name;
            float score;
            Image& image = track_data->image.get()->image;
            FaceRetCode retCode = extractAndSearch(&image, face_gallery, track_data->faceHandle, face_name, score);
            printf("[FACEPASS_TEST] (%f)--%s\n", score, face_name.c_str());

            if( 71 < score ){
                printf("[FACEPASS_TEST] pass the recognize!!\n");
                float liveness_score;
                retCode = getLiveness_bgr(&image, track_data->faceHandle, &liveness_score);

                if(RET_OK == retCode){
                    if(0.6 < liveness_score){
                        printf("livenss score = %f== pass the liveness process!!!\n", liveness_score);
                        std::lock_guard<std::mutex> guard(track_list_mutex);
                        update_track_states(track_list, track_data->trackId,Track_Pass);
                    }
                    else{
                        printf("livenss score = %f==can not pass the liveness process!!!\n", liveness_score);
                        std::lock_guard<std::mutex> guard(track_list_mutex);
                        update_track_states(track_list, track_data->trackId,Track_Reject);
                    }
                }
                else{
                    printf("[FACEPASS_TEST] can not detect liveness!\n");
                }
            }
            else if(score < 55){
                printf("regard as a stranger!!!");
                std::lock_guard<std::mutex> guard(track_list_mutex);
                update_track_states(track_list, track_data->trackId,Track_Strange);
            }
            else{
                printf("cannot pass the recognize!!\n");
                std::lock_guard<std::mutex> guard(track_list_mutex);
                update_track_states(track_list, track_data->trackId,Track_Reject);
            }

            // release faces in frame
            retCode = releaseFace(track_data->faceHandle);
        }
    };

    std::thread producer_thread(detect_thread);
    std::thread consumer_thread(recognize_thread);

    consumer_thread.join();
    printf("process recognize finished!!\n");

    producer_thread.join();
    printf("process detect finished!!\n");

    ::release();

    return 0;
}

