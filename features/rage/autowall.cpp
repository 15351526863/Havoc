#include <algorithm>
#include <cstring>
#include "autowall.h"

// Link: https://developer.valvesoftware.com/wiki/CFuncBrush
// I even don't know what CFuncBrush is :)
static bool is_cfuncbrush(IClientEntity* entity)
{
    if (!entity || entity->Index() == 0)
        return false;

    auto cc = entity->GetClientClass();
    return cc && std::strstr(cc->szNetworkName, "CFuncBrush");
}

static bool is_breakable_entity(CBaseEntity* entity)
{
    if (!entity)
        return false;

    auto cc = entity->GetClientClass();

    if (entity->IsBreakable())
        return true;

    if (cc) {
        const char* name = cc->szNetworkName;
        if (!name)
            return false;

		// if entites can be breakable
        if (!std::strcmp(name, "CBaseDoor") ||
            !std::strcmp(name, "CBreakableSurface") ||
            (!std::strcmp(name, "CBaseEntity") &&
                entity->GetCollideable() &&
                entity->GetCollideable()->GetSolid() == SOLID_BSP) ||
            is_cfuncbrush(reinterpret_cast<IClientEntity*>(entity)))
            return true;
    }

    return false;
}

// @idea from fatality: but it is an extremely simplified version
bool CAutowall::TestHitboxes(CGameTrace& tr, Ray_t& ray, CBasePlayer* player, int force_hitgroup)
{
    if (!player)
        return false;

	// Do hitbox ray tracing, different from ClipTraceToPlayer function
    // @TODO: Maybe we can do 1:1 fatality's version?
    Interfaces::EngineTrace->ClipRayToEntity(ray, MASK_SHOT_HULL | CONTENTS_HITBOX, player, &tr);

    if (tr.flFraction == 1.f)
        return false;

    if (force_hitgroup != -1 && tr.iHitGroup != force_hitgroup)
        return false;

    return true;
}

PenetrationData CAutowall::FireBullet(
    CBasePlayer* shooter,
    CBasePlayer* target,
    const CCSWeaponData* weaponData,
    bool isTaser,
    Vector src,
    const Vector& dst,
    bool penetrate)
{
    const float penModGlobal = std::max((3.f / weaponData->flPenetration) * 1.25f, 0.f);
    PenetrationData data{};
    float maxDist = weaponData->flRange;
    float curDist{};
    data.dmg = static_cast<float>(weaponData->iDamage);

    CGameTrace trace{};
    CTraceFilterSkipTwoEntities filter{ shooter, nullptr };
    CBasePlayer* last{};
    Vector dir = dst - src;
    if (!dir.NormalizeInPlace())
        return {};

    while (data.dmg > 0.f) {
        float remain = maxDist - curDist;
        Vector end = src + dir * remain;
        filter.pSkip2 = last;
        Interfaces::EngineTrace->TraceRay({ src, end }, MASK_SHOT_PLAYER, &filter, &trace);

        if (target && filter.ShouldHitEntity(target, MASK_SHOT_PLAYER))
            ClipTraceToPlayer(end + dir * 40.f, src, &trace, target);

        if (trace.flFraction == 1.f)
            break;

        curDist += remain * trace.flFraction;
        data.dmg *= std::pow(weaponData->flRangeModifier, curDist * 0.002f);

        auto* hit = static_cast<CBasePlayer*>(trace.pHitEntity);
        if (hit && (hit == target || (!target && hit->IsPlayer() && !hit->IsTeammate(shooter)))) {
            Ray_t hitRay(src, trace.vecEnd);
            CGameTrace hitboxTrace{};

            // Do secondly check
            // Is it useless?
            if (!TestHitboxes(hitboxTrace, hitRay, hit, isTaser ? 0 : -1)) {
                last = hit;
                src = trace.vecEnd + dir * 4.f;
                continue;
            }

            data.target = hit;
            data.hitbox = hitboxTrace.iHitbox ? hitboxTrace.iHitbox : trace.iHitbox;
            data.hitgroup = isTaser ? 0 : (hitboxTrace.iHitGroup ? hitboxTrace.iHitGroup : trace.iHitGroup);
            ScaleDamage(hit, data.dmg, weaponData->flArmorRatio, data.hitgroup, 4.f);
            return data;
        }

        auto* enterSurf = Interfaces::PhysicsProps->GetSurfaceData(trace.surface.nSurfaceProps);
        if (!enterSurf || enterSurf->game.flPenetrationModifier < 0.1f)
            break;

        if (curDist > weaponData->flRange && weaponData->flPenetration > 0.f)
            break;

        last = trace.pHitEntity && trace.pHitEntity->IsPlayer() ? static_cast<CBasePlayer*>(trace.pHitEntity) : nullptr;
        if (!penetrate && !(trace.iContents & CONTENTS_WINDOW))
            break;

        if (isTaser || HandleBulletPenetration(shooter, weaponData, trace, src, dir,
            data.penetrationCount, data.dmg, penModGlobal))
            break;
    }
    return {};
}

