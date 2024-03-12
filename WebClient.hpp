#ifndef UNTITLED_WEBCLIENT_HPP
#define UNTITLED_WEBCLIENT_HPP

#include <functional>
#include <string>

#include <curl/curl.h>

class RequestAwaitable;

class WebClient {
public:
    WebClient();
    ~WebClient();

    struct Result
    {
        int code;
        std::string data;
    };

    using CallbackFn = std::function<void(Result result)>;
    void runLoop();
    void stopLoop();
    void performRequest(const std::string& url, CallbackFn cb);
    RequestAwaitable performRequestAsync(std::string url);

private:
    struct Request
    {
        CallbackFn callback;
        std::string buffer;
    };

    static size_t writeToBuffer(char* ptr, size_t, size_t nmemb, void* tab)
    {
        auto r = reinterpret_cast<Request*>(tab);
        r->buffer.append(ptr, nmemb);
        return nmemb;
    }

    CURLM* m_multiHandle;
    std::atomic_bool m_break{false};
};

WebClient::WebClient()
{
    m_multiHandle = curl_multi_init();
}

WebClient::~WebClient()
{
    curl_multi_cleanup(m_multiHandle);
}

void WebClient::performRequest(const std::string& url, CallbackFn cb)
{
    Request* requestPtr = new Request{std::move(cb), {}};
    CURL* handle = curl_easy_init();
    curl_easy_setopt(handle, CURLOPT_URL, url.c_str());
    curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, &WebClient::writeToBuffer);
    curl_easy_setopt(handle, CURLOPT_WRITEDATA, requestPtr);
    curl_easy_setopt(handle, CURLOPT_PRIVATE, requestPtr);
    curl_multi_add_handle(m_multiHandle, handle);
}

void WebClient::stopLoop()
{
    m_break = true;
    curl_multi_wakeup(m_multiHandle);
}

void WebClient::runLoop()
{
    int msgs_left;
    int still_running = 1;

    while (!m_break) {
        curl_multi_perform(m_multiHandle, &still_running);
        curl_multi_poll(m_multiHandle, nullptr, 0, 1000, nullptr);

        CURLMsg* msg;
        while (!m_break && (msg = curl_multi_info_read(m_multiHandle, &msgs_left)))
        {
            if (msg->msg == CURLMSG_DONE)
            {
                CURL* handle = msg->easy_handle;
                int code;
                Request* requestPtr;
                curl_easy_getinfo(handle, CURLINFO_RESPONSE_CODE, &code);
                curl_easy_getinfo(handle, CURLINFO_PRIVATE, &requestPtr);

                requestPtr->callback({code, std::move(requestPtr->buffer)});
                curl_multi_remove_handle(m_multiHandle, handle);
                curl_easy_cleanup(handle);
                delete requestPtr;
            }
        }
    }
}

#endif //UNTITLED_WEBCLIENT_HPP
