#ifndef DBC_FILES_H
#define DBC_FILES_H

#include "../tmp/tmp.h"
#include "../tmp/tmp_string.h"
#include "dbc_record.h"

enum class dbc_file_type
{
    AreaTable,
    CreatureDisplayInfo,
    CreatureFamily,
    CreatureModelData,
    CreatureSpellData,
    CreatureType,
    Emotes,
    Faction,
    FactionTemplate,
    GemProperties,
    Item,
    ItemClass,
    ItemDisplayInfo,
    ItemExtendedCost,
    ItemRandomProperties,
    ItemRandomSuffix,
    ItemSet,
    ItemSubClass,
    ItemSubClassMask,
    Languages,
    Lock,
    LockType,
    Map,
    SkillLine,
    Spell,
    SpellItemEnchantment,
    Title,
    TotemCategory
};


class creature_family_dbc
{
public:

    static constexpr const tmp::sized_cstring file_name = {"CreatureFamily"};

    enum class field_index
    {
        Id,MinScale,MinScaleLevel,MaxScale,MaxScaleLevel,SkillLine,ItemPetFoot,PetTalentType,Name,IconFile,

        SIZE
    };

    static constexpr const field_index key_field = field_index::Id;

    //                   ID          MinScale      MinScaleLvl MaxScale      MaxScaleLevel SkillLine   ItemPetFoot
    typedef tmp::tuple_t<dbc_int<4>, dbc_float<4>, dbc_int<4>, dbc_float<4>, dbc_int<4>,   dbc_int<4>, dbc_int<4>,
    //                  PetTalentType Name              IconFile
                        dbc_int<4>,   dbc_string<4*17>, dbc_string<4>> field_types;
};


class creature_type_dbc
{
public:
    static constexpr const tmp::sized_cstring file_name = {"CreatureType"};
    enum class field_index
    {
        Id,Name,NoXP,

        SIZE
    };

    static constexpr const field_index key_field = field_index::Id;

    typedef tmp::tuple_t<dbc_int<4>, dbc_string<4*17>, dbc_int<4>> field_types;
};




#endif // DBC_FILES_H
