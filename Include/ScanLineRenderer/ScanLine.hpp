#pragma once
#include <Base/Common.hpp>
#include <Base/Memory.hpp>

template<typename Vert, typename Out, typename Uniform>
using VSF = void(*)(Vert in,Uniform uniform, vec3& pos,Out& out);

template<typename Out, typename Uniform, typename FrameBuffer>
using FSF = void(*)(unsigned int triID,ivec2 uv,float z, Out in, Uniform uniform,
    FrameBuffer& frameBuffer);

template<typename Out>
struct VertexInfo {
    vec3 pos;
    /*
    NDC.x=pos.x/pos.z
    NDC.y=pos.y/pos.z
    z=pos.z
    */
    Out out;
};

template<typename Out>
CUDAInline VertexInfo<Out> lerpZ(VertexInfo<Out> a, VertexInfo<Out> b, float z) {
    auto u = (z - b.pos.z) / (a.pos.z - b.pos.z),v=1.0f-u;
    VertexInfo<Out> res;
    res.pos = { a.pos.x*u + b.pos.x*v,a.pos.y*u + b.pos.y*v,z};
    res.out = a.out*u + b.out*v;
    return res;
}

template<typename Vert, typename Out, typename Uniform,typename Converter,
    VSF<Vert, Out, Uniform> vs>
CALLABLE void runVS(unsigned int size,ReadOnlyCache(Vert) in,
    ReadOnlyCache(Uniform) u,VertexInfo<Out>* res,Converter converter) {
    auto id = getID();
    if (id >= size)return;
    auto& vert = res[id];
    vs(in[id], *u, vert.pos, vert.out);
    vert.pos = converter(vert.pos);
}

template<typename Uniform, typename FrameBuffer>
using FSFSF = void(*)(ivec2 NDC, Uniform uniform, FrameBuffer frameBuffer);

template<typename Uniform, typename FrameBuffer,FSFSF<Uniform,FrameBuffer> fs>
    CALLABLE void runFSFS(ReadOnlyCache(Uniform) u,
        FrameBuffer frameBuffer,uvec2 size) {
    auto x = blockIdx.x*blockDim.x + threadIdx.x;
    auto y = blockIdx.y*blockDim.y + threadIdx.y;
    if(x<size.x & y<size.y)
        fs(ivec2{ x,y }, *u, frameBuffer);
}

class UniqueIndex final {
private:
    unsigned int mSize;
public:
    UniqueIndex(unsigned int size) :mSize(size) {}
    BOTH auto size() const {
        return mSize;
    }
    CUDA uvec3 operator[](unsigned int off) const {
        auto base = off * 3;
        return { base,base + 1,base + 2 };
    }
};

class SharedIndex final {
private:
    ReadOnlyCache(uvec3) mPtr;
    unsigned int mSize;
public:
    SharedIndex(ReadOnlyCache(uvec3) idx, unsigned int size) :mPtr(idx), mSize(size) {}
    SharedIndex(DataViewer<uvec3> ibo) :mPtr(ibo.begin()), mSize(ibo.size()) {}
    BOTH auto size() const {
        return mSize;
    }
    CUDA auto operator[](unsigned int off) const {
        return mPtr[off];
    }
};

constexpr auto maxv = std::numeric_limits<unsigned int>::max();
