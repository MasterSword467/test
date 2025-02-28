/*
 * File: z_en_fw.c
 * Overlay: ovl_En_Fw
 * Description: Flare Dancer Core
 */

#include "z_en_fw.h"
#include "objects/object_fw/object_fw.h"
#include "overlays/actors/ovl_En_Bom/z_en_bom.h"
#include "objects/gameplay_keep/gameplay_keep.h"

#define FLAGS (ACTOR_FLAG_0 | ACTOR_FLAG_2 | ACTOR_FLAG_4 | ACTOR_FLAG_9)

void EnFw_Init(Actor* thisx, GlobalContext* globalCtx);
void EnFw_Destroy(Actor* thisx, GlobalContext* globalCtx);
void EnFw_Update(Actor* thisx, GlobalContext* globalCtx);
void EnFw_Draw(Actor* thisx, GlobalContext* globalCtx);
void EnFw_UpdateEffects(EnFw* this);
void EnFw_DrawEffects(EnFw* this, GlobalContext* globalCtx);
void EnFw_SpawnEffectDust(EnFw* this, Vec3f* initialPos, Vec3f* initialSpeed, Vec3f* accel, u8 initialTimer, f32 scale,
                          f32 scaleStep);
void EnFw_Bounce(EnFw* this, GlobalContext* globalCtx);
void EnFw_Run(EnFw* this, GlobalContext* globalCtx);
void EnFw_JumpToParentInitPos(EnFw* this, GlobalContext* globalCtx);
void EnFw_TurnToParentInitPos(EnFw* this, GlobalContext* globalCtx);

const ActorInit En_Fw_InitVars = {
    ACTOR_EN_FW,
    ACTORCAT_ENEMY,
    FLAGS,
    OBJECT_FW,
    sizeof(EnFw),
    (ActorFunc)EnFw_Init,
    (ActorFunc)EnFw_Destroy,
    (ActorFunc)EnFw_Update,
    (ActorFunc)EnFw_Draw,
};

static ColliderJntSphElementInit sJntSphElementsInit[1] = {
    {
        {
            ELEMTYPE_UNK0,
            { 0x00000000, 0x00, 0x04 },
            { 0xFFCFFFFE, 0x00, 0x00 },
            TOUCH_NONE,
            BUMP_ON | BUMP_HOOKABLE,
            OCELEM_ON,
        },
        { 2, { { 1200, 0, 0 }, 16 }, 100 },
    },
};

static ColliderJntSphInit sJntSphInit = {
    {
        COLTYPE_HIT6,
        AT_ON | AT_TYPE_ENEMY,
        AC_ON | AC_TYPE_PLAYER,
        OC1_ON | OC1_TYPE_ALL,
        OC2_TYPE_1,
        COLSHAPE_JNTSPH,
    },
    1,
    sJntSphElementsInit,
};

static CollisionCheckInfoInit2 D_80A1FB94 = { 8, 2, 25, 25, MASS_IMMOVABLE };

typedef enum {
    /* 0 */ ENFW_ANIM_0,
    /* 1 */ ENFW_ANIM_1,
    /* 2 */ ENFW_ANIM_2
} EnFwAnimation;

static AnimationInfo sAnimationInfo[] = {
    { &gFlareDancerCoreInitRunCycleAnim, 0.0f, 0.0f, -1.0f, ANIMMODE_ONCE_INTERP, 0.0f },
    { &gFlareDancerCoreRunCycleAnim, 1.0f, 0.0f, -1.0f, ANIMMODE_ONCE_INTERP, -8.0f },
    { &gFlareDancerCoreEndRunCycleAnim, 1.0f, 0.0f, -1.0f, ANIMMODE_LOOP_INTERP, -8.0f },
};

