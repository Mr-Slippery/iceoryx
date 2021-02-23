// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 by Apex.AI Inc. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef IOX_POSH_POPO_SAMPLE_HPP
#define IOX_POSH_POPO_SAMPLE_HPP

#include "iceoryx_posh/mepoo/chunk_header.hpp"
#include "iceoryx_utils/cxx/unique_ptr.hpp"

namespace iox
{
namespace popo
{
template <typename T>
class PublisherInterface;

namespace internal
{
/// @brief helper struct for sample
template <typename T>
struct SamplePrivateData
{
    SamplePrivateData(cxx::unique_ptr<T>&& samplePtr, PublisherInterface<T>& publisher);

    SamplePrivateData(SamplePrivateData&& rhs);
    SamplePrivateData& operator=(SamplePrivateData&& rhs);

    SamplePrivateData(const SamplePrivateData&) = delete;
    SamplePrivateData& operator=(const SamplePrivateData&) = delete;

    cxx::unique_ptr<T> samplePtr{[](T* const) {}}; // Placeholder. This is overwritten on sample construction.
    std::reference_wrapper<PublisherInterface<T>> publisherRef;
};

/// @brief specialization of helper struct for sample for const T
template <typename T>
struct SamplePrivateData<const T>
{
    SamplePrivateData(cxx::unique_ptr<const T>&& samplePtr) noexcept;

    SamplePrivateData(SamplePrivateData&& rhs);
    SamplePrivateData& operator=(SamplePrivateData&& rhs);

    SamplePrivateData(const SamplePrivateData&) = delete;
    SamplePrivateData& operator=(const SamplePrivateData&) = delete;

    cxx::unique_ptr<const T> samplePtr{
        [](const T* const) {}}; // Placeholder. This is overwritten on sample construction.
};
} // namespace internal

///
/// @brief The Sample class is a mutable abstraction over types which are written to loaned shared memory.
/// These samples are publishable to the iceoryx system.
///
template <typename T>
class Sample
{
  public:
    /// @brief constructor for a Sample used by the Publisher
    /// @tparam S is a dummy template parameter to enable the constructor only for non-const T
    /// @param samplePtr is a `rvalue` to a `cxx::unique_ptr<T>` with to the data of the encapsulated type T
    /// @param publisher is a reference to the publisher to be able to use the `publish` and `release` methods
    template <typename S = T, typename = std::enable_if_t<std::is_same<S, T>::value && !std::is_const<S>::value, S>>
    Sample(cxx::unique_ptr<T>&& samplePtr, PublisherInterface<T>& publisher);

    /// @brief constructor for a Sample used by the Subscriber
    /// @tparam S is a dummy template parameter to enable the constructor only for const T
    /// @param samplePtr is a `rvalue` to a `cxx::unique_ptr<T>` with to the data of the encapsulated type T
    template <typename S = T, typename = std::enable_if_t<std::is_same<S, T>::value && std::is_const<S>::value, S>>
    Sample(cxx::unique_ptr<T>&& samplePtr) noexcept;

    Sample(std::nullptr_t) noexcept;
    ~Sample();

    Sample<T>& operator=(Sample<T>&& rhs);
    Sample(Sample<T>&& rhs);

    Sample(const Sample<T>&) = delete;
    Sample<T>& operator=(const Sample<T>&) = delete;


    ///
    /// @brief operator -> Transparent access to the encapsulated type.
    /// @return a pointer to the encapsulated type.
    /// @details Only available for non-const type T.
    ///
    template <typename S = T, typename = std::enable_if_t<std::is_same<S, T>::value && !std::is_const<S>::value, S>>
    T* operator->() noexcept;

    ///
    /// @brief operator -> Transparent read-only access to the encapsulated type.
    /// @return a const pointer to the encapsulated type.
    ///
    const T* operator->() const noexcept;

    ///
    /// @brief operator* Provide a reference to the encapsulated type.
    /// @return A T& to the encapsulated type.
    /// @details Only available for non-const type T.
    ///
    template <typename S = T, typename = std::enable_if_t<std::is_same<S, T>::value && !std::is_const<S>::value, S>>
    T& operator*() noexcept;

    ///
    /// @brief operator* Provide a const reference to the encapsulated type.
    /// @return A const T& to the encapsulated type.
    ///
    const T& operator*() const noexcept;

    ///
    /// @brief operator bool Indciates whether the sample is valid, i.e. refers to allocated memory.
    /// @return true if the sample is valid, false otherwise.
    ///
    operator bool() const;

    ///
    /// @brief allocation Access to the encapsulated type loaned to the sample.
    /// @return a pointer to the encapsulated type.
    /// @details Only available for non-const type T.
    ///
    template <typename S = T, typename = std::enable_if_t<std::is_same<S, T>::value && !std::is_const<S>::value, S>>
    T* get() noexcept;

    ///
    /// @brief allocation Read-only access to the encapsulated type loaned to the sample.
    /// @return a const pointer to the encapsulated type.
    ///
    const T* get() const noexcept;

    ///
    /// @brief header Retrieve the header of the underlying memory chunk loaned to the sample.
    /// @return The ChunkHeader of the underlying memory chunk.
    /// @details Only available for non-const type T.
    ///
    template <typename S = T, typename = std::enable_if_t<std::is_same<S, T>::value && !std::is_const<S>::value, S>>
    mepoo::ChunkHeader* getHeader() noexcept;

    ///
    /// @brief header Retrieve the header of the underlying memory chunk loaned to the sample.
    /// @return The const ChunkHeader of the underlying memory chunk.
    ///
    const mepoo::ChunkHeader* getHeader() const noexcept;

    ///
    /// @brief publish Publish the sample via the publisher from which it was loaned and automatically
    /// release ownership to it.
    /// @details Only available for non-const type T.
    ///
    template <typename S = T, typename = std::enable_if_t<std::is_same<S, T>::value && !std::is_const<S>::value, S>>
    void publish() noexcept;

    ///
    /// @brief release Manually release ownership of the loaned memory chunk.
    /// @details This prevents the sample from automatically releasing ownership on destruction and is only available
    /// for non-const type T.
    ///
    template <typename S = T, typename = std::enable_if_t<std::is_same<S, T>::value && !std::is_const<S>::value, S>>
    void release() noexcept;

  protected:
    internal::SamplePrivateData<T> m_members;
};

} // namespace popo
} // namespace iox

#include "iceoryx_posh/internal/popo/sample.inl"

#endif // IOX_POSH_POPO_SAMPLE_HPP
