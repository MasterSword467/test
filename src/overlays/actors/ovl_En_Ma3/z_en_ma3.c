/*
 * File: z_en_ma3.c
 * Overlay: En_Ma3
 * Description: Adult Malon (Ranch)
 */

#include "z_en_ma3.h"
#include "objects/object_ma2/object_ma2.h"

#define FLAGS (ACTOR_FLAG_0 | ACTOR_FLAG_3 | ACTOR_FLAG_4 | ACTOR_FLAG_5)

void EnMa3_Init(Actor* thisx, GlobalContext* globalCtx);
void EnMa3_Destroy(Actor* thisx, GlobalContext* globalCtx);
void EnMa3_Update(Actor* thisx, GlobalContext* globalCtx);
void EnMa3_Draw(Actor* thisx, GlobalContext* globalCtx);

u16 func_80AA2AA0(GlobalContext* globalCtx, Actor* this);
s16 func_80AA2BD4(GlobalContext* globalCtx, Actor* this);

void func_80AA2E54(EnMa3* this, GlobalContext* globalCtx);
s32 func_80AA2EC8(EnMa3* this, GlobalContext* globalCtx);
s32 func_80AA2F28(EnMa3* this);
void EnMa3_UpdateEyes(EnMa3* this);
void func_80AA3200(EnMa3* this, GlobalContext* globalCtx);

const ActorInit En_Ma3_InitVars = {
    ACTOR_EN_MA3,
    ACTORCAT_NPC,
    FLAGS,
    OBJECT_MA2,
    sizeof(EnMa3),
    (ActorFunc)EnMa3_Init,
    (ActorFunc)EnMa3_Destroy,
    (ActorFunc)EnMa3_Update,
    (ActorFunc)EnMa3_Draw,
};

static ColliderCylinderInit sCylinderInit = {
    {
        COLTYPE_NONE,
        AT_NONE,
        AC_NONE,
        OC1_ON | OC1_TYPE_ALL,
        OC2_TYPE_2,
        COLSHAPE_CYLINDER,
    },
    {
        ELEMTYPE_UNK0,
        { 0x00000000, 0x00, 0x00 },
        { 0x00000000, 0x00, 0x00 },
        TOUCH_NONE,
        BUMP_NONE,
        OCELEM_ON,
    },
    { 18, 46, 0, { 0, 0, 0 } },
};

static CollisionCheckInfoInit2 sColChkInfoInit = { 0, 0, 0, 0, MASS_IMMOVABLE };

typedef enum {
    /* 0 */ ENMA3_ANIM_0,
    /* 1 */ ENMA3_ANIM_1,
    /* 2 */ ENMA3_ANIM_2,
    /* 3 */ ENMA3_ANIM_3,
    /* 4 */ ENMA3_ANIM_4
} EnMa3Animation;

static AnimationFrameCountInfo sAnimationInfo[] = {
    { &gMalonAdultIdleAnim, 1.0f, ANIMMODE_LOOP, 0.0f },       { &gMalonAdultIdleAnim, 1.0f, ANIMMODE_LOOP, -10.0f },
    { &gMalonAdultStandStillAnim, 1.0f, ANIMMODE_LOOP, 0.0f }, { &gMalonAdultSingAnim, 1.0f, ANIMMODE_LOOP, 0.0f },
    { &gMalonAdultSingAnim, 1.0f, ANIMMODE_LOOP, -10.0f },
};

u16 func_80AA2AA0(GlobalContext* globalCtx, Actor* thisx) {
    Player* player = GET_PLAYER(globalCtx);
    s16* timer1ValuePtr; // weirdness with this necessary to match

    if (!GET_INFTABLE(INFTABLE_B8)) {
        return 0x2000;
    }
    timer1ValuePtr = &gSaveContext.timer1Value;
    if (GET_EVENTINF(EVENTINF_HORSES_0A)) {
        gSaveContext.timer1Value = gSaveContext.timer1Value;
        thisx->flags |= ACTOR_FLAG_16;
        if (gSaveContext.timer1Value >= 0xD3) {
            return 0x208E;
        }
        if ((HIGH_SCORE(HS_HORSE_RACE) == 0) || (HIGH_SCORE(HS_HORSE_RACE) >= 0xB4)) {
            HIGH_SCORE(HS_HORSE_RACE) = 0xB4;
            gSaveContext.timer1Value = *timer1ValuePtr;
        }
        if (!GET_EVENTCHKINF(EVENTCHKINF_1E) && (gSaveContext.timer1Value < 0x32)) {
            return 0x208F;
        } else if (gSaveContext.timer1Value < HIGH_SCORE(HS_HORSE_RACE)) {
            return 0x2012;
        } else {
            return 0x2004;
        }
    }
    if (!(player->stateFlags1 & PLAYER_STATE1_23) &&
        (Actor_FindNearby(globalCtx, thisx, ACTOR_EN_HORSE, 1, 1200.0f) == NULL)) {
        return 0x2001;
    }
    if (!GET_INFTABLE(INFTABLE_B9)) {
        return 0x2002;
    } else {
        return 0x2003;
    }
}