s32 EnFw_DoBounce(EnFw* this, s32 totalBounces, f32 yVelocity) {
    if (!(this->actor.bgCheckFlags & BGCHECKFLAG_GROUND) || (this->actor.velocity.y > 0.0f)) {
        // not on the ground or moving upwards.
        return false;
    }

    Audio_PlayActorSound2(&this->actor, NA_SE_EN_DODO_M_GND);
    this->bounceCnt--;
    if (this->bounceCnt <= 0) {
        if (this->bounceCnt == 0) {
            this->bounceCnt = 0;
            this->actor.velocity.y = 0.0f;
            return true;
        }
        this->bounceCnt = totalBounces;
    }
    this->actor.velocity.y = yVelocity;
    this->actor.velocity.y *= (f32)this->bounceCnt / totalBounces;
    return true;
}

s32 EnFw_PlayerInRange(EnFw* this, GlobalContext* globalCtx) {
    Player* player = GET_PLAYER(globalCtx);
    CollisionPoly* poly;
    s32 bgId;
    Vec3f collisionPos;

    if (this->actor.xzDistToPlayer > 300.0f) {
        return false;
    }

    if (ABS((s16)((f32)this->actor.yawTowardsPlayer - (f32)this->actor.shape.rot.y)) > 0x1C70) {
        return false;
    }

    if (BgCheck_EntityLineTest1(&globalCtx->colCtx, &this->actor.world.pos, &player->actor.world.pos, &collisionPos,
                                &poly, true, false, false, true, &bgId)) {
        return false;
    }

    return true;
}

Vec3f* EnFw_GetPosAdjAroundCircle(Vec3f* dst, EnFw* this, f32 radius, s16 dir) {
    s16 angle;
    Vec3f posAdj;

    // increase rotation around circle ~30 degrees.
    angle = Math_Vec3f_Yaw(&this->actor.parent->home.pos, &this->actor.world.pos) + (dir * 0x1554);
    posAdj.x = (Math_SinS(angle) * radius) + this->actor.parent->home.pos.x;
    posAdj.z = (Math_CosS(angle) * radius) + this->actor.parent->home.pos.z;
    posAdj.x -= this->actor.world.pos.x;
    posAdj.z -= this->actor.world.pos.z;
    *dst = posAdj;
    return dst;
}

s32 EnFw_CheckCollider(EnFw* this, GlobalContext* globalCtx) {
    ColliderInfo* info;

    if (this->collider.base.acFlags & AC_HIT) {
        info = &this->collider.elements[0].info;
        if (info->acHitInfo->toucher.dmgFlags & 0x80) {
            this->lastDmgHook = true;
        } else {
            this->lastDmgHook = false;
        }
        this->collider.base.acFlags &= ~AC_HIT;
        if (Actor_ApplyDamage(&this->actor) <= 0) {
            if (this->actor.parent->colChkInfo.health <= 8) {
                Enemy_StartFinishingBlow(globalCtx, &this->actor);
                this->actor.parent->colChkInfo.health = 0;
            } else {
                this->actor.parent->colChkInfo.health -= 8;
            }
            this->returnToParentTimer = 0;
        }
        return true;
    } else {
        return false;
    }
}

s32 EnFw_SpawnDust(EnFw* this, u8 timer, f32 scale, f32 scaleStep, s32 dustCnt, f32 radius, f32 xzAccel, f32 yAccel) {
    Vec3f pos = { 0.0f, 0.0f, 0.0f };
    Vec3f velocity = { 0.0f, 0.0f, 0.0f };
    Vec3f accel = { 0.0f, 0.0f, 0.0f };
    s16 angle;
    s32 i;

    pos = this->actor.world.pos;
    pos.y = this->actor.floorHeight + 2.0f;
    angle = ((Rand_ZeroOne() - 0.5f) * 0x10000);
    i = dustCnt;
    while (i >= 0) {
        accel.x = (Rand_ZeroOne() - 0.5f) * xzAccel;
        accel.y = yAccel;
        accel.z = (Rand_ZeroOne() - 0.5f) * xzAccel;
        pos.x = (Math_SinS(angle) * radius) + this->actor.world.pos.x;
        pos.z = (Math_CosS(angle) * radius) + this->actor.world.pos.z;
        EnFw_SpawnEffectDust(this, &pos, &velocity, &accel, timer, scale, scaleStep);
        angle += (s16)(0x10000 / dustCnt);
        i--;
    }
    return 0;
}

