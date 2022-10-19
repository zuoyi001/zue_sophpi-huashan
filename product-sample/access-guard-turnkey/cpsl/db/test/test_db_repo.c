#include "cvi_hal_db_repo.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

#define REPO_PATH "repo.db"

static void generate_feature(unsigned char *feature, size_t size)
{
    for (int i = 0; i < size; i++) {
        feature[i] = rand() % 256;
    }
}

int test_face_add()
{
    int face_id = -1;
    unsigned char feature[512] = {0};
    char add_name[256] = {0};
    cvi_hal_db_repo_t repo = {0};

    if (0 > cvi_hal_repo_open(&repo, REPO_PATH)) {
        printf("repo open fail\n");
        return -1;
    }

    generate_feature(feature, sizeof(feature));
    snprintf(add_name, sizeof(add_name), "%s_%d", "player", rand() % 1000);

    if (0 > (face_id = cvi_hal_repo_add_face(&repo, add_name, feature, sizeof(feature)))) {
        printf("add face fail\n");
        return -1;
    }

    char name[512] = {0};
    unsigned char find_feature[512] = {0};
    if (0 > cvi_hal_repo_find_face(&repo, face_id, name, sizeof(name), find_feature, sizeof(find_feature))) {
        printf("find face fail\n");
        return -1;
    }

    if (strcmp(name, add_name) != 0) {
        printf("name different\n");
        return -1;
    }

    if (memcmp(feature, find_feature, sizeof(find_feature)) != 0) {
        printf("feature different\n");
        return -1;
    }

    cvi_hal_repo_close(&repo);
    unlink(REPO_PATH);
    return 0;
}

int test_face_delete()
{
    int face_id = -1;
    unsigned char feature[512] = {0};
    char name[256] = {0};
    cvi_hal_db_repo_t repo = {0};

    if (0 > cvi_hal_repo_open(&repo, REPO_PATH)) {
        printf("repo open fail\n");
        return -1;
    }

    generate_feature(feature, sizeof(feature));
    snprintf(name, sizeof(name), "%s_%d", "player", rand() % 1000);

    if (0 > (face_id = cvi_hal_repo_add_face(&repo, name, feature, sizeof(feature)))) {
        printf("add face fail\n");
        return -1;
    }

    if (0 > cvi_hal_repo_delete_face(&repo, face_id)) {
        printf("delete face fail\n");
        return -1;
    }

    char find_name[512] = {0};
    unsigned char find_feature[512] = {0};
    if (0 > cvi_hal_repo_find_face(&repo, face_id, find_name, sizeof(find_name), find_feature, sizeof(find_feature))) {
        printf("find face fail\n");
        return -1;
    }

    if (strlen(find_name) != 0) {
        printf("test_face_delete fail\n");
        return -1;
    }

    cvi_hal_repo_close(&repo);
    unlink(REPO_PATH);
    return 0;
}

int test_face_update()
{
    int face_id = -1;
    unsigned char feature[512] = {0};
    char name[256] = {0};
    cvi_hal_db_repo_t repo = {0};

    if (0 > cvi_hal_repo_open(&repo, REPO_PATH)) {
        printf("repo open fail\n");
        return -1;
    }

    generate_feature(feature, sizeof(feature));
    snprintf(name, sizeof(name), "%s_%d", "player", rand() % 1000);

    if (0 > (face_id = cvi_hal_repo_add_face(&repo, name, feature, sizeof(feature)))) {
        printf("add face fail\n");
        return -1;
    }

    memset(feature, 0, sizeof(feature));
    snprintf(name, sizeof(name), "%s_%d", "new", rand() % 1000);
    sleep(1);
    generate_feature(feature, sizeof(feature));
    if (0 > cvi_hal_repo_update_face(&repo, face_id, name, feature, sizeof(feature))) {
        printf("update name fail\n");
        return -1;
    }

    char find_name[512] = {0};
    unsigned char find_feature[512] = {0};
    if (0 > cvi_hal_repo_find_face(&repo, face_id, find_name, sizeof(find_name), find_feature, sizeof(find_feature))) {
        printf("find face fail\n");
        return -1;
    }

    if (strcmp(find_name, name) != 0) {
        printf("update name different, res: %s, ans: %s\n", find_name, name);
    }

    if (memcmp(feature, find_feature, sizeof(find_feature)) != 0) {
        printf("update feature different\n");
        return -1;
    }

    cvi_hal_repo_close(&repo);
    unlink(REPO_PATH);
    return 0;
}

