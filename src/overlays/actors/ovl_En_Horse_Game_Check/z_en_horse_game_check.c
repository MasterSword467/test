/*
 * File: z_en_horse_game_check.c
 * Overlay: ovl_En_Horse_Game_Check
 * Description: Horseback Minigames
 */

#include "z_en_horse_game_check.h"
#include "overlays/actors/ovl_En_Horse/z_en_horse.h"

#define FLAGS ACTOR_FLAG_4

#define AT_FINISH_LINE(actor)                                                                                     \
    (Math3D_PointInSquare2D(sFinishLine[0], sFinishLine[1], sFinishLine[2], sFinishLine[3], (actor)->world.pos.x, \
                            (actor)->world.pos.z))
#define AT_RANCH_EXIT(actor)                                                                                  \
    (Math3D_PointInSquare2D(sRanchExit[0], sRanchExit[1], sRanchExit[2], sRanchExit[3], (actor)->world.pos.x, \
                            (actor)->world.pos.z))

#define INGORACE_PLAYER_MOVE (1 << 0)
#define INGORACE_SET_TIMER (1 << 1)
#define INGORACE_INGO_MOVE (1 << 2)

typedef enum {
    /* 0 */ INGORACE_NO_RESULT,
    /* 1 */ INGORACE_PLAYER_WIN,
    /* 2 */ INGORACE_INGO_WIN,
    /* 3 */ INGORACE_TIME_UP
} HorseGameIngoRaceResult;

#define MALONRACE_PLAYER_MOVE (1 << 0)
#define MALONRACE_SET_TIMER (1 << 1)
#define MALONRACE_SECOND_LAP (1 << 2)
#define MALONRACE_BROKE_RULE (1 << 3)
#define MALONRACE_START_SFX (1 << 4)
#define MALONRACE_PLAYER_START (1 << 5)
#define MALONRACE_PLAYER_ON_MARK (1 << 6)

typedef enum {
    /* 0 */ MALONRACE_NO_RESULT,
    /* 1 */ MALONRACE_SUCCESS,
    /* 2 */ MALONRACE_TIME_UP,
    /* 3 */ MALONRACE_UNUSED,
    /* 4 */ MALONRACE_FAILURE
} HorseGameMalonRaceResult;

void EnHorseGameCheck_Init(Actor* thisx, GlobalContext* globalCtx);
void EnHorseGameCheck_Destroy(Actor* thisx, GlobalContext* globalCtx);
void EnHorseGameCheck_Update(Actor* thisx, GlobalContext* globalCtx);
void EnHorseGameCheck_Draw(Actor* thisx, GlobalContext* globalCtx);

const ActorInit En_Horse_Game_Check_InitVars = {
    ACTOR_EN_HORSE_GAME_CHECK,
    ACTORCAT_BG,
    FLAGS,
    OBJECT_GAMEPLAY_KEEP,
    sizeof(EnHorseGameCheck),
    (ActorFunc)EnHorseGameCheck_Init,
    (ActorFunc)EnHorseGameCheck_Destroy,
    (ActorFunc)EnHorseGameCheck_Update,
    (ActorFunc)EnHorseGameCheck_Draw,
};

static Vec3f sIngoRaceCheckpoints[] = {
    { 1700.0f, 1.0f, -540.0f },
    { 117.0f, 1.0f, 488.0f },
    { -1700.0f, 1.0f, -540.0f },
};

static f32 sFinishLine[4] = { -200.0f, 80.0f, -2300.0f, -1470.0f };

static f32 sRanchExit[4] = { 800.0f, 1000.0f, -2900.0f, -2700.0f };

static Vec3f sUnusedZeroVec = { 0.0f, 0.0f, 0.0f };

static Vec3f sFencePos[] = {
    { 820.0f, -44.0f, -1655.0f }, { 1497.0f, -21.0f, -1198.0f },  { 1655.0f, -44.0f, -396.0f },
    { 1291.0f, -44.0f, 205.0f },  { 379.0f, -21.0f, 455.0f },     { -95.0f, -21.0f, 455.0f },
    { -939.0f, 1.0f, 455.0f },    { -1644.0f, -21.0f, -1035.0f },
};

