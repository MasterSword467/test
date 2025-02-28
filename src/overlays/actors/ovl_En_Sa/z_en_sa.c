#include "z_en_sa.h"
#include "overlays/actors/ovl_En_Elf/z_en_elf.h"
#include "objects/object_sa/object_sa.h"
#include "scenes/overworld/spot04/spot04_scene.h"
#include "scenes/overworld/spot05/spot05_scene.h"

#define FLAGS (ACTOR_FLAG_0 | ACTOR_FLAG_3 | ACTOR_FLAG_4 | ACTOR_FLAG_25)

void EnSa_Init(Actor* thisx, GlobalContext* globalCtx);
void EnSa_Destroy(Actor* thisx, GlobalContext* globalCtx);
void EnSa_Update(Actor* thisx, GlobalContext* globalCtx);
void EnSa_Draw(Actor* thisx, GlobalContext* globalCtx);

void func_80AF6448(EnSa* this, GlobalContext* globalCtx);
void func_80AF67D0(EnSa* this, GlobalContext* globalCtx);
void func_80AF683C(EnSa* this, GlobalContext* globalCtx);
void func_80AF68E4(EnSa* this, GlobalContext* globalCtx);
void func_80AF6B20(EnSa* this, GlobalContext* globalCtx);

typedef enum {
    /* 0 */ SARIA_EYE_OPEN,
    /* 1 */ SARIA_EYE_HALF,
    /* 2 */ SARIA_EYE_CLOSED,
    /* 3 */ SARIA_EYE_SUPRISED,
    /* 4 */ SARIA_EYE_SAD
} SariaEyeState;

typedef enum {
    /* 0 */ SARIA_MOUTH_CLOSED2,
    /* 1 */ SARIA_MOUTH_SUPRISED,
    /* 2 */ SARIA_MOUTH_CLOSED,
    /* 3 */ SARIA_MOUTH_SMILING_OPEN,
    /* 4 */ SARIA_MOUTH_FROWNING
} SariaMouthState;

const ActorInit En_Sa_InitVars = {
    ACTOR_EN_SA,
    ACTORCAT_NPC,
    FLAGS,
    OBJECT_SA,
    sizeof(EnSa),
    (ActorFunc)EnSa_Init,
    (ActorFunc)EnSa_Destroy,
    (ActorFunc)EnSa_Update,
    (ActorFunc)EnSa_Draw,
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
    { 20, 46, 0, { 0, 0, 0 } },
};

static CollisionCheckInfoInit2 sColChkInfoInit = {
    0, 0, 0, 0, MASS_IMMOVABLE,
};

typedef enum {
    /*  0 */ ENSA_ANIM1_0,
    /*  1 */ ENSA_ANIM1_1,
    /*  2 */ ENSA_ANIM1_2,
    /*  3 */ ENSA_ANIM1_3,
    /*  4 */ ENSA_ANIM1_4,
    /*  5 */ ENSA_ANIM1_5,
    /*  6 */ ENSA_ANIM1_6,
    /*  7 */ ENSA_ANIM1_7,
    /*  8 */ ENSA_ANIM1_8,
    /*  9 */ ENSA_ANIM1_9,
    /* 10 */ ENSA_ANIM1_10,
    /* 11 */ ENSA_ANIM1_11
} EnSaAnimation1;

static AnimationFrameCountInfo sAnimationInfo1[] = {
    { &gSariaWaitArmsToSideAnim, 1.0f, ANIMMODE_LOOP, 0.0f },
    { &gSariaLookUpArmExtendedAnim, 1.0f, ANIMMODE_ONCE, -10.0f },
    { &gSariaWaveAnim, 1.0f, ANIMMODE_LOOP, -10.0f },
    { &gSariaRunAnim, 1.0f, ANIMMODE_LOOP, -10.0f },
    { &gSariaWaitArmsToSideAnim, 1.0f, ANIMMODE_LOOP, -10.0f },
    { &gSariaLookOverShoulderAnim, 1.0f, ANIMMODE_LOOP, -10.0f },
    { &gSariaPlayingOcarinaAnim, 1.0f, ANIMMODE_LOOP, -10.0f },
    { &gSariaStopPlayingOcarinaAnim, 1.0f, ANIMMODE_ONCE, -10.0f },
    { &gSariaOcarinaToMouthAnim, 1.0f, ANIMMODE_ONCE, -10.0f },
    { &gSariaLinkLearnedSariasSongAnim, 1.0f, ANIMMODE_ONCE, -10.0f },
    { &gSariaReturnToOcarinaAnim, 1.0f, ANIMMODE_ONCE, -10.0f },
    { &gSariaPlayingOcarinaAnim, 1.0f, ANIMMODE_LOOP, 0.0f },
};