void EnFw_Init(Actor* thisx, GlobalContext* globalCtx) {
    EnFw* this = (EnFw*)thisx;

    SkelAnime_InitFlex(globalCtx, &this->skelAnime, &gFlareDancerCoreSkel, NULL, this->jointTable, this->morphTable,
                       11);
    Animation_ChangeByInfo(&this->skelAnime, sAnimationInfo, ENFW_ANIM_0);
    ActorShape_Init(&this->actor.shape, 0.0f, ActorShadow_DrawCircle, 20.0f);
    Collider_InitJntSph(globalCtx, &this->collider);
    Collider_SetJntSph(globalCtx, &this->collider, &this->actor, &sJntSphInit, this->sphs);
    CollisionCheck_SetInfo2(&this->actor.colChkInfo, DamageTable_Get(0x10), &D_80A1FB94);
    Actor_SetScale(&this->actor, 0.01f);
    this->runDirection = -this->actor.params;
    this->actionFunc = EnFw_Bounce;
    this->actor.gravity = -1.0f;
}

void EnFw_Destroy(Actor* thisx, GlobalContext* globalCtx) {
    EnFw* this = (EnFw*)thisx;

    Collider_DestroyJntSph(globalCtx, &this->collider);
}

void EnFw_Bounce(EnFw* this, GlobalContext* globalCtx) {
    if (EnFw_DoBounce(this, 3, 8.0f) && this->bounceCnt == 0) {
        this->returnToParentTimer = Rand_S16Offset(300, 150);
        this->actionFunc = EnFw_Run;
    }
}