s32 EnHorseGameCheck_InitIngoRace(EnHorseGameCheckBase* base, GlobalContext* globalCtx) {
    EnHorseGameCheckIngoRace* this = (EnHorseGameCheckIngoRace*)base;
    s32 i;

    this->base.type = HORSEGAME_INGO_RACE;
    this->startFlags = 0;
    for (i = 0; i < 3; i++) {
        this->playerCheck[i] = 0;
    }
    this->ingoHorse =
        Actor_Spawn(&globalCtx->actorCtx, globalCtx, ACTOR_EN_HORSE, -250.0f, 1.0f, -1650.0f, 0, 0x4000, 0, 0x8003);

    if (this->ingoHorse == NULL) {
        LogUtils_HungupThread("../z_en_horse_game_check.c", 385);
    }
    this->startTimer = 0;
    this->finishTimer = 0;
    this->result = INGORACE_NO_RESULT;
    this->playerFinish = 0;
    this->ingoFinish = 0;

    return true;
}

s32 EnHorseGameCheck_DestroyIngoRace(EnHorseGameCheckBase* base, GlobalContext* globalCtx) {
    return true;
}

void EnHorseGameCheck_FinishIngoRace(EnHorseGameCheckIngoRace* this, GlobalContext* globalCtx) {
    gSaveContext.cutsceneIndex = 0;
    if (this->result == INGORACE_PLAYER_WIN) {
        globalCtx->nextEntranceIndex = ENTR_SPOT20_7;
        if (GET_EVENTINF(EVENTINF_HORSES_06)) {
            SET_EVENTINF_HORSES_STATE(EVENTINF_HORSES_STATE_6);
            SET_EVENTINF_HORSES_0F(1);
            globalCtx->transitionType = TRANS_TYPE_FADE_WHITE;
            Environment_ForcePlaySequence(NA_BGM_INGO);
        } else {
            SET_EVENTINF_HORSES_STATE(EVENTINF_HORSES_STATE_4);
            SET_EVENTINF_HORSES_0F(1);
            Environment_ForcePlaySequence(NA_BGM_INGO);
            globalCtx->transitionType = TRANS_TYPE_CIRCLE(TCA_STARBURST, TCC_WHITE, TCS_FAST);
        }
    } else {
        globalCtx->nextEntranceIndex = ENTR_SPOT20_8;
        SET_EVENTINF_HORSES_STATE(EVENTINF_HORSES_STATE_3);
        globalCtx->transitionType = TRANS_TYPE_CIRCLE(TCA_NORMAL, TCC_BLACK, TCS_FAST);
        SET_EVENTINF_HORSES_0F(1);
    }
    DREG(25) = 0;
    globalCtx->transitionTrigger = TRANS_TRIGGER_START;
    gSaveContext.timer1State = 0;
}

