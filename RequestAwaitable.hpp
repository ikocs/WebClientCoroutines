#ifndef UNTITLED_REQUESTAWAITABLE_HPP
#define UNTITLED_REQUESTAWAITABLE_HPP

#include <coroutine>

#include "WebClient.hpp"

struct RequestAwaitable {
    RequestAwaitable(WebClient& client_, std::string url_) : client(client_), url(std::move(url_)) {};

    bool await_ready() const noexcept { return false; }
    void await_suspend(std::coroutine_handle<> handle) noexcept
    {
        client.performRequest(std::move(url), [handle, this](WebClient::Result res)
        {
            result = std::move(res);
            handle.resume();
        });
    }
    WebClient::Result await_resume() const noexcept { return std::move(result); }

    WebClient& client;
    std::string url;
    WebClient::Result result;
};

RequestAwaitable WebClient::performRequestAsync(std::string url)
{
    return RequestAwaitable(*this, std::move(url));
}

struct promise;

struct Task : std::coroutine_handle<promise>
{
    using promise_type = ::promise;
};

struct promise
{
    Task get_return_object() { return Task{}; }
    std::suspend_never initial_suspend() noexcept { return {}; }
    std::suspend_never final_suspend() noexcept { return {}; }
    void return_void() {}
    void unhandled_exception() {}
};

#endif //UNTITLED_REQUESTAWAITABLE_HPP