void EnFw_Run(EnFw* this, GlobalContext* globalCtx) {
    f32 tmpAngle;
    s16 curFrame;
    f32 facingDir;
    EnBom* bomb;
    Actor* flareDancer;

    Math_SmoothStepToF(&this->skelAnime.playSpeed, 1.0f, 0.1f, 1.0f, 0.0f);
    if (this->skelAnime.animation == &gFlareDancerCoreInitRunCycleAnim) {
        if (Animation_OnFrame(&this->skelAnime, this->skelAnime.endFrame) == 0) {
            this->runRadius = Math_Vec3f_DistXYZ(&this->actor.world.pos, &this->actor.parent->world.pos);
            Animation_ChangeByInfo(&this->skelAnime, sAnimationInfo, ENFW_ANIM_2);
        }
        return;
    }

    if (this->damageTimer == 0 && this->explosionTimer == 0 && EnFw_CheckCollider(this, globalCtx)) {
        if (this->actor.parent->colChkInfo.health > 0) {
            if (!this->lastDmgHook) {
                this->actor.velocity.y = 6.0f;
            }
            Audio_PlayActorSound2(&this->actor, NA_SE_EN_FLAME_MAN_DAMAGE);
            this->damageTimer = 20;
        } else {
            Audio_PlayActorSound2(&this->actor, NA_SE_EN_FLAME_MAN_DAMAGE);
            this->explosionTimer = 6;
        }
        this->actor.speedXZ = 0.0f;
    }

    if (this->explosionTimer != 0) {
        this->skelAnime.playSpeed = 0.0f;
        Math_SmoothStepToF(&this->actor.scale.x, 0.024999999f, 0.08f, 0.6f, 0.0f);
        Actor_SetScale(&this->actor, this->actor.scale.x);
        if (this->actor.colorFilterTimer == 0) {
            Actor_SetColorFilter(&this->actor, 0x4000, 0xC8, 0, this->explosionTimer);
            this->explosionTimer--;
        }

        if (this->explosionTimer == 0) {
            bomb = (EnBom*)Actor_Spawn(&globalCtx->actorCtx, globalCtx, ACTOR_EN_BOM, this->bompPos.x, this->bompPos.y,
                                       this->bompPos.z, 0, 0, 0x600, 0);
            if (bomb != NULL) {
                bomb->timer = 0;
            }
            flareDancer = this->actor.parent;
            flareDancer->params |= 0x4000;
            Item_DropCollectibleRandom(globalCtx, NULL, &this->actor.world.pos, 0xA0);
            Actor_Kill(&this->actor);
            return;
        }
    } else {
        if (!(this->actor.bgCheckFlags & BGCHECKFLAG_GROUND) || this->actor.velocity.y > 0.0f) {
            Actor_SetColorFilter(&this->actor, 0x4000, 0xC8, 0, this->damageTimer);
            return;
        }
        DECR(this->damageTimer);
        if ((200.0f - this->runRadius) < 0.9f) {
            if (DECR(this->returnToParentTimer) == 0) {
                this->actor.speedXZ = 0.0f;
                this->actionFunc = EnFw_TurnToParentInitPos;
                return;
            }
        }

        // Run outwards until the radius of the run circle is 200
        Math_SmoothStepToF(&this->runRadius, 200.0f, 0.3f, 100.0f, 0.0f);

        if (this->turnAround) {
            Math_SmoothStepToF(&this->actor.speedXZ, 0.0f, 0.1f, 1.0f, 0.0f);
            tmpAngle = (s16)(this->actor.world.rot.y ^ 0x8000);
            facingDir = this->actor.shape.rot.y;
            tmpAngle = Math_SmoothStepToF(&facingDir, tmpAngle, 0.1f, 10000.0f, 0.0f);
            this->actor.shape.rot.y = facingDir;
            if (tmpAngle > 0x1554) {
                return;
            }
            this->turnAround = false;
        } else {
            Vec3f sp48;

            EnFw_GetPosAdjAroundCircle(&sp48, this, this->runRadius, this->runDirection);
            Math_SmoothStepToS(&this->actor.shape.rot.y, RAD_TO_BINANG(Math_FAtan2F(sp48.x, sp48.z)), 4, 0xFA0, 1);
        }

        this->actor.world.rot = this->actor.shape.rot;

        if (this->slideTimer == 0 && EnFw_PlayerInRange(this, globalCtx)) {
            Audio_PlayActorSound2(&this->actor, NA_SE_EN_FLAME_MAN_SURP);
            this->slideSfxTimer = 8;
            this->slideTimer = 8;
        }

        if (this->slideTimer != 0) {
            if (DECR(this->slideSfxTimer) == 0) {
                Audio_PlayActorSound2(&this->actor, NA_SE_EN_FLAME_MAN_SLIDE);
                this->slideSfxTimer = 4;
            }
            Math_SmoothStepToF(&this->actor.speedXZ, 0.0f, 0.1f, 1.0f, 0.0f);
            this->skelAnime.playSpeed = 0.0f;
            EnFw_SpawnDust(this, 8, 0.16f, 0.2f, 3, 8.0f, 20.0f, ((Rand_ZeroOne() - 0.5f) * 0.2f) + 0.3f);
            this->slideTimer--;
            if (this->slideTimer == 0) {
                this->turnAround = true;
                this->runDirection = -this->runDirection;
            }
        } else {
            Math_SmoothStepToF(&this->actor.speedXZ, 6.0f, 0.1f, 1.0f, 0.0f);
            curFrame = this->skelAnime.curFrame;
            if (curFrame == 1 || curFrame == 4) {
                Audio_PlayActorSound2(&this->actor, NA_SE_EN_FLAME_MAN_RUN);
                EnFw_SpawnDust(this, 8, 0.16f, 0.1f, 1, 0.0f, 20.0f, 0.0f);
            }
        }
    }
}