s16 func_80AA2BD4(GlobalContext* globalCtx, Actor* thisx) {
    s16 ret = 1;

    switch (Message_GetState(&globalCtx->msgCtx)) {
        case TEXT_STATE_EVENT:
            if (Message_ShouldAdvance(globalCtx)) {
                globalCtx->nextEntranceIndex = ENTR_SPOT20_0;
                gSaveContext.nextCutsceneIndex = 0xFFF0;
                globalCtx->transitionType = TRANS_TYPE_CIRCLE(TCA_STARBURST, TCC_BLACK, TCS_FAST);
                globalCtx->transitionTrigger = TRANS_TRIGGER_START;
                SET_EVENTINF(EVENTINF_HORSES_0A);
                gSaveContext.timer1State = 0xF;
            }
            break;
        case TEXT_STATE_CHOICE:
            if (Message_ShouldAdvance(globalCtx)) {
                SET_INFTABLE(INFTABLE_B9);
                if (globalCtx->msgCtx.choiceIndex == 0) {
                    if (GET_EVENTCHKINF(EVENTCHKINF_1E)) {
                        Message_ContinueTextbox(globalCtx, 0x2091);
                    } else if (HIGH_SCORE(HS_HORSE_RACE) == 0) {
                        Message_ContinueTextbox(globalCtx, 0x2092);
                    } else {
                        Message_ContinueTextbox(globalCtx, 0x2090);
                    }
                }
            }
            break;
        case TEXT_STATE_CLOSING:
            switch (thisx->textId) {
                case 0x2000:
                    SET_INFTABLE(INFTABLE_B8);
                    ret = 0;
                    break;
                case 0x208F:
                    SET_EVENTCHKINF(EVENTCHKINF_1E);
                case 0x2004:
                case 0x2012:
                    if (HIGH_SCORE(HS_HORSE_RACE) > gSaveContext.timer1Value) {
                        HIGH_SCORE(HS_HORSE_RACE) = gSaveContext.timer1Value;
                    }
                case 0x208E:
                    CLEAR_EVENTINF(EVENTINF_HORSES_0A);
                    thisx->flags &= ~ACTOR_FLAG_16;
                    ret = 0;
                    gSaveContext.timer1State = 0xA;
                    break;
                case 0x2002:
                    SET_INFTABLE(INFTABLE_B9);
                case 0x2003:
                    if (!GET_EVENTINF(EVENTINF_HORSES_0A)) {
                        ret = 0;
                    }
                    break;
                default:
                    ret = 0;
            }
            break;
        case TEXT_STATE_NONE:
        case TEXT_STATE_DONE_HAS_NEXT:
        case TEXT_STATE_DONE_FADING:
        case TEXT_STATE_DONE:
        case TEXT_STATE_SONG_DEMO_DONE:
        case TEXT_STATE_8:
        case TEXT_STATE_9:
            break;
    }
    return ret;
}

void func_80AA2E54(EnMa3* this, GlobalContext* globalCtx) {
    Player* player = GET_PLAYER(globalCtx);
    s16 phi_a3;

    if ((this->unk_1E0.unk_00 == 0) && (this->skelAnime.animation == &gMalonAdultSingAnim)) {
        phi_a3 = 1;
    } else {
        phi_a3 = 0;
    }

    this->unk_1E0.unk_18 = player->actor.world.pos;
    this->unk_1E0.unk_14 = 0.0f;
    func_80034A14(&this->actor, &this->unk_1E0, 0, phi_a3);
}

s32 func_80AA2EC8(EnMa3* this, GlobalContext* globalCtx) {
    if (LINK_IS_CHILD) {
        return 2;
    }
    if (!GET_EVENTCHKINF(EVENTCHKINF_18)) {
        return 2;
    }
    if (GET_EVENTINF(EVENTINF_HORSES_0A)) {
        return 1;
    }
    return 0;
}

