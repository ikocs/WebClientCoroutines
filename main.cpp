#include <thread>
#include <iostream>

#include "RequestAwaitable.hpp"


Task doSomething(WebClient& client)
{
    auto r1 = co_await client.performRequestAsync("https://postman-echo.com/get");
    std::cout << "Req1 ready: " << r1.code << " - " << r1.data << std::endl;

    auto r2 = co_await client.performRequestAsync("http://httpbin.org/user-agent");
    std::cout << "Req2 ready: " << r2.code << " - " << r2.data << std::endl;
}

int main()
{
    WebClient client;
    std::thread worker(std::bind(&WebClient::runLoop, &client));

    doSomething(client);

    std::cin.get();
    client.stopLoop();
    worker.join();

    std::cout << "\nmain() end";

    return 0;
}