typedef enum {
    /* 0 */ ENSA_ANIM2_0,
    /* 1 */ ENSA_ANIM2_1,
    /* 2 */ ENSA_ANIM2_2,
    /* 3 */ ENSA_ANIM2_3,
    /* 4 */ ENSA_ANIM2_4,
    /* 5 */ ENSA_ANIM2_5,
    /* 6 */ ENSA_ANIM2_6,
    /* 7 */ ENSA_ANIM2_7,
    /* 8 */ ENSA_ANIM2_8,
    /* 9 */ ENSA_ANIM2_9
} EnSaAnimation2;

static AnimationInfo sAnimationInfo2[] = {
    { &gSariaTransitionHandsSideToChestToSideAnim, 1.0f, 0.0f, -1.0f, ANIMMODE_ONCE, -1.0f },
    { &gSariaTransitionHandsSideToBackAnim, 1.0f, 0.0f, -1.0f, ANIMMODE_LOOP, -4.0f },
    { &gSariaRightArmExtendedWaitAnim, 1.0f, 0.0f, -1.0f, ANIMMODE_LOOP, -1.0f },
    { &gSariaHandsOutAnim, 1.0f, 0.0f, -1.0f, ANIMMODE_ONCE, -1.0f },
    { &gSariaStandHandsOnHipsAnim, 1.0f, 0.0f, -1.0f, ANIMMODE_LOOP, -1.0f },
    { &gSariaExtendRightArmAnim, 1.0f, 0.0f, -1.0f, ANIMMODE_ONCE, -1.0f },
    { &gSariaTransitionHandsSideToHipsAnim, 1.0f, 0.0f, -1.0f, ANIMMODE_LOOP, -1.0f },
    { &gSariaHandsBehindBackWaitAnim, 1.0f, 0.0f, -1.0f, ANIMMODE_LOOP, -1.0f },
    { &gSariaHandsOnFaceAnim, 1.0f, 0.0f, -1.0f, ANIMMODE_ONCE, -1.0f },
    { &gSariaWaitArmsToSideAnim, 1.0f, 0.0f, -1.0f, ANIMMODE_LOOP, -8.0f },
};

s16 func_80AF5560(EnSa* this, GlobalContext* globalCtx) {
    s16 textState = Message_GetState(&globalCtx->msgCtx);

    if (this->unk_209 == TEXT_STATE_AWAITING_NEXT || this->unk_209 == TEXT_STATE_EVENT ||
        this->unk_209 == TEXT_STATE_CLOSING || this->unk_209 == TEXT_STATE_DONE_HAS_NEXT) {
        if (textState != this->unk_209) {
            this->unk_208++;
        }
    }
    this->unk_209 = textState;
    return textState;
}

u16 func_80AF55E0(GlobalContext* globalCtx, Actor* thisx) {
    EnSa* this = (EnSa*)thisx;
    u16 reaction = Text_GetFaceReaction(globalCtx, 0x10);

    if (reaction != 0) {
        return reaction;
    }
    if (CHECK_QUEST_ITEM(QUEST_SONG_SARIA)) {
        return 0x10AD;
    }
    if (CHECK_QUEST_ITEM(QUEST_KOKIRI_EMERALD)) {
        this->unk_208 = 0;
        this->unk_209 = TEXT_STATE_NONE;
        if (GET_INFTABLE(INFTABLE_05)) {
            return 0x1048;
        } else {
            return 0x1047;
        }
    }
    if (GET_EVENTCHKINF(EVENTCHKINF_02)) {
        this->unk_208 = 0;
        this->unk_209 = TEXT_STATE_NONE;
        if (GET_INFTABLE(INFTABLE_03)) {
            return 0x1032;
        } else {
            return 0x1031;
        }
    }
    if (GET_INFTABLE(INFTABLE_00)) {
        this->unk_208 = 0;
        this->unk_209 = TEXT_STATE_NONE;
        if (GET_INFTABLE(INFTABLE_01)) {
            return 0x1003;
        } else {
            return 0x1002;
        }
    }
    return 0x1001;
}

