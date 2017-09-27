#include "curl/curl.h"
#include <pthread.h>
#include <unistd.h>
#include <string>

const char* url = "http://www.baidu.com";
const int count = 1000;
CURL* curls[count] = {0};
CURLSH* share_handler;
bool first = true;

pthread_mutex_t s_dns_lock = PTHREAD_MUTEX_INITIALIZER;

void lock(CURL*, curl_lock_data, curl_lock_access, void*) {
    pthread_mutex_lock(&s_dns_lock);
}

void unlock(CURL*, curl_lock_data, void*) {
    pthread_mutex_unlock(&s_dns_lock);
}

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
        curl_easy_setopt(curl, CURLOPT_SHARE, share_handler);
        curl_easy_setopt(curl, CURLOPT_DNS_CACHE_TIMEOUT, 5 * 60);//5min
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
    if (!first) {
        curl_share_cleanup(share_handler);
    }

    return 0;
}

int main() {
    struct timeval t1;
    struct timeval t2;

    share_handler = curl_share_init();
    curl_share_setopt(share_handler, CURLSHOPT_LOCKFUNC, lock);
    curl_share_setopt(share_handler, CURLSHOPT_UNLOCKFUNC, unlock);
    curl_share_setopt(share_handler, CURLSHOPT_SHARE, CURL_LOCK_DATA_DNS);

    curl_multi();
    first = false;
    sleep(1);
    gettimeofday(&t1, NULL);
    curl_multi();
    gettimeofday(&t2, NULL);

    printf("%lu\n", time_delta(t1, t2));

    return 0;
}
