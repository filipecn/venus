/* Copyright (c) 2026, FilipeCN.
 *
 * The MIT License (MIT)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

/// \file   indexed_handle.h
/// \author FilipeCN (filipedecn@gmail.com)
/// \date   2026-02-21

#pragma once

#include <venus/utils/result.h>

#include <variant>

namespace venus::utils {

/// \brief This holds a handle associated to an index.
/// \note The handle can be one of two types: a HandleType or an IdType.
/// \tparam HandleType (ex: VkDescriptorSet, VkDescriptorSetLayout)
template <typename HandleType, typename IdType> class IndexedHandle {
public:
  static IndexedHandle<HandleType, IdType> fromId(h_index index, IdType id) {
    IndexedHandle<HandleType> dsh;
    dsh.data_ = id;
    dsh.index_ = index;
    return dsh;
  }
  static IndexedHandle<HandleType, IdType> fromHandle(h_index index,
                                                      HandleType vk_handle) {
    IndexedHandle<HandleType> dsh;
    dsh.data_ = vk_handle;
    dsh.index_ = index;
    return dsh;
  }

  bool isHandle() const { return data_.index() == 0; }

  bool isIndex() const { return data_.index() == 1; }

  Result<HandleType> handle() const {
    if (const HandleType *val = std::get_if<HandleType>(&data_)) {
      return Result<HandleType>(*val);
    }
    return VeResult::notFound();
  }

  Result<h_index> id() const {
    if (const h_index *val = std::get_if<h_index>(&data_)) {
      return Result<h_index>(*val);
    }
    return VeResult::notFound();
  }

  h_index index() const { return index_; }

private:
  h_index index_{0};
  std::variant<HandleType, IdType> data_{VK_NULL_HANDLE};
};

} // namespace venus::utils

#ifdef VENUS_INCLUDE_DEBUG_TRAITS
namespace hermes {

template <typename HandleType, typename IdType>
struct DebugTraits<venus::utils::IndexedHandle<HandleType, IdType>> {
  static HERMES_CONST_OR_CONSTEXPR bool is_string_serializable = true;
  static DebugMessage
  message(const venus::utils::IndexedHandle<HandleType, IdType> &data) {
    DebugMessage m;
    if (data.isHandle())
      m.addFmt("DS[idx={}, handle={}]", data.index(),
               VENUS_VK_HANDLE_STRING(data.handle().value()));
    else
      m.addFmt("DS[idx={}, id={}]", data.index(), data.id().value());
    return m;
  }
};

} // namespace hermes

#endif // VENUS_INCLUDE_DEBUG_TRAITS