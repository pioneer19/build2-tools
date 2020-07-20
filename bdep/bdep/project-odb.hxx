// -*- C++ -*-
//
// This file was generated by ODB, object-relational mapping (ORM)
// compiler for C++.
//

#ifndef BDEP_PROJECT_ODB_HXX
#define BDEP_PROJECT_ODB_HXX

// Begin prologue.
//
#include <bdep/wrapper-traits.hxx>
#include <bdep/value-traits.hxx>
//
// End prologue.

#include <odb/version.hxx>

#if (ODB_VERSION != 20469UL)
#error ODB runtime version mismatch
#endif

#include <odb/pre.hxx>

#include <bdep/project.hxx>

#include <memory>
#include <cstddef>
#include <utility>

#include <odb/core.hxx>
#include <odb/traits.hxx>
#include <odb/callback.hxx>
#include <odb/wrapper-traits.hxx>
#include <odb/pointer-traits.hxx>
#include <odb/container-traits.hxx>
#include <odb/session.hxx>
#include <odb/cache-traits.hxx>
#include <odb/result.hxx>
#include <odb/simple-object-result.hxx>
#include <odb/view-image.hxx>
#include <odb/view-result.hxx>

#include <odb/details/unused.hxx>
#include <odb/details/shared-ptr.hxx>

namespace odb
{
  // configuration
  //
  template <>
  struct class_traits< ::bdep::configuration >
  {
    static const class_kind kind = class_object;
  };

  template <>
  class access::object_traits< ::bdep::configuration >
  {
    public:
    typedef ::bdep::configuration object_type;
    typedef ::std::shared_ptr< ::bdep::configuration > pointer_type;
    typedef odb::pointer_traits<pointer_type> pointer_traits;

    static const bool polymorphic = false;

    typedef ::bdep::optional_uint64_t id_type;

    static const bool auto_id = true;

    static const bool abstract = false;

    static id_type
    id (const object_type&);

    typedef
    odb::pointer_cache_traits<
      pointer_type,
      odb::session >
    pointer_cache_traits;

    typedef
    odb::reference_cache_traits<
      object_type,
      odb::session >
    reference_cache_traits;

    static void
    callback (database&, object_type&, callback_event);

    static void
    callback (database&, const object_type&, callback_event);
  };

  // configuration_count
  //
  template <>
  struct class_traits< ::bdep::configuration_count >
  {
    static const class_kind kind = class_view;
  };

  template <>
  class access::view_traits< ::bdep::configuration_count >
  {
    public:
    typedef ::bdep::configuration_count view_type;
    typedef ::bdep::configuration_count* pointer_type;

    static void
    callback (database&, view_type&, callback_event);
  };
}

#include <odb/details/buffer.hxx>

#include <odb/sqlite/version.hxx>
#include <odb/sqlite/forward.hxx>
#include <odb/sqlite/binding.hxx>
#include <odb/sqlite/sqlite-types.hxx>
#include <odb/sqlite/query.hxx>

namespace odb
{
  // package_state
  //
  template <>
  class access::composite_value_traits< ::bdep::package_state, id_sqlite >
  {
    public:
    typedef ::bdep::package_state value_type;

    struct image_type
    {
      // name
      //
      details::buffer name_value;
      std::size_t name_size;
      bool name_null;
    };

    static bool
    grow (image_type&,
          bool*);

    static void
    bind (sqlite::bind*,
          image_type&,
          sqlite::statement_kind);

    static bool
    init (image_type&,
          const value_type&,
          sqlite::statement_kind);

    static void
    init (value_type&,
          const image_type&,
          database*);

    static bool
    get_null (const image_type&);

    static void
    set_null (image_type&,
              sqlite::statement_kind);

    static const std::size_t column_count = 1UL;
  };

  // configuration
  //
  template <typename A>
  struct query_columns< ::bdep::configuration, id_sqlite, A >
  {
    // id
    //
    typedef
    sqlite::query_column<
      sqlite::value_traits<
        ::odb::optional_uint64_traits::wrapped_type,
        sqlite::id_integer >::query_type,
      sqlite::id_integer >
    id_type_;

    static const id_type_ id;

    // name
    //
    typedef
    sqlite::query_column<
      sqlite::value_traits<
        ::std::basic_string< char >,
        sqlite::id_text >::query_type,
      sqlite::id_text >
    name_type_;

    static const name_type_ name;

