// SPDX-FileCopyrightText: NVIDIA CORPORATION & AFFILIATES
// Copyright (c) 2021-2023 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef __NVDSINFERSERVER_I_OPTIONS_H__
#define __NVDSINFERSERVER_I_OPTIONS_H__

#include <infer_defines.h>
#include <nvdsinfer.h>

#include <functional>
#include <list>
#include <memory>
#include <string>
#include <type_traits>
#include <vector>

namespace nvdsinferserver {

enum class OptionType : int {
    oBool = 0,
    oDouble,
    oInt,
    oUint,
    oString,
    oObject,
    oArray,
    oNone = -1,
};

#define OPTION_SEQUENCE_ID "sequence_id"  // uint64_t
#define OPTION_SEQUENCE_START "sequence_start"  // bool
#define OPTION_SEQUENCE_END "sequence_end"  // bool
#define OPTION_PRIORITY "priority"  // uint64_t
#define OPTION_TIMEOUT "timeout_ms"  // uint64_t
#define OPTION_NVDS_UNIQUE_ID "nvds_unique_id"  // int64_t
#define OPTION_NVDS_SREAM_IDS "nvds_stream_ids"  // source_id list, vector<uint64_t>
#define OPTION_NVDS_FRAME_META_LIST "nvds_frame_meta_list"  // vector<NvDsFrameMeta*>
#define OPTION_NVDS_OBJ_META_LIST "nvds_obj_meta_list"  // vector<NvDsObjectMeta*>
#define OPTION_NVDS_BATCH_META "nvds_batch_meta"  // NvDsBatchMeta*
#define OPTION_NVDS_GST_BUFFER "nvds_gst_buffer"  // GstBuffer*
#define OPTION_NVDS_BUF_SURFACE "nvds_buf_surface"  // NvBufSurface*
#define OPTION_NVDS_BUF_SURFACE_PARAMS_LIST "nvds_buf_surface_params_list"  // vector<NvBufSurfaceParams*>
#define OPTION_TIMESTAMP "timestamp"  // uint64_t timestamp nano seconds

class IOptions {
public:
    IOptions() = default;
    virtual ~IOptions() = default;
    virtual bool hasValue(const std::string& key) const = 0;
    virtual OptionType getType(const std::string& name) const = 0;
    virtual uint32_t getCount() const = 0;
    virtual std::string getKey(uint32_t idx) const = 0;

protected:
    virtual NvDsInferStatus getValuePtr(
        const std::string& name, OptionType t, void*& ptr) const = 0;
    virtual NvDsInferStatus getArraySize(const std::string& key, uint32_t& size) const = 0;
    virtual NvDsInferStatus getRawPtrArray(
        const std::string& name, OptionType ot, void** ptrBase, uint32_t size) const = 0;

    template <OptionType V>
    struct OTypeV {
        static constexpr OptionType v = V;
    };
    template <typename Value>
    struct oType;

public:
    NvDsInferStatus getDouble(const std::string& name, double& v) const
    {
        return getValue<double>(name, v);
    }
    NvDsInferStatus getInt(const std::string& name, int64_t& v) const
    {
        return getValue<int64_t>(name, v);
    }
    NvDsInferStatus getUInt(const std::string& name, uint64_t& v) const
    {
        return getValue<uint64_t>(name, v);
    }
    NvDsInferStatus getString(const std::string& name, std::string& v)
    {
        return getValue<std::string>(name, v);
    }
    NvDsInferStatus getBool(const std::string& name, bool& v) const
    {
        return getValue<bool>(name, v);
    }
    template <typename Obj>
    NvDsInferStatus getObj(const std::string& name, Obj*& obj) const
    {
        return getValue<Obj*>(name, obj);
    }

    template <typename Value>
    NvDsInferStatus getValue(const std::string& name, Value& value) const
    {
        using ValueType = std::remove_const_t<Value>;
        OptionType otype = oType<ValueType>::v;
        void* ptr = nullptr;
        auto status = getValuePtr(name, otype, ptr);
        if (status == NVDSINFER_SUCCESS) {
            assert(ptr);
            value = *reinterpret_cast<ValueType*>(ptr);
        }
        return status;
    }

    template <typename Value>
    NvDsInferStatus getValueArray(const std::string& name, std::vector<Value>& values) const
    {
        using ValueType = std::remove_const_t<Value>;
        OptionType otype = oType<ValueType>::v;
        uint32_t size = 0;
        auto status = getArraySize(name, size);
        if (status != NVDSINFER_SUCCESS) {
            return status;
        }
        std::vector<ValueType*> valuePtrs(size);
        void** ptrBase = reinterpret_cast<void**>(valuePtrs.data());
        values.resize(size);
        status = getRawPtrArray(name, otype, ptrBase, size);
        if (status == NVDSINFER_SUCCESS) {
            values.resize(size);
            for (uint32_t i = 0; i < size; ++i) {
                values[i] = *valuePtrs[i];
            }
        }
        return status;
    }
};

template <OptionType v>
constexpr OptionType IOptions::OTypeV<v>::v;

template <typename Value>
struct IOptions::oType<Value*> : IOptions::OTypeV<OptionType::oObject> {
};
template <>
struct IOptions::oType<bool> : IOptions::OTypeV<OptionType::oBool> {
};
template <>
struct IOptions::oType<double> : IOptions::OTypeV<OptionType::oDouble> {
};
template <>
struct IOptions::oType<int64_t> : IOptions::OTypeV<OptionType::oInt> {
};
template <>
struct IOptions::oType<uint64_t> : IOptions::OTypeV<OptionType::oUint> {
};
template <>
struct IOptions::oType<std::string> : IOptions::OTypeV<OptionType::oString> {
};

}  // namespace nvdsinferserver

#endif  //  __NVDSINFERSERVER_I_OPTIONS_H__