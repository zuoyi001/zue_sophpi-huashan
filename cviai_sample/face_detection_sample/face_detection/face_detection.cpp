#include <cviruntime.h>
#include <fstream>
#include <iostream>
#include <map>
#include <numeric>
#include <opencv2/opencv.hpp>
#include <stdio.h>
#include <string>
#include <vector>

#define IMG_RESIZE_DIMS 640, 640
#define BGR_MEAN 127.5, 127.5, 127.5
#define INPUT_SCALE 0.00781 // 1/128.0
#define FACE_POINTS_SIZE 5
std::map<int, std::map<std::string, std::string>> fpn_out_nodes_;

struct anchor_cfg {
public:
  int STRIDE;
  std::vector<int> SCALES;
  int BASE_SIZE;
  std::vector<float> RATIOS;
  int ALLOWED_BORDER;

  anchor_cfg() {
    STRIDE = 0;
    SCALES.clear();
    BASE_SIZE = 0;
    RATIOS.clear();
    ALLOWED_BORDER = 0;
  }
};

typedef struct {
  float x1;
  float y1;
  float x2;
  float y2;
  float score;
} cvai_bbox_t;

typedef struct {
  float *x;
  float *y;
  uint32_t size;
} cvai_pts_t;

typedef struct {
  cvai_bbox_t bbox;
  cvai_pts_t pts;
} cvai_face_info_t;

typedef struct {
  uint32_t size;
  uint32_t width;
  uint32_t height;
  cvai_face_info_t *info;
} cvai_face_t;

struct anchor_box {
  float x1;
  float y1;
  float x2;
  float y2;
};



// base anchor
std::vector<std::vector<float>>
generate_mmdet_base_anchors(float base_size, float center_offset,
                            const std::vector<float> &ratios,
                            const std::vector<int> &scales) {
  std::vector<std::vector<float>> base_anchors;
  float x_center = base_size * center_offset;
  float y_center = base_size * center_offset;

  for (size_t i = 0; i < ratios.size(); i++) {
    float h_ratio = sqrt(ratios[i]);
    float w_ratio = 1 / h_ratio;
    for (size_t j = 0; j < scales.size(); j++) {
      float halfw = base_size * w_ratio * scales[j] / 2;
      float halfh = base_size * h_ratio * scales[j] / 2;
      // x1,y1,x2,y2
      std::vector<float> base_anchor = {x_center - halfw, y_center - halfh,
                                        x_center + halfw, y_center + halfh};
      std::cout << "anchor:" << base_anchor[0] << "," << base_anchor[1] << ","
                << base_anchor[2] << "," << base_anchor[3] << std::endl;
      base_anchors.emplace_back(base_anchor);
    }
  }
  return base_anchors;
}


std::vector<std::vector<float>>
generate_mmdet_grid_anchors(int feat_w, int feat_h, int stride,
                             std::vector<std::vector<float>> &base_anchors) {
  std::vector<std::vector<float>> grid_anchors;
  for (int ih = 0; ih < feat_h; ih++) {
    int sh = ih * stride;
    for (int iw = 0; iw < feat_w; iw++) {
      int sw = iw * stride;
      for (size_t k = 0; k < base_anchors.size(); k++) {
        auto &base_anchor = base_anchors[k];
        std::vector<float> grid_anchor = {
            base_anchor[0] + sw, base_anchor[1] + sh, base_anchor[2] + sw,
            base_anchor[3] + sh};
        grid_anchors.emplace_back(grid_anchor);
      }
    }
  }
  return grid_anchors;
}

float *getOutputRawPtr(const char *name, CVI_TENSOR *tensors, int num) {
  CVI_TENSOR *t = CVI_NN_GetTensorByName(name, tensors, num);
  if (t == nullptr) {
    return nullptr;
  }
  float *ptr = (float *)CVI_NN_TensorPtr(t);
  return ptr;
}

