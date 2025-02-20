/*
 * File: z_obj_comb.c
 * Overlay: ovl_Obj_Comb
 * Description: Beehive
 */

#include "z_obj_comb.h"
#include "overlays/effects/ovl_Effect_Ss_Kakera/z_eff_ss_kakera.h"
#include "objects/gameplay_field_keep/gameplay_field_keep.h"

#define FLAGS 0

void ObjComb_Init(Actor* thisx, GlobalContext* globalCtx);
void ObjComb_Destroy(Actor* thisx, GlobalContext* globalCtx);
void ObjComb_Update(Actor* thisx, GlobalContext* globalCtx);
void ObjComb_Draw(Actor* thisx, GlobalContext* globalCtx);

void ObjComb_Break(ObjComb* this, GlobalContext* globalCtx);
void ObjComb_ChooseItemDrop(ObjComb* this, GlobalContext* globalCtx);
void ObjComb_SetupWait(ObjComb* this);
void ObjComb_Wait(ObjComb* this, GlobalContext* globalCtx);

const ActorInit Obj_Comb_InitVars = {
    ACTOR_OBJ_COMB,
    ACTORCAT_PROP,
    FLAGS,
    OBJECT_GAMEPLAY_FIELD_KEEP,
    sizeof(ObjComb),
    (ActorFunc)ObjComb_Init,
    (ActorFunc)ObjComb_Destroy,
    (ActorFunc)ObjComb_Update,
    (ActorFunc)ObjComb_Draw,
};

static ColliderJntSphElementInit sJntSphElementsInit[1] = {
    {
        {
            ELEMTYPE_UNK0,
            { 0x00000000, 0x00, 0x00 },
            { 0x4001FFFE, 0x00, 0x00 },
            TOUCH_NONE,
            BUMP_ON,
            OCELEM_ON,
        },
        { 0, { { 0, 0, 0 }, 15 }, 100 },
    },
};

static ColliderJntSphInit sJntSphInit = {
    {
        COLTYPE_NONE,
        AT_NONE,
        AC_ON | AC_TYPE_PLAYER,
        OC1_ON | OC1_TYPE_PLAYER,
        OC2_TYPE_2,
        COLSHAPE_JNTSPH,
    },
    1,
    sJntSphElementsInit,
};

static InitChainEntry sInitChain[] = {
    ICHAIN_VEC3F_DIV1000(scale, 100, ICHAIN_CONTINUE),
    ICHAIN_F32(uncullZoneForward, 1100, ICHAIN_CONTINUE),
    ICHAIN_F32(uncullZoneScale, 100, ICHAIN_CONTINUE),
    ICHAIN_F32(uncullZoneDownward, 900, ICHAIN_STOP),
};

void ObjComb_Break(ObjComb* this, GlobalContext* globalCtx) {
    Vec3f pos1;
    Vec3f pos;
    Vec3f velocity;
    Gfx* dlist = gFieldBeehiveFragmentDL;
    s16 scale;
    s16 angle = 0;
    s16 gravity;
    u8 arg5;
    u8 arg6;
    f32 rand1;
    f32 rand2;
    s32 i;

    for (i = 0; i < 31; i++) {
        angle += 0x4E20;
        rand1 = Rand_ZeroOne() * 10.0f;

        pos1.x = Math_SinS(angle) * rand1;
        pos1.y = (i - 15) * 0.7f;
        pos1.z = Math_CosS(angle) * rand1;

        Math_Vec3f_Sum(&pos1, &this->actor.world.pos, &pos);

        velocity.x = (Rand_ZeroOne() - 0.5f) + pos1.x * 0.5f;
        velocity.y = (Rand_ZeroOne() - 0.5f) + pos1.y * 0.6f;
        velocity.z = (Rand_ZeroOne() - 0.5f) + pos1.z * 0.5f;

        scale = Rand_ZeroOne() * 72.0f + 25.0f;

        if (scale < 40) {
            gravity = -200;
            arg6 = 40;
        } else if (scale < 70) {
            gravity = -280;
            arg6 = 30;
        } else {
            gravity = -340;
            arg6 = 20;
        }

        rand2 = Rand_ZeroOne();

        if (rand2 < 0.1f) {
            arg5 = 96;
        } else if (rand2 < 0.8f) {
            arg5 = 64;
        } else {
            arg5 = 32;
        }

        EffectSsKakera_Spawn(globalCtx, &pos, &velocity, &pos, gravity, arg5, arg6, 4, 0, scale, 0, 0, 80,
                             KAKERA_COLOR_NONE, OBJECT_GAMEPLAY_FIELD_KEEP, dlist);
    }

    pos.x = this->actor.world.pos.x;
    pos.y = this->actor.world.pos.y - 10.0f;
    pos.z = this->actor.world.pos.z;
    func_80033480(globalCtx, &pos, 40.0f, 6, 70, 60, 1);
}