void EnFw_TurnToParentInitPos(EnFw* this, GlobalContext* globalCtx) {
    s16 angleToParentInit;

    angleToParentInit = Math_Vec3f_Yaw(&this->actor.world.pos, &this->actor.parent->home.pos);
    Math_SmoothStepToS(&this->actor.shape.rot.y, angleToParentInit, 4, 0xFA0, 1);
    if (ABS(angleToParentInit - this->actor.shape.rot.y) < 0x65) {
        // angle to parent init pos is ~0.5 degrees
        this->actor.world.rot = this->actor.shape.rot;
        this->actor.velocity.y = 14.0f;
        this->actor.home.pos = this->actor.world.pos;
        Audio_PlayActorSound2(&this->actor, NA_SE_EN_STAL_JUMP);
        Animation_ChangeByInfo(&this->skelAnime, sAnimationInfo, ENFW_ANIM_1);
        this->actionFunc = EnFw_JumpToParentInitPos;
    }
}

void EnFw_JumpToParentInitPos(EnFw* this, GlobalContext* globalCtx) {
    if ((this->actor.bgCheckFlags & BGCHECKFLAG_GROUND) && this->actor.velocity.y <= 0.0f) {
        this->actor.parent->params |= 0x8000;
        Actor_Kill(&this->actor);
    } else {
        Math_SmoothStepToF(&this->actor.world.pos.x, this->actor.parent->home.pos.x, 0.6f, 8.0f, 0.0f);
        Math_SmoothStepToF(&this->actor.world.pos.z, this->actor.parent->home.pos.z, 0.6f, 8.0f, 0.0f);
    }
}

void EnFw_Update(Actor* thisx, GlobalContext* globalCtx) {
    EnFw* this = (EnFw*)thisx;

    SkelAnime_Update(&this->skelAnime);
    if (!CHECK_FLAG_ALL(this->actor.flags, ACTOR_FLAG_13)) {
        // not attached to hookshot.
        Actor_MoveForward(&this->actor);
        Actor_UpdateBgCheckInfo(globalCtx, &this->actor, 10.0f, 20.0f, 0.0f,
                                UPDBGCHECKINFO_FLAG_0 | UPDBGCHECKINFO_FLAG_2);
        this->actionFunc(this, globalCtx);
        if (this->damageTimer == 0 && this->explosionTimer == 0 && this->actionFunc == EnFw_Run) {
            CollisionCheck_SetAC(globalCtx, &globalCtx->colChkCtx, &this->collider.base);
        }
        CollisionCheck_SetOC(globalCtx, &globalCtx->colChkCtx, &this->collider.base);
    }
}

s32 EnFw_OverrideLimbDraw(GlobalContext* globalContext, s32 limbIndex, Gfx** dList, Vec3f* pos, Vec3s* rot,
                          void* thisx) {
    return false;
}

void EnFw_PostLimbDraw(GlobalContext* globalCtx, s32 limbIndex, Gfx** dList, Vec3s* rot, void* thisx) {
    EnFw* this = (EnFw*)thisx;
    Vec3f zeroVec = { 0.0f, 0.0f, 0.0f };

    if (limbIndex == 2) {
        // body
        Matrix_MultVec3f(&zeroVec, &this->bompPos);
    }

    if (limbIndex == 3) {
        // head
        Matrix_MultVec3f(&zeroVec, &this->actor.focus.pos);
    }

    Collider_UpdateSpheres(limbIndex, &this->collider);
}

void EnFw_Draw(Actor* thisx, GlobalContext* globalCtx) {
    EnFw* this = (EnFw*)thisx;

    EnFw_UpdateEffects(this);
    Matrix_Push();
    EnFw_DrawEffects(this, globalCtx);
    Matrix_Pop();
    func_80093D18(globalCtx->state.gfxCtx);
    SkelAnime_DrawFlexOpa(globalCtx, this->skelAnime.skeleton, this->skelAnime.jointTable, this->skelAnime.dListCount,
                          EnFw_OverrideLimbDraw, EnFw_PostLimbDraw, this);
}

