#include <Base/Pipeline.hpp>

Pipeline::Pipeline() {
    checkError(cudaStreamCreateWithFlags(&mStream,cudaStreamNonBlocking));
    mMaxThread = getEnvironment().getProp().maxThreadsPerBlock;
}

Pipeline::~Pipeline() {
    checkError(cudaStreamDestroy(mStream));
}

void Pipeline::sync() {
    checkError(cudaStreamSynchronize(mStream));
}

cudaStream_t Pipeline::getId() const {
    return mStream;
}

Environment::Environment(){}

void Environment::init() {
    int cnt;
    checkError(cudaGetDeviceCount(&cnt));
    cudaDeviceProp prop;
    int id = -1, maxwell = 0;
    size_t size = 0;
    for (int i = 0; i < cnt; ++i) {
        checkError(cudaGetDeviceProperties(&prop, i));
        int ver = prop.major * 10000 + prop.minor;
        if (ver < 50002)continue;
        if (!prop.unifiedAddressing)continue;
        if (!prop.managedMemory)continue;
        if (maxwell < ver || (maxwell==ver && prop.totalGlobalMem>size))
            id = i, maxwell = ver,size=prop.totalGlobalMem;
    }
    if (id == -1)
        throw std::exception("Failed to initialize the CUDA environment.");
    checkError(cudaSetDevice(id));
    checkError(cudaGetDeviceProperties(&mProp, id));
}

const cudaDeviceProp& Environment::getProp() const {
    return mProp;
}

Environment::~Environment() {
    checkError(cudaDeviceSynchronize());
    checkError(cudaDeviceReset());
}

Environment& getEnvironment() {
    static Environment env;
    return env;
}

Event::Event(Pipeline& pipeline) {
    checkError(cudaEventCreateWithFlags(&mEvent, cudaEventDisableTiming));
    checkError(cudaEventRecord(mEvent, pipeline.getId()));
}

void Event::wait() {
    checkError(cudaEventSynchronize(mEvent));
}

void Event::wait(Pipeline & pipeline) {
    checkError(cudaStreamWaitEvent(pipeline.getId(), mEvent, 0));
}

Event::~Event() {
    checkError(cudaEventDestroy(mEvent));
}