s16 func_80AF56F4(GlobalContext* globalCtx, Actor* thisx) {
    s16 ret = 1;
    EnSa* this = (EnSa*)thisx;

    switch (func_80AF5560(this, globalCtx)) {
        case TEXT_STATE_CLOSING:
            switch (this->actor.textId) {
                case 0x1002:
                    SET_INFTABLE(INFTABLE_01);
                    ret = 0;
                    break;
                case 0x1031:
                    SET_EVENTCHKINF(EVENTCHKINF_03);
                    SET_INFTABLE(INFTABLE_03);
                    ret = 0;
                    break;
                case 0x1047:
                    SET_INFTABLE(INFTABLE_05);
                    ret = 0;
                    break;
                default:
                    ret = 0;
                    break;
            }
            break;
        case TEXT_STATE_NONE:
        case TEXT_STATE_DONE_HAS_NEXT:
        case TEXT_STATE_DONE_FADING:
        case TEXT_STATE_CHOICE:
        case TEXT_STATE_EVENT:
        case TEXT_STATE_SONG_DEMO_DONE:
        case TEXT_STATE_8:
        case TEXT_STATE_9:
            break;
    }
    return ret;
}

void func_80AF57D8(EnSa* this, GlobalContext* globalCtx) {
    if (globalCtx->sceneNum != SCENE_SPOT05 ||
        ABS((s16)(this->actor.yawTowardsPlayer - this->actor.shape.rot.y)) < 0x1555 || this->unk_1E0.unk_00 != 0) {
        func_800343CC(globalCtx, &this->actor, &this->unk_1E0.unk_00, this->collider.dim.radius + 30.0f, func_80AF55E0,
                      func_80AF56F4);
    }
}

f32 func_80AF5894(EnSa* this) {
    f32 endFrame = this->skelAnime.endFrame;
    f32 startFrame = this->skelAnime.startFrame;

    this->skelAnime.startFrame = endFrame;
    this->skelAnime.curFrame = endFrame;
    this->skelAnime.endFrame = startFrame;
    this->skelAnime.playSpeed = -1.0f;
    return startFrame;
}

void func_80AF58B8(EnSa* this) {
    switch (this->unk_20A) {
        case 0:
            Animation_ChangeByInfo(&this->skelAnime, sAnimationInfo2, ENSA_ANIM2_3);
            this->unk_20A++;
        case 1:
            if (Animation_OnFrame(&this->skelAnime, this->skelAnime.endFrame)) {
                Animation_ChangeByInfo(&this->skelAnime, sAnimationInfo2, ENSA_ANIM2_2);
                this->unk_20A++;
            }
            break;
    }
}

void func_80AF594C(EnSa* this) {
    switch (this->unk_20A) {
        case 0:
            Animation_ChangeByInfo(&this->skelAnime, sAnimationInfo2, ENSA_ANIM2_8);
            this->unk_20A++;
        case 1:
            if (Animation_OnFrame(&this->skelAnime, this->skelAnime.endFrame)) {
                Animation_ChangeByInfo(&this->skelAnime, sAnimationInfo2, ENSA_ANIM2_9);
                this->unk_20A++;
            }
            break;
    }
}

void func_80AF59E0(EnSa* this) {
    switch (this->unk_20A) {
        case 0:
            Animation_ChangeByInfo(&this->skelAnime, sAnimationInfo2, ENSA_ANIM2_1);
            this->unk_20A++;
        case 1:
            if (Animation_OnFrame(&this->skelAnime, this->skelAnime.endFrame)) {
                Animation_ChangeByInfo(&this->skelAnime, sAnimationInfo2, ENSA_ANIM2_7);
                this->unk_20A++;
            }
            break;
    }
}

void func_80AF5A74(EnSa* this) {
    switch (this->unk_20A) {
        case 0:
            Animation_ChangeByInfo(&this->skelAnime, sAnimationInfo2, ENSA_ANIM2_1);
            func_80AF5894(this);
            this->unk_20A++;
        case 1:
            if (Animation_OnFrame(&this->skelAnime, this->skelAnime.endFrame)) {
                Animation_ChangeByInfo(&this->skelAnime, sAnimationInfo2, ENSA_ANIM2_9);
                this->unk_20A++;
            }
            break;
    }
}

void func_80AF5B10(EnSa* this) {
    switch (this->unk_20A) {
        case 0:
            Animation_ChangeByInfo(&this->skelAnime, sAnimationInfo2, ENSA_ANIM2_6);
            this->unk_20A++;
        case 1:
            if (Animation_OnFrame(&this->skelAnime, this->skelAnime.endFrame)) {
                Animation_ChangeByInfo(&this->skelAnime, sAnimationInfo2, ENSA_ANIM2_4);
                this->unk_20A++;
            }
            break;
    }
}