s32 func_80AA2F28(EnMa3* this) {
    if (this->skelAnime.animation != &gMalonAdultSingAnim) {
        return 0;
    }
    if (this->unk_1E0.unk_00 != 0) {
        return 0;
    }
    this->blinkTimer = 0;
    if (this->eyeIndex != 2) {
        return 0;
    }
    this->mouthIndex = 2;
    return 1;
}

void EnMa3_UpdateEyes(EnMa3* this) {
    if ((!func_80AA2F28(this)) && (DECR(this->blinkTimer) == 0)) {
        this->eyeIndex += 1;
        if (this->eyeIndex >= 3) {
            this->blinkTimer = Rand_S16Offset(30, 30);
            this->eyeIndex = 0;
        }
    }
}

void EnMa3_ChangeAnim(EnMa3* this, s32 index) {
    f32 frameCount = Animation_GetLastFrame(sAnimationInfo[index].animation);

    Animation_Change(&this->skelAnime, sAnimationInfo[index].animation, 1.0f, 0.0f, frameCount,
                     sAnimationInfo[index].mode, sAnimationInfo[index].morphFrames);
}

void EnMa3_Init(Actor* thisx, GlobalContext* globalCtx) {
    EnMa3* this = (EnMa3*)thisx;
    s32 pad;

    ActorShape_Init(&this->actor.shape, 0.0f, ActorShadow_DrawCircle, 18.0f);
    SkelAnime_InitFlex(globalCtx, &this->skelAnime, &gMalonAdultSkel, NULL, NULL, NULL, 0);
    Collider_InitCylinder(globalCtx, &this->collider);
    Collider_SetCylinder(globalCtx, &this->collider, &this->actor, &sCylinderInit);
    CollisionCheck_SetInfo2(&this->actor.colChkInfo, DamageTable_Get(22), &sColChkInfoInit);

    switch (func_80AA2EC8(this, globalCtx)) {
        case 0:
            EnMa3_ChangeAnim(this, ENMA3_ANIM_0);
            this->actionFunc = func_80AA3200;
            break;
        case 1:
            EnMa3_ChangeAnim(this, ENMA3_ANIM_0);
            this->actionFunc = func_80AA3200;
            break;
        case 2:
            Actor_Kill(&this->actor);
            return;
    }

    Actor_UpdateBgCheckInfo(globalCtx, &this->actor, 0.0f, 0.0f, 0.0f, UPDBGCHECKINFO_FLAG_2);
    Actor_SetScale(&this->actor, 0.01f);
    this->unk_1E0.unk_00 = (u16)0;
}

void EnMa3_Destroy(Actor* thisx, GlobalContext* globalCtx) {
    EnMa3* this = (EnMa3*)thisx;

    SkelAnime_Free(&this->skelAnime, globalCtx);
    Collider_DestroyCylinder(globalCtx, &this->collider);
}

void func_80AA3200(EnMa3* this, GlobalContext* globalCtx) {
    if (this->unk_1E0.unk_00 == 2) {
        this->actor.flags &= ~ACTOR_FLAG_16;
        this->unk_1E0.unk_00 = 0;
    }
}

void EnMa3_Update(Actor* thisx, GlobalContext* globalCtx) {
    EnMa3* this = (EnMa3*)thisx;
    s32 pad;

    Collider_UpdateCylinder(&this->actor, &this->collider);
    CollisionCheck_SetOC(globalCtx, &globalCtx->colChkCtx, &this->collider.base);
    SkelAnime_Update(&this->skelAnime);
    EnMa3_UpdateEyes(this);
    this->actionFunc(this, globalCtx);
    func_80AA2E54(this, globalCtx);
    func_800343CC(globalCtx, &this->actor, &this->unk_1E0.unk_00, (f32)this->collider.dim.radius + 150.0f,
                  func_80AA2AA0, func_80AA2BD4);
    if (this->unk_1E0.unk_00 == 0) {
        if (this->unk_20A != 0) {
            func_800F6584(0);
            this->unk_20A = 0;
        }
    } else if (this->unk_20A == 0) {
        func_800F6584(1);
        this->unk_20A = 1;
    }
}