int test_face_multi_update()
{
    int face_id = -1;
    unsigned char feature[512] = {0};
    char name[256] = {0};
    cvi_hal_db_repo_t repo = {0};

    if (0 > cvi_hal_repo_open(&repo, REPO_PATH)) {
        printf("repo open fail\n");
        return -1;
    }

    generate_feature(feature, sizeof(feature));
    snprintf(name, sizeof(name), "%s_%d", "player", rand() % 1000);

    if (0 > (face_id = cvi_hal_repo_add_face(&repo, name, feature, sizeof(feature)))) {
        return -1;
    }

    // check multi-feature update
    // add 3 feature including the fisrt
    for (int i = 0; i < 2; i++) {
        generate_feature(feature, sizeof(feature));
        sleep(1);
        if (0 > cvi_hal_repo_update_face(&repo, face_id, NULL, feature, sizeof(feature))) {
            printf("update name fail\n");
            return -1;
        }
    }

    // add the 4th feature, and it should replace the first one
    unsigned char dummy[512] = {0};
    generate_feature(dummy, sizeof(dummy));
    sleep(1);
    if (0 > cvi_hal_repo_update_face(&repo, face_id, NULL, dummy, sizeof(dummy))) {
        printf("update name fail\n");
        return -1;
    }

    // compare the feature, it should be the 3rd one
    char find_name[512] = {0};
    unsigned char find_feature[512] = {0};
    if (0 > cvi_hal_repo_find_face(&repo, face_id, find_name, sizeof(find_name), find_feature, sizeof(find_feature))) {
        printf("find face fail\n");
        return -1;
    }

    if (memcmp(feature, find_feature, sizeof(find_feature)) != 0) {
        printf("update feature different\n");
        return -1;
    }

    cvi_hal_repo_close(&repo);
    unlink(REPO_PATH);
    return 0;
}

int test_feature_get_all()
{
    cvi_hal_db_repo_t repo = {0};
    if (0 > cvi_hal_repo_open(&repo, REPO_PATH)) {
        printf("repo open fail\n");
        return -1;
    }

    for (int i = 0; i < 5; i++) {
        int face_id = 0;
        unsigned char feature1[512] = {0};
        unsigned char feature2[512] = {0};
        unsigned char feature3[512] = {0};
        char name[128] = {0};
        generate_feature(feature1, sizeof(feature1));
        generate_feature(feature2, sizeof(feature2));
        generate_feature(feature3, sizeof(feature3));
        snprintf(name, sizeof(name), "user_%d", i);
        if (0 > (face_id = cvi_hal_repo_add_face(&repo, name, feature1, sizeof(feature1)))) {
            printf("add face fail\n");
            return -1;
        }

        if (0 > cvi_hal_repo_update_face(&repo, face_id, NULL, feature2, sizeof(feature2))) {
            printf("update name fail\n");
            return -1;
        }

        if (0 > cvi_hal_repo_update_face(&repo, face_id, NULL, feature3, sizeof(feature3))) {
            printf("update name fail\n");
            return -1;
        }
    }

    unsigned char *features = NULL;
    int count = cvi_hal_repo_get_features(&repo, 0, -1, &features);
    if (count != 15) {
        printf("get features result fail, %d\n", count);
        return -1;
    }

	free(features);
    features = NULL;
    count = cvi_hal_repo_get_features(&repo, 10, -1, &features);
    if (count != 5) {
        printf("get features result fail, %d\n", count);
        return -1;
    }

    free(features);
    features = NULL;
    cvi_hal_repo_close(&repo);
    unlink(REPO_PATH);
    return 0;
}

