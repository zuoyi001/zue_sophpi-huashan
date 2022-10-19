#include "cviconfigwebserver.h"

#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

//const std::string key_system = "system";

cvi_device_config_webserver_t device_cfg_webserver;

// static functions

//static bool is_file_json_valid(const json &file_json) {
//    return file_json.contains(key_system);
//}

static int get_file_json(json &file_json) {
    std::ifstream ifs(CONFIG_WEBSERVER_PATH);

    try {
        file_json = json::parse(ifs);
    } catch (std::exception &e) {
        std::cout << e.what() << '\n';
        return -1;
    }

//    if (!is_file_json_valid(file_json)) {
//        return -1;
//    }

    return 0;
}

template <typename T>
static int get_device_config(const char *key, T *val) {
    json file_json;

    if (0 != get_file_json(file_json)) {
        return -1;
    }

    try {
        *val = file_json.at(key);
    } catch (json::out_of_range &e) {
        std::cout << e.what() << '\n';
        return -1;
    }

    return 0;
}

template <typename T>
static int set_device_config(const char *key, T val) {
    json file_json;

    if (0 != get_file_json(file_json)) {
        return -1;
    }

    file_json[key] = val;

    std::ofstream of(CONFIG_WEBSERVER_PATH);
    of << file_json << std::endl;

    return 0;
}

// public functions

int cvi_get_device_config_webserver(cvi_device_config_webserver_t *config) {
    cvi_get_device_config_webserver_str(CONF_KEY_CLOUD_SERVER, config->cloud_server, sizeof(config->cloud_server));    

    return 0;
}

int cvi_set_device_config_webserver(cvi_device_config_webserver_t *config) {
    json file_json;

    if (0 != get_file_json(file_json)) {
        return -1;
    }

    file_json[CONF_KEY_CLOUD_SERVER] = config->cloud_server;
   
    std::ofstream of(CONFIG_WEBSERVER_PATH);
    of << file_json << std::endl;

    return 0;
}

int cvi_get_device_config_webserver_int(const char *key, int *val) {
    return get_device_config(key, val);
}

int cvi_get_device_config_webserver_str(const char *key, char *buf, size_t buf_size) {
    std::string str_val;
    int ret = get_device_config(key, &str_val);

    if (0 != ret) {
        return ret;
    }

    snprintf(buf, buf_size, str_val.c_str());

    return 0;
}

int cvi_get_device_config_webserver_float(const char *key, float *val) {
    return get_device_config(key, val);
}

int cvi_set_device_config_webserver_int(const char *key, int val) {
    set_device_config(key, val);

    return 0;
}

int cvi_set_device_config_webserver_str(const char *key, char *buf) {
    set_device_config(key, buf);

    return 0;
}

int cvi_set_device_config_webserver_float(const char *key, float val) {
    set_device_config(key, val);

    return 0;
}