void func_80AF5BA4(EnSa* this) {
    switch (this->unk_20A) {
        case 0:
            Animation_ChangeByInfo(&this->skelAnime, sAnimationInfo2, ENSA_ANIM2_6);
            func_80AF5894(this);
            this->unk_20A++;
        case 1:
            if (Animation_OnFrame(&this->skelAnime, this->skelAnime.endFrame)) {
                Animation_ChangeByInfo(&this->skelAnime, sAnimationInfo2, ENSA_ANIM2_9);
                this->unk_20A++;
            }
            break;
    }
}

void func_80AF5C40(EnSa* this) {
    switch (this->unk_20A) {
        case 0:
            Animation_ChangeByInfo(&this->skelAnime, sAnimationInfo2, ENSA_ANIM2_5);
            this->unk_20A++;
        case 1:
            if (Animation_OnFrame(&this->skelAnime, this->skelAnime.endFrame)) {
                Animation_ChangeByInfo(&this->skelAnime, sAnimationInfo2, ENSA_ANIM2_0);
                this->unk_20A++;
            }
            break;
    }
}

void func_80AF5CD4(EnSa* this, u8 arg1) {
    this->unk_20B = arg1;
    this->unk_20A = 0;
}

void func_80AF5CE4(EnSa* this) {
    switch (this->unk_20B) {
        case 1:
            func_80AF58B8(this);
            break;
        case 2:
            func_80AF594C(this);
            break;
        case 3:
            func_80AF59E0(this);
            break;
        case 4:
            func_80AF5A74(this);
            break;
        case 5:
            func_80AF5B10(this);
            break;
        case 6:
            func_80AF5BA4(this);
            break;
        case 7:
            func_80AF5C40(this);
            break;
    }
}

void EnSa_ChangeAnim(EnSa* this, s32 index) {
    Animation_Change(&this->skelAnime, sAnimationInfo1[index].animation, 1.0f, 0.0f,
                     Animation_GetLastFrame(sAnimationInfo1[index].animation), sAnimationInfo1[index].mode,
                     sAnimationInfo1[index].morphFrames);
}

s32 func_80AF5DFC(EnSa* this, GlobalContext* globalCtx) {
    if (gSaveContext.cutsceneIndex >= 0xFFF0 && gSaveContext.cutsceneIndex != 0xFFFD) {
        if (globalCtx->sceneNum == SCENE_SPOT04) {
            return 4;
        }
        if (globalCtx->sceneNum == SCENE_SPOT05) {
            return 5;
        }
    }
    if (globalCtx->sceneNum == SCENE_KOKIRI_HOME5 && !LINK_IS_ADULT &&
        INV_CONTENT(ITEM_OCARINA_FAIRY) == ITEM_OCARINA_FAIRY && !GET_EVENTCHKINF(EVENTCHKINF_40)) {
        return 1;
    }
    if (globalCtx->sceneNum == SCENE_SPOT05 && GET_EVENTCHKINF(EVENTCHKINF_40)) {
        return CHECK_QUEST_ITEM(QUEST_SONG_SARIA) ? 2 : 5;
    }
    if (globalCtx->sceneNum == SCENE_SPOT04 && !CHECK_QUEST_ITEM(QUEST_KOKIRI_EMERALD)) {
        if (GET_INFTABLE(INFTABLE_00)) {
            return 1;
        }
        return 4;
    }
    return 0;
}

void func_80AF5F34(EnSa* this, GlobalContext* globalCtx) {
    Player* player = GET_PLAYER(globalCtx);
    s16 phi_a3 = 0;

    if (globalCtx->sceneNum == SCENE_SPOT04) {
        phi_a3 = (this->actionFunc == func_80AF68E4) ? 1 : 4;
    }
    if (globalCtx->sceneNum == SCENE_SPOT05) {
        phi_a3 = (this->skelAnime.animation == &gSariaPlayingOcarinaAnim) ? 1 : 3;
    }
    if (globalCtx->sceneNum == SCENE_SPOT05 && this->actionFunc == func_80AF6448 &&
        this->skelAnime.animation == &gSariaStopPlayingOcarinaAnim) {
        phi_a3 = 1;
    }
    if (globalCtx->sceneNum == SCENE_SPOT05 && this->actionFunc == func_80AF68E4 &&
        this->skelAnime.animation == &gSariaOcarinaToMouthAnim) {
        phi_a3 = 1;
    }
    this->unk_1E0.unk_18 = player->actor.world.pos;
    this->unk_1E0.unk_14 = 4.0f;
    func_80034A14(&this->actor, &this->unk_1E0, 2, phi_a3);
}

