/*
  This file is part of MADNESS.

  Copyright (C) 2007,2010 Oak Ridge National Laboratory

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA

  For more information please contact:

  Robert J. Harrison
  Oak Ridge National Laboratory
  One Bethel Valley Road
  P.O. Box 2008, MS-6367

  email: harrisonrj@ornl.gov
  tel:   865-241-3937
  fax:   865-572-0680
*/

#ifndef MADNESS_WORLD_TYPE_TRAITS_H__INCLUDED
#define MADNESS_WORLD_TYPE_TRAITS_H__INCLUDED

/// \file typestuff.h
/// \brief type traits and templates

/*
 * N.B. this must be pure c++, usable without any context other than
 *      the current compiler + library + C++ standard.
 *      DO NOT include non-standard headers here!
 */

#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <iosfwd>
#include <madness/world/meta.h>

namespace madness {

   namespace operators {
   class __x {};
   std::ostream& operator<<(std::ostream&, const __x&);
   std::ostream& operator>>(std::ostream&, __x&);
   }  // namespace operators

   // fwd decls
   namespace archive {
     template <typename Archive, typename T, typename Enabler = void>
     struct ArchiveSerializeImpl;

     template <class Archive, class T, typename Enabler = void>
     struct ArchiveLoadImpl;

     template <class Archive, class T, typename Enabler = void>
     struct ArchiveStoreImpl;
    }

    template <typename> class Future;
    template <typename> struct add_future;
    template <typename> struct remove_future;

    // Remove Future, const, volatile, and reference qualifiers from the type
    template <typename T>
    struct remove_fcvr {
        typedef typename remove_future<typename std::remove_cv<
                   typename std::remove_reference<T>::type>::type>::type type;
    };
    template <typename T>
    using remove_fcvr_t = typename remove_fcvr<T>::type;

    /// is true type if \p T is a pointer to a free function
    template <typename T, typename Enabler = void> struct is_function_pointer : public std::false_type {};
    template <typename T> struct is_function_pointer<T, std::enable_if_t<std::is_function<typename std::remove_pointer<T>::type>::value>> : public std::true_type {};
    template <typename T> constexpr bool is_function_pointer_v = is_function_pointer<T>::value;

    // use std::is_member_function_pointer<T> if looking for is_member_function_pointer

    /// is true type if \p T is a pointer to free or member function
    template <typename T, typename Enabler = void> struct is_any_function_pointer : public std::false_type {};
    template <typename T> struct is_any_function_pointer<T, std::enable_if_t<std::is_member_function_pointer<T>::value || is_function_pointer_v<T>>> : public std::true_type {};
    template <typename T> constexpr bool is_any_function_pointer_v = is_any_function_pointer<T>::value;

    /// This defines stuff that is serialiable by bitwise copy.
    /// \warning This reports true for \c T that is an aggregate type
    ///          (struct or array) that includes pointers.
    template <typename T>
    struct is_trivially_serializable {
      static const bool value = \
        std::is_arithmetic<T>::value || \
        std::is_function<T>::value  || \
        is_any_function_pointer_v<T> || \
        (std::is_pod<T>::value && !std::is_pointer<T>::value);
//        ((std::is_class<T>::value || std::is_array<T>::value) && std::is_trivially_copyable<T>::value);
    };

    // namespace hiding implementation details of is_ostreammable ... by ensuring that the detector lives in a different namespace branch than the operators we do not accidentally pick them up
    namespace is_ostreammable_ns {

    template <typename To, typename From> using left_shift = decltype(std::declval<To>() << std::declval<From>());
    template <typename To, typename From> using left_shift_in_ns_madness_operators = decltype(madness::operators::operator<<(std::declval<To>(), std::declval<From>()));

    template <typename T> struct impl : public meta::disjunction<meta::is_detected_exact<std::ostream&, left_shift, std::ostream&, const T&>,
                                                                 meta::is_detected_exact<std::ostream&, left_shift_in_ns_madness_operators, std::ostream&, const T&>> {};
    }  // namespace is_ostreammable_ns

    /// True for types that are "serializable" to a std::ostream
    /// \note \c operator<<(std::ostream&,const T&) must be visible via ADL or defined in namespace madness::operators
    template <typename T>
    struct is_ostreammable : public is_ostreammable_ns::impl<T> {};

    /// Shortcut for \c is_ostreammable<T>::value
    template <typename T> constexpr bool is_ostreammable_v = is_ostreammable<T>::value;

    // namespace hiding implementation details of is_istreammable ... by ensuring that the detector lives in a different namespace branch than the operators we do not accidentally pick them up
    namespace is_istreammable_ns {

