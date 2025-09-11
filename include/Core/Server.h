#ifndef CORE_SERVER_
#define CORE_SERVER_

#include <memory>

class ServerImpl; // Forward declaration of the implementation class

class Server {
    std::unique_ptr<ServerImpl> pimpl; // Pointer to implementation

public:
    Server();
    ~Server(); // Destructor must be defined in the .cpp file

    // Move-only semantics
    Server(Server&&) noexcept;
    Server& operator=(Server&&) noexcept;

    bool Init();
    void Start();
    void Stop();

private:
    // Disable copy semantics
    Server(const Server&) = delete;
    Server& operator=(const Server&) = delete;
};

#endif // CORE_SERVER_