s32 func_80AF603C(EnSa* this) {
    if (this->skelAnime.animation != &gSariaPlayingOcarinaAnim &&
        this->skelAnime.animation != &gSariaOcarinaToMouthAnim) {
        return 0;
    }
    if (this->unk_1E0.unk_00 != 0) {
        return 0;
    }
    this->unk_20E = 0;
    if (this->rightEyeIndex != SARIA_EYE_CLOSED) {
        return 0;
    }
    return 1;
}

void func_80AF609C(EnSa* this) {
    s16 phi_v1;

    if (func_80AF603C(this) == 0) {
        if (this->unk_20E == 0) {
            phi_v1 = 0;
        } else {
            this->unk_20E--;
            phi_v1 = this->unk_20E;
        }
        if (phi_v1 == 0) {
            this->rightEyeIndex++;
            if (this->rightEyeIndex < SARIA_EYE_SUPRISED) {
                this->leftEyeIndex = this->rightEyeIndex;
            } else {
                this->unk_20E = Rand_S16Offset(30, 30);
                this->leftEyeIndex = SARIA_EYE_OPEN;
                this->rightEyeIndex = this->leftEyeIndex;
            }
        }
    }
}

void func_80AF6130(CsCmdActorAction* csAction, Vec3f* dst) {
    dst->x = csAction->startPos.x;
    dst->y = csAction->startPos.y;
    dst->z = csAction->startPos.z;
}

void func_80AF6170(CsCmdActorAction* csAction, Vec3f* dst) {
    dst->x = csAction->endPos.x;
    dst->y = csAction->endPos.y;
    dst->z = csAction->endPos.z;
}

void EnSa_Init(Actor* thisx, GlobalContext* globalCtx) {
    EnSa* this = (EnSa*)thisx;
    s32 pad;

    ActorShape_Init(&this->actor.shape, 0.0f, ActorShadow_DrawCircle, 12.0f);
    SkelAnime_InitFlex(globalCtx, &this->skelAnime, &gSariaSkel, NULL, this->jointTable, this->morphTable, 17);
    Collider_InitCylinder(globalCtx, &this->collider);
    Collider_SetCylinder(globalCtx, &this->collider, &this->actor, &sCylinderInit);
    CollisionCheck_SetInfo2(&this->actor.colChkInfo, NULL, &sColChkInfoInit);

    switch (func_80AF5DFC(this, globalCtx)) {
        case 2:
            EnSa_ChangeAnim(this, ENSA_ANIM1_11);
            this->actionFunc = func_80AF6448;
            break;
        case 5:
            EnSa_ChangeAnim(this, ENSA_ANIM1_11);
            this->actionFunc = func_80AF683C;
            break;
        case 1:
            this->actor.gravity = -1.0f;
            EnSa_ChangeAnim(this, ENSA_ANIM1_0);
            this->actionFunc = func_80AF6448;
            break;
        case 4:
            this->unk_210 = 0;
            this->actor.gravity = -1.0f;
            globalCtx->csCtx.segment = SEGMENTED_TO_VIRTUAL(gSpot04Cs_10E20);
            gSaveContext.cutsceneTrigger = 1;
            EnSa_ChangeAnim(this, ENSA_ANIM1_4);
            this->actionFunc = func_80AF68E4;
            break;
        case 3:
            this->unk_210 = 0;
            this->actor.gravity = -1.0f;
            EnSa_ChangeAnim(this, ENSA_ANIM1_0);
            this->actionFunc = func_80AF68E4;
            break;
        case 0:
            Actor_Kill(&this->actor);
            return;
    }

    Actor_SetScale(&this->actor, 0.01f);

    this->actor.targetMode = 6;
    this->unk_1E0.unk_00 = 0;
    this->alpha = 255;
    this->unk_21A = this->actor.shape.rot;

    Actor_SpawnAsChild(&globalCtx->actorCtx, &this->actor, globalCtx, ACTOR_EN_ELF, this->actor.world.pos.x,
                       this->actor.world.pos.y, this->actor.world.pos.z, 0, 0, 0, FAIRY_KOKIRI);
}

