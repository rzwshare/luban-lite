/*
* Copyright (C) 2020-2023 ArtInChip Technology Co. Ltd
*
*  author: <jun.ma@artinchip.com>
*  Desc: aic_player
*/

#include <string.h>
#include <malloc.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <inttypes.h>

#include "OMX_Core.h"
#include "OMX_CoreExt1.h"
#include "mpp_dec_type.h"
#include "mpp_log.h"
#include "mpp_mem.h"
#include "mpp_dec_type.h"
#include "aic_parser.h"
#include "aic_player.h"

#define AIC_PLAYER_STATE_IDLE                          0
#define AIC_PLAYER_STATE_INITIALIZED                   1
#define AIC_PLAYER_STATE_PREPARING                     2
#define AIC_PLAYER_STATE_PREPARED                      3
#define AIC_PLAYER_STATE_STARTED                       4
#define AIC_PLAYER_STATE_PLAYING                       5
#define AIC_PLAYER_STATE_PAUSED                        6
#define AIC_PLAYER_STATE_STOPPED                       7
#define AIC_PLAYER_STATE_PLAYBACK_COMPLETED            8

#define AIC_VIDEO 0x01
#define AIC_AUDIO 0x02

#define AIC_PLAYER_PREPARE_FORMAT_DETECTING 0
#define AIC_PLAYER_PREPARE_FORMAT_DETECTED 1
#define AIC_PLAYER_PREPARE_FORMAT_NOT_DETECTED 2

struct aic_player {
    event_handler event_handle;
    void* app_data;
    OMX_HANDLETYPE demuxer_handle;
    OMX_HANDLETYPE vdecoder_handle;
    OMX_HANDLETYPE adecoder_handle;
    OMX_HANDLETYPE video_render_handle;
    OMX_HANDLETYPE audio_render_handle;
    OMX_HANDLETYPE clock_handle;
    s32 format_detected;//0-dectecting , 1-dectected ,2- not_detected
    struct aic_parser_av_media_info media_info;
    u32 video_audio_end_mask;
    u32 video_audio_seek_mask;
    int state;
    struct mpp_rect disp_rect;
    u32 sync_flag;
    pthread_t threadId;
    s32 thread_runing;
    OMX_PARAM_CONTENTURITYPE * uri_param;
    s64 video_pts;
    s64 audio_pts;
    s32 volume;
    s8 mute;
    s8 seeking;
};


#define  wait_state(\
        hComponent,\
        des_state)\
         {\
            OMX_STATETYPE state;\
            while(1) {\
                OMX_GetState(hComponent, &state);\
                if (state == des_state) {\
                    break;\
                } else {\
                    usleep(1000);\
                }\
            }\
        }                /* Macro End */

#define _CLOCK_COMPONENT_

static OMX_ERRORTYPE component_event_handler (
    OMX_HANDLETYPE hComponent,
    OMX_PTR pAppData,
    OMX_EVENTTYPE eEvent,
    OMX_U32 Data1,
    OMX_U32 Data2,
    OMX_PTR pEventData)
{
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    struct aic_player *player = (struct aic_player *)pAppData;
    //struct aic_parser_av_media_info sMediaInfo;

    switch((OMX_S32)eEvent) {
        case OMX_EventCmdComplete:
            break;
        case OMX_EventBufferFlag:
            if (player->media_info.has_video) {
                if (player->video_render_handle == hComponent) {
                    player->video_audio_end_mask &= ~AIC_VIDEO;
                    printf("[%s:%d]rececive video_render_end,video_audio_end_mask:%d!!!\n",__FUNCTION__,__LINE__,player->video_audio_end_mask);
                    if (player->video_audio_end_mask == 0) {
                        player->event_handle(player->app_data,AIC_PLAYER_EVENT_PLAY_END,0,0);
                        player->state = AIC_PLAYER_STATE_PLAYBACK_COMPLETED;
                        printf("[%s:%d]play end!!!\n",__FUNCTION__,__LINE__);
                    }
                }
            }
            if (player->media_info.has_audio) {
                if (player->audio_render_handle == hComponent) {
                    player->video_audio_end_mask &= ~AIC_AUDIO;
                    printf("[%s:%d]rececive audio_render_handle,video_audio_end_mask:%d!!!\n",__FUNCTION__,__LINE__,player->video_audio_end_mask);
                    if (player->video_audio_end_mask == 0) {
                        player->event_handle(player->app_data,AIC_PLAYER_EVENT_PLAY_END,0,0);
                        player->state = AIC_PLAYER_STATE_PLAYBACK_COMPLETED;
                        printf("[%s:%d]play end!!!\n",__FUNCTION__,__LINE__);
                    }
                }
            }

            break;
        case OMX_EventPortFormatDetected:
            printf("[%s:%d]OMX_EventPortFormatDetected\n",__FUNCTION__,__LINE__);
            memcpy(&player->media_info,pEventData,sizeof(struct aic_parser_av_media_info));
            player->format_detected = AIC_PLAYER_PREPARE_FORMAT_DETECTED;
            //player->event_handle(player->app_data,AIC_PLAYER_EVENT_DEMUXER_FORMAT_DETECTED,0,0);
            break;
        case OMX_EventError:
            if (Data1 == OMX_ErrorFormatNotDetected) {
                player->format_detected = AIC_PLAYER_PREPARE_FORMAT_NOT_DETECTED;
               // player->event_handle(player->app_data,AIC_PLAYER_EVENT_DEMUXER_FORMAT_NOT_DETECTED,0,0);
            } else if (Data1 == OMX_ErrorMbErrorsInFrame || Data1 == OMX_ErrorInsufficientResources) {
                player->event_handle(player->app_data,AIC_PLAYER_EVENT_PLAY_END,0,0);
                player->state = AIC_PLAYER_STATE_PLAYBACK_COMPLETED;
                printf("[%s:%d]play end!!!\n",__FUNCTION__,__LINE__);
            }
            break;
        case OMX_EventVideoRenderPts:
            player->video_pts = Data1;
            player->video_pts = (player->video_pts << (sizeof(OMX_U32)*8));
            player->video_pts |= Data2;
            //loge("video_pts:%lld\n",video_pts);
            break;
        case OMX_EventAudioRenderPts:
            player->audio_pts = Data1;
            player->audio_pts = (player->audio_pts << (sizeof(OMX_U32)*8));
            player->audio_pts |= Data2;
            player->event_handle(player->app_data,AIC_PLAYER_EVENT_PLAY_TIME,Data1,Data2);
            break;
        case OMX_EventVideoRenderFirstFrame:
        case OMX_EventAudioRenderFirstFrame:
            if (player->media_info.has_video) {
                if (player->video_render_handle == hComponent) {
                    printf("[%s:%d]first video frame come!!!\n",__FUNCTION__,__LINE__);
                    player->video_audio_seek_mask &= ~AIC_VIDEO;
                    if (player->video_audio_seek_mask == 0) {
                        player->seeking = 0;
                    }
                }
            }
            if (player->media_info.has_audio) {
                if (player->audio_render_handle == hComponent) {
                    printf("[%s:%d]first audio  frame come!!!\n",__FUNCTION__,__LINE__);
                    player->video_audio_seek_mask &= ~AIC_AUDIO;
                    if (player->video_audio_seek_mask == 0) {
                        player->seeking = 0;
                    }
                }
            }
            break;
        default:
            break;
    }
    return eError;
}


