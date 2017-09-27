#include "curl/curl.h"
#include <unistd.h>
#include <string>

const char* url = "http://www.baidu.com";
const int count = 1000;
CURL* curls[count] = {0};
bool first = true;

size_t curl_writer(void* buffer, size_t size, size_t count, void* stream) {
    (void)buffer;
    (void)stream;

    return size * count;
}

int64_t time_delta(const struct timeval& start_time, const struct timeval& end_time) {
    return (end_time.tv_sec - start_time.tv_sec) * 1e6 + (end_time.tv_usec - start_time.tv_usec);
}
int curl_multi() {
    CURLM* curlm = curl_multi_init();

    for (int i = 0; i < count; ++i) {
        CURL* curl = NULL;
        if (curl) {
            curl = curl_easy_init();
            curls[i] = curl;
        } else {
            curl = curls[i];
        }

        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_writer);
        curl_multi_add_handle(curlm, curl);
    }

    int running_handlers = 0;
    do {
        int numfds = 0;
        int res = curl_multi_wait(curlm, NULL, 0, 2000, &numfds);
        if (res != CURLM_OK) {
            fprintf(stderr, "error: curl_multi_wait return %d\n", res);
            return -1;
        }
        curl_multi_perform(curlm, &running_handlers);
    } while (running_handlers > 0);

    int msgs_left;
    CURLMsg* msg;
    while ((msg = curl_multi_info_read(curlm, &msgs_left))) {
        if (msg->msg == CURLMSG_DONE) {
            int http_status_code = 0;
            curl_easy_getinfo(msg->easy_handle, CURLINFO_RESPONSE_CODE, &http_status_code);
            const char* effective_url = NULL;
            curl_easy_getinfo(msg->easy_handle, CURLINFO_EFFECTIVE_URL, &effective_url);
            // printf("url:%s status:%s\n", effective_url, curl_easy_strerror(msg->data.result));
            curl_multi_remove_handle(curlm, msg->easy_handle);
            if (!first) {
                curl_easy_cleanup(msg->easy_handle);
            }
        }
    }

    curl_multi_cleanup(curlm);

    return 0;
}

int main() {
    struct timeval t1;
    struct timeval t2;

    curl_multi();
    first = false;
    sleep(1);
    gettimeofday(&t1, NULL);
    curl_multi();
    gettimeofday(&t2, NULL);

    printf("%lu\n", time_delta(t1, t2));

    return 0;
}