template <typename T>
void NonMaximumSuppression(std::vector<T> &bboxes, std::vector<T> &bboxes_nms,
                           const float threshold, const char method) {
  std::sort(bboxes.begin(), bboxes.end(),
            [](cvai_face_info_t &a, cvai_face_info_t &b) {
              return a.bbox.score > b.bbox.score;
            });

  int select_idx = 0;
  int num_bbox = bboxes.size();
  std::vector<int> mask_merged(num_bbox, 0);
  bool all_merged = false;

  while (!all_merged) {
    while (select_idx < num_bbox && mask_merged[select_idx] == 1)
      select_idx++;
    if (select_idx == num_bbox) {
      all_merged = true;
      continue;
    }

    bboxes_nms.emplace_back(bboxes[select_idx]);
    mask_merged[select_idx] = 1;

    cvai_bbox_t select_bbox = bboxes[select_idx].bbox;
    float area1 = static_cast<float>((select_bbox.x2 - select_bbox.x1 + 1) *
                                     (select_bbox.y2 - select_bbox.y1 + 1));
    float x1 = static_cast<float>(select_bbox.x1);
    float y1 = static_cast<float>(select_bbox.y1);
    float x2 = static_cast<float>(select_bbox.x2);
    float y2 = static_cast<float>(select_bbox.y2);

    select_idx++;
    for (int i = select_idx; i < num_bbox; i++) {
      if (mask_merged[i] == 1)
        continue;

      cvai_bbox_t &bbox_i(bboxes[i].bbox);
      float x = std::max<float>(x1, static_cast<float>(bbox_i.x1));
      float y = std::max<float>(y1, static_cast<float>(bbox_i.y1));
      float w = std::min<float>(x2, static_cast<float>(bbox_i.x2)) - x + 1;
      float h = std::min<float>(y2, static_cast<float>(bbox_i.y2)) - y + 1;
      if (w <= 0 || h <= 0) {
        continue;
      }

      float area2 = static_cast<float>((bbox_i.x2 - bbox_i.x1 + 1) *
                                       (bbox_i.y2 - bbox_i.y1 + 1));
      float area_intersect = w * h;
      if (method == 'u' && static_cast<float>(area_intersect) /
                                   (area1 + area2 - area_intersect) >
                               threshold) {
        mask_merged[i] = 1;
        continue;
      }
      if (method == 'm' &&
          static_cast<float>(area_intersect) / std::min(area1, area2) >
              threshold) {
        mask_merged[i] = 1;
      }
    }
  }
}

void clip_boxes(int width, int height, cvai_bbox_t &box) {
  if (box.x1 < 0) {
    box.x1 = 0;
  }
  if (box.y1 < 0) {
    box.y1 = 0;
  }
  if (box.x2 > width - 1) {
    box.x2 = width - 1;
  }
  if (box.y2 > height - 1) {
    box.y2 = height - 1;
  }
}
static void print_dim(CVI_SHAPE sp, std::string strprefix) {
  std::stringstream ss;
  ss << "shape:";
  for (int i = 0; i < 4; i++) {
    ss << sp.dim[i] << ",";
  }
  std::cout << strprefix << "," << ss.str() << "\n";
}

