#pragma once
#include "../../sdk/interfaces/iphysicssurfaceprops.h"
#include "../../sdk/entity.h"
#include "../../context.h"
#include "../../utils/math.h"

struct PenetrationData {
    CBasePlayer* target{};
    int penetrationCount{ 4 };
    float dmg{ -1.f };
    int hitgroup{ -1 };
    int hitbox{ -1 };
};

class CAutowall {
public:
    PenetrationData FireBullet(CBasePlayer* shooter, CBasePlayer* target,
        const CCSWeaponData* wpn_data, bool is_taser,
        Vector src, const Vector& dst, bool penetrate);

    void ClipTraceToPlayer(Vector end, Vector start, CGameTrace* oldtrace, CBasePlayer* ent);

    void ScaleDamage(CBasePlayer* player, float& damage, float armourRatio,
        int hitgroup, float headshotMultiplier);

    bool HandleBulletPenetration(CBasePlayer* shooter, const CCSWeaponData* wpn_data,
        const CGameTrace& enter_trace, Vector& src, const Vector& dir,
        int& pen_count, float& cur_dmg, float pen_modifier);

    bool TraceToExit(CBasePlayer* shooter, const Vector& src, const Vector& dir,
        const CGameTrace& enter_trace, CGameTrace& exit_trace);

    bool CanPenetrate();

private:
    bool TestHitboxes(CGameTrace& tr, Ray_t& ray, CBasePlayer* player, int force_hitgroup = -1);
};

namespace Features { inline CAutowall Autowall; }