s32 EnHorseGameCheck_UpdateIngoRace(EnHorseGameCheckBase* base, GlobalContext* globalCtx) {
    EnHorseGameCheckIngoRace* this = (EnHorseGameCheckIngoRace*)base;
    Player* player = GET_PLAYER(globalCtx);
    s32 i;
    EnHorse* ingoHorse;
    EnHorse* horse;

    if ((this->startTimer > 50) && !(this->startFlags & INGORACE_SET_TIMER)) {
        this->startFlags |= INGORACE_SET_TIMER;
        func_80088B34(0);
    } else if ((this->startTimer > 80) && (player->rideActor != NULL) && !(this->startFlags & INGORACE_PLAYER_MOVE)) {
        this->startFlags |= INGORACE_PLAYER_MOVE;
        horse = (EnHorse*)player->rideActor;
        horse->inRace = 1;
    } else if ((this->startTimer > 81) && !(this->startFlags & INGORACE_INGO_MOVE)) {
        ingoHorse = (EnHorse*)this->ingoHorse;

        ingoHorse->inRace = 1;
        this->startFlags |= INGORACE_INGO_MOVE;
        Audio_PlaySoundGeneral(NA_SE_SY_START_SHOT, &gSfxDefaultPos, 4, &gSfxDefaultFreqAndVolScale,
                               &gSfxDefaultFreqAndVolScale, &gSfxDefaultReverb);
    }

    this->startTimer++;

    for (i = 0; i < 3; i++) {
        if ((player->rideActor != NULL) &&
            (Math3D_Vec3f_DistXYZ(&sIngoRaceCheckpoints[i], &player->rideActor->world.pos) < 400.0f)) {
            if ((i > 0) && (this->playerCheck[i - 1] == 1)) {
                this->playerCheck[i] = 1;
            } else if (i == 0) {
                this->playerCheck[i] = 1;
            }
        }
        if (Math3D_Vec3f_DistXYZ(&sIngoRaceCheckpoints[i], &this->ingoHorse->world.pos) < 400.0f) {
            if ((i > 0) && (this->ingoCheck[i - 1] == 1)) {
                this->ingoCheck[i] = 1;
            } else if (i == 0) {
                this->ingoCheck[i] = 1;
            }
        }
    }

    if (this->result == INGORACE_NO_RESULT) {
        Player* player2 = player;

        if ((player2->rideActor != NULL) && (this->playerCheck[2] == 1) && AT_FINISH_LINE(player2->rideActor)) {
            this->playerFinish++;
            if (this->playerFinish > 0) {
                this->result = INGORACE_PLAYER_WIN;
                this->finishTimer = 55;
                Audio_QueueSeqCmd(SEQ_PLAYER_BGM_MAIN << 24 | NA_BGM_HORSE_GOAL);
                Audio_PlaySoundGeneral(NA_SE_SY_START_SHOT, &gSfxDefaultPos, 4, &gSfxDefaultFreqAndVolScale,
                                       &gSfxDefaultFreqAndVolScale, &gSfxDefaultReverb);
            }
            for (i = 0; i < 3; i++) {
                this->playerCheck[i] = 0;
            }
        }
        if ((this->ingoHorse != NULL) && (this->ingoCheck[2] == 1) && AT_FINISH_LINE(this->ingoHorse)) {
            this->ingoFinish++;
            if (this->ingoFinish > 0) {
                ingoHorse = (EnHorse*)this->ingoHorse;

                this->result = INGORACE_INGO_WIN;
                this->finishTimer = 70;
                ingoHorse->stateFlags |= ENHORSE_INGO_WON;
                Audio_QueueSeqCmd(SEQ_PLAYER_BGM_MAIN << 24 | NA_BGM_HORSE_GOAL);
                Audio_PlaySoundGeneral(NA_SE_SY_START_SHOT, &gSfxDefaultPos, 4, &gSfxDefaultFreqAndVolScale,
                                       &gSfxDefaultFreqAndVolScale, &gSfxDefaultReverb);
            }
            for (i = 0; i < 3; i++) {
                this->ingoCheck[i] = 0;
            }
        }
        if (((player2->rideActor != NULL) && AT_RANCH_EXIT(player2->rideActor)) || AT_RANCH_EXIT(&player2->actor)) {
            Audio_QueueSeqCmd(SEQ_PLAYER_BGM_MAIN << 24 | NA_BGM_HORSE_GOAL);
            this->result = INGORACE_INGO_WIN;
            this->finishTimer = 20;
        }
        if ((gSaveContext.timer1Value >= 180) && (this->startFlags & 2)) {
            Audio_QueueSeqCmd(SEQ_PLAYER_BGM_MAIN << 24 | NA_BGM_HORSE_GOAL);
            this->result = INGORACE_TIME_UP;
            this->finishTimer = 20;
        }
    } else {
        if (this->finishTimer > 0) {
            this->finishTimer--;
        } else {
            EnHorseGameCheck_FinishIngoRace(this, globalCtx);
        }
    }
    return true;
}