std::vector<cvai_face_info_t> outputParser(int frame_width, int frame_height,
                                           CVI_TENSOR *output_tensors,
                                           int num_output,const std::vector<float>&rescale_param) {
  std::vector<anchor_cfg> cfg;
  anchor_cfg tmp;

  std::vector<int> m_feat_stride_fpn = {8, 16, 32};

  tmp.SCALES = {1, 2};
  tmp.BASE_SIZE = 16;
  tmp.RATIOS = {1.0};
  tmp.ALLOWED_BORDER = 9999;
  tmp.STRIDE = 8;
  cfg.push_back(tmp);

  tmp.SCALES = {4, 8};
  tmp.BASE_SIZE = 16;
  tmp.RATIOS = {1.0};
  tmp.ALLOWED_BORDER = 9999;
  tmp.STRIDE = 16;
  cfg.push_back(tmp);

  tmp.SCALES = {16, 32};
  tmp.BASE_SIZE = 16;
  tmp.RATIOS = {1.0};
  tmp.ALLOWED_BORDER = 9999;
  tmp.STRIDE = 32;
  cfg.push_back(tmp);

  float im_scale_w = rescale_param[0];
  float im_scale_h = rescale_param[1];
  float pad_x = rescale_param[2];
  float pad_y = rescale_param[3];
  
  std::cout << "start to parse node\n";
  std::map<std::string, std::vector<anchor_box>> anchors_fpn_map;
  std::map<int, std::vector<std::vector<float>>> fpn_anchors_;
  // CVI_SHAPE input_shape = getoutputTensorshape("input.1_quant_i8",
  // input_tensors);
  for (size_t i = 0; i < cfg.size(); i++) {
    std::vector<std::vector<float>> base_anchors = generate_mmdet_base_anchors(
        cfg[i].BASE_SIZE, 0, cfg[i].RATIOS, cfg[i].SCALES);
    int stride = cfg[i].STRIDE;
    int input_w = frame_width;
    int input_h = frame_height;
    int feat_w = int(input_w / float(stride) + 0.5);
    int feat_h = int(input_h / float(stride) + 0.5);
    fpn_anchors_[stride] =
        generate_mmdet_grid_anchors(feat_w, feat_h, stride, base_anchors);
    std::cout << "baseanchor:" << base_anchors.size() << std::endl;
    std::cout << "gridanchor:" << fpn_anchors_[stride].size() << std::endl;
  }

  m_feat_stride_fpn = {8, 16, 32};
  std::vector<cvai_face_info_t> vec_bbox;
  std::vector<cvai_face_info_t> vec_bbox_nms;
  fpn_out_nodes_[8]["score"] = "score_8_Sigmoid_dequant";
  fpn_out_nodes_[16]["score"] = "score_16_Sigmoid_dequant";
  fpn_out_nodes_[32]["score"] = "score_32_Sigmoid_dequant";

  fpn_out_nodes_[8]["bbox"] = "bbox_8_Reshape_dequant";
  fpn_out_nodes_[16]["bbox"] = "bbox_16_Reshape_dequant";
  fpn_out_nodes_[32]["bbox"] = "bbox_32_Reshape_dequant";

  fpn_out_nodes_[8]["landmark"] = "kps_8_Reshape_dequant";
  fpn_out_nodes_[16]["landmark"] = "kps_16_Reshape_dequant";
  fpn_out_nodes_[32]["landmark"] = "kps_32_Reshape_dequant";

  for (size_t i = 0; i < m_feat_stride_fpn.size(); i++) {
    int stride = m_feat_stride_fpn[i];
    std::string score_str = fpn_out_nodes_[stride]["score"];
    CVI_TENSOR *score_tensor =
        CVI_NN_GetTensorByName(score_str.c_str(), output_tensors, num_output);
    if (score_tensor == nullptr) {
      std::cout << "got empty tensor by name:" << score_str << std::endl;
      continue;
    }
    CVI_SHAPE score_shape = CVI_NN_TensorShape(score_tensor);
    // std::cout << "stride:" << m_feat_stride_fpn[i] << "\n";
    print_dim(score_shape, "score");
    float *score_blob =
        getOutputRawPtr(score_str.c_str(), output_tensors, num_output);

    std::string bbox_str = fpn_out_nodes_[stride]["bbox"];
    // print_dim(blob_shape, "bbox");
    float *bbox_blob =
        getOutputRawPtr(bbox_str.c_str(), output_tensors, num_output);

    std::string landmark_str = fpn_out_nodes_[stride]["landmark"];
    // print_dim(blob_shape, "bbox");
    float *landmark_blob =
        getOutputRawPtr(landmark_str.c_str(), output_tensors, num_output);

    std::vector<std::vector<float>> &fpn_grids = fpn_anchors_[stride];
    for (size_t num = 0; num < score_shape.dim[1]; num++) { // anchor index
      float conf = score_blob[num];                         // conf
      if (conf <= 0.5) {
        continue;
      }
      std::vector<float> &grid = fpn_grids[num];
      float grid_cx = (grid[0] + grid[2]) / 2;
      float grid_cy = (grid[1] + grid[3]) / 2;

      cvai_face_info_t box;
      memset(&box, 0, sizeof(box));
      box.pts.size = 5;
      box.pts.x = (float *)malloc(sizeof(float) * box.pts.size);
      box.pts.y = (float *)malloc(sizeof(float) * box.pts.size);
      box.bbox.score = conf;
      // box.hardhat_score = 0;

      cv::Vec4f regress;
      // bbox_blob:b x (num_anchors*num_elem) x h x w
      box.bbox.x1 = grid_cx - bbox_blob[num * 4 + 0] * stride;
      box.bbox.y1 = grid_cy - bbox_blob[num * 4 + 1] * stride;
      box.bbox.x2 = grid_cx + bbox_blob[num * 4 + 2] * stride;
      box.bbox.y2 = grid_cy + bbox_blob[num * 4 + 3] * stride;

      for (size_t k = 0; k < box.pts.size; k++) {
        box.pts.x[k] = landmark_blob[num * 10 + k * 2] * stride + grid_cx;
        box.pts.y[k] = landmark_blob[num * 10 + k * 2 + 1] * stride + grid_cy;
      }

      vec_bbox.push_back(box);
    }
  }

  vec_bbox_nms.clear();
  NonMaximumSuppression(vec_bbox, vec_bbox_nms, 0.4, 'u');
  for (uint32_t i = 0; i < vec_bbox_nms.size(); ++i) {
    clip_boxes(frame_width, frame_height, vec_bbox_nms[i].bbox);
    
    cvai_bbox_t &bbox = vec_bbox_nms[i].bbox;
    bbox.x1 = (bbox.x1 - pad_x) * im_scale_w;
    bbox.y1 = (bbox.y1 - pad_y) * im_scale_h;
    bbox.x2 = (bbox.x2 - pad_x) * im_scale_w;
    bbox.y2 = (bbox.y2 - pad_y) * im_scale_h;
    cvai_pts_t &pts = vec_bbox_nms[i].pts;
    for(int k = 0; k < pts.size; k++){
      pts.x[k] = (pts.x[k] - pad_x)*im_scale_w;
      pts.y[k] = (pts.y[k] - pad_y)*im_scale_h; 
    }
  }
  return vec_bbox_nms;
}