int test_feature_get_face_id()
{
    cvi_hal_db_repo_t repo = {0};
    if (0 > cvi_hal_repo_open(&repo, REPO_PATH)) {
        printf("repo open fail\n");
        return -1;
    }

    for (int i = 0; i < 5; i++) {
        int face_id = 0;
        unsigned char feature1[512] = {0};
        unsigned char feature2[512] = {0};
        unsigned char feature3[512] = {0};
        char name[128] = {0};
        generate_feature(feature1, sizeof(feature1));
        generate_feature(feature2, sizeof(feature2));
        generate_feature(feature3, sizeof(feature3));
        snprintf(name, sizeof(name), "user_%d", i);
        if (0 > (face_id = cvi_hal_repo_add_face(&repo, name, feature1, sizeof(feature1)))) {
            printf("add face fail\n");
            return -1;
        }

        if (0 > cvi_hal_repo_update_face(&repo, face_id, NULL, feature2, sizeof(feature2))) {
            printf("update name fail\n");
            return -1;
        }

        if (0 > cvi_hal_repo_update_face(&repo, face_id, NULL, feature3, sizeof(feature3))) {
            printf("update name fail\n");
            return -1;
        }
    }

    int id1 = -1;
    char name1[128] = {0};
    if (0 > cvi_hal_repo_get_face_by_offset(&repo, 0, &id1, name1, sizeof(name1))) {
        printf("cvi_hal_repo_get_face_by_offset fail\n");
        return -1;
    }
    if (id1 != 1 || strcmp(name1, "user_0") != 0) {
        printf("get face_id by feature offset result error, %d, %s\n", id1, name1);
        return -1;
    }
    int id2 = -1;
    char name2[128] = {0};
    if (0 > cvi_hal_repo_get_face_by_offset(&repo, 10, &id2, name2, sizeof(name2))) {
        printf("cvi_hal_repo_get_face_by_offset fail\n");
        return -1;
    }
    if (id2 != 4 || strcmp(name2, "user_3") != 0) {
        printf("get face_id by feature offset result error, %d, %s\n", id2, name2);
        return -1;
    }

    cvi_hal_repo_close(&repo);
    unlink(REPO_PATH);
    return 0;
}

int test_repo_person_info()
{
    int face_id = -1;
    unsigned char feature[512] = {0};
    char add_name[256] = {0};
    cvi_hal_db_repo_t repo = {0};

    if (0 > cvi_hal_repo_open(&repo, REPO_PATH)) {
        printf("repo open fail\n");
        return -1;
    }

    generate_feature(feature, sizeof(feature));
    snprintf(add_name, sizeof(add_name), "%s_%d", "player", rand() % 1000);

    if (0 > (face_id = cvi_hal_repo_add_face(&repo, add_name, feature, sizeof(feature)))) {
        printf("add face fail\n");
        return -1;
    }

    cvi_person_t person = {0};
    snprintf(person.serial, sizeof(person.serial), "%s", "seroal001");
    snprintf(person.ic_card, sizeof(person.ic_card), "%s", "ic_card002");

    if (0 > cvi_hal_repo_set_person(&repo, face_id, &person)) {
        printf("set person info fail\n");
        return -1;
    }

    cvi_person_t person2 = {0};
    if (0 > cvi_hal_repo_get_person(&repo, face_id, &person2)) {
        printf("get person info fail\n");
        return -1;
    }

    if (strcmp(person.serial, person2.serial) != 0) {
        printf("serial different, %s, %s\n", person.serial, person2.serial);
        return -1;
    }

    if (strcmp(person.ic_card, person2.ic_card) != 0) {
        printf("ic_card different, %s, %s\n", person.ic_card, person2.ic_card);
        return -1;
    }

    cvi_hal_repo_close(&repo);
    unlink(REPO_PATH);
    return 0;
}