OMX_CALLBACKTYPE component_event_callbacks = {
    .EventHandler    = component_event_handler,
    .EmptyBufferDone = NULL,
    .FillBufferDone  = NULL
};

struct aic_player* aic_player_create(char *uri)
{
    OMX_ERRORTYPE eError;
    struct aic_player * player = mpp_alloc(sizeof(struct aic_player));

    if (player == NULL) {
        loge("mpp_alloc aic_player error\n");
        return NULL;
    }

    memset(player,0x00,sizeof(struct aic_player));

    player->uri_param = (OMX_PARAM_CONTENTURITYPE *)mpp_alloc(sizeof(OMX_PARAM_CONTENTURITYPE) + OMX_MAX_STRINGNAME_SIZE);

    if (player->uri_param == NULL) {
        loge("mpp_alloc aic_player error\n");
        goto _exit;
    }

    eError = OMX_Init();

    if (eError != OMX_ErrorNone) {
        loge("OMX_init error!!!\n");
        goto _exit;
    }

    if (uri != NULL) {
        if (aic_player_set_uri(player,uri)) {
            loge("aic_player_set_uri error!!!\n");
            goto _exit;
        }
    } else {
        player->state = AIC_PLAYER_STATE_IDLE;
    }

    if (OMX_ErrorNone !=OMX_GetHandle(&player->demuxer_handle, OMX_COMPONENT_DEMUXER_NAME,player, &component_event_callbacks)) {
        loge("unable to get demuxer handle.\n");
        goto _exit;
    }

    return player;

_exit:
    if (player->uri_param) {
        mpp_free(player->uri_param);
        player->uri_param = NULL;
    }
    mpp_free(player);
    return NULL;
}

s32 aic_player_set_uri(struct aic_player *player,char *uri)
{
    int uri_len;

    if (uri == NULL) {
        loge("param  error\n");
        return -1;
    }
    if (player->uri_param  == NULL) {
        loge("player->uri_param=NULL\n");
        return -1;
    }
    uri_len = strlen(uri);
    if (uri_len > OMX_MAX_STRINGNAME_SIZE-1) {
        loge("path too long\n");
        return -1;
    }
    memset(player->uri_param->contentURI,0x00,OMX_MAX_STRINGNAME_SIZE);
    player->uri_param->nSize = sizeof(OMX_PARAM_CONTENTURITYPE) + uri_len;
    strcpy((char *)player->uri_param->contentURI,uri);
    player->state = AIC_PLAYER_STATE_INITIALIZED;
    return 0;
}

static void* player_index_param_content_uri_thread(void *pThreadData)
{
    struct aic_player *player = (struct aic_player *)pThreadData;
    player->format_detected = AIC_PLAYER_PREPARE_FORMAT_DETECTING;
    player->state = AIC_PLAYER_STATE_PREPARING;
    /*OMX_SetParameter is blocking*/
    player->thread_runing = 1;
    OMX_SetParameter(player->demuxer_handle, OMX_IndexParamContentURI, player->uri_param);
    if (player->format_detected != AIC_PLAYER_PREPARE_FORMAT_DETECTED) {
        loge("OMX_ErrorFormatNotDetected !!!!");
         player->event_handle(player->app_data,AIC_PLAYER_EVENT_DEMUXER_FORMAT_NOT_DETECTED,0,0);
        return (void*)-1;
    } else {
        player->state = AIC_PLAYER_STATE_PREPARED;
        player->event_handle(player->app_data,AIC_PLAYER_EVENT_DEMUXER_FORMAT_DETECTED,0,0);
    }
    player->thread_runing = 0;
    return (void*)0;
}

s32 aic_player_prepare_async(struct aic_player *player)
{
    int ret = 0;
    if (player->state == AIC_PLAYER_STATE_PREPARING) {
        logw("player->state hase been in AIC_PLAYER_STATE_PREPARING \n");
        return 0;
    }
    if (NULL == player->demuxer_handle) {
        loge("player->demuxer_handle has not been created \n");
        return -1;
    }

    player->sync_flag = AIC_PLAYER_PREPARE_ASYNC;

    ret = pthread_create(&player->threadId, NULL, player_index_param_content_uri_thread, player);
    if (ret) {
        loge("pthread_create fail!");
        return -1;
    }
    return 0;
}