bool compute_pad_resize_param(cv::Size src_size, cv::Size dst_size,
                              std::vector<float> &rescale_params) {
  rescale_params.clear();
  float src_w = src_size.width;
  float src_h = src_size.height;
  float ratio_w = src_w / dst_size.width;
  float ratio_h = src_h / dst_size.height;
  float ratio = std::max(ratio_w, ratio_h);
  // LOG(INFO)<<src_size<<"->"<<dst_size<<",ratio:"<<ratio_w<<","<<ratio_h;
  rescale_params.push_back(ratio);
  rescale_params.push_back(ratio);
  cv::Mat dst_img;
  if (ratio_w != ratio_h) {
    int src_resized_w = lrint(src_w / ratio);
    int src_resized_h = lrint(src_h / ratio);
    int roi_x = (dst_size.width - src_resized_w + 1) / 2;
    int roi_y = (dst_size.height - src_resized_h + 1) / 2;
    // LOG(INFO)<<"scale:"<<ratio<<",src_size:"<<src_resized_w<<","<<src_resized_h<<",roi_xy:"<<roi_x<<","<<roi_y;
    rescale_params.push_back(roi_x);
    rescale_params.push_back(roi_y);
    return true;
  } else {

    rescale_params.push_back(0.0f);
    rescale_params.push_back(0.0f);
    return false;
  }
}
void pad_resize_to_dst(
    const cv::Mat &src_img, cv::Mat &dst_img,
    std::vector<float> &rescale_params,
    cv::InterpolationFlags inter_flag /*= cv::INTER_NEAREST*/) {
  bool need_pad_resize =
      compute_pad_resize_param(src_img.size(), dst_img.size(), rescale_params);
  std::cout << "needpad:" << need_pad_resize
            << ",scale params:" << rescale_params.size() << std::endl;
  if (!need_pad_resize) {
    cv::resize(src_img, dst_img, dst_img.size(), 0, 0, inter_flag);
  } else {
    std::cout << "rescaleval:" << rescale_params[0] << "," << rescale_params[1]
              << "," << rescale_params[2] << "," << rescale_params[3]
              << std::endl;
    int src_resized_w = lrint(src_img.cols / rescale_params[0]);
    int src_resized_h = lrint(src_img.rows / rescale_params[1]);
    cv::Rect roi(rescale_params[2], rescale_params[3], src_resized_w,
                 src_resized_h);
    cv::Mat resized_img;
    std::cout << "opencv padresize,size:" << src_resized_w << ","
              << src_resized_h << ",offset:" << roi.x << "," << roi.y;
    cv::resize(src_img, dst_img(roi), cv::Size(src_resized_w, src_resized_h), 0,
               0, inter_flag);
  }
}

void rgb_split_scale(const cv::Mat &src_mat,
                     const std::vector<cv::Mat> &input_channels,
                     const std::vector<float> &mean,
                     const std::vector<float> &scale) {
  // cv::split is faster than vpp
  std::vector<cv::Mat> tmp_bgr;
  cv::split(src_mat, tmp_bgr);
  for (int i = 0; i < tmp_bgr.size(); i++) {
    float m = 0, s = 1;
    if (mean.size() > i)
      m = mean[i];
    if (scale.size() > i)
      s = scale[i];
    tmp_bgr[i].convertTo(input_channels[i], input_channels[i].type(), s, m);
  }
}