s32 EnHorseGameCheck_InitGerudoArchery(EnHorseGameCheckBase* base, GlobalContext* globalCtx) {
    EnHorseGameCheckGerudoArchery* this = (EnHorseGameCheckGerudoArchery*)base;

    this->base.type = HORSEGAME_GERUDO_ARCHERY;
    this->unk_150 = 0;
    this->startTimer = 0;
    return true;
}

s32 EnHorseGameCheck_DestroyGerudoArchery(EnHorseGameCheckBase* base, GlobalContext* globalCtx) {
    return true;
}

s32 EnHorseGameCheck_UpdateGerudoArchery(EnHorseGameCheckBase* base, GlobalContext* globalCtx) {
    EnHorseGameCheckGerudoArchery* this = (EnHorseGameCheckGerudoArchery*)base;
    Player* player = GET_PLAYER(globalCtx);
    EnHorse* horse = (EnHorse*)player->rideActor;

    if (horse == NULL) {
        return true;
    }

    if (this->startTimer > 90) {
        if (globalCtx) {}
        horse->hbaStarted = 1;
    }
    this->startTimer++;
    return true;
}

s32 EnHorseGameCheck_InitType3(EnHorseGameCheckBase* base, GlobalContext* globalCtx) {
    EnHorseGameCheck3* this = (EnHorseGameCheck3*)base;

    this->base.type = HORSEGAME_TYPE3;
    this->unk_150 = 0;
    return true;
}

s32 EnHorseGameCheck_DestroyType3(EnHorseGameCheckBase* base, GlobalContext* globalCtx) {
    return true;
}

s32 EnHorseGameCheck_UpdateType3(EnHorseGameCheckBase* base, GlobalContext* globalCtx) {
    return true;
}

s32 EnHorseGameCheck_InitMalonRace(EnHorseGameCheckBase* base, GlobalContext* globalCtx) {
    EnHorseGameCheckMalonRace* this = (EnHorseGameCheckMalonRace*)base;
    s32 i;

    this->base.type = HORSEGAME_MALON_RACE;
    this->raceFlags = 0;
    this->finishTimer = 0;
    this->result = MALONRACE_NO_RESULT;
    for (i = 0; i < 16; i++) {
        this->fenceCheck[i] = 0;
    }
    this->lapCount = 0;
    return true;
}

s32 EnHorseGameCheck_DestroyMalonRace(EnHorseGameCheckBase* base, GlobalContext* globalCtx) {
    return true;
}

void EnHorseGameCheck_FinishMalonRace(EnHorseGameCheckMalonRace* this, GlobalContext* globalCtx) {
    if ((this->result == MALONRACE_SUCCESS) || (this->result == MALONRACE_TIME_UP)) {
        gSaveContext.cutsceneIndex = 0;
        globalCtx->nextEntranceIndex = ENTR_SPOT20_7;
        globalCtx->transitionType = TRANS_TYPE_CIRCLE(TCA_STARBURST, TCC_WHITE, TCS_FAST);
        globalCtx->transitionTrigger = TRANS_TRIGGER_START;
    } else if (this->result == MALONRACE_FAILURE) {
        gSaveContext.timer1Value = 240;
        gSaveContext.timer1State = 0xF;
        gSaveContext.cutsceneIndex = 0;
        globalCtx->nextEntranceIndex = ENTR_SPOT20_7;
        globalCtx->transitionType = TRANS_TYPE_CIRCLE(TCA_STARBURST, TCC_WHITE, TCS_FAST);
        globalCtx->transitionTrigger = TRANS_TRIGGER_START;
    } else {
        // "not supported"
        osSyncPrintf("En_HGC_Spot20_Ta_end():対応せず\n");
        gSaveContext.cutsceneIndex = 0;
        globalCtx->nextEntranceIndex = ENTR_SPOT20_0;
        globalCtx->transitionType = TRANS_TYPE_CIRCLE(TCA_STARBURST, TCC_WHITE, TCS_FAST);
        globalCtx->transitionTrigger = TRANS_TRIGGER_START;
    }
}