s32 aic_player_prepare_sync(struct aic_player *player)
{
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    if (player->state != AIC_PLAYER_STATE_INITIALIZED) {
        loge("player->state is not  in AIC_PLAYER_STATE_INITIALIZED,plaese set uri!!!\n");
        return -1;
    }

    if (NULL == player->demuxer_handle) {
        loge("player->demuxer_handle has not been created \n");
        return -1;
    }

    player->sync_flag = AIC_PLAYER_PREPARE_SYNC;
    player->format_detected = AIC_PLAYER_PREPARE_FORMAT_DETECTING;
    /*OMX_SetParameter is blocking*/
    eError = OMX_SetParameter(player->demuxer_handle, OMX_IndexParamContentURI, player->uri_param);
    if ((eError != OMX_ErrorNone) || (player->format_detected != AIC_PLAYER_PREPARE_FORMAT_DETECTED)) {
        loge("OMX_ErrorFormatNotDetected!!!!");
        return -1;
    } else {
        player->state = AIC_PLAYER_STATE_PREPARED;
    }
    return 0;
}


s32 aic_player_start(struct aic_player *player)
{
    OMX_VIDEO_PARAM_PORTFORMATTYPE video_port_format;
    OMX_AUDIO_PARAM_PORTFORMATTYPE audio_port_format;
    //OMX_PARAM_FRAMEEND sFrameEnd;

    if (player->state == AIC_PLAYER_STATE_STARTED) {
        loge("player->state has been  in AIC_PLAYER_STATE_STARTED \n");
        return 0;
    }

    if (player->state != AIC_PLAYER_STATE_PREPARED) {
        loge("player->state is not in AIC_PLAYER_STATE_PREPARED ,it can not do this opt\n");
        return -1;
    }

    if (player->sync_flag == AIC_PLAYER_PREPARE_ASYNC) {
        if (player->threadId != 0) {
            printf("[%s:%d]wait pthread_join\n",__FUNCTION__,__LINE__);
            pthread_join(player->threadId, NULL);
            printf("[%s:%d]pthread_join ok\n",__FUNCTION__,__LINE__);
            player->threadId = 0;
        }
    }

    if (player->media_info.has_video) {
        video_port_format.nSize = sizeof(OMX_AUDIO_PARAM_PORTFORMATTYPE);
        video_port_format.nPortIndex = DEMUX_PORT_VIDEO_INDEX;
        video_port_format.nIndex = 0;
        if (OMX_ErrorNone !=OMX_GetParameter(player->demuxer_handle, OMX_IndexParamVideoPortFormat,&video_port_format)) {
            loge("OMX_GetParameter Error!!!!.\n");
            goto _EXIT;
        }

        if (player->vdecoder_handle) {
            loge("please call aic_player_stop,free(vdecoder_handle)\n");
            goto _EXIT;
        }

        if (OMX_ErrorNone !=OMX_GetHandle(&player->vdecoder_handle, OMX_COMPONENT_VDEC_NAME,player, &component_event_callbacks)) {
                loge("unable to get vdecoder_handle handle.\n");
                goto _EXIT;
        }

        video_port_format.eColorFormat = OMX_COLOR_FormatYUV420Planar;
        if (video_port_format.eCompressionFormat == OMX_VIDEO_CodingMJPEG) {
            if (strcmp(PRJ_CHIP,"d12x") == 0) {
                switch (AICFB_FORMAT) {
                    case 0x00:
                        video_port_format.eColorFormat = OMX_COLOR_Format32bitARGB8888;
                        break;
                    case 0x08:
                        video_port_format.eColorFormat = OMX_COLOR_Format24bitRGB888;
                        break;
                    case 0x0a:
                        video_port_format.eColorFormat = OMX_COLOR_Format16bitARGB1555;
                        break;
                    case 0x0e:
                        video_port_format.eColorFormat = OMX_COLOR_Format16bitRGB565;
                        break;
                    default:
                        loge("unsupport format:%d\n",AICFB_FORMAT);
                        goto _EXIT;
                }
            } else {
                video_port_format.eColorFormat = OMX_COLOR_FormatYUV420SemiPlanar;
            }
        }

        video_port_format.nPortIndex = VDEC_PORT_IN_INDEX;

        if (OMX_ErrorNone != OMX_SetParameter(player->vdecoder_handle, OMX_IndexParamVideoPortFormat,&video_port_format)) {
            OMX_PARAM_SKIP_TRACK skip_track;
            loge("OMX_IndexParamVideoPortFormat Error!!!!.\n");
            skip_track.nPortIndex = DEMUX_PORT_VIDEO_INDEX;
            OMX_SetParameter(player->demuxer_handle, OMX_IndexVendorDemuxerSkipTrack,&skip_track);
            OMX_FreeHandle(player->vdecoder_handle);
            player->vdecoder_handle = NULL;
            player->media_info.has_video = 0;
            goto _AUDIO;
        }

        if (player->video_render_handle) {
            loge("please call aic_player_stop,free(vdecoder_handle)\n");
            goto _EXIT;
        }

        if (OMX_ErrorNone !=OMX_GetHandle(&player->video_render_handle, OMX_COMPONENT_VIDEO_RENDER_NAME,player, &component_event_callbacks)) {
            loge("unable to get video_render_handle handle.\n");
            goto _EXIT;
        }

        if (OMX_ErrorNone != OMX_SetConfig(player->video_render_handle,OMX_IndexVendorVideoRenderInit, NULL)) {
            loge("OMX_SetConfig Error!!!!.\n");
            goto _EXIT;
        }

        if (OMX_ErrorNone !=OMX_SetupTunnel(player->demuxer_handle, DEMUX_PORT_VIDEO_INDEX, player->vdecoder_handle, VDEC_PORT_IN_INDEX)) {
                loge("OMX_SetupTunnel error.\n");
                goto _EXIT;
        }

        if (OMX_ErrorNone !=OMX_SetupTunnel(player->vdecoder_handle, VDEC_PORT_OUT_INDEX, player->video_render_handle, VIDEO_RENDER_PORT_IN_VIDEO_INDEX)) {
                loge("OMX_SetupTunnel error.\n");
                goto _EXIT;
        }

        player->video_audio_end_mask |= AIC_VIDEO;
    }

_AUDIO:
    if (player->media_info.has_audio) {
        audio_port_format.nSize = sizeof(OMX_AUDIO_PARAM_PORTFORMATTYPE);
        audio_port_format.nPortIndex = DEMUX_PORT_AUDIO_INDEX;
        audio_port_format.nIndex = 0;

        if (OMX_ErrorNone !=OMX_GetParameter(player->demuxer_handle, OMX_IndexParamAudioPortFormat,&audio_port_format)) {
            loge("OMX_GetParameter Error!!!!.\n");
            goto _EXIT;
        }

        if (player->adecoder_handle) {
            loge("please call aic_player_stop,free(adecoder_handle)\n");
            goto _EXIT;
        }

        if (OMX_ErrorNone !=OMX_GetHandle(&player->adecoder_handle, OMX_COMPONENT_ADEC_NAME,player, &component_event_callbacks)) {
            loge("unable to get adecoder_handle handle.\n");
            goto _EXIT;
        }

        audio_port_format.nPortIndex = ADEC_PORT_IN_INDEX;

        if(OMX_ErrorNone != OMX_SetParameter(player->adecoder_handle, OMX_IndexParamAudioPortFormat,&audio_port_format)) {
            OMX_PARAM_SKIP_TRACK skip_track;
            loge("OMX_IndexParamAudioPortFormat Error!!!!.\n");
            skip_track.nPortIndex = DEMUX_PORT_AUDIO_INDEX;
            OMX_SetParameter(player->demuxer_handle, OMX_IndexVendorDemuxerSkipTrack,&skip_track);
            OMX_FreeHandle(player->adecoder_handle);
            player->adecoder_handle = NULL;
            player->media_info.has_audio = 0;
            goto _CLOCK;
        }

        if (player->audio_render_handle) {
                    loge("please call aic_player_stop,free(audio_render_handle)\n");
                    goto _EXIT;
        }

        if (OMX_ErrorNone !=OMX_GetHandle(&player->audio_render_handle, OMX_COMPONENT_AUDIO_RENDER_NAME,player, &component_event_callbacks)) {
            loge("unable to get audio_render_handle handle.\n");
            goto _EXIT;
        }

        if (OMX_ErrorNone != OMX_SetConfig(player->audio_render_handle,OMX_IndexVendorAudioRenderInit, NULL)) {
            loge("OMX_SetConfig Error!!!!.\n");
            goto _EXIT;
        }

        if (OMX_ErrorNone !=OMX_SetupTunnel(player->demuxer_handle, DEMUX_PORT_AUDIO_INDEX, player->adecoder_handle, ADEC_PORT_IN_INDEX)) {
                loge("OMX_SetupTunnel error.\n");
                goto _EXIT;
        }

        if (OMX_ErrorNone !=OMX_SetupTunnel(player->adecoder_handle, ADEC_PORT_OUT_INDEX, player->audio_render_handle, AUDIO_RENDER_PORT_IN_AUDIO_INDEX)) {
                loge("OMX_SetupTunnel error.\n");
                goto _EXIT;
        }

        player->video_audio_end_mask |= AIC_AUDIO;
    }

#ifdef _CLOCK_COMPONENT_
_CLOCK:
    if (player->media_info.has_video && player->media_info.has_audio) {
        OMX_TIME_CONFIG_CLOCKSTATETYPE clock_state;

        if (player->clock_handle) {
            loge("please call aic_player_stop,free(clock_handle)\n");
            goto _EXIT;
        }

        if (OMX_ErrorNone !=OMX_GetHandle(&player->clock_handle, OMX_COMPONENT_CLOCK_NAME,player, &component_event_callbacks)) {
            loge("unable to get clock_handle handle.\n");
            goto _EXIT;
        }

        if (OMX_ErrorNone !=OMX_SetupTunnel(player->clock_handle, CLOCK_PORT_OUT_VIDEO, player->video_render_handle, VIDEO_RENDER_PORT_IN_CLOCK_INDEX)) {
                loge("unable to get video_render handle.\n");
                goto _EXIT;
        }

        if (OMX_ErrorNone !=OMX_SetupTunnel(player->clock_handle, CLOCK_PORT_OUT_AUDIO, player->audio_render_handle, AUDIO_RENDER_PORT_IN_CLOCK_INDEX)) {
                loge("unable to get video_render handle.\n");
                goto _EXIT;
        }
        memset(&clock_state,0x00,sizeof(OMX_TIME_CONFIG_CLOCKSTATETYPE));
        clock_state.eState = OMX_TIME_ClockStateWaitingForStartTime;
        clock_state.nWaitMask |= OMX_CLOCKPORT0;
        clock_state.nWaitMask |= OMX_CLOCKPORT1;
        OMX_SetConfig(player->clock_handle, OMX_IndexConfigTimeClockState,&clock_state);
    }
#endif
    if(!player->vdecoder_handle && !player->adecoder_handle) {
        loge("video and audio all  do not support !!!!.\n");
        goto _EXIT;
    }
#ifdef _CLOCK_COMPONENT_
    if (player->clock_handle) {
        OMX_SendCommand(player->clock_handle, OMX_CommandStateSet, OMX_StateIdle, NULL);
    }
#endif
    if (player->media_info.has_video && player->video_render_handle && player->vdecoder_handle) {
        OMX_SendCommand(player->video_render_handle, OMX_CommandStateSet, OMX_StateIdle, NULL);
        OMX_SendCommand(player->vdecoder_handle, OMX_CommandStateSet, OMX_StateIdle, NULL);
    }
    if (player->media_info.has_audio && player->audio_render_handle && player->adecoder_handle) {
        OMX_SendCommand(player->audio_render_handle, OMX_CommandStateSet, OMX_StateIdle, NULL);
        OMX_SendCommand(player->adecoder_handle, OMX_CommandStateSet, OMX_StateIdle, NULL);
    }
    if (player->demuxer_handle) {
        OMX_SendCommand(player->demuxer_handle, OMX_CommandStateSet, OMX_StateIdle, NULL);
    }
    player->state = AIC_PLAYER_STATE_STARTED;

    if (aic_player_play(player)) {
        loge("aic_player_play fail !!!!.\n");
        goto _EXIT;
    }

    return 0;

_EXIT:

#ifdef _CLOCK_COMPONENT_
    if (player->clock_handle) {
        OMX_FreeHandle(player->clock_handle);
        player->clock_handle = NULL;
    }
#endif

    if (player->audio_render_handle) {
        OMX_FreeHandle(player->audio_render_handle);
        player->audio_render_handle = NULL;
    }

    if (player->adecoder_handle) {
        OMX_FreeHandle(player->adecoder_handle);
        player->adecoder_handle = NULL;
    }

    if (player->video_render_handle) {
        OMX_FreeHandle(player->video_render_handle);
        player->video_render_handle = NULL;
    }

    if (player->vdecoder_handle) {
        OMX_FreeHandle(player->vdecoder_handle);
        player->vdecoder_handle = NULL;
    }

    return -1;
}