int main(int argc, char **argv) {
  if (argc != 3) {
    exit(-1);
  }

  // load model file
  const char *model_file = argv[1];
  CVI_MODEL_HANDLE model = nullptr;
  int ret = CVI_NN_RegisterModel(model_file, &model);
  if (CVI_RC_SUCCESS != ret) {
    printf("CVI_NN_RegisterModel failed, err %d\n", ret);
    exit(1);
  }
  printf("CVI_NN_RegisterModel succeeded\n");
  CVI_NN_SetConfig(model, OPTION_INPUT_MEM_TYPE, CVI_MEM_DEVICE);

  // get input output tensors
  CVI_TENSOR *input_tensors;
  CVI_TENSOR *output_tensors;
  int32_t input_num;
  int32_t output_num;
  CVI_NN_GetInputOutputTensors(model, &input_tensors, &input_num,
                               &output_tensors, &output_num);
  for (int i = 0; i < output_num; i++) {
    std::string out_name = CVI_NN_TensorName(output_tensors + i);
    std::cout << "out idx:" << i << ",name:" << out_name << std::endl;
  }
  CVI_TENSOR *input =
      CVI_NN_GetTensorByName(CVI_NN_DEFAULT_TENSOR, input_tensors, input_num);
  assert(input);
  printf("input, name:%s\n", input->name);
  // CVI_TENSOR *output = CVI_NN_GetTensorByName("score_32_Sigmoid_dequant",
  // output_tensors, output_num); assert(output);
  float qscale = CVI_NN_TensorQuantScale(input);
  printf("1qscale:%f\n", qscale);
  CVI_SHAPE shape = CVI_NN_TensorShape(input);
  std::cout << "to print input shape\n";
  // nchw
  int32_t height = shape.dim[2];
  int32_t width = shape.dim[3];

  std::cout << "to read input image\n";
  // imread
  cv::Mat src_img;
  src_img = cv::imread(argv[2]);
  cv::cvtColor(src_img, src_img, cv::COLOR_BGR2RGB);
  if (!src_img.data) {
    printf("Could not open or find the image\n");
    return -1;
  }
  std::cout << "readimg,cols:" << src_img.cols << ",rows:" << src_img.rows
            << std::endl;

  cv::Mat dst_img(cv::Size(width, height), CV_8UC3);
  std::vector<float> scale_params;
  pad_resize_to_dst(src_img, dst_img, scale_params, cv::INTER_NEAREST);

  // normalize
  std::vector<float> means, scales;
  std::vector<cv::Mat> channels;
  for (int i = 0; i < 3; i++) {
    means.push_back(127.5 / 128.0 * qscale);
    scales.push_back(-1.0 / 128.0 * qscale);
    channels.push_back(cv::Mat(height, width, CV_8SC1));
  }

  rgb_split_scale(dst_img, channels, means, scales);

  // fill to input tensor
  std::cout << "to copy data\n";
  int8_t *ptr = (int8_t *)CVI_NN_TensorPtr(input);
  int channel_size = height * width;
  for (int i = 0; i < 3; ++i) {
    memcpy(ptr + i * channel_size, channels[i].data, channel_size);
  }
  std::cout << "to inference\n";
  // run inference
  CVI_NN_Forward(model, input_tensors, input_num, output_tensors, output_num);
  printf("CVI_NN_Forward succeeded\n");

  std::vector<cvai_face_info_t> res =
      outputParser(width, height,
                   output_tensors, output_num,scale_params);
  for (uint32_t i = 0; i < res.size(); i++) {
    cvai_bbox_t bbox = res[i].bbox;
    int x1 = (int)bbox.x1;
    int y1 = (int)bbox.y1;
    int x2 = (int)bbox.x2;
    int y2 = (int)bbox.y2;
    cv::Rect box(bbox.x1, bbox.y1, bbox.x2 - bbox.x1, bbox.y2 - bbox.y1);
    printf("bbox: %d,%d,%d,%d ",x1, y1, x2, y2);
    cv::rectangle(src_img, box, cv::Scalar(255, 0, 0), 2);
    cvai_pts_t pti = res[i].pts;
    printf("kps: ");
    for (uint32_t j = 0; j < pti.size; j++) {
      cv::circle(src_img, cv::Point(pti.x[j], pti.y[j]), 2,
                 cv::Scalar(255, 0, 0), 2);
      int x = (int)pti.x[j];
      int y = (int)pti.y[j];
      printf("%d,%d,",x,y);
    }
    printf("\n");
    free(pti.x);
    free(pti.y);
  }
  cv::cvtColor(src_img, src_img, cv::COLOR_RGB2BGR);

  cv::imwrite("result.jpg", src_img);
  CVI_NN_CleanupModel(model);
  printf("CVI_NN_CleanupModel succeeded\n");
  return 0;
}
