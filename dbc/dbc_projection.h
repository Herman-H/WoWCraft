#ifndef DBC_PROJECTION_H
#define DBC_PROJECTION_H

#include "dbc_files.h"

/*
 *  Describes projections of fields from a dbc_file
 *
 *  Format:
 *         X(...): Collection of mappings
 *         P(F,T): A mapping from F to T
 *
 *         X(P(f1,t1),
 *           P(f2,t2),
 *           P(f3,t3) )
 *
 *         Here we assume t1 = 0, t2 = 1, t3 = 2, i.e. they are given by their position in X, so we dont need to specify them.
 *         With this assumption it reduces to:
 *
 *         X(f1,f2,f3)
 *
 *         enum class { Ti... } : Names of the T side of the map, think of renaming in SQL queries (AS ...)
 *
 *  This essentially maps a subset of fields to renamed fields
 */


enum class dbc_projection_type
{
    CreatureFamilyName
};

template <dbc_projection_type P>
class dbc_projection;

struct creature_family_projection
{
    typedef tmp::tuple_i<0,8> map;

    enum class map_index
    {
        Id,
        Name
    };

    static constexpr const map_index map_key = map_index::Id;
};

struct creature_type_projection
{
    typedef tmp::tuple_i<0,1> map;
    enum class map_index
    {
        Id,Name
    };
    static constexpr const map_index map_key = map_index::Id;
};

// Unused
/*template <typename PROJECTION, typename OF>
class dbc_projection_of : public PROJECTION, OF
{

    using typename OF::field_types;
    typedef dbc_record<field_types> record_type;
    using typename OF::field_index;
    using OF::key_field;

    using typename PROJECTION::map;
    using typename PROJECTION::map_index;
    using PROJECTION::map_key;

};*/


#endif // DBC_RELATION_H