s32 aic_player_play(struct aic_player *player)
{

    if (player->state == AIC_PLAYER_STATE_PLAYING) {
        logi("it is already in AIC_PLAYER_STATE_PLAYING\n");
        return 0;
    }
    if (player->state != AIC_PLAYER_STATE_STARTED && player->state != AIC_PLAYER_STATE_PAUSED) {
        loge("player->state:[%d] in AIC_PLAYER_STATE_STARTED or AIC_PLAYER_STATE_PAUSED ,it can not do this opt\n",player->state);
        return -1;
    }

#ifdef _CLOCK_COMPONENT_
        if (player->clock_handle) {
            OMX_SendCommand(player->clock_handle, OMX_CommandStateSet, OMX_StateExecuting, NULL);
        }
#endif

    if (player->media_info.has_video && player->video_render_handle) {
        OMX_SendCommand(player->video_render_handle, OMX_CommandStateSet, OMX_StateExecuting, NULL);
    }
    if (player->media_info.has_audio && player->audio_render_handle) {
        OMX_SendCommand(player->audio_render_handle, OMX_CommandStateSet, OMX_StateExecuting, NULL);
    }
    if (player->media_info.has_video && player->vdecoder_handle) {
        OMX_SendCommand(player->vdecoder_handle, OMX_CommandStateSet, OMX_StateExecuting, NULL);
    }
    if (player->media_info.has_audio && player->adecoder_handle) {
        OMX_SendCommand(player->adecoder_handle, OMX_CommandStateSet, OMX_StateExecuting, NULL);
    }
    if (player->demuxer_handle) {
        OMX_SendCommand(player->demuxer_handle, OMX_CommandStateSet, OMX_StateExecuting, NULL);
    }

    player->state = AIC_PLAYER_STATE_PLAYING;

    return 0;
}