void EnSa_Destroy(Actor* thisx, GlobalContext* globalCtx) {
    EnSa* this = (EnSa*)thisx;

    Collider_DestroyCylinder(globalCtx, &this->collider);
}

void func_80AF6448(EnSa* this, GlobalContext* globalCtx) {
    if (globalCtx->sceneNum == SCENE_SPOT04) {
        if (this->unk_1E0.unk_00 != 0) {
            switch (this->actor.textId) {
                case 0x1002:
                    if (this->unk_208 == 0 && this->unk_20B != 1) {
                        func_80AF5CD4(this, 1);
                        this->mouthIndex = 1;
                    }
                    if (this->unk_208 == 2 && this->unk_20B != 2) {
                        func_80AF5CD4(this, 2);
                        this->mouthIndex = 1;
                    }
                    if (this->unk_208 == 5) {
                        this->mouthIndex = 0;
                    }
                    break;
                case 0x1003:
                    if (this->unk_208 == 0 && this->unk_20B != 4) {
                        func_80AF5CD4(this, 4);
                    }
                    break;
                case 0x1031:
                    if (this->unk_208 == 0 && this->unk_20B != 4 &&
                        this->skelAnime.animation == &gSariaHandsBehindBackWaitAnim) {
                        func_80AF5CD4(this, 4);
                        this->mouthIndex = 3;
                    }
                    if (this->unk_208 == 2 && this->unk_20B != 5) {
                        func_80AF5CD4(this, 5);
                        this->mouthIndex = 2;
                    }
                    if (this->unk_208 == 4 && this->unk_20B != 6) {
                        func_80AF5CD4(this, 6);
                        this->mouthIndex = 0;
                    }
                    break;
                case 0x1032:
                    if (this->unk_208 == 0 && this->unk_20B != 4 &&
                        this->skelAnime.animation == &gSariaHandsBehindBackWaitAnim) {
                        func_80AF5CD4(this, 4);
                    }
                    break;
                case 0x1047:
                    if (this->unk_208 == 1 && this->unk_20B != 7) {
                        func_80AF5CD4(this, 7);
                    }
                    break;
                case 0x1048:
                    if (this->unk_208 == 0 && this->unk_20B != 7) {
                        func_80AF5CD4(this, 7);
                    }
                    break;
            }
        } else if (!CHECK_QUEST_ITEM(QUEST_KOKIRI_EMERALD) &&
                   (GET_INFTABLE(INFTABLE_01) || GET_INFTABLE(INFTABLE_03))) {
            if (this->unk_20B != 3) {
                func_80AF5CD4(this, 3);
            }
        } else {
            func_80AF5CD4(this, 0);
        }
        func_80AF5CE4(this);
    }
    if (this->skelAnime.animation == &gSariaStopPlayingOcarinaAnim) {
        this->skelAnime.playSpeed = -1.0f;
        if ((s32)this->skelAnime.curFrame == 0) {
            EnSa_ChangeAnim(this, ENSA_ANIM1_6);
        }
    }
    if (this->unk_1E0.unk_00 != 0 && globalCtx->sceneNum == SCENE_SPOT05) {
        Animation_Change(&this->skelAnime, &gSariaStopPlayingOcarinaAnim, 1.0f, 0.0f, 10.0f, ANIMMODE_ONCE, -10.0f);
        this->actionFunc = func_80AF67D0;
    }
}

void func_80AF67D0(EnSa* this, GlobalContext* globalCtx) {
    if (this->unk_1E0.unk_00 == 0) {
        Animation_Change(&this->skelAnime, &gSariaStopPlayingOcarinaAnim, 0.0f, 10.0f, 0.0f, ANIMMODE_ONCE, -10.0f);
        this->actionFunc = func_80AF6448;
    }
}

void func_80AF683C(EnSa* this, GlobalContext* globalCtx) {
    Player* player = GET_PLAYER(globalCtx);

    if (!(player->actor.world.pos.z >= -2220.0f) && !Gameplay_InCsMode(globalCtx)) {
        globalCtx->csCtx.segment = SEGMENTED_TO_VIRTUAL(spot05_scene_Cs_005730);
        gSaveContext.cutsceneTrigger = 1;
        this->actionFunc = func_80AF68E4;
    }
}

