//
// Created by megvii on 19-6-25.
//


#include "tool.h"
#include "argparse.h"

void featureCompare(const vector<pair<string, vector<char>>> &base, const vector<pair<string, vector<char>>> &query)
{

    FaceGroupHandle faceGroupHandle;
    createFaceGroup(&faceGroupHandle);

    for(size_t i = 0; i < base.size(); i++)
    {
        FaceRetCode state = insertFaceGroup(faceGroupHandle, base[i].second.data(), base[i].second.size(), i);
        if(RET_OK != state){
            printf("[FACEPASS_TEST] can not insert %d to group!\n", i);
            break;
        }
    }
    size_t sum;
    getFaceGroupCount(faceGroupHandle, &sum);
    if(0==sum){
        printf("[FACEPASS_TEST] base group is empty!\n");
        return;
    }
    else{
        printf("[FACEPASS_TEST] base size = %d\n", sum);
    }

    //just demonstrate the usage
    const int featIndex = 0;
    FaceRetCode state = removeFaceGroup(faceGroupHandle, featIndex);
    if(RET_OK != state){
        printf("[FACEPASS_TEST] can not delete %d in base group!\n", featIndex);
    }
    state = insertFaceGroup(faceGroupHandle, base[featIndex].second.data(), base[featIndex].second.size(), featIndex);

#if 1//top 1
    uint64_t imageIndex;
    float score;
    int total_count = query.size();
    int positive_count = 0;
    for(auto &item : query)
    {
        state = identifyFromFaceGroup(faceGroupHandle, item.second.data(), item.second.size(), &score, &imageIndex);
        if(RET_OK == state){
            printf("[FACEPASS_TEST] %s--(%f)--%s\n", item.first.c_str(), score, base[imageIndex].first.c_str());
            string baseName = base[imageIndex].first;
            if(item.first.substr(0, baseName.length()) == baseName && score > 70.7)//name compare need modify by user.
            {
                positive_count++;
            }
        }
        else{
            printf("[FACEPASS_TEST] can not query %s from base group!\n", item.first.c_str());
            break;
        }

    }

    printf("[FACEPASS_TEST] total_count = %d--(acc = %f)--positive_count = %d\n", total_count, positive_count*1.0/total_count, positive_count);
#else 	// top k
    int k = 5;
    for(auto &item : query) {
        vector<uint64> index(k, 0);
        vector<float> scoreData(k, 0);
        state = top_k_FromFaceGroup(faceGroupHandle, item.second.data(), FEATURE_LENGTH, k, scoreData.data(), index.data());
        printf("[FACEPASS_TEST] %s query:\n", item.first.c_str());
        for (int i = 0; i < k; ++i) {
            printf("[FACEPASS_TEST] top_%d: (%f)--%s\n", i, scoreData[i], base[index[i]].first.c_str());
        }
    }
#endif

    releaseFaceGroup(faceGroupHandle);
    faceGroupHandle = nullptr;
}

void searchProcess(const char *basePath, const char *queryPath,  const string &image_type, int width = 0, int height = 0)
{
    auto baseFeatures = getFeatureWithName(basePath, "bgr", width, height);
    printf("[FACEPASS_TEST] base feature num : %d\n", baseFeatures.size());

    auto queryFeatures = getFeatureWithName(queryPath, image_type, width, height);
    printf("[FACEPASS_TEST] query feature num : %d\n", queryFeatures.size());

    sort(queryFeatures.begin(), queryFeatures.end());
    featureCompare(baseFeatures, queryFeatures);
}


static void help()
{
    cout << "\nThis program demostrates search interface on facepass dv300 \n"
        "input is image path or folder, for base and query\n"
        "give the highest score between query and every base, printf some match information\n"
        "Usage: \n"
        "query support bgr, yuv420 and binary image format, the last two need setting width and height, default 1080*1920\n"
        "./search  model.yaml -r bgr -i ./querypath/ -b ./basepath\n"
        "./search  model.yaml -r yuv420 -i ./querypath/ -b ./basepath/ -w width -h height\n"
        "../search model.yaml  -r binary -i ./query.jpg -b ./basepath/ -w width -h height\n\n";

}



int main(int argc, char *argv[])
{

    help();

    ArgumentParser parser("FACEPASS_DV300_TEST SEARCH");
    parser.add_positional_arg("model_config", "1", "the path of the model.yaml");
    parser.add_keyword_arg("-r", "--run_option", "", true, "", "bgr means bgr image type, yuv420 stands for yuv input");
    parser.add_keyword_arg("-i", "--input_path", "", true, "", "the folder or path for query images");
    parser.add_keyword_arg("-b", "--base_group", "", true, "", "the folder of base group");
    parser.add_keyword_arg("-w", "--width", "", false, "1080", "the width of input image");
    parser.add_keyword_arg("-h", "--height", "", false, "1920", "the height of input image");

    ArgumentResult args = parser.parse_args(argc, argv);

    const auto modelConfig =  args.args["model_config"];
    const auto image_type   =  args.args["run_option"];
    const auto basePath    =  args.args["base_group"];
    const auto queryPath   =  args.args["input_path"];
    const auto width = std::stoi(args.args["width"]);
    const auto height = std::stoi(args.args["height"]);

    if(initSDK(modelConfig) != 0){
        printf("[FACEPASS_TEST] sdk init error! \n");
    }
    else{
        searchProcess(basePath.c_str(), queryPath.c_str(), image_type, width, height);

    }
    ::release();

    return 0;
}





