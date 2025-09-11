#ifndef CORE_SERVER_IMPL_
#define CORE_SERVER_IMPL_

class ServerImpl {
public:
    virtual ~ServerImpl() = default;
    virtual bool Init() = 0;
    virtual void Start() = 0;
    virtual void Stop() = 0;

    // You can add more pure virtual functions here for platform-specific logic
    // e.g., virtual void WorkerThread() = 0;
};

#endif // CORE_SERVER_IMPL_
