#ifndef CREATURE_TEMPLATE_H
#define CREATURE_TEMPLATE_H

#include <array>
#include "dbtmp.h"

template <size_t N>
using varchar = std::array<char,N>;

/* Description of creature_template table  */
struct creature_template_tbc
{
    enum class field_index
    {
        Entry,Name,SubName,IconName,MinLevel,MaxLevel,HeroicEntry,ModelId1,ModelId2,ModelId3,ModelId4,
        FactionAlliance,FactionHorde,Scale,Family,CreatureType,InhabitType,RegenerateStats,RacialLeader,
        NpcFlags,UnitFlags,DynamicFlags,ExtraFlags,CreatureTypeFlags,SpeedWalk,SpeedRun,UnitClass,Rank,
        Expansion,HealthMultiplier,PowerMultiplier,DamageMultiplier,DamageVariance,ArmorMultiplier,
        ExperienceMultiplier,MinLevelHealth,MaxLevelHealth,MinLevelMana,MaxLevelMana,MinMeleeDmg,
        MaxMeleeDmg,MinRangedDmg,MaxRangedDmg,Armor,MeleeAttackPower,RangedAttackPower,
        MeleeBaseAttackTime,RangedBaseAttackTime,DamageSchool,MinLootGold,MaxLootGold,LootId,
        PickPocketLootId,SkinningLootId,KillCredit1,KillCredit2,MechanicImmuneMask,ResistanceHoly,
        ResistanceFire,ResistanceNature,ResistanceFrost,ResistanceShadow,ResistanceArcane,PetSpellDataId,
        MovementType,TrainerType,TrainerSpell,TrainerClass,TrainerRace,TrainerTemplateId,VendorTemplateId,
        EquipmentTemplateId,GossipMenuId,AIName,ScriptName,

        SIZE
    };

    typedef dbtmp::tuple_v<static_cast<size_t>(field_index::Entry)> primary_key_fields;

    //                     E   N     S     I     M   M   H   M1  M2  M3  M4  FA  FH  S     F   C   I   R    R
    typedef dbtmp::tuple_t<int,varchar<100>,varchar<100>,varchar<100>,int,int,int,int,int,int,int,int,int,float,int,int,int,bool,bool,
    //                     N   U   D   E   C   S     S     U   R   E   H     P     D     D     A     E     MH  MH
                           int,int,int,int,int,float,float,int,int,int,float,float,float,float,float,float,int,int,
    //                     MM  MM  MM    MM    MR    MR    A   MA  RA  MB  RB  D   MG  MG  L   PL  SL  K1  K2  M   H
                           int,int,float,float,float,float,int,int,int,int,int,int,int,int,int,int,int,int,int,int,int,
    //                     F   N   F   S   A   PS  M   TT  TS  TC  TR  TT  VT  ET  GM  AI          S
                           int,int,int,int,int,int,int,int,int,int,int,int,int,int,int,varchar<64>,varchar<64>> field_types;

    static constexpr const dbtmp::sized_cstring field_name[] = { {"Entry"},{"Name"},{"SubName"},{"IconName"},{"MinLevel"},
                                                                 {"MaxLevel"},{"HeroicEntry"},{"ModelId1"},{"ModelId2"},
                                                                 {"ModelId3"},{"ModelId4"},{"FactionAlliance"},{"FactionHorde"},
                                                                 {"Scale"},{"Family"},{"CreatureType"},{"InhabitType"},
                                                                 {"RegenerateStats"},{"RacialLeader"},{"NpcFlags"},
                                                                 {"UnitFlags"},{"DynamicFlags"},{"ExtraFlags"},
                                                                 {"CreatureTypeFlags"},{"SpeedWalk"},{"SpeedRun"},{"UnitClass"},
                                                                 {"Rank"},{"Expansion"},{"HealthMultiplier"},{"PowerMultiplier"},
                                                                 {"DamageMultiplier"},{"DamageVariance"},{"ArmorMultiplier"},
                                                                 {"ExperienceMultiplier"},{"MinLevelHealth"},{"MaxLevelHealth"},
                                                                 {"MinLevelMana"},{"MaxLevelMana"},{"MinMeleeDmg"},
                                                                 {"MaxMeleeDmg"},{"MinRangedDmg"},{"MaxRangedDmg"},{"Armor"},
                                                                 {"MeleeAttackPower"},{"RangedAttackPower"},
                                                                 {"MeleeBaseAttackTime"},{"RangedBaseAttackTime"},
                                                                 {"DamageSchool"},{"MinLootGold"},{"MaxLootGold"},{"LootId"},
                                                                 {"PickpocketLootId"},{"SkinningLootId"},{"KillCredit1"},
                                                                 {"KillCredit2"},{"MechanicImmuneMask"},{"ResistanceHoly"},
                                                                 {"ResistanceFire"},{"ResistanceNature"},{"ResistanceFrost"},
                                                                 {"ResistanceShadow"},{"ResistanceArcane"},{"PetSpellDataId"},
                                                                 {"MovementType"},{"TrainerType"},{"TrainerSpell"},
                                                                 {"TrainerClass"},{"TrainerRace"},{"TrainerTemplateId"},
                                                                 {"VendorTemplateId"},{"EquipmentTemplateId"},
                                                                 {"GossipMenuId"},{"AIName"},{"ScriptName"} };

    static constexpr const dbtmp::sized_cstring table_name = {"creature_template"};

}; // creature_template_tbc


#endif // CREATURE_TEMPLATE_H