int test_repo_get_person_list()
{
    cvi_hal_db_repo_t repo = {0};

    if (0 > cvi_hal_repo_open(&repo, REPO_PATH)) {
        printf("repo open fail\n");
        return -1;
    }

    for (int i = 0; i < 20; i++) {
        int face_id = 0;
        unsigned char feature[512] = {0};
        char name[128] = {0};
        generate_feature(feature, sizeof(feature));
        snprintf(name, sizeof(name), "user_%d", i);
        if (0 > (face_id = cvi_hal_repo_add_face(&repo, name, feature, sizeof(feature)))) {
            printf("add face fail\n");
            return -1;
        }

        cvi_person_t person = {0};
        snprintf(person.name, sizeof(person.name), "name_%d", i);
        snprintf(person.identifier, sizeof(person.identifier), "identifier_%d", i);
        snprintf(person.serial, sizeof(person.serial), "serial_%d", i);
        snprintf(person.ic_card, sizeof(person.ic_card), "ic_card_%d", i);
        snprintf(person.image_path, sizeof(person.image_path), "%d.jpg", i);
        if (0 > cvi_hal_repo_set_person(&repo, face_id, &person)) {
            printf("set person info fail\n");
            return -1;
        }
    }

    cvi_person_t *people;
    int count= 0;

    if (0 > (count = cvi_hal_repo_get_person_list(&repo, 15, 100, &people))) {
        printf("get person list fail\n");
        return -1;
    }

	if (count != 5) {
        printf("person list num error, count: %d\n", count);
        return -1;
    }

    for (int i = 0; i < 5; i++) {
        char name[128] = {0}, identifier[128] = {0}, serial[128] = {0}, ic_card[128] = {0}, image_path[128] = {0};
        snprintf(name, sizeof(name), "name_%d", i+15);
        snprintf(identifier, sizeof(identifier), "identifier_%d", i+15);
        snprintf(serial, sizeof(serial), "serial_%d", i+15);
        snprintf(ic_card, sizeof(ic_card), "ic_card_%d", i+15);
        snprintf(image_path, sizeof(image_path), "%d.jpg", i+15);

        if (strcmp(people[i].name, name) != 0) {
            printf("name different, %s, %s\n", people[i].name, name);
            return -1;
        }

        if (strcmp(people[i].identifier, identifier) != 0) {
            printf("identifier different, %s, %s\n", people[i].identifier, identifier);
            return -1;
        }

        if (strcmp(people[i].serial, serial) != 0) {
            printf("serial different, %s, %s\n", people[i].serial, serial);
            return -1;
        }

        if (strcmp(people[i].ic_card, ic_card) != 0) {
            printf("ic_card different, %s, %s\n", people[i].ic_card, ic_card);
            return -1;
        }

        if (strcmp(people[i].image_path, image_path) != 0) {
            printf("image_path different, %s, %s\n", people[i].image_path, image_path);
            return -1;
        }
    }

    cvi_hal_repo_close(&repo);
    unlink(REPO_PATH);
    return 0;
}


int main(int argc, const char *argv[])
{
    srand(time(NULL));

    unlink(REPO_PATH);

    if (0 > test_face_add()) {
        printf("test_face_add fail\n");
        return -1;
    }

    if (0 > test_face_delete()) {
        printf("test_face_delete fail\n");
        return -1;
    }
    if (0 > test_face_update()) {
        printf("test_face_update fail\n");
        return -1;
    }
    if (0 > test_face_multi_update()) {
        printf("test_face_multi_update fail\n");
        return -1;
    }

    if (0 > test_feature_get_all()) {
        printf("test_feature_get_all fail\n");
        return -1;
    }
    
    if (0 > test_feature_get_face_id()) {
        printf("test_feature_get_face_id fail\n");
        return -1;
    }

    if (0 > test_repo_person_info()) {
        printf("test_repo_person_info fail\n");
        return -1;
    }

    if (0 > test_repo_get_person_list()) {
        printf("test_repo_get_person_list fail\n");
        return -1;
    }

    return 0;
}