void CAutowall::ScaleDamage(
    CBasePlayer* player,
    float& dmg,
    float armourRatio,
    int hitgroup,
    float hsMult)
{
    bool CT = player->m_iTeamNum() == TEAM_CT;
    bool heavy = player->m_bHasHeavyArmor();
    auto* headCvar = CT ? Displacement::Cvars.mp_damage_scale_ct_head
        : Displacement::Cvars.mp_damage_scale_t_head;
    float headScale = headCvar ? headCvar->GetFloat() : 1.f;
    
    if (heavy)
        headScale *= 0.5f;

    auto* bodyCvar = CT ? Displacement::Cvars.mp_damage_scale_ct_body
        : Displacement::Cvars.mp_damage_scale_t_body;
    float bodyScale = bodyCvar ? bodyCvar->GetFloat() : 1.f;

    switch (hitgroup) {
    case HITGROUP_HEAD: dmg *= headScale * hsMult; break;
    case HITGROUP_CHEST:
    case HITGROUP_LEFTARM:
    case HITGROUP_RIGHTARM:
    case HITGROUP_NECK: dmg *= bodyScale; break;
    case HITGROUP_STOMACH: dmg *= bodyScale * 1.25f; break;
    case HITGROUP_LEFTLEG:
    case HITGROUP_RIGHTLEG: dmg *= bodyScale * 0.75f; break;
    default: break;
    }

    auto armored = [heavy](CBasePlayer* p, int hg) {
        switch (hg) {
        case HITGROUP_HEAD: return p->m_bHasHelmet() || heavy;
        case HITGROUP_GENERIC:
        case HITGROUP_CHEST:
        case HITGROUP_STOMACH:
        case HITGROUP_LEFTARM:
        case HITGROUP_RIGHTARM:
        case HITGROUP_NECK: return true;
        default: return heavy;
        }
        };

    int armor = player->m_ArmorValue();
    if (armor > 0 && armored(player, hitgroup)) {
        float bonus = heavy ? 0.33f : 0.5f;
        float ratio = (heavy ? 0.25f : 0.5f) * armourRatio;
        float dmgToHealth = dmg * ratio;
        if ((dmg - dmgToHealth) * bonus > armor)
            dmgToHealth = dmg - armor / bonus;
        dmg = dmgToHealth;
    }
}

