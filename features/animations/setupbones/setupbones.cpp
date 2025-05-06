#include "../animation.h"
#include "bonesetup.h"

class CBoneMergeCache
{
public:
    void* operator new(size_t size)
    {
        CBoneMergeCache* ptr = (CBoneMergeCache*)Interfaces::MemAlloc->Alloc(sizeof(CBoneMergeCache));
        Construct(ptr);
        return ptr;
    }
    void operator delete(void* ptr) { Interfaces::MemAlloc->Free(ptr); }
    void Init(CBasePlayer* pOwner) { reinterpret_cast<void(__thiscall*)(void*, CBasePlayer*)>(Displacement::Sigs.CBoneMergeCache__Init)(this, pOwner); }
    static void Construct(CBoneMergeCache* bmc) { reinterpret_cast<void(__thiscall*)(void*)>(Displacement::Sigs.CBoneMergeCache__Construct)(bmc); }
    void MergeMatchingPoseParams() { reinterpret_cast<void(__thiscall*)(void*)>(Displacement::Sigs.CBoneMergeCache__MergeMatchingPoseParams)(this); }
    void CopyFromFollow(Vector* followPos, Quaternion* followQ, int boneMask, Vector* myPos, Quaternion* myQ) { reinterpret_cast<void(__thiscall*)(void*, Vector*, Quaternion*, int, Vector*, Quaternion*)>(Displacement::Sigs.CBoneMergeCache__CopyFromFollow)(this, followPos, followQ, boneMask, myPos, myQ); }
    void CopyToFollow(Vector* myPos, Quaternion* myQ, int boneMask, Vector* followPos, Quaternion* followQ) { reinterpret_cast<void(__thiscall*)(void*, Vector*, Quaternion*, int, Vector*, Quaternion*)>(Displacement::Sigs.CBoneMergeCache__CopyToFollow)(this, myPos, myQ, boneMask, followPos, followQ); }
    uint8_t pad1[0xA0];
    unsigned short m_iRawIndexMapping[256];
    char pad2[0x4];
};

static int GetNumSeq(CStudioHdr* hdr)
{
    return hdr->pVirtualModel ? hdr->pVirtualModel->vecSequence.Count() : hdr->pStudioHdr->nLocalSequences;
}

void CAnimationSys::GetSkeleton(CBasePlayer* player, CStudioHdr* hdr, Vector* pos, Quaternion* q, int boneMask, CIKContext* ik)
{
    CBoneSetup boneSetup{ hdr, boneMask, player->m_flPoseParameter().data() };
    boneSetup.InitPose(pos, q);
    boneSetup.AccumulatePose(pos, q, player->m_nSequence(), player->m_flCycle(), 1.f, Interfaces::Globals->flCurTime, ik);
    constexpr int MAX_OVERLAYS = 15;
    int layer[MAX_OVERLAYS] = {};
    for (int i = 0; i < player->m_iAnimationLayersCount(); i++)
        layer[i] = MAX_OVERLAYS;
    for (int i = 0; i < player->m_iAnimationLayersCount(); i++)
    {
        CAnimationLayer& pLayer{ player->m_AnimationLayers()[i] };
        if (pLayer.flWeight > 0 && pLayer.IsActive() && pLayer.iOrder >= 0 && pLayer.iOrder < player->m_iAnimationLayersCount())
            layer[pLayer.iOrder] = i;
    }
    CWeaponCSBase* weapon{};
    CBasePlayer* weaponWorldModel{};
    bool doWeaponSetup{};
    if (player->m_bUseNewAnimstate())
    {
        weapon = player->GetWeapon();
        if (weapon)
        {
            weaponWorldModel = (CBasePlayer*)Interfaces::ClientEntityList->GetClientEntityFromHandle(weapon->m_hWeaponWorldModel());
            if (weaponWorldModel && weaponWorldModel->m_pStudioHdr())
            {
                if (!weaponWorldModel->m_pBoneMergeCache())
                {
                    weaponWorldModel->m_pBoneMergeCache() = new CBoneMergeCache();
                    weaponWorldModel->m_pBoneMergeCache()->Init(weaponWorldModel);
                }
                if (weaponWorldModel->m_pBoneMergeCache())
                    doWeaponSetup = true;
            }
        }
    }
    if (doWeaponSetup)
    {
        CStudioHdr* weaponStudioHdr{ weaponWorldModel->m_pStudioHdr() };
        weaponWorldModel->m_pBoneMergeCache()->MergeMatchingPoseParams();
        auto weaponIK = new CIKContext;
        weaponIK->Init(weaponStudioHdr, player->GetAbsAngles(), player->GetAbsOrigin(), Interfaces::Globals->flCurTime, 0, BONE_USED_BY_BONE_MERGE);
        CBoneSetup weaponSetup(weaponStudioHdr, BONE_USED_BY_BONE_MERGE, weaponWorldModel->m_flPoseParameter().data());
        alignas(16) Vector weaponPos[256];
        alignas(16) Quaternion weaponQ[256];
        weaponSetup.InitPose(weaponPos, weaponQ);
        for (int i = 0; i < player->m_iAnimationLayersCount(); i++)
        {
            auto pLayer = &player->m_AnimationLayers()[i];
            if (pLayer->nSequence <= 1 || pLayer->flWeight <= 0.f)
                continue;
            player->UpdateDispatchLayer(pLayer, weaponStudioHdr, pLayer->nSequence);
            if (pLayer->nDispatchedDst > 0 && pLayer->nDispatchedDst < GetNumSeq(weaponStudioHdr))
            {
                weaponWorldModel->m_pBoneMergeCache()->CopyFromFollow(pos, q, BONE_USED_BY_BONE_MERGE, weaponPos, weaponQ);
                mstudioseqdesc_t& seqdesc{ hdr->pStudioHdr->GetSequenceDescription(pLayer->nSequence) };
                ik->AddDependencies(seqdesc, pLayer->nSequence, pLayer->flCycle, player->m_flPoseParameter().data(), pLayer->flWeight);
                weaponSetup.AccumulatePose(weaponPos, weaponQ, pLayer->nDispatchedDst, pLayer->flCycle, pLayer->flWeight, Interfaces::Globals->flCurTime, weaponIK);
                weaponWorldModel->m_pBoneMergeCache()->CopyToFollow(weaponPos, weaponQ, BONE_USED_BY_BONE_MERGE, pos, q);
                weaponIK->CopyTo(ik, weaponWorldModel->m_pBoneMergeCache()->m_iRawIndexMapping);
            }
            else
            {
                boneSetup.AccumulatePose(pos, q, pLayer->nSequence, pLayer->flCycle, pLayer->flWeight, Interfaces::Globals->flCurTime, player->m_pIk());
            }
        }
        delete weaponIK;
    }
    else
    {
        for (int i = 0; i < player->m_iAnimationLayersCount(); i++)
        {
            if (layer[i] >= 0 && layer[i] < player->m_iAnimationLayersCount())
            {
                CAnimationLayer& pLayer = player->m_AnimationLayers()[layer[i]];
                boneSetup.AccumulatePose(pos, q, pLayer.nSequence, pLayer.flCycle, pLayer.flWeight, Interfaces::Globals->flCurTime, player->m_pIk());
            }
        }
    }
    auto autoIk = new CIKContext;
    autoIk->Init(hdr, player->GetAbsAngles(), player->GetAbsOrigin(), Interfaces::Globals->flCurTime, 0, boneMask);
    boneSetup.CalcAutoplaySequences(pos, q, Interfaces::Globals->flCurTime, autoIk);
    boneSetup.CalcBoneAdj(pos, q, player->m_flEncodedController());
    delete autoIk;
}