    template <typename From, typename To> using right_shift = decltype(std::declval<From>() >> std::declval<To>());
    template <typename From, typename To> using right_shift_in_ns_madness_operators = decltype(madness::operators::operator<<(std::declval<From>(), std::declval<To>()));

    template <typename T> struct impl : public meta::disjunction<meta::is_detected_exact<std::istream&, right_shift, std::istream&, T&>,
                                                                 meta::is_detected_exact<std::istream&, right_shift_in_ns_madness_operators, std::istream&, T&>> {};

    }  // namespace is_istreammable_ns

    /// True for types that are "deserialiable" from an std::istream
    /// \note \c operator>>(std::ostream&,T&) must be visible via ADL or defined in namespace madness::operators
    template <typename T>
    struct is_istreammable : public is_istreammable_ns::impl<T> {};

    /// Shortcut for \c is_istreammable<T>::value
    template <typename T> constexpr bool is_istreammable_v = is_istreammable<T>::value;

    /// providing automatic support for serializing to/from std streams requires bidirectional streammability
    template <typename T> constexpr bool is_iostreammable_v = is_istreammable_v<T> && is_ostreammable_v<T>;

    template <typename T> constexpr bool is_always_serializable =
    std::is_arithmetic<T>::value || \
    std::is_same<std::nullptr_t, typename std::remove_cv<T>::type>::value || \
    is_any_function_pointer_v<T> || \
    std::is_function<T>::value;

    /// helps to detect that `T` has a member serialization method that
    /// accepts single argument of type `Archive`
    /// @note use in combination with madness::meta::is_detected_v
    template<typename T, typename Archive>
    using has_member_serialize_t = decltype(std::declval<T&>().serialize(std::declval<Archive&>()));

    /// helps to detect that `T` has a member serialization method that
    /// accepts one argument of type `Archive` and an unsigned version
    /// @note use in combination with madness::meta::is_detected_v
    template<typename T, typename Archive>
    using has_member_serialize_with_version_t = decltype(std::declval<T&>().serialize(std::declval<Archive&>(),0u));

    /// helps to detect that `T` supports nonintrusive symmetric serialization
    /// @note use in combination with madness::meta::is_detected_v
    template<typename T, typename Archive>
    using has_nonmember_serialize_t = decltype(madness::archive::ArchiveSerializeImpl<Archive, T>::serialize(std::declval<Archive&>(), std::declval<T&>()));

    /// helps to detect that `T` supports nonintrusive asymmetric serialization via load
    /// @note use in combination with madness::meta::is_detected_v
    template<typename T, typename Archive>
    using has_nonmember_load_t = decltype(madness::archive::ArchiveLoadImpl<Archive, T>::load(std::declval<Archive&>(), std::declval<T&>()));

    /// helps to detect that `T` supports nonintrusive asymmetric serialization via load
    /// @note use in combination with madness::meta::is_detected_v
    template<typename T, typename Archive>
    using has_nonmember_store_t = decltype(madness::archive::ArchiveStoreImpl<Archive, T>::store(std::declval<Archive&>(), std::declval<T&>()));

    /// helps to detect that `T` supports freestanding serialize function
    /// @note use in combination with madness::meta::is_detected_v
    template<typename T, typename Archive>
    using has_freestanding_serialize_t = decltype(serialize(std::declval<Archive&>(), std::declval<T&>()));

    /// helps to detect that `T=U*` supports freestanding serialize function
    /// @note use in combination with madness::meta::is_detected_v
    template<typename T, typename Archive, typename = std::enable_if_t<std::is_pointer_v<T>>>
    using has_freestanding_serialize_with_size_t = decltype(serialize(std::declval<Archive&>(), std::declval<T&>(), 1u));

    /// helps to detect that `T` supports freestanding serialize function that accepts version
    /// @note use in combination with madness::meta::is_detected_v
    template<typename T, typename Archive, typename = std::enable_if_t<!std::is_pointer_v<T>>>
    using has_freestanding_serialize_with_version_t = decltype(serialize(std::declval<Archive&>(), std::declval<T&>(), 0u));

    /// true if this is well-formed:
    /// \code
    ///   // T t; Archive ar;
    ///   t.serialize(ar);
    /// \endcode
    template <typename T, typename Archive>
    inline constexpr bool has_member_serialize_v = madness::meta::is_detected_v<madness::has_member_serialize_t,T,Archive>;

    /// true if this is well-formed:
    /// \code
    ///   // T t; Archive ar;
    ///   t.serialize(ar, 0u);
    /// \endcode
    template <typename T, typename Archive>
    inline constexpr bool has_member_serialize_with_version_v = madness::meta::is_detected_v<madness::has_member_serialize_with_version_t,T,Archive>;

