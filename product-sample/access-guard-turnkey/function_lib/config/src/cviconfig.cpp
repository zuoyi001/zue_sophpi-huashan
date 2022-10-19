#include "cviconfig.h"

#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

const std::string key_setting = "setting";

cvi_device_config_t device_cfg1;

// static functions

static bool is_file_json_valid(const json &file_json) {
    return file_json.contains(key_setting);
}

static int get_file_json(json &file_json) {
    std::ifstream ifs(CONFIG_PATH);

    try {
        file_json = json::parse(ifs);
    } catch (std::exception &e) {
        std::cout << e.what() << '\n';
        return -1;
    }

    if (!is_file_json_valid(file_json)) {
        return -1;
    }

    return 0;
}

template <typename T>
static int get_device_config(const char *key, T *val) {
    json file_json;

    if (0 != get_file_json(file_json)) {
        return -1;
    }

    try {
        *val = file_json[key_setting].at(key);
    } catch (json::out_of_range &e) {
        std::cout << e.what() << '\n';
        return -1;
    }

    return 0;
}

template <typename T>
static int set_device_config(const char *key, const T val) {
    json file_json;

    if (0 != get_file_json(file_json)) {
        return -1;
    }

    file_json[key_setting][key] = val;

    std::ofstream of(CONFIG_PATH);
    of << file_json << std::endl;

    return 0;
}

// public functions

int cvi_get_device_config(cvi_device_config_t *config) {
    cvi_get_device_config_str(CONF_KEY_COMPNAY_NAME, config->company_name, sizeof(config->company_name));
    cvi_get_device_config_str(CONF_KEY_DEVICE_PASSWORD, config->device_password, sizeof(config->device_password));
    cvi_get_device_config_str(CONF_KEY_DEVICE_NAME, config->device_name, sizeof(config->device_name));
    cvi_get_device_config_float(CONF_KEY_THRESHOLD, &(config->threshold));
	cvi_get_device_config_float(CONF_KEY_THRESHOLD_LIVENESS, &(config->threshold_liveness));
    cvi_get_device_config_int(CONF_KEY_OPEN_DELAY, &(config->open_delay));
    cvi_get_device_config_int(CONF_KEY_INTERVAL, &(config->interval));
    cvi_get_device_config_int(CONF_KEY_VOICE_MODE, &(config->voice_mode));
    cvi_get_device_config_str(CONF_KEY_VOICE_CUSTOM, config->voice_custom, sizeof(config->voice_custom));
    cvi_get_device_config_int(CONF_KEY_VOICE_SOUND, &(config->voice_sound));
    cvi_get_device_config_int(CONF_KEY_DISPLAY_MODE, &(config->display_mode));
    cvi_get_device_config_str(CONF_KEY_DISPLAY_CUSTOM, config->display_custom, sizeof(config->display_custom));
    cvi_get_device_config_int(CONF_KEY_DISPLAY_BRIGHTNESS, &(config->display_brightness));
    cvi_get_device_config_int(CONF_KEY_DISPLAY_ENTER_LOCK_TIME, &(config->display_enter_lock_time));
    cvi_get_device_config_int(CONF_KEY_DISPLAY_ENTER_CLOSE_TIME, &(config->display_enter_close_time));
    cvi_get_device_config_int(CONF_KEY_STRANGER_MODE, &(config->stranger_mode));
    cvi_get_device_config_str(CONF_KEY_STRANGER_CUSTOM, config->stranger_custom, sizeof(config->stranger_custom));
    cvi_get_device_config_int(CONF_KEY_STRANGER_VOICE_MODE, &(config->stranger_voice_mode));
    cvi_get_device_config_str(CONF_KEY_STRANGERVOICE_CUSTOM, config->stranger_voice_custom, sizeof(config->stranger_voice_custom));
    cvi_get_device_config_int(CONF_KEY_MAX_FACE_SIZE, &(config->max_face_size));
    cvi_get_device_config_int(CONF_KEY_LIVENESS_TYPE, &(config->liveness_type));
    cvi_get_device_config_int(CONF_KEY_SIGN_DISTANCE, &(config->sign_distance));
    cvi_get_device_config_int(CONF_KEY_SUCCESS_RETRY_DELAY, &(config->success_retry_delay));
    cvi_get_device_config_int(CONF_KEY_SUCCESS_RETRY, &(config->success_retry));
    cvi_get_device_config_int(CONF_KEY_UPLOAD_RECORD_IMAGE, &(config->upload_record_image));
    cvi_get_device_config_int(CONF_KEY_IR_LIVE_PREVIEW, &(config->ir_live_preview));
    cvi_get_device_config_int(CONF_KEY_REBOOT_EVERY_DAY, &(config->reboot_every_day));
    cvi_get_device_config_int(CONF_KEY_REBOOT_HOUR, &(config->reboot_hour));
    cvi_get_device_config_int(CONF_KEY_REBOOT_MIN, &(config->reboot_min));
    cvi_get_device_config_int(CONF_KEY_VERSION_CODE, &(config->version_code));
    cvi_get_device_config_str(CONF_KEY_PACKAGE_NAME, config->package_name, sizeof(config->package_name));
    cvi_get_device_config_str(CONF_KEY_VERSION_NAME, config->version_name, sizeof(config->version_name));
    cvi_get_device_config_int(CONF_KEY_FACE_QUALITY, &(config->face_quality));
    cvi_get_device_config_float(CONF_KEY_FACE_QUALITY_THRESHOLD, &(config->face_quality_threshold));
	cvi_get_device_config_int(CONF_KEY_NET_MODE, &(config->net_mode));
	cvi_get_device_config_str(CONF_KEY_NET_IP, config->ip_address, sizeof(config->ip_address));
	cvi_get_device_config_str(CONF_KEY_NET_MASK, config->netmask, sizeof(config->netmask));
	cvi_get_device_config_str(CONF_KEY_GATEWAY, config->gateway, sizeof(config->gateway));
	cvi_get_device_config_str(CONF_KEY_DNS, config->dns, sizeof(config->dns));

    return 0;
}