s32 EnHorseGameCheck_UpdateMalonRace(EnHorseGameCheckBase* base, GlobalContext* globalCtx) {
    EnHorseGameCheckMalonRace* this = (EnHorseGameCheckMalonRace*)base;
    s32 i;
    Player* player = GET_PLAYER(globalCtx);
    EnHorse* horse;

    if (!(this->raceFlags & MALONRACE_PLAYER_ON_MARK) && AT_FINISH_LINE(player->rideActor)) {
        this->raceFlags |= MALONRACE_PLAYER_ON_MARK;
    } else if ((this->raceFlags & MALONRACE_PLAYER_ON_MARK) && !(this->raceFlags & MALONRACE_PLAYER_START) &&
               !AT_FINISH_LINE(player->rideActor)) {
        this->raceFlags |= MALONRACE_PLAYER_START;
    }
    if ((this->startTimer > 50) && !(this->raceFlags & MALONRACE_SET_TIMER)) {
        this->raceFlags |= MALONRACE_SET_TIMER;
        func_80088B34(0);
    } else if ((this->startTimer > 80) && (player->rideActor != NULL) && !(this->raceFlags & MALONRACE_PLAYER_MOVE)) {
        this->raceFlags |= MALONRACE_PLAYER_MOVE;
        horse = (EnHorse*)player->rideActor;

        horse->inRace = 1;
    } else if ((this->startTimer > 81) && !(this->raceFlags & MALONRACE_START_SFX)) {
        this->raceFlags |= MALONRACE_START_SFX;
        Audio_PlaySoundGeneral(NA_SE_SY_START_SHOT, &gSfxDefaultPos, 4, &gSfxDefaultFreqAndVolScale,
                               &gSfxDefaultFreqAndVolScale, &gSfxDefaultReverb);
    }

    this->startTimer++;
    if (this->result == MALONRACE_NO_RESULT) {
        Player* player2 = player;
        f32 dist;

        for (i = 0; i < 16; i++) {
            if ((this->lapCount == 0) && (i >= 8)) {
                break;
            }
            dist = Math_Vec3f_DistXZ(&sFencePos[i % 8], &player2->rideActor->world.pos);
            if ((player->rideActor != NULL) && (dist < 250.0f)) {
                horse = (EnHorse*)player2->rideActor;

                if (horse->stateFlags & ENHORSE_JUMPING) {
                    if ((i > 0) && (this->fenceCheck[i - 1] == 1)) {
                        this->fenceCheck[i] = 1;
                    } else if (i == 0) {
                        this->fenceCheck[i] = 1;
                    }

                    if ((this->fenceCheck[i - 1] == 0) && !(this->raceFlags & MALONRACE_BROKE_RULE)) {
                        this->raceFlags |= MALONRACE_BROKE_RULE;
                        Message_StartTextbox(globalCtx, 0x208C, NULL);
                        this->result = 4;
                        this->finishTimer = 30;
                    }
                }
            }
        }
        if ((player2->rideActor != NULL) && (this->raceFlags & MALONRACE_PLAYER_START) &&
            AT_FINISH_LINE(player2->rideActor)) {
            if ((this->lapCount == 1) && (this->fenceCheck[15] == 0) && (player2->rideActor->prevPos.x < -200.0f)) {
                this->raceFlags |= MALONRACE_BROKE_RULE;
                Message_StartTextbox(globalCtx, 0x208C, NULL);
                this->result = MALONRACE_FAILURE;
                this->finishTimer = 30;
            } else if (this->fenceCheck[15] == 1) {
                this->lapCount = 2;
                Audio_QueueSeqCmd(SEQ_PLAYER_BGM_MAIN << 24 | NA_BGM_HORSE_GOAL);
                Audio_PlaySoundGeneral(NA_SE_SY_START_SHOT, &gSfxDefaultPos, 4, &gSfxDefaultFreqAndVolScale,
                                       &gSfxDefaultFreqAndVolScale, &gSfxDefaultReverb);
                this->result = MALONRACE_SUCCESS;
                this->finishTimer = 70;
                gSaveContext.timer1State = 0xF;
            } else if ((this->fenceCheck[7] == 1) && !(this->raceFlags & MALONRACE_SECOND_LAP)) {
                this->lapCount = 1;
                this->raceFlags |= MALONRACE_SECOND_LAP;
                Message_StartTextbox(globalCtx, 0x208D, NULL);
            } else if (this->fenceCheck[7] == 0) {
                this->raceFlags |= MALONRACE_BROKE_RULE;
                Message_StartTextbox(globalCtx, 0x208C, NULL);
                this->result = MALONRACE_FAILURE;
                this->finishTimer = 30;
            } else if (player2->rideActor->prevPos.x > 80.0f) {
                this->raceFlags |= MALONRACE_BROKE_RULE;
                Message_StartTextbox(globalCtx, 0x208C, NULL);
                this->result = MALONRACE_FAILURE;
                this->finishTimer = 30;
            }
        }
        if ((gSaveContext.timer1Value >= 180) && (this->raceFlags & MALONRACE_SET_TIMER)) {
            gSaveContext.timer1Value = 240;
            this->result = MALONRACE_TIME_UP;
            this->finishTimer = 30;
            gSaveContext.timer1State = 0;
        }
    } else {
        if (this->finishTimer > 0) {
            this->finishTimer--;
        } else {
            EnHorseGameCheck_FinishMalonRace(this, globalCtx);
        }
    }
    return true;
}