    /// true if this is well-formed:
    /// \code
    ///   // T t; Archive ar;
    ///   madness::archive::ArchiveSerializeImpl<Archive, T>::serialize(ar, t);
    /// \endcode
    template <typename T, typename Archive>
    inline constexpr bool has_nonmember_serialize_v = madness::meta::is_detected_v<madness::has_nonmember_serialize_t,T,Archive>;

    /// true if this is well-formed:
    /// \code
    ///   // T t; Archive ar;
    ///   madness::archive::ArchiveLoadImpl<Archive, T>::load(ar, t);
    /// \endcode
    template <typename T, typename Archive>
    inline constexpr bool has_nonmember_load_v = madness::meta::is_detected_v<madness::has_nonmember_load_t,T,Archive>;

    /// true if this is well-formed:
    /// \code
    ///   // T t; Archive ar;
    ///   madness::archive::ArchiveStoreImpl<Archive, T>::store(ar, t);
    /// \endcode
    template <typename T, typename Archive>
    inline constexpr bool has_nonmember_store_v = madness::meta::is_detected_v<madness::has_nonmember_store_t,T,Archive>;

    template <typename T, typename Archive>
    inline constexpr bool has_nonmember_load_and_store_v = has_nonmember_load_v<T, Archive> && has_nonmember_store_v<T, Archive>;

    /// true if this is well-formed:
    /// \code
    ///   // T t; Archive ar;
    ///   serialize(ar, t);
    /// \endcode
    template <typename T, typename Archive>
    inline constexpr bool has_freestanding_serialize_v = madness::meta::is_detected_v<madness::has_freestanding_serialize_t,T,Archive>;

    /// true if this is well-formed:
    /// \code
    ///   // T t; Archive ar;
    ///   serialize(ar, t, 1u);
    /// \endcode
    template <typename T, typename Archive>
    inline constexpr bool has_freestanding_serialize_with_size_v = madness::meta::is_detected_v<madness::has_freestanding_serialize_with_size_t,T,Archive>;

    /// true if this is well-formed:
    /// \code
    ///   // T t; Archive ar;
    ///   serialize(ar, t, 0u);
    /// \endcode
    template <typename T, typename Archive>
    inline constexpr bool has_freestanding_serialize_with_version_v = madness::meta::is_detected_v<madness::has_freestanding_serialize_with_version_t,T,Archive>;

    template <typename Archive, typename T, typename Enabler = void>
    struct is_default_serializable_helper : public std::false_type {};

    /// \brief is \c std::true_type if \c T can be serialized to \c Archive
    ///        without specialized \c serialize() method
    ///
    /// For text stream-based \c Archive this is \c std::true_type if \c is_iostreammable<T>::value is true.
    /// For other \c Archive types this is \c std::true_type if \c is_trivially_serializable<T>::value is true.
    /// \tparam Archive an Archive type
    /// \tparam T a type
    template <typename Archive, typename T>
    struct is_default_serializable {
      static constexpr bool value = is_default_serializable_helper<Archive,T>::value;
    };

    template <typename Archive, typename T>
    inline constexpr bool is_default_serializable_v = is_default_serializable<Archive, T>::value;


    // forward declare archives to provide archive-specific overloads
    namespace archive {
    class BaseArchive;
    class BaseInputArchive;
    class BaseOutputArchive;
    class BinaryFstreamOutputArchive;
    class BinaryFstreamInputArchive;
    class BufferOutputArchive;
    class BufferInputArchive;
    class VectorOutputArchive;
    class VectorInputArchive;
    class TextFstreamOutputArchive;
    class TextFstreamInputArchive;
    class MPIRawOutputArchive;
    class MPIRawInputArchive;
    class MPIOutputArchive;
    class MPIInputArchive;
    template <typename T>
    class archive_array;
    }  // namespace archive

    /// Checks if \c T is an archive type.

    /// If \c T is an archive type, then \c is_archive will be inherited
    /// from \c std::true_type, otherwise it is inherited from
    /// \c std::false_type.
    /// \tparam T The type to check.
    template <typename T>
    struct is_archive : public std::is_base_of<archive::BaseArchive, T>{};

    template <typename T>
    inline constexpr bool is_archive_v = is_archive<T>::value;

    /// Checks if \c T is an input archive type.

    /// If \c T is an input archive type, then \c is_input_archive will be
    /// inherited from \c std::true_type, otherwise it is inherited from
    /// \c std::false_type.
    /// \tparam T The type to check.
    template <typename T>
    struct is_input_archive : public std::is_base_of<archive::BaseInputArchive, T> {};

    template <typename T>
    inline constexpr bool is_input_archive_v = is_input_archive<T>::value;

    /// Checks if \c T is an output archive type.