void func_80AF68E4(EnSa* this, GlobalContext* globalCtx) {
    s16 phi_v0;
    Vec3f startPos;
    Vec3f endPos;
    Vec3f D_80AF7448 = { 0.0f, 0.0f, 0.0f };
    CsCmdActorAction* csAction;
    f32 temp_f0;
    f32 gravity;

    if (globalCtx->csCtx.state == CS_STATE_IDLE) {
        this->actionFunc = func_80AF6B20;
        return;
    }
    csAction = globalCtx->csCtx.npcActions[1];
    if (csAction != NULL) {
        func_80AF6130(csAction, &startPos);
        func_80AF6170(csAction, &endPos);

        if (this->unk_210 == 0) {
            this->actor.world.pos = startPos;
        }
        if (this->unk_210 != csAction->action) {
            switch (csAction->action) {
                case 2:
                    this->mouthIndex = 1;
                    break;
                case 9:
                    this->mouthIndex = 1;
                    break;
                default:
                    this->mouthIndex = 0;
                    break;
            }
            EnSa_ChangeAnim(this, csAction->action);
            this->unk_210 = csAction->action;
        }
        if (phi_v0) {}
        if (csAction->action == 3) {
            if (this->unk_20C == 0) {
                phi_v0 = 0;
            } else {
                this->unk_20C--;
                phi_v0 = this->unk_20C;
            }
            if (phi_v0 == 0) {
                Audio_PlaySoundGeneral(NA_SE_PL_WALK_GROUND, &this->actor.projectedPos, 4, &gSfxDefaultFreqAndVolScale,
                                       &gSfxDefaultFreqAndVolScale, &gSfxDefaultReverb);
                this->unk_20C = 8;
            }
        }
        this->actor.shape.rot.x = csAction->urot.x;
        this->actor.shape.rot.y = csAction->urot.y;
        this->actor.shape.rot.z = csAction->urot.z;
        this->actor.velocity = D_80AF7448;

        if (globalCtx->csCtx.frames < csAction->endFrame) {
            temp_f0 = csAction->endFrame - csAction->startFrame;
            this->actor.velocity.x = (endPos.x - startPos.x) / temp_f0;
            this->actor.velocity.y = (endPos.y - startPos.y) / temp_f0;
            gravity = this->actor.gravity;
            if (globalCtx->sceneNum == SCENE_SPOT05) {
                gravity = 0.0f;
            }
            this->actor.velocity.y += gravity;
            if (this->actor.velocity.y < this->actor.minVelocityY) {
                this->actor.velocity.y = this->actor.minVelocityY;
            }
            this->actor.velocity.z = (endPos.z - startPos.z) / temp_f0;
        }
    }
}

void func_80AF6B20(EnSa* this, GlobalContext* globalCtx) {
    if (globalCtx->sceneNum == SCENE_SPOT05) {
        Item_Give(globalCtx, ITEM_SONG_SARIA);
        EnSa_ChangeAnim(this, ENSA_ANIM1_6);
    }

    if (globalCtx->sceneNum == SCENE_SPOT04) {
        EnSa_ChangeAnim(this, ENSA_ANIM1_4);
        this->actor.world.pos = this->actor.home.pos;
        this->actor.world.rot = this->unk_21A;
        this->mouthIndex = 0;
        SET_INFTABLE(INFTABLE_00);
    }

    this->actionFunc = func_80AF6448;
}

void EnSa_Update(Actor* thisx, GlobalContext* globalCtx) {
    EnSa* this = (EnSa*)thisx;
    s32 pad;

    Collider_UpdateCylinder(&this->actor, &this->collider);
    CollisionCheck_SetOC(globalCtx, &globalCtx->colChkCtx, &this->collider.base);
    SkelAnime_Update(&this->skelAnime);

    if (this->skelAnime.animation == &gSariaOcarinaToMouthAnim &&
        this->skelAnime.curFrame >= Animation_GetLastFrame(&gSariaOcarinaToMouthAnim)) {
        EnSa_ChangeAnim(this, ENSA_ANIM1_6);
    }

    if (this->actionFunc != func_80AF68E4) {
        this->alpha = func_80034DD4(&this->actor, globalCtx, this->alpha, 400.0f);
    } else {
        this->alpha = 255;
    }

    this->actor.shape.shadowAlpha = this->alpha;

    if (this->actionFunc == func_80AF68E4) {
        this->actor.world.pos.x += this->actor.velocity.x;
        this->actor.world.pos.y += this->actor.velocity.y;
        this->actor.world.pos.z += this->actor.velocity.z;
    } else {
        func_8002D7EC(&this->actor);
    }

    if (globalCtx->sceneNum != SCENE_SPOT05) {
        Actor_UpdateBgCheckInfo(globalCtx, &this->actor, 0.0f, 0.0f, 0.0f, UPDBGCHECKINFO_FLAG_2);
    }

    func_80AF609C(this);
    this->actionFunc(this, globalCtx);
    func_80AF57D8(this, globalCtx);
    func_80AF5F34(this, globalCtx);
}

