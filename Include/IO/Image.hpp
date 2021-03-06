#pragma once
#include <Base/Builtin.hpp>
#include <functional>

std::shared_ptr<BuiltinArray<RGBA>> loadRGBA(const std::string& path,Stream& stream);
std::shared_ptr<BuiltinMipmapedArray<RGBA>> loadMipmapedRGBA(const std::string& path,
    Stream& stream);
std::shared_ptr<BuiltinCubeMap<RGBA>> loadCubeMap(const std::function<std::string(size_t id)>& path,
    Stream& stream);