static inline void QuaternionMatrix(const Quaternion& q, matrix3x4_t& m)
{
    float xx = q.x * q.x, yy = q.y * q.y, zz = q.z * q.z, xy = q.x * q.y, xz = q.x * q.z, yz = q.y * q.z, wx = q.w * q.x, wy = q.w * q.y, wz = q.w * q.z;
    m[0][0] = 1.f - 2.f * (yy + zz);
    m[0][1] = 2.f * (xy - wz);
    m[0][2] = 2.f * (xz + wy);
    m[1][0] = 2.f * (xy + wz);
    m[1][1] = 1.f - 2.f * (xx + zz);
    m[1][2] = 2.f * (yz - wx);
    m[2][0] = 2.f * (xz - wy);
    m[2][1] = 2.f * (yz + wx);
    m[2][2] = 1.f - 2.f * (xx + yy);
    m[0][3] = m[1][3] = m[2][3] = 0.f;
}

static inline void QuaternionMatrix(const Quaternion& q, const Vector& pos, matrix3x4_t& m)
{
    QuaternionMatrix(q, m);
    m[0][3] = pos.x;
    m[1][3] = pos.y;
    m[2][3] = pos.z;
}

static inline void ConcatTransforms(const matrix3x4_t& m0, const matrix3x4_t& m1, matrix3x4_t& out)
{
    for (int i = 0; i < 3; i++)
    {
        out[i][0] = m0[i][0] * m1[0][0] + m0[i][1] * m1[1][0] + m0[i][2] * m1[2][0];
        out[i][1] = m0[i][0] * m1[0][1] + m0[i][1] * m1[1][1] + m0[i][2] * m1[2][1];
        out[i][2] = m0[i][0] * m1[0][2] + m0[i][1] * m1[1][2] + m0[i][2] * m1[2][2];
        out[i][3] = m1[0][3] * m0[i][0] + m1[1][3] * m0[i][1] + m1[2][3] * m0[i][2] + m0[i][3];
    }
}

void CAnimationSys::BuildMatrices(CBasePlayer* player, CStudioHdr* hdr, Vector* pos, Quaternion* q, matrix3x4a_t* bones, int boneMask, uint8_t[])
{
    int chain[256];
    int chain_length = hdr->pStudioHdr->nBones;
    for (int i = 0; i < chain_length; i++)
        chain[chain_length - i - 1] = i;
    matrix3x4a_t rot;
    rot.SetAngles(player->GetAbsAngles().y);
    rot.SetOrigin(player->GetAbsOrigin());
    for (int j = chain_length - 1; j >= 0; j--)
    {
        int i = chain[j];
        if (!(hdr->vecBoneFlags[i] & boneMask))
            continue;
        matrix3x4_t boneMatrix;
        QuaternionMatrix(q[i], pos[i], boneMatrix);
        if (hdr->vecBoneParent[i] == -1)
            ConcatTransforms(rot, boneMatrix, bones[i]);
        else
            ConcatTransforms(bones[hdr->vecBoneParent[i]], boneMatrix, bones[i]);
    }
}