    // path
    //
    typedef
    sqlite::query_column<
      sqlite::value_traits<
        ::std::string,
        sqlite::id_text >::query_type,
      sqlite::id_text >
    path_type_;

    static const path_type_ path;

    // relative_path
    //
    typedef
    sqlite::query_column<
      sqlite::value_traits<
        ::std::basic_string< char >,
        sqlite::id_text >::query_type,
      sqlite::id_text >
    relative_path_type_;

    static const relative_path_type_ relative_path;

    // default_
    //
    typedef
    sqlite::query_column<
      sqlite::value_traits<
        bool,
        sqlite::id_integer >::query_type,
      sqlite::id_integer >
    default__type_;

    static const default__type_ default_;

    // forward
    //
    typedef
    sqlite::query_column<
      sqlite::value_traits<
        bool,
        sqlite::id_integer >::query_type,
      sqlite::id_integer >
    forward_type_;

    static const forward_type_ forward;

    // auto_sync
    //
    typedef
    sqlite::query_column<
      sqlite::value_traits<
        bool,
        sqlite::id_integer >::query_type,
      sqlite::id_integer >
    auto_sync_type_;

    static const auto_sync_type_ auto_sync;
  };

  template <typename A>
  const typename query_columns< ::bdep::configuration, id_sqlite, A >::id_type_
  query_columns< ::bdep::configuration, id_sqlite, A >::
  id (A::table_name, "\"id\"", 0);

  template <typename A>
  const typename query_columns< ::bdep::configuration, id_sqlite, A >::name_type_
  query_columns< ::bdep::configuration, id_sqlite, A >::
  name (A::table_name, "\"name\"", 0);

  template <typename A>
  const typename query_columns< ::bdep::configuration, id_sqlite, A >::path_type_
  query_columns< ::bdep::configuration, id_sqlite, A >::
  path (A::table_name, "\"path\"", 0);

  template <typename A>
  const typename query_columns< ::bdep::configuration, id_sqlite, A >::relative_path_type_
  query_columns< ::bdep::configuration, id_sqlite, A >::
  relative_path (A::table_name, "\"relative_path\"", 0);

  template <typename A>
  const typename query_columns< ::bdep::configuration, id_sqlite, A >::default__type_
  query_columns< ::bdep::configuration, id_sqlite, A >::
  default_ (A::table_name, "\"default\"", 0);

  template <typename A>
  const typename query_columns< ::bdep::configuration, id_sqlite, A >::forward_type_
  query_columns< ::bdep::configuration, id_sqlite, A >::
  forward (A::table_name, "\"forward\"", 0);

  template <typename A>
  const typename query_columns< ::bdep::configuration, id_sqlite, A >::auto_sync_type_
  query_columns< ::bdep::configuration, id_sqlite, A >::
  auto_sync (A::table_name, "\"auto_sync\"", 0);

  template <typename A>
  struct pointer_query_columns< ::bdep::configuration, id_sqlite, A >:
    query_columns< ::bdep::configuration, id_sqlite, A >
  {
  };