void ObjComb_ChooseItemDrop(ObjComb* this, GlobalContext* globalCtx) {
    s16 params = this->actor.params & 0x1F;

    if ((params > 0) || (params < ITEM00_MAX)) { // conditional always true. May have been intended to be &&
        if (params == ITEM00_HEART_PIECE) {
            if (Flags_GetCollectible(globalCtx, (this->actor.params >> 8) & 0x3F)) {
                params = -1;
            } else {
                params = (params | (((this->actor.params >> 8) & 0x3F) << 8));
            }
        } else if (Rand_ZeroOne() < 0.5f) {
            params = -1;
        }
        if (params >= 0) {
            Item_DropCollectible(globalCtx, &this->actor.world.pos, params);
        }
    }
}

void ObjComb_Init(Actor* thisx, GlobalContext* globalCtx) {
    ObjComb* this = (ObjComb*)thisx;

    Actor_ProcessInitChain(&this->actor, sInitChain);
    Collider_InitJntSph(globalCtx, &this->collider);
    Collider_SetJntSph(globalCtx, &this->collider, &this->actor, &sJntSphInit, this->colliderItems);
    ObjComb_SetupWait(this);
}

void ObjComb_Destroy(Actor* thisx, GlobalContext* globalCtx2) {
    GlobalContext* globalCtx = globalCtx2;
    ObjComb* this = (ObjComb*)thisx;

    Collider_DestroyJntSph(globalCtx, &this->collider);
}

void ObjComb_SetupWait(ObjComb* this) {
    this->actionFunc = ObjComb_Wait;
}

void ObjComb_Wait(ObjComb* this, GlobalContext* globalCtx) {
    s32 dmgFlags;

    this->unk_1B0 -= 50;
    if (this->unk_1B0 < 0) {
        this->unk_1B0 = 0;
    }

    if ((this->collider.base.acFlags & AC_HIT) != 0) {
        this->collider.base.acFlags &= ~AC_HIT;
        dmgFlags = this->collider.elements[0].info.acHitInfo->toucher.dmgFlags;
        if (dmgFlags & 0x4001F866) {
            this->unk_1B0 = 1500;
        } else {
            ObjComb_Break(this, globalCtx);
            ObjComb_ChooseItemDrop(this, globalCtx);
            Actor_Kill(&this->actor);
        }
    } else {
        CollisionCheck_SetAC(globalCtx, &globalCtx->colChkCtx, &this->collider.base);
    }

    if (this->actor.update != NULL) {
        CollisionCheck_SetOC(globalCtx, &globalCtx->colChkCtx, &this->collider.base);
    }
}

void ObjComb_Update(Actor* thisx, GlobalContext* globalCtx) {
    ObjComb* this = (ObjComb*)thisx;

    this->unk_1B2 += 0x2EE0;
    this->actionFunc(this, globalCtx);
    this->actor.shape.rot.x = Math_SinS(this->unk_1B2) * this->unk_1B0 + this->actor.home.rot.x;
}

void ObjComb_Draw(Actor* thisx, GlobalContext* globalCtx) {
    ObjComb* this = (ObjComb*)thisx;

    OPEN_DISPS(globalCtx->state.gfxCtx, "../z_obj_comb.c", 369);

    func_80093D18(globalCtx->state.gfxCtx);

    Matrix_Translate(this->actor.world.pos.x, this->actor.world.pos.y + (118.0f * this->actor.scale.y),
                     this->actor.world.pos.z, MTXMODE_NEW);
    Matrix_RotateY(BINANG_TO_RAD(this->actor.shape.rot.y), MTXMODE_APPLY);
    Matrix_RotateX(BINANG_TO_RAD(this->actor.shape.rot.x), MTXMODE_APPLY);
    Matrix_RotateZ(BINANG_TO_RAD(this->actor.shape.rot.z), MTXMODE_APPLY);
    Matrix_Translate(0, -(this->actor.scale.y * 118.0f), 0, MTXMODE_APPLY);
    Matrix_Scale(this->actor.scale.x, this->actor.scale.y, this->actor.scale.z, MTXMODE_APPLY);

    gSPMatrix(POLY_OPA_DISP++, Matrix_NewMtx(globalCtx->state.gfxCtx, "../z_obj_comb.c", 394),
              G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);

    gSPDisplayList(POLY_OPA_DISP++, gFieldBeehiveDL);

    Collider_UpdateSpheres(0, &this->collider);

    CLOSE_DISPS(globalCtx->state.gfxCtx, "../z_obj_comb.c", 402);
}