void CAutowall::ClipTraceToPlayer(Vector dst, Vector src, CGameTrace* old, CBasePlayer* ent)
{
    if (!ent)
        return;

    Vector pos = ent->GetAbsOrigin() + (ent->m_vecMins() + ent->m_vecMaxs()) * 0.5f;
    Vector to = pos - src;
    Vector dir = src - dst;
    float len = dir.NormalizeInPlace();
    float along = dir.DotProduct(to);
    float range = along < 0.f ? -to.Length() : along > len ? -(pos - dst).Length()
        : (pos - (src + dir * along)).Length();

    if (range > 60.f)
        return;

    CGameTrace tr{};
    Ray_t ray{ src, dst };
    Interfaces::EngineTrace->ClipRayToEntity(ray, MASK_SHOT_PLAYER, ent, &tr);

    if (tr.flFraction > old->flFraction)
        return;

    *old = tr;
}

bool CAutowall::TraceToExit(
    CBasePlayer* shooter,
    const Vector& start,
    const Vector& dir,
    const CGameTrace& enter,
    CGameTrace& exit)
{
    float dist{};
    int startContents{};
    CTraceFilterSkipTwoEntities filter{ shooter, nullptr };

    while (dist <= 90.f) {
        dist += 4.f;
        Vector end = start + dir * dist;
        Vector back = end - dir * 4.f;
        int cur = Interfaces::EngineTrace->GetPointContents(end, MASK_SHOT_PLAYER);

        if (!startContents)
            startContents = cur;

        if (cur & MASK_SHOT_HULL && (!(cur & CONTENTS_HITBOX) || cur == startContents))
            continue;

        if (!(cur & CS_MASK_SHOOT) || ((cur & CONTENTS_HITBOX) && startContents != cur)) {
            Interfaces::EngineTrace->TraceRay({ end, back }, MASK_SHOT_PLAYER,
                reinterpret_cast<ITraceFilter*>(&filter), &exit);

            if (exit.bStartSolid && (exit.surface.uFlags & SURF_HITBOX)) {
                filter.pSkip2 = exit.pHitEntity;
                Interfaces::EngineTrace->TraceRay({ end, start }, MASK_SHOT_PLAYER,
                    reinterpret_cast<ITraceFilter*>(&filter), &exit);
                if (exit.DidHit() && !exit.bStartSolid)
                    return true;
            }
            else if (exit.DidHit() && !exit.bStartSolid) {
                bool enterNoDraw = enter.surface.uFlags & SURF_NODRAW;
                bool exitNoDraw = exit.surface.uFlags & SURF_NODRAW;
                if (exitNoDraw &&
                    is_breakable_entity(static_cast<CBaseEntity*>(exit.pHitEntity)) &&
                    is_breakable_entity(static_cast<CBaseEntity*>(enter.pHitEntity)))
                    return true;
                if (!exitNoDraw || (enterNoDraw && exitNoDraw))
                    return true;
            }
            else if (enter.pHitEntity && enter.pHitEntity->Index() &&
                is_breakable_entity(static_cast<CBaseEntity*>(enter.pHitEntity))) {
                exit = enter;
                exit.vecEnd = start + dir;
                return true;
            }
        }
    }
    return false;
}