s32 EnSa_OverrideLimbDraw(GlobalContext* globalCtx, s32 limbIndex, Gfx** dList, Vec3f* pos, Vec3s* rot, void* thisx,
                          Gfx** gfx) {
    EnSa* this = (EnSa*)thisx;
    s32 pad;
    Vec3s sp18;

    if (limbIndex == 16) {
        Matrix_Translate(900.0f, 0.0f, 0.0f, MTXMODE_APPLY);
        sp18 = this->unk_1E0.unk_08;
        Matrix_RotateX(BINANG_TO_RAD_ALT(sp18.y), MTXMODE_APPLY);
        Matrix_RotateZ(BINANG_TO_RAD_ALT(sp18.x), MTXMODE_APPLY);
        Matrix_Translate(-900.0f, 0.0f, 0.0f, MTXMODE_APPLY);
    }

    if (limbIndex == 9) {
        sp18 = this->unk_1E0.unk_0E;
        Matrix_RotateY(BINANG_TO_RAD_ALT(sp18.y), MTXMODE_APPLY);
        Matrix_RotateX(BINANG_TO_RAD_ALT(sp18.x), MTXMODE_APPLY);
    }

    if (globalCtx->sceneNum == SCENE_SPOT05 && limbIndex == 15) {
        *dList = gSariaRightHandAndOcarinaDL;
    }

    return 0;
}

void EnSa_PostLimbDraw(GlobalContext* globalCtx, s32 limbIndex, Gfx** dList, Vec3s* rot, void* thisx, Gfx** gfx) {
    EnSa* this = (EnSa*)thisx;
    Vec3f D_80AF7454 = { 400.0, 0.0f, 0.0f };

    if (limbIndex == 16) {
        Matrix_MultVec3f(&D_80AF7454, &this->actor.focus.pos);
    }
}

void EnSa_Draw(Actor* thisx, GlobalContext* globalCtx) {
    static void* mouthTextures[] = {
        gSariaMouthClosed2Tex,  gSariaMouthSmilingOpenTex, gSariaMouthFrowningTex,
        gSariaMouthSuprisedTex, gSariaMouthClosedTex,
    };
    static void* eyeTextures[] = {
        gSariaEyeOpenTex, gSariaEyeHalfTex, gSariaEyeClosedTex, gSariaEyeSuprisedTex, gSariaEyeSadTex,
    };
    EnSa* this = (EnSa*)thisx;

    OPEN_DISPS(globalCtx->state.gfxCtx, "../z_en_sa.c", 1444);

    if (this->alpha == 255) {
        gSPSegment(POLY_OPA_DISP++, 0x08, SEGMENTED_TO_VIRTUAL(eyeTextures[this->rightEyeIndex]));
        gSPSegment(POLY_OPA_DISP++, 0x09, SEGMENTED_TO_VIRTUAL(eyeTextures[this->leftEyeIndex]));
        gSPSegment(POLY_OPA_DISP++, 0x0A, SEGMENTED_TO_VIRTUAL(mouthTextures[this->mouthIndex]));
        func_80034BA0(globalCtx, &this->skelAnime, EnSa_OverrideLimbDraw, EnSa_PostLimbDraw, &this->actor, this->alpha);
    } else if (this->alpha != 0) {
        gSPSegment(POLY_XLU_DISP++, 0x08, SEGMENTED_TO_VIRTUAL(eyeTextures[this->rightEyeIndex]));
        gSPSegment(POLY_XLU_DISP++, 0x09, SEGMENTED_TO_VIRTUAL(eyeTextures[this->leftEyeIndex]));
        gSPSegment(POLY_XLU_DISP++, 0x0A, SEGMENTED_TO_VIRTUAL(mouthTextures[this->mouthIndex]));
        func_80034CC4(globalCtx, &this->skelAnime, EnSa_OverrideLimbDraw, EnSa_PostLimbDraw, &this->actor, this->alpha);
    }

    CLOSE_DISPS(globalCtx->state.gfxCtx, "../z_en_sa.c", 1497);
}