static EnHorseGameCheckFunc sInitFuncs[] = {
    NULL,
    EnHorseGameCheck_InitIngoRace,
    EnHorseGameCheck_InitGerudoArchery,
    EnHorseGameCheck_InitType3,
    EnHorseGameCheck_InitMalonRace,
};

static EnHorseGameCheckFunc sDestroyFuncs[] = {
    NULL,
    EnHorseGameCheck_DestroyIngoRace,
    EnHorseGameCheck_DestroyGerudoArchery,
    EnHorseGameCheck_DestroyType3,
    EnHorseGameCheck_DestroyMalonRace,
};

static EnHorseGameCheckFunc sUpdateFuncs[] = {
    NULL,
    EnHorseGameCheck_UpdateIngoRace,
    EnHorseGameCheck_UpdateGerudoArchery,
    EnHorseGameCheck_UpdateType3,
    EnHorseGameCheck_UpdateMalonRace,
};

void EnHorseGameCheck_Init(Actor* thisx, GlobalContext* globalCtx) {
    s32 pad;
    EnHorseGameCheckBase* this = (EnHorseGameCheckBase*)thisx;

    if ((globalCtx->sceneNum == SCENE_SPOT20) && (Flags_GetEventChkInf(EVENTCHKINF_18) || DREG(1))) {
        this->actor.params = HORSEGAME_MALON_RACE;
    }
    if (sInitFuncs[this->actor.params] != NULL) {
        sInitFuncs[this->actor.params](this, globalCtx);
    }
}

void EnHorseGameCheck_Destroy(Actor* thisx, GlobalContext* globalCtx) {
    s32 pad;
    EnHorseGameCheckBase* this = (EnHorseGameCheckBase*)thisx;

    if (sDestroyFuncs[this->actor.params] != NULL) {
        sDestroyFuncs[this->actor.params](this, globalCtx);
    }
}

void EnHorseGameCheck_Update(Actor* thisx, GlobalContext* globalCtx) {
    s32 pad;
    EnHorseGameCheckBase* this = (EnHorseGameCheckBase*)thisx;

    if (sUpdateFuncs[this->type] != NULL) {
        sUpdateFuncs[this->type](this, globalCtx);
    }
}

void EnHorseGameCheck_Draw(Actor* thisx, GlobalContext* globalCtx) {
}