bool CAutowall::HandleBulletPenetration(
    CBasePlayer* shooter,
    const CCSWeaponData* weaponData,
    const CGameTrace& enter,
    Vector& src,
    const Vector& dir,
    int& penCount,
    float& curDmg,
    float penModIn)
{
    if (penCount <= 0 || weaponData->flPenetration <= 0.f)
        return true;

    CGameTrace exit{};
    if (!TraceToExit(shooter, enter.vecEnd, dir, enter, exit) &&
        !(Interfaces::EngineTrace->GetPointContents(enter.vecEnd, MASK_SHOT_HULL) & MASK_SHOT_HULL))
        return true;

    bool grate = enter.iContents & CONTENTS_GRATE;
    bool noDraw = enter.surface.uFlags & SURF_NODRAW;
    auto* exitSurf = Interfaces::PhysicsProps->GetSurfaceData(exit.surface.nSurfaceProps);
    auto* enterSurf = Interfaces::PhysicsProps->GetSurfaceData(enter.surface.nSurfaceProps);

    if (!exitSurf || !enterSurf)
        return true;

    float penMod = enterSurf->game.flPenetrationModifier;
    float dmgMod = enterSurf->game.flDamageModifier;
    int enterMat = enterSurf->game.hMaterial;
    int exitMat = exitSurf->game.hMaterial;

    if (Displacement::Cvars.sv_penetration_type->GetInt() != 1) {
        if (grate || noDraw) {
            penMod = 1.f;
            dmgMod = 0.99f;
        }
        else {
            penMod = std::min(penMod, exitSurf->game.flPenetrationModifier);
            dmgMod = std::min(dmgMod, exitSurf->game.flDamageModifier);
        }

        if (enterMat == exitMat && (exitMat == 87 || exitMat == 77))
            penMod *= 2.f;

        if ((exit.vecEnd - enter.vecEnd).Length() > weaponData->flPenetration * penMod)
            return true;

        curDmg *= dmgMod;
    }
    else {
        float loss = 0.16f;
        if (grate || noDraw || enterMat == CHAR_TEX_GLASS || enterMat == CHAR_TEX_GRATE) {
            if (enterMat == CHAR_TEX_GLASS || enterMat == CHAR_TEX_GRATE) {
                penMod = 3.f;
                loss = 0.05f;
            }
            else {
                penMod = 1.f;
            }
        }
        else if (enterMat == CHAR_TEX_FLESH &&
            !Displacement::Cvars.ff_damage_reduction_bullets->GetFloat() &&
            enter.pHitEntity &&
            enter.pHitEntity->IsPlayer() &&
            !static_cast<CBasePlayer*>(enter.pHitEntity)->IsTeammate(shooter)) {
            float ff = Displacement::Cvars.ff_damage_reduction_bullets->GetFloat();
            if (!ff)
                return true;
            penMod = ff;
        }
        else {
            penMod = (penMod + exitSurf->game.flPenetrationModifier) * 0.5f;
        }

        if (enterMat == exitMat) {
            if (exitMat == CHAR_TEX_WOOD || exitMat == CHAR_TEX_CARDBOARD)
                penMod = 3.f;
            else if (exitMat == CHAR_TEX_PLASTIC)
                penMod = 2.f;
        }

        float dist = (exit.vecEnd - enter.vecEnd).Length();
        float frac = std::max(0.f, 1.f / penMod);
        float wepLoss = curDmg * loss + penModIn * frac * 3.f;
        float total = wepLoss + dist * dist * frac / 24.f;
        curDmg -= std::max(0.f, total);

        if (curDmg < 1.f)
            return true;
    }

    --penCount;
    src = exit.vecEnd + dir * 4.f;
    return false;
}

//a.k.a can hit
bool CAutowall::CanPenetrate()
{
    if (ctx.m_pLocal->IsDead())
        return false;

    if (!ctx.m_pWeapon || ctx.m_pWeapon->IsKnife() || ctx.m_pWeapon->IsGrenade() || !ctx.m_pWeaponData)
        return false;

    Vector dir;
    Math::AngleVectors(ctx.m_angOriginalViewangles, &dir);
    CGameTrace trace{};
    CTraceFilter filter{ ctx.m_pLocal };
    Vector eye = ctx.m_vecEyePos;
    Interfaces::EngineTrace->TraceRay({ eye, eye + dir * ctx.m_pWeaponData->flRange },
        MASK_SHOT_HULL, reinterpret_cast<ITraceFilter*>(&filter), &trace);

    if (trace.flFraction == 1.f)
        return false;

    int pen = 1;
    float dmg = static_cast<float>(ctx.m_pWeaponData->iDamage);
    float penMod = std::max((3.f / ctx.m_pWeaponData->flPenetration) * 1.25f, 0.f);
    bool can = !HandleBulletPenetration(ctx.m_pLocal, ctx.m_pWeaponData,
        trace, eye, dir, pen, dmg, penMod);
    ctx.m_iPenetrationDamage = static_cast<int>(dmg);
    return can;
}