    /// If \c T is an output archive type, then \c is_output_archive will
    /// be inherited from \c std::true_type, otherwise it is inherited from
    /// \c std::false_type.
    /// \tparam T The type to check.
    template <typename T>
    struct is_output_archive : public std::is_base_of<archive::BaseOutputArchive, T> {};

    template <typename T>
    inline constexpr bool is_output_archive_v = is_output_archive<T>::value;

    template <typename T>
    struct is_default_serializable_helper<archive::BinaryFstreamOutputArchive, T, std::enable_if_t<is_trivially_serializable<T>::value>> : std::true_type {};
    template <typename T>
    struct is_default_serializable_helper<archive::BinaryFstreamInputArchive, T, std::enable_if_t<is_trivially_serializable<T>::value>> : std::true_type {};
    template <typename T>
    struct is_default_serializable_helper<archive::BufferOutputArchive, T, std::enable_if_t<is_trivially_serializable<T>::value>> : std::true_type {};
    template <typename T>
    struct is_default_serializable_helper<archive::BufferInputArchive, T, std::enable_if_t<is_trivially_serializable<T>::value>> : std::true_type {};
    template <typename T>
    struct is_default_serializable_helper<archive::VectorOutputArchive, T, std::enable_if_t<is_trivially_serializable<T>::value>> : std::true_type {};
    template <typename T>
    struct is_default_serializable_helper<archive::VectorInputArchive, T, std::enable_if_t<is_trivially_serializable<T>::value>> : std::true_type {};
    template <typename T>
    struct is_default_serializable_helper<archive::TextFstreamOutputArchive, T, std::enable_if_t<is_iostreammable_v<T>>> : std::true_type {};
    template <typename T>
    struct is_default_serializable_helper<archive::TextFstreamInputArchive, T, std::enable_if_t<is_iostreammable_v<T>>> : std::true_type {};
    template <typename T>
    struct is_default_serializable_helper<archive::MPIRawOutputArchive, T, std::enable_if_t<is_trivially_serializable<T>::value>> : std::true_type {};
    template <typename T>
    struct is_default_serializable_helper<archive::MPIRawInputArchive, T, std::enable_if_t<is_trivially_serializable<T>::value>> : std::true_type {};
    template <typename T>
    struct is_default_serializable_helper<archive::MPIOutputArchive, T, std::enable_if_t<is_trivially_serializable<T>::value>> : std::true_type {};
    template <typename T>
    struct is_default_serializable_helper<archive::MPIInputArchive, T, std::enable_if_t<is_trivially_serializable<T>::value>> : std::true_type {};
    template <typename Archive, typename T>
    struct is_default_serializable_helper<Archive, archive::archive_array<T>, std::enable_if_t<is_default_serializable_helper<Archive,T>::value>> : std::true_type {};

    /// Evaluates to true is can serialize an object of type `T` to an object of type `Archive`
    /// \tparam Archive
    /// \tparam T
    template <typename Archive, typename T>
    inline constexpr bool is_serializable_v = is_archive_v<Archive> && (is_default_serializable_v<Archive, T> ||
        has_member_serialize_v<Archive, T> ||
        has_member_serialize_with_version_v<Archive, T> ||
        has_nonmember_serialize_v<Archive, T> ||
        (has_nonmember_load_v<Archive, T> && is_input_archive_v<Archive>) ||
        (has_nonmember_store_v<Archive, T> && is_output_archive_v<Archive>));

    template <typename Archive, typename T>
    struct is_serializable : std::bool_constant<is_serializable_v<Archive,T>> {};

    /// \brief This trait types tests if \c Archive is a text archive
    /// \tparam Archive an archive type
    /// \note much be specialized for each archive
    template <typename Archive, typename Enabler = void>
    struct is_text_archive : std::false_type {};

    template <>
    struct is_text_archive<archive::TextFstreamOutputArchive> : std::true_type {};
    template <>
    struct is_text_archive<archive::TextFstreamInputArchive> : std::true_type {};

    /// \brief \c is_text_archive_v<A> is a shorthand for \c is_text_archive<A>::value
    /// \tparam Archive an archive type
    template <typename Archive>
    constexpr const bool is_text_archive_v = is_text_archive<Archive>::value;

    /* Macros to make some of this stuff more readable */

    /**
       \def REMCONST(TYPE)
       \brief Macro to make remove_const<T> easier to use

       \def MEMFUN_RETURNT(TYPE)
       \brief Macro to make member function type traits easier to use
    */

#define REMCONST(TYPE)  typename std::remove_const< TYPE >::type
#define MEMFUN_RETURNT(MEMFUN) typename madness::detail::memfunc_traits< MEMFUN >::result_type

} // namespace madness

#endif // MADNESS_WORLD_TYPE_TRAITS_H__INCLUDED