int cvi_set_device_config(cvi_device_config_t *config) {
    json file_json;

    if (0 != get_file_json(file_json)) {
        return -1;
    }

    file_json[key_setting][CONF_KEY_COMPNAY_NAME] = config->company_name;
    file_json[key_setting][CONF_KEY_DEVICE_PASSWORD] = config->device_password;
    file_json[key_setting][CONF_KEY_DEVICE_NAME] = config->device_name;
    file_json[key_setting][CONF_KEY_THRESHOLD] = config->threshold;
	file_json[key_setting][CONF_KEY_THRESHOLD_LIVENESS] = config->threshold_liveness;
    file_json[key_setting][CONF_KEY_OPEN_DELAY] = config->open_delay;
    file_json[key_setting][CONF_KEY_INTERVAL] = config->interval;
    file_json[key_setting][CONF_KEY_VOICE_MODE] = config->voice_mode;
    file_json[key_setting][CONF_KEY_VOICE_CUSTOM] = config->voice_custom;
    file_json[key_setting][CONF_KEY_VOICE_SOUND] = config->voice_sound;
    file_json[key_setting][CONF_KEY_DISPLAY_MODE] = config->display_mode;
    file_json[key_setting][CONF_KEY_DISPLAY_CUSTOM] = config->display_custom;
    file_json[key_setting][CONF_KEY_DISPLAY_BRIGHTNESS] = config->display_brightness;
    file_json[key_setting][CONF_KEY_DISPLAY_ENTER_LOCK_TIME] = config->display_enter_lock_time;
    file_json[key_setting][CONF_KEY_DISPLAY_ENTER_CLOSE_TIME] = config->display_enter_close_time;
    file_json[key_setting][CONF_KEY_STRANGER_MODE] = config->stranger_mode;
    file_json[key_setting][CONF_KEY_STRANGER_CUSTOM] = config->stranger_custom;
    file_json[key_setting][CONF_KEY_STRANGER_VOICE_MODE] = config->stranger_voice_mode;
    file_json[key_setting][CONF_KEY_STRANGERVOICE_CUSTOM] = config->stranger_voice_custom;
    file_json[key_setting][CONF_KEY_MAX_FACE_SIZE] = config->max_face_size;
    file_json[key_setting][CONF_KEY_LIVENESS_TYPE] = config->liveness_type;
    file_json[key_setting][CONF_KEY_SIGN_DISTANCE] = config->sign_distance;
    file_json[key_setting][CONF_KEY_SUCCESS_RETRY_DELAY] = config->success_retry_delay;
    file_json[key_setting][CONF_KEY_SUCCESS_RETRY] = config->success_retry;
    file_json[key_setting][CONF_KEY_UPLOAD_RECORD_IMAGE] = config->upload_record_image;
    file_json[key_setting][CONF_KEY_IR_LIVE_PREVIEW] = config->ir_live_preview;
    file_json[key_setting][CONF_KEY_REBOOT_EVERY_DAY] = config->reboot_every_day;
    file_json[key_setting][CONF_KEY_REBOOT_HOUR] = config->reboot_hour;
    file_json[key_setting][CONF_KEY_REBOOT_MIN] = config->reboot_min;
    file_json[key_setting][CONF_KEY_VERSION_CODE] = config->version_code;
    file_json[key_setting][CONF_KEY_PACKAGE_NAME] = config->package_name;
    file_json[key_setting][CONF_KEY_VERSION_NAME] = config->version_name;
    file_json[key_setting][CONF_KEY_FACE_QUALITY] = config->face_quality;
    file_json[key_setting][CONF_KEY_NET_MODE] = config->net_mode;
	file_json[key_setting][CONF_KEY_NET_IP] = config->ip_address;
	file_json[key_setting][CONF_KEY_NET_MASK] = config->netmask;
	file_json[key_setting][CONF_KEY_GATEWAY] = config->gateway;
	file_json[key_setting][CONF_KEY_DNS] = config->dns;

    std::ofstream of(CONFIG_PATH);
    of << file_json << std::endl;

    return 0;
}

int cvi_get_device_config_int(const char *key, int *val) {
    return get_device_config(key, val);
}

int cvi_get_device_config_str(const char *key, char *buf, size_t buf_size) {
    std::string str_val;
    int ret = get_device_config(key, &str_val);

    if (0 != ret) {
        return ret;
    }

    snprintf(buf, buf_size, str_val.c_str());

    return 0;
}

int cvi_get_device_config_float(const char *key, float *val) {
    return get_device_config(key, val);
}

int cvi_set_device_config_int(const char *key, const int val) {
    set_device_config(key, val);

    return 0;
}

int cvi_set_device_config_str(const char *key, const char *buf) {
    set_device_config(key, buf);

    return 0;
}

int cvi_set_device_config_float(const char *key, const float val) {
    set_device_config(key, val);

    return 0;
}