  template <>
  class access::object_traits_impl< ::bdep::configuration, id_sqlite >:
    public access::object_traits< ::bdep::configuration >
  {
    public:
    struct id_image_type
    {
      long long id_value;
      bool id_null;

      std::size_t version;
    };

    struct image_type
    {
      // id
      //
      long long id_value;
      bool id_null;

      // name
      //
      details::buffer name_value;
      std::size_t name_size;
      bool name_null;

      // path
      //
      details::buffer path_value;
      std::size_t path_size;
      bool path_null;

      // relative_path
      //
      details::buffer relative_path_value;
      std::size_t relative_path_size;
      bool relative_path_null;

      // default_
      //
      long long default_value;
      bool default_null;

      // forward
      //
      long long forward_value;
      bool forward_null;

      // auto_sync
      //
      long long auto_sync_value;
      bool auto_sync_null;

      std::size_t version;
    };

    struct extra_statement_cache_type;

    // packages
    //
    struct packages_traits
    {
      static const std::size_t id_column_count = 1UL;
      static const std::size_t data_column_count = 3UL;

      static const bool versioned = false;

      static const char insert_statement[];
      static const char select_statement[];
      static const char delete_statement[];

      typedef ::std::vector< ::bdep::package_state > container_type;
      typedef
      odb::access::container_traits<container_type>
      container_traits_type;
      typedef container_traits_type::index_type index_type;
      typedef container_traits_type::value_type value_type;

      typedef ordered_functions<index_type, value_type> functions_type;
      typedef sqlite::container_statements< packages_traits > statements_type;

      struct data_image_type
      {
        // index
        //
        long long index_value;
        bool index_null;

        // value
        //
        composite_value_traits< value_type, id_sqlite >::image_type value_value;

        std::size_t version;
      };

      static void
      bind (sqlite::bind*,
            const sqlite::bind* id,
            std::size_t id_size,
            data_image_type&);

      static void
      grow (data_image_type&,
            bool*);

      static void
      init (data_image_type&,
            index_type*,
            const value_type&);

      static void
      init (index_type&,
            value_type&,
            const data_image_type&,
            database*);

      static void
      insert (index_type, const value_type&, void*);

      static bool
      select (index_type&, value_type&, void*);

      static void
      delete_ (void*);

      static void
      persist (const container_type&,
               statements_type&);

      static void
      load (container_type&,
            statements_type&);

      static void
      update (const container_type&,
              statements_type&);

      static void
      erase (statements_type&);
    };

    using object_traits<object_type>::id;

    static id_type
    id (const id_image_type&);

    static id_type
    id (const image_type&);

    static bool
    grow (image_type&,
          bool*);

    static void
    bind (sqlite::bind*,
          image_type&,
          sqlite::statement_kind);

    static void
    bind (sqlite::bind*, id_image_type&);

    static bool
    init (image_type&,
          const object_type&,
          sqlite::statement_kind);

    static void
    init (object_type&,
          const image_type&,
          database*);

    static void
    init (id_image_type&, const id_type&);

    typedef sqlite::object_statements<object_type> statements_type;

    typedef sqlite::query_base query_base_type;

    static const std::size_t column_count = 7UL;
    static const std::size_t id_column_count = 1UL;
    static const std::size_t inverse_column_count = 0UL;
    static const std::size_t readonly_column_count = 0UL;
    static const std::size_t managed_optimistic_column_count = 0UL;

    static const std::size_t separate_load_column_count = 0UL;
    static const std::size_t separate_update_column_count = 0UL;

    static const bool versioned = false;

    static const char persist_statement[];
    static const char find_statement[];
    static const char update_statement[];
    static const char erase_statement[];
    static const char query_statement[];
    static const char erase_query_statement[];

    static const char table_name[];

    static void
    persist (database&, object_type&);

    static pointer_type
    find (database&, const id_type&);

    static bool
    find (database&, const id_type&, object_type&);

    static bool
    reload (database&, object_type&);

    static void
    update (database&, const object_type&);

    static void
    erase (database&, const id_type&);

    static void
    erase (database&, const object_type&);

    static result<object_type>
    query (database&, const query_base_type&);

    static unsigned long long
    erase_query (database&, const query_base_type&);

    public:
    static bool
    find_ (statements_type&,
           const id_type*);

    static void
    load_ (statements_type&,
           object_type&,
           bool reload);
  };

  template <>
  class access::object_traits_impl< ::bdep::configuration, id_common >:
    public access::object_traits_impl< ::bdep::configuration, id_sqlite >
  {
  };

  // configuration_count
  //
  template <>
  class access::view_traits_impl< ::bdep::configuration_count, id_sqlite >:
    public access::view_traits< ::bdep::configuration_count >
  {
    public:
    struct image_type
    {
      // result
      //
      long long result_value;
      bool result_null;

      std::size_t version;
    };

    typedef sqlite::view_statements<view_type> statements_type;

    typedef sqlite::query_base query_base_type;
    struct query_columns;

    static const bool versioned = false;

    static bool
    grow (image_type&,
          bool*);

    static void
    bind (sqlite::bind*,
          image_type&);

    static void
    init (view_type&,
          const image_type&,
          database*);

    static const std::size_t column_count = 1UL;

    static query_base_type
    query_statement (const query_base_type&);

    static result<view_type>
    query (database&, const query_base_type&);
  };

  template <>
  class access::view_traits_impl< ::bdep::configuration_count, id_common >:
    public access::view_traits_impl< ::bdep::configuration_count, id_sqlite >
  {
  };

  // configuration
  //
  // configuration_count
  //
  struct access::view_traits_impl< ::bdep::configuration_count, id_sqlite >::query_columns:
    odb::pointer_query_columns<
      ::bdep::configuration,
      id_sqlite,
      odb::access::object_traits_impl< ::bdep::configuration, id_sqlite > >
  {
  };
}

#include <bdep/project-odb.ixx>

#include <odb/post.hxx>

#endif // BDEP_PROJECT_ODB_HXX
