#include <sys/time.h>
#include <string>

#include "curl/curl.h"

const char* url = "http://www.baidu.com";
const int thread_count = 10;
const int count = 1000;

size_t write_data(void* buffer, size_t size, size_t count, void* stream) {
    (void)buffer;
    (void)stream;
    return size * count;
}

int64_t time_delta(const struct timeval& start_time, const struct timeval& end_time) {
    return (end_time.tv_sec - start_time.tv_sec) * 1e6 + (end_time.tv_usec - start_time.tv_usec);
}

void* thread_curl_easy(void*) {
    for (int i = 0; i < count / thread_count; ++i) {
        CURL* curl = curl_easy_init();

        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);

        curl_easy_perform(curl);
    }

    return NULL;
}

void curl_easy_parallel() {
    pthread_t pid[thread_count];
    for (int i = 0; i < thread_count; ++i) {
        pthread_create(&pid[i], NULL, thread_curl_easy, NULL);
    }

    for (int i = 0; i < thread_count; ++i) {
        pthread_join(pid[i], NULL);
    }
}

int main() {
    struct timeval t1;
    struct timeval t2;

    gettimeofday(&t1, NULL);
    curl_easy_parallel();
    gettimeofday(&t2, NULL);

    printf("%lu\n", time_delta(t1, t2));

    return 0;
}