s32 aic_player_pause(struct aic_player *player)
{
    if (player->state == AIC_PLAYER_STATE_PAUSED) {
        logi("it is already in AIC_PLAYER_STATE_PAUSED\n");
        return aic_player_play(player);
    } else if (player->state != AIC_PLAYER_STATE_PLAYING && player->state != AIC_PLAYER_STATE_PLAYBACK_COMPLETED) {
        loge("player->state:[%d] in AIC_PLAYER_STATE_STARTED or AIC_PLAYER_STATE_PAUSED ,it can not do this opt\n",player->state);
        return -1;
    }
    if (player->media_info.has_audio && player->audio_render_handle && player->adecoder_handle) {
        OMX_SendCommand(player->audio_render_handle, OMX_CommandStateSet, OMX_StatePause, NULL);
        wait_state(player->audio_render_handle,OMX_StatePause);
        OMX_SendCommand(player->adecoder_handle, OMX_CommandStateSet, OMX_StatePause, NULL);
        wait_state(player->adecoder_handle,OMX_StatePause);
    }
    if (player->media_info.has_video && player->video_render_handle && player->vdecoder_handle) {
        OMX_SendCommand(player->video_render_handle, OMX_CommandStateSet, OMX_StatePause, NULL);
        wait_state(player->video_render_handle,OMX_StatePause);
        OMX_SendCommand(player->vdecoder_handle, OMX_CommandStateSet, OMX_StatePause, NULL);
        wait_state(player->vdecoder_handle,OMX_StatePause);
    }
    if (player->demuxer_handle) {
        OMX_SendCommand(player->demuxer_handle, OMX_CommandStateSet, OMX_StatePause, NULL);
        wait_state(player->demuxer_handle,OMX_StatePause);
    }
#ifdef _CLOCK_COMPONENT_
    if (player->clock_handle) {
        OMX_SendCommand(player->clock_handle, OMX_CommandStateSet, OMX_StatePause, NULL);
        wait_state(player->clock_handle,OMX_StatePause);
    }
#endif
        player->state = AIC_PLAYER_STATE_PAUSED;
    return 0;
}

static int do_seek(struct aic_player *player,u64 seek_time)
{
    OMX_TIME_CONFIG_TIMESTAMPTYPE  time_stamp;
    player->seeking = 1;
    time_stamp.nTimestamp = seek_time;
    if (OMX_ErrorNone !=  OMX_SetConfig(player->demuxer_handle,OMX_IndexConfigTimePosition,&time_stamp)) {
        goto _exit;
    }

    player->video_audio_seek_mask = 0;

    if (player->media_info.has_video && player->video_render_handle && player->vdecoder_handle) {
        if(OMX_ErrorNone !=  OMX_SetConfig(player->video_render_handle,OMX_IndexConfigTimePosition,&time_stamp)) {
            goto _exit;
        }
        if (OMX_ErrorNone !=  OMX_SetConfig(player->vdecoder_handle,OMX_IndexConfigTimePosition,&time_stamp)) {
            goto _exit;
        }
        player->video_audio_seek_mask |= AIC_VIDEO;
        player->video_audio_end_mask |= AIC_VIDEO;

    }

    if (player->media_info.has_audio && player->audio_render_handle && player->adecoder_handle) {
        if(OMX_ErrorNone !=  OMX_SetConfig(player->audio_render_handle,OMX_IndexConfigTimePosition,&time_stamp)) {
            goto _exit;
        }
        if (OMX_ErrorNone !=  OMX_SetConfig(player->adecoder_handle,OMX_IndexConfigTimePosition,&time_stamp)) {
            goto _exit;
        }
        player->video_audio_seek_mask |= AIC_AUDIO;
        player->video_audio_end_mask |= AIC_AUDIO;
    }

    if (player->media_info.has_video && player->media_info.has_audio && player->clock_handle) {
        if(OMX_ErrorNone !=  OMX_SetConfig(player->clock_handle,OMX_IndexConfigTimePosition,&time_stamp)) {
            goto _exit;
        }
    }

    if (OMX_ErrorNone !=  OMX_SetConfig(player->demuxer_handle,OMX_IndexVendorClearBuffer,&time_stamp)) {
            goto _exit;
    }
    return 0;

_exit:
    loge("seek error!\n");
    player->seeking = 0;
    player->video_audio_seek_mask = 0;
    return -1;

}

s32 aic_player_seek(struct aic_player *player,u64 seek_time)
{
    int ret = 0;
    OMX_TIME_CONFIG_TIMESTAMPTYPE  time_stamp;
    if (player->seeking) {
        loge("palyer in seeking\n");
        return -1;
    }
    if ((player->state == AIC_PLAYER_STATE_PREPARED) || (player->state == AIC_PLAYER_STATE_STARTED)) {
        time_stamp.nTimestamp = seek_time;
        logd("time_stamp.nTimestamp:%"PRId64"\n",time_stamp.nTimestamp);
        player->seeking = 1;
        if (OMX_ErrorNone !=  OMX_SetConfig(player->demuxer_handle,OMX_IndexConfigTimePosition,&time_stamp)) {
            loge("seek error!\n");
            player->seeking = 0;
            ret = -1;
        }
    } else if ((player->state == AIC_PLAYER_STATE_PLAYING) || (player->state == AIC_PLAYER_STATE_PLAYBACK_COMPLETED)) {
        aic_player_pause(player);
        ret = do_seek(player,seek_time);
        if (ret != 0) {
            loge("seek error!\n");
            ret = -1;
        } else {
            aic_player_play(player);
        }
    } else if (player->state == AIC_PLAYER_STATE_PAUSED) {
        ret = do_seek(player,seek_time);
        if (ret != 0) {
            loge("seek error!\n");
            ret = -1;
        } else {
            aic_player_play(player);
        }
    } else {
        return -1;
    }

    return ret;
}

s32 aic_player_stop(struct aic_player *player)
{

    if (player->state == AIC_PLAYER_STATE_STOPPED) {
         loge("player->state has been  in AIC_PLAYER_STATE_STOPPED \n");
         return 0;
    }

    if (player->sync_flag == AIC_PLAYER_PREPARE_ASYNC) {
        if (player->threadId != 0) {
            printf("[%s:%d]pthread_cancel,player->thread_runing:%d\n",__FUNCTION__,__LINE__,player->thread_runing);
            if (player->thread_runing == 1) {
                printf("[%s:%d]pthread_cancel\n",__FUNCTION__,__LINE__);
                pthread_cancel(player->threadId);
            }
            printf("[%s:%d]wait   pthread_join\n",__FUNCTION__,__LINE__);
            pthread_join(player->threadId, NULL);
            printf("[%s:%d]pthread_join ok\n",__FUNCTION__,__LINE__);
            player->threadId = 0;
        }
    }

    if (player->media_info.has_video) {
        if (player->video_render_handle) {
            OMX_SendCommand(player->video_render_handle, OMX_CommandStateSet, OMX_StateIdle, NULL);
            wait_state(player->video_render_handle,OMX_StateIdle);
            OMX_SendCommand(player->video_render_handle, OMX_CommandStateSet, OMX_StateLoaded, NULL);
            wait_state(player->video_render_handle,OMX_StateLoaded);
        }
        if (player->vdecoder_handle) {
            OMX_SendCommand(player->vdecoder_handle, OMX_CommandStateSet, OMX_StateIdle, NULL);
            wait_state(player->vdecoder_handle,OMX_StateIdle);
            OMX_SendCommand(player->vdecoder_handle, OMX_CommandStateSet, OMX_StateLoaded, NULL);
            wait_state(player->vdecoder_handle,OMX_StateLoaded);
        }
    }

    if (player->media_info.has_audio) {
        if (player->audio_render_handle) {
            OMX_SendCommand(player->audio_render_handle, OMX_CommandStateSet, OMX_StateIdle, NULL);
            wait_state(player->audio_render_handle,OMX_StateIdle);
            OMX_SendCommand(player->audio_render_handle, OMX_CommandStateSet, OMX_StateLoaded, NULL);
            wait_state(player->audio_render_handle,OMX_StateLoaded);
        }
        if (player->adecoder_handle) {
            OMX_SendCommand(player->adecoder_handle, OMX_CommandStateSet, OMX_StateIdle, NULL);
            wait_state(player->adecoder_handle,OMX_StateIdle);
            OMX_SendCommand(player->adecoder_handle, OMX_CommandStateSet, OMX_StateLoaded, NULL);
            wait_state(player->adecoder_handle,OMX_StateLoaded);
        }
    }

    if (player->demuxer_handle) {
        OMX_SendCommand(player->demuxer_handle, OMX_CommandStateSet, OMX_StateIdle, NULL);
        wait_state(player->demuxer_handle,OMX_StateIdle);
        OMX_SendCommand(player->demuxer_handle, OMX_CommandStateSet, OMX_StateLoaded, NULL);
        wait_state(player->demuxer_handle,OMX_StateLoaded);
    }

#ifdef _CLOCK_COMPONENT_
    if (player->media_info.has_video && player->media_info.has_audio) {
        if (player->clock_handle) {
            OMX_SendCommand(player->clock_handle, OMX_CommandStateSet, OMX_StateIdle, NULL);
            OMX_SendCommand(player->clock_handle, OMX_CommandStateSet, OMX_StateLoaded, NULL);
        }
    }
#endif

    if (player->media_info.has_video) {
        if (player->demuxer_handle && player->vdecoder_handle && player->video_render_handle) {
            OMX_SetupTunnel(player->demuxer_handle,DEMUX_PORT_VIDEO_INDEX,NULL,0);
            OMX_SetupTunnel(NULL,0,player->vdecoder_handle,VDEC_PORT_IN_INDEX);
            OMX_SetupTunnel(player->vdecoder_handle,VDEC_PORT_OUT_INDEX,NULL,0);
            OMX_SetupTunnel(NULL,0,player->video_render_handle,VIDEO_RENDER_PORT_IN_VIDEO_INDEX);
        }

    }

    if (player->media_info.has_audio) {
        if (player->demuxer_handle && player->adecoder_handle && player->audio_render_handle) {
            OMX_SetupTunnel(player->demuxer_handle,DEMUX_PORT_AUDIO_INDEX,NULL,0);
            OMX_SetupTunnel(NULL,0,player->adecoder_handle,ADEC_PORT_IN_INDEX);
            OMX_SetupTunnel(player->adecoder_handle,ADEC_PORT_OUT_INDEX,NULL,0);
            OMX_SetupTunnel(NULL,0,player->audio_render_handle,AUDIO_RENDER_PORT_IN_AUDIO_INDEX);
        }

    }

#ifdef _CLOCK_COMPONENT_
    if (player->media_info.has_video && player->media_info.has_audio) {
        if (player->clock_handle && player->video_render_handle && player->audio_render_handle) {
            OMX_SetupTunnel(player->clock_handle,CLOCK_PORT_OUT_VIDEO,NULL,0);
            OMX_SetupTunnel(NULL,0,player->audio_render_handle,AUDIO_RENDER_PORT_IN_CLOCK_INDEX);
            OMX_SetupTunnel(player->clock_handle,CLOCK_PORT_OUT_AUDIO,NULL,0);
            OMX_SetupTunnel(NULL,0,player->video_render_handle,VIDEO_RENDER_PORT_IN_CLOCK_INDEX);
        }
    }
#endif

    player->video_audio_end_mask = 0;

    if (player->media_info.has_video) {
        if (player->video_render_handle) {
            OMX_FreeHandle(player->video_render_handle);
            player->video_render_handle = NULL;
        }
        if (player->vdecoder_handle) {
            OMX_FreeHandle(player->vdecoder_handle);
            player->vdecoder_handle = NULL;
        }
    }

    if (player->media_info.has_audio) {
        if (player->audio_render_handle) {
            OMX_FreeHandle(player->audio_render_handle);
            player->audio_render_handle = NULL;
        }
        if (player->adecoder_handle) {
            OMX_FreeHandle(player->adecoder_handle);
            player->adecoder_handle = NULL;
        }
    }

#ifdef _CLOCK_COMPONENT_
    if (player->media_info.has_video && player->media_info.has_audio) {
        if (player->clock_handle) {
            OMX_FreeHandle(player->clock_handle);
            player->clock_handle = NULL;
        }

    }
#endif

    memset(&player->media_info,0x00,sizeof(struct aic_parser_av_media_info));
    player->state = AIC_PLAYER_STATE_STOPPED;
    player->seeking = 0;
    return 0;
}

s32 aic_player_destroy(struct aic_player *player)
{
    if (player->state != AIC_PLAYER_STATE_STOPPED) {
        aic_player_stop(player);
    }

    if (player->demuxer_handle) {
        OMX_FreeHandle(player->demuxer_handle);
        player->demuxer_handle = NULL;
    }

    if (player->uri_param) {
        mpp_free(player->uri_param);
        player->uri_param = NULL;
    }

    mpp_free(player);

    OMX_Deinit();

    return 0;
}

s32 aic_player_set_event_callback(struct aic_player *player,void* app_data,event_handler event_handle)
{
    player->event_handle = event_handle;
    player->app_data = app_data;
    return 0;
}

s32 aic_player_get_media_info(struct aic_player *player,struct av_media_info *media_info)
{
    if (media_info == NULL) {
        return -1;
    }
    if ((!player->media_info.has_video) && (!player->media_info.has_audio)) {
        return -1;
    }
    media_info->duration = player->media_info.duration;
    media_info->file_size = player->media_info.file_size;
    media_info->has_video = player->media_info.has_video;
    media_info->has_audio = player->media_info.has_audio;
    if (media_info->has_video) {
        media_info->video_stream.width = player->media_info.video_stream.width;
        media_info->video_stream.height = player->media_info.video_stream.height;
    }
    if (media_info->has_audio) {
        media_info->audio_stream.bits_per_sample = player->media_info.audio_stream.bits_per_sample;
        media_info->audio_stream.nb_channel = player->media_info.audio_stream.nb_channel;
        media_info->audio_stream.sample_rate = player->media_info.audio_stream.sample_rate;
    }
    return 0;
}

s32 aic_player_get_screen_size(struct aic_player *player,struct mpp_size *screen_size)
{
    OMX_PARAM_SCREEN_SIZE rect;

    if (!player->media_info.has_video || !player->video_render_handle) {
        loge("no video!!!!\n");
        return -1;
    }

    OMX_GetParameter(player->video_render_handle, OMX_IndexVendorVideoRenderScreenSize, &rect);
    screen_size->width = rect.nWidth;
    screen_size->height = rect.nHeight;
    return 0;
}

s32 aic_player_set_disp_rect(struct aic_player *player,struct mpp_rect *disp_rect)
{
    OMX_CONFIG_RECTTYPE rect;

    if (!player->media_info.has_video || !player->video_render_handle) {
        loge("no video!!!!\n");
        return -1;
    }
    rect.nLeft = disp_rect->x;
    rect.nTop = disp_rect->y;
    rect.nWidth = disp_rect->width;
    rect.nHeight = disp_rect->height;
    OMX_SetParameter(player->video_render_handle, OMX_IndexConfigCommonOutputCrop, &rect);
    return 0;
}


s32 aic_player_get_disp_rect(struct aic_player *player,struct mpp_rect *disp_rect)
{
    OMX_CONFIG_RECTTYPE rect;

    if (!player->media_info.has_video || !player->video_render_handle) {
        loge("no video!!!!\n");
        return -1;
    }
    OMX_GetParameter(player->video_render_handle, OMX_IndexConfigCommonOutputCrop, &rect);
    disp_rect->x = rect.nLeft;
    disp_rect->y = rect.nTop;
    disp_rect->width = rect.nWidth;
    disp_rect->height = rect.nHeight;
    return 0;
}


s64 aic_player_get_play_time(struct aic_player *player)
{
    //to do
    if (player->media_info.has_audio) {
        return player->audio_pts;
    } else if (player->media_info.has_video) {
        return player->video_pts;
    } else {
        return -1;
    }
}

s32 aic_player_set_mute(struct aic_player *player)
{
    OMX_PARAM_AUDIO_VOLUME sVolume;
    if (!player->media_info.has_audio || !player->audio_render_handle) {
        return -1;
    }
    if (player->mute) {
        sVolume.nVolume = player->volume;
        player->mute = 0;
    } else {
        player->mute = 1;
        sVolume.nVolume = 0;
    }
    OMX_SetParameter(player->audio_render_handle, OMX_IndexVendorAudioRenderVolume, &sVolume);
    return 0;
}

s32 aic_player_set_volum(struct aic_player *player,s32 vol)
{
    OMX_PARAM_AUDIO_VOLUME sVolume;

    if (!player->media_info.has_audio || !player->audio_render_handle) {
        loge("aic_player_set_volum\n");
        return -1;
    }
    logd("aic_player_set_volum\n");
    player->volume = vol;
    sVolume.nVolume = vol;
    OMX_SetParameter(player->audio_render_handle, OMX_IndexVendorAudioRenderVolume, &sVolume);
    return 0;
}

s32 aic_player_get_volum(struct aic_player *player,s32 *vol)
{
    OMX_PARAM_AUDIO_VOLUME sVolume;
    if (!player->media_info.has_audio || !player->audio_render_handle || !vol) {
        return -1;
    }

    OMX_GetParameter(player->video_render_handle, OMX_IndexVendorAudioRenderVolume, &sVolume);
    *vol = sVolume.nVolume;
    return 0;
}


s32 aic_player_capture(struct aic_player *player, struct aic_capture_info *capture_info)
{
    return 0;
}

s32 aic_player_set_rotation(struct aic_player *player, int rotation_angle)
{
    OMX_CONFIG_ROTATIONTYPE rotation;
    if (!player->media_info.has_video || !player->video_render_handle) {
        loge("no video!!!!\n");
        return -1;
    }

    if (rotation_angle != MPP_ROTATION_0
        && rotation_angle != MPP_ROTATION_90
        && rotation_angle != MPP_ROTATION_180
        && rotation_angle != MPP_ROTATION_270) {
            loge("param error!!!!\n");
            return -1;
    }
    rotation.nRotation = rotation_angle;
    if (OMX_ErrorNone != OMX_SetConfig(player->video_render_handle,OMX_IndexConfigCommonRotate,&rotation)) {
        loge("no video!!!!\n");
        return -1;
    }
    return 0;
}

s32 aic_player_get_rotation(struct aic_player *player)
{
    OMX_CONFIG_ROTATIONTYPE rotation;
    if (!player->media_info.has_video || !player->video_render_handle) {
        loge("no video!!!!\n");
        return -1;
    }

    if (OMX_ErrorNone != OMX_GetConfig(player->video_render_handle,OMX_IndexConfigCommonRotate,&rotation)) {
        loge("no video!!!!\n");
        return -1;
    }

    return rotation.nRotation;
}