s32 EnMa3_OverrideLimbDraw(GlobalContext* globalCtx, s32 limbIndex, Gfx** dList, Vec3f* pos, Vec3s* rot, void* thisx) {
    EnMa3* this = (EnMa3*)thisx;
    Vec3s vec;

    if ((limbIndex == MALON_ADULT_LIMB_LEFT_THIGH) || (limbIndex == MALON_ADULT_LIMB_RIGHT_THIGH)) {
        *dList = NULL;
    }
    if (limbIndex == MALON_ADULT_LIMB_HEAD) {
        Matrix_Translate(1400.0f, 0.0f, 0.0f, MTXMODE_APPLY);
        vec = this->unk_1E0.unk_08;
        Matrix_RotateX(BINANG_TO_RAD_ALT(vec.y), MTXMODE_APPLY);
        Matrix_RotateZ(BINANG_TO_RAD_ALT(vec.x), MTXMODE_APPLY);
        Matrix_Translate(-1400.0f, 0.0f, 0.0f, MTXMODE_APPLY);
    }
    if (limbIndex == MALON_ADULT_LIMB_CHEST_AND_NECK) {
        vec = this->unk_1E0.unk_0E;
        Matrix_RotateY(BINANG_TO_RAD_ALT(-vec.y), MTXMODE_APPLY);
        Matrix_RotateX(BINANG_TO_RAD_ALT(-vec.x), MTXMODE_APPLY);
    }
    if ((limbIndex == MALON_ADULT_LIMB_CHEST_AND_NECK) || (limbIndex == MALON_ADULT_LIMB_LEFT_SHOULDER) ||
        (limbIndex == MALON_ADULT_LIMB_RIGHT_SHOULDER)) {
        rot->y += Math_SinS(this->unk_212[limbIndex].y) * 200.0f;
        rot->z += Math_CosS(this->unk_212[limbIndex].z) * 200.0f;
    }
    return false;
}

void EnMa3_PostLimbDraw(GlobalContext* globalCtx, s32 limbIndex, Gfx** dList, Vec3s* rot, void* thisx) {
    EnMa3* this = (EnMa3*)thisx;
    Vec3f vec = { 900.0f, 0.0f, 0.0f };

    OPEN_DISPS(globalCtx->state.gfxCtx, "../z_en_ma3.c", 927);

    if (limbIndex == MALON_ADULT_LIMB_HEAD) {
        Matrix_MultVec3f(&vec, &this->actor.focus.pos);
    }

    if ((limbIndex == MALON_ADULT_LIMB_LEFT_HAND) && (this->skelAnime.animation == &gMalonAdultStandStillAnim)) {
        gSPDisplayList(POLY_OPA_DISP++, gMalonAdultBasketDL);
    }

    CLOSE_DISPS(globalCtx->state.gfxCtx, "../z_en_ma3.c", 950);
}

void EnMa3_Draw(Actor* thisx, GlobalContext* globalCtx) {
    static void* sMouthTextures[] = { gMalonAdultMouthNeutralTex, gMalonAdultMouthSadTex, gMalonAdultMouthHappyTex };
    static void* sEyeTextures[] = { gMalonAdultEyeOpenTex, gMalonAdultEyeHalfTex, gMalonAdultEyeClosedTex };
    EnMa3* this = (EnMa3*)thisx;
    Camera* activeCam;
    f32 someFloat;
    s32 pad;

    OPEN_DISPS(globalCtx->state.gfxCtx, "../z_en_ma3.c", 978);

    activeCam = GET_ACTIVE_CAM(globalCtx);
    someFloat = Math_Vec3f_DistXZ(&this->actor.world.pos, &activeCam->eye);
    func_800F6268(someFloat, NA_BGM_LONLON);
    func_80093D18(globalCtx->state.gfxCtx);

    gSPSegment(POLY_OPA_DISP++, 0x09, SEGMENTED_TO_VIRTUAL(sMouthTextures[this->mouthIndex]));
    gSPSegment(POLY_OPA_DISP++, 0x08, SEGMENTED_TO_VIRTUAL(sEyeTextures[this->eyeIndex]));

    SkelAnime_DrawFlexOpa(globalCtx, this->skelAnime.skeleton, this->skelAnime.jointTable, this->skelAnime.dListCount,
                          EnMa3_OverrideLimbDraw, EnMa3_PostLimbDraw, this);

    CLOSE_DISPS(globalCtx->state.gfxCtx, "../z_en_ma3.c", 1013);
}