void EnFw_SpawnEffectDust(EnFw* this, Vec3f* initialPos, Vec3f* initialSpeed, Vec3f* accel, u8 initialTimer, f32 scale,
                          f32 scaleStep) {
    EnFwEffect* eff = this->effects;
    s16 i;

    for (i = 0; i < EN_FW_EFFECT_COUNT; i++, eff++) {
        if (eff->type != 1) {
            eff->scale = scale;
            eff->scaleStep = scaleStep;
            eff->initialTimer = eff->timer = initialTimer;
            eff->type = 1;
            eff->pos = *initialPos;
            eff->accel = *accel;
            eff->velocity = *initialSpeed;
            return;
        }
    }
}

void EnFw_UpdateEffects(EnFw* this) {
    EnFwEffect* eff = this->effects;
    s16 i;

    for (i = 0; i < EN_FW_EFFECT_COUNT; i++, eff++) {
        if (eff->type != 0) {
            if ((--eff->timer) == 0) {
                eff->type = 0;
            }
            eff->accel.x = (Rand_ZeroOne() * 0.4f) - 0.2f;
            eff->accel.z = (Rand_ZeroOne() * 0.4f) - 0.2f;
            eff->pos.x += eff->velocity.x;
            eff->pos.y += eff->velocity.y;
            eff->pos.z += eff->velocity.z;
            eff->velocity.x += eff->accel.x;
            eff->velocity.y += eff->accel.y;
            eff->velocity.z += eff->accel.z;
            eff->scale += eff->scaleStep;
        }
    }
}

void EnFw_DrawEffects(EnFw* this, GlobalContext* globalCtx) {
    static void* dustTextures[] = {
        gDust8Tex, gDust7Tex, gDust6Tex, gDust5Tex, gDust4Tex, gDust3Tex, gDust2Tex, gDust1Tex,
    };
    EnFwEffect* eff = this->effects;
    s16 materialFlag;
    s16 alpha;
    s16 i;
    s16 idx;

    OPEN_DISPS(globalCtx->state.gfxCtx, "../z_en_fw.c", 1191);

    materialFlag = false;
    func_80093D84(globalCtx->state.gfxCtx);
    if (1) {}

    for (i = 0; i < EN_FW_EFFECT_COUNT; i++, eff++) {
        if (eff->type != 0) {
            if (!materialFlag) {
                POLY_XLU_DISP = Gfx_CallSetupDL(POLY_XLU_DISP, 0U);
                gSPDisplayList(POLY_XLU_DISP++, gFlareDancerDL_7928);
                gDPSetEnvColor(POLY_XLU_DISP++, 100, 60, 20, 0);
                materialFlag = true;
            }

            alpha = eff->timer * (255.0f / eff->initialTimer);
            gDPSetPrimColor(POLY_XLU_DISP++, 0, 0, 170, 130, 90, alpha);
            gDPPipeSync(POLY_XLU_DISP++);
            Matrix_Translate(eff->pos.x, eff->pos.y, eff->pos.z, MTXMODE_NEW);
            Matrix_ReplaceRotation(&globalCtx->billboardMtxF);
            Matrix_Scale(eff->scale, eff->scale, 1.0f, MTXMODE_APPLY);
            gSPMatrix(POLY_XLU_DISP++, Matrix_NewMtx(globalCtx->state.gfxCtx, "../z_en_fw.c", 1229),
                      G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
            idx = eff->timer * (8.0f / eff->initialTimer);
            gSPSegment(POLY_XLU_DISP++, 0x8, SEGMENTED_TO_VIRTUAL(dustTextures[idx]));
            gSPDisplayList(POLY_XLU_DISP++, gFlareDancerSquareParticleDL);
        }
    }

    CLOSE_DISPS(globalCtx->state.gfxCtx, "../z_en_fw.c", 1243);
}
