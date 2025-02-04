/*
* Copyright (C) 2020-2023 ArtInChip Technology Co. Ltd
*
*  author: <jun.ma@artinchip.com>
*  Desc: OMX_DemuxerComponent
*/


#include "OMX_DemuxerComponent.h"

#define  aic_pthread_mutex_lock(mutex)\
{\
    pthread_mutex_lock(mutex);\
}


#define aic_pthread_mutex_unlock(mutex)\
{\
    pthread_mutex_unlock(mutex);\
}

#define OMX_DemuxerListEmpty(list,mutex)\
({\
    int ret = 0;\
    aic_pthread_mutex_lock(&mutex);\
    ret = mpp_list_empty(list);\
    aic_pthread_mutex_unlock(&mutex);\
    (ret);\
})


static void* OMX_DemuxerComponentThread(void* pThreadData);
static OMX_ERRORTYPE OMX_DemuxerSendCommand(
        OMX_IN    OMX_HANDLETYPE hComponent,
        OMX_IN    OMX_COMMANDTYPE Cmd,
        OMX_IN    OMX_U32 nParam1,
        OMX_IN    OMX_PTR pCmdData);

static OMX_ERRORTYPE OMX_DemuxerGetParameter(
        OMX_IN    OMX_HANDLETYPE hComponent,
        OMX_IN    OMX_INDEXTYPE nParamIndex,
        OMX_INOUT OMX_PTR pComponentParameterStructure);

static OMX_ERRORTYPE OMX_DemuxerSetParameter(
        OMX_IN    OMX_HANDLETYPE hComponent,
        OMX_IN    OMX_INDEXTYPE nIndex,
        OMX_IN    OMX_PTR pComponentParameterStructure);

static OMX_ERRORTYPE OMX_DemuxerGetConfig(
        OMX_IN    OMX_HANDLETYPE hComponent,
        OMX_IN    OMX_INDEXTYPE nIndex,
        OMX_INOUT OMX_PTR pComponentConfigStructure);

static OMX_ERRORTYPE OMX_DemuxerSetConfig(
        OMX_IN    OMX_HANDLETYPE hComponent,
        OMX_IN    OMX_INDEXTYPE nIndex,
        OMX_IN    OMX_PTR pComponentConfigStructure);

static OMX_ERRORTYPE OMX_DemuxerGetState(
        OMX_IN    OMX_HANDLETYPE hComponent,
        OMX_OUT OMX_STATETYPE* pState);

static OMX_ERRORTYPE OMX_DemuxerComponentTunnelRequest(
    OMX_IN    OMX_HANDLETYPE hComp,
    OMX_IN    OMX_U32 nPort,
    OMX_IN    OMX_HANDLETYPE hTunneledComp,
    OMX_IN    OMX_U32 nTunneledPort,
    OMX_INOUT  OMX_TUNNELSETUPTYPE* pTunnelSetup);

static OMX_ERRORTYPE OMX_DemuxerEmptyThisBuffer(
        OMX_IN    OMX_HANDLETYPE hComponent,
        OMX_IN    OMX_BUFFERHEADERTYPE* pBuffer);


static OMX_ERRORTYPE OMX_DemuxerFillThisBuffer(
        OMX_IN    OMX_HANDLETYPE hComponent,
        OMX_IN    OMX_BUFFERHEADERTYPE* pBuffer);

static OMX_ERRORTYPE OMX_DemuxerSetCallbacks(
        OMX_IN    OMX_HANDLETYPE hComponent,
        OMX_IN    OMX_CALLBACKTYPE* pCallbacks,
        OMX_IN    OMX_PTR pAppData);

static int OMX_DemuxerClearPackets(DEMUXER_DATA_TYPE *pDemuxerDataType);

static int OMX_DemuxerClearPackets(DEMUXER_DATA_TYPE *pDemuxerDataType)
{
        //wait for    all  audio packet from other component or app to back.
        logi("Before OMX_DemuxerComponentThread exit,it must wait for sOutAudioProcessingPkt empty\n");
        while(!OMX_DemuxerListEmpty(&pDemuxerDataType->sOutAudioProcessingPkt,pDemuxerDataType->sAudioPktLock)) {
            usleep(1000);
        }
        //wait for    all  video packet from other component or app to back.
        logi("Before OMX_DemuxerComponentThread exit,it must wait for sOutVideoProcessingPkt empty\n");
        while(!OMX_DemuxerListEmpty(&pDemuxerDataType->sOutVideoProcessingPkt,pDemuxerDataType->sVideoPktLock)) {
            usleep(1000);
        }
        // move sOutAudioReadyPkt to sOutAudioEmptyPkt
        if (!OMX_DemuxerListEmpty(&pDemuxerDataType->sOutAudioReadyPkt,pDemuxerDataType->sAudioPktLock)) {
            logd("sOutAudioReadyPkt is not empty\n");
            DEMUXER_OUT_PACKET *pktNode1,*pktNode2;
            aic_pthread_mutex_lock(&pDemuxerDataType->sAudioPktLock);
            mpp_list_for_each_entry_safe(pktNode1, pktNode2, &pDemuxerDataType->sOutAudioReadyPkt, sList) {
                mpp_list_del(&pktNode1->sList);
                mpp_list_add_tail(&pktNode1->sList, &pDemuxerDataType->sOutAudioEmptyPkt);
            }
            aic_pthread_mutex_unlock(&pDemuxerDataType->sAudioPktLock);
        }

        // move sOutVideoReadyPkt to sOutVideoEmptyPkt
        if (!OMX_DemuxerListEmpty(&pDemuxerDataType->sOutVideoReadyPkt,pDemuxerDataType->sVideoPktLock)) {
            loge("sOutVideoReadyPkt is not empty\n");
            DEMUXER_OUT_PACKET *pktNode1,*pktNode2;
            aic_pthread_mutex_lock(&pDemuxerDataType->sVideoPktLock);
            mpp_list_for_each_entry_safe(pktNode1, pktNode2, &pDemuxerDataType->sOutVideoReadyPkt, sList) {
                mpp_list_del(&pktNode1->sList);
                mpp_list_add_tail(&pktNode1->sList, &pDemuxerDataType->sOutVideoEmptyPkt);
            }
            aic_pthread_mutex_unlock(&pDemuxerDataType->sVideoPktLock);
        }

    return 0;
}

static OMX_ERRORTYPE OMX_DemuxerSendCommand(
        OMX_IN    OMX_HANDLETYPE hComponent,
        OMX_IN    OMX_COMMANDTYPE Cmd,
        OMX_IN    OMX_U32 nParam1,
        OMX_IN    OMX_PTR pCmdData)
{
    DEMUXER_DATA_TYPE *pDemuxerDataType;
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    struct aic_message sMsg;
    memset(&sMsg,0x00,sizeof(struct aic_message));
    pDemuxerDataType = (DEMUXER_DATA_TYPE *)(((OMX_COMPONENTTYPE*)hComponent)->pComponentPrivate);
    sMsg.message_id = Cmd;
    sMsg.param = nParam1;
    sMsg.data_size = 0;

    //now not use always NULL
    if (pCmdData != NULL)
    {
        sMsg.data = pCmdData;
        sMsg.data_size = strlen((char*)pCmdData);
    }

    aic_msg_put(&pDemuxerDataType->sMsgQue, &sMsg);
    return eError;
}

static OMX_ERRORTYPE OMX_DemuxerGetParameter(
        OMX_IN    OMX_HANDLETYPE hComponent,
        OMX_IN    OMX_INDEXTYPE nParamIndex,
        OMX_INOUT OMX_PTR pComponentParameterStructure)
{
    DEMUXER_DATA_TYPE *pDemuxerDataType;
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    OMX_S32 tmp1,tmp2;
    OMX_PARAM_PORTDEFINITIONTYPE *pAudioPort,*pVideoPort;
    OMX_PARAM_U32TYPE *pAudioStreamNum,*pVideoStreamNum;
    OMX_PARAM_BUFFERSUPPLIERTYPE *pAudioBufSupplier,*pVideoBufSupplier;
    OMX_S32 *pAudioActiveStreamIndex,*pVideoActiveStreamIndex;

    pDemuxerDataType = (DEMUXER_DATA_TYPE *)(((OMX_COMPONENTTYPE*)hComponent)->pComponentPrivate);
    pAudioPort = &pDemuxerDataType->sOutPortDef[DEMUX_PORT_AUDIO_INDEX];
    pVideoPort = &pDemuxerDataType->sOutPortDef[DEMUX_PORT_VIDEO_INDEX];
    pAudioStreamNum = &pDemuxerDataType->sStreamNum[DEMUX_PORT_AUDIO_INDEX];
    pVideoStreamNum = &pDemuxerDataType->sStreamNum[DEMUX_PORT_VIDEO_INDEX];
    pAudioBufSupplier = &pDemuxerDataType->sOutBufSupplier[DEMUX_PORT_AUDIO_INDEX];
    pVideoBufSupplier = &pDemuxerDataType->sOutBufSupplier[DEMUX_PORT_VIDEO_INDEX];
    pAudioActiveStreamIndex = &pDemuxerDataType->sActiveStreamIndex[DEMUX_PORT_AUDIO_INDEX];
    pVideoActiveStreamIndex = &pDemuxerDataType->sActiveStreamIndex[DEMUX_PORT_VIDEO_INDEX];


    switch (nParamIndex) {
    case OMX_IndexConfigTimePosition:
        break;
    case OMX_IndexConfigTimeSeekMode:
        break;
    case OMX_IndexParamContentURI:
        memcpy(pComponentParameterStructure,
            &pDemuxerDataType->pDemuxerChnAttr,
            ((OMX_PARAM_CONTENTURITYPE*)pComponentParameterStructure)->nSize);
        break;
    case OMX_IndexParamPortDefinition: {//OMX_PARAM_PORTDEFINITIONTYPE
        OMX_PARAM_PORTDEFINITIONTYPE *port = (OMX_PARAM_PORTDEFINITIONTYPE*)pComponentParameterStructure;
        if (port->nPortIndex == DEMUX_PORT_AUDIO_INDEX) {
            memcpy(port,pAudioPort,sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
        } else if (port->nPortIndex == DEMUX_PORT_VIDEO_INDEX) {
            memcpy(port,pVideoPort,sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
        } else if (port->nPortIndex == DEMUX_PORT_CLOCK_INDEX) {
            memcpy(port,&pDemuxerDataType->sInPortDef,sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
        } else {
            eError = OMX_ErrorBadParameter;
        }
        break;
    }
    case OMX_IndexParamNumAvailableStreams://OMX_PARAM_U32TYPE
            tmp1 = ((OMX_PARAM_U32TYPE*)pComponentParameterStructure)->nPortIndex;
            if (tmp1 == DEMUX_PORT_AUDIO_INDEX) {
                ((OMX_PARAM_U32TYPE*)pComponentParameterStructure)->nU32 = pAudioStreamNum->nU32;
            } else if (tmp1 == DEMUX_PORT_VIDEO_INDEX) {
                ((OMX_PARAM_U32TYPE*)pComponentParameterStructure)->nU32 = pVideoStreamNum->nU32;
            } else {
                eError = OMX_ErrorBadParameter;
            }
        break;
    case OMX_IndexParamActiveStream://OMX_PARAM_U32TYPE
        tmp1 = ((OMX_PARAM_U32TYPE*)pComponentParameterStructure)->nPortIndex;
        tmp2 = ((OMX_PARAM_U32TYPE*)pComponentParameterStructure)->nU32; // start from 0
        if (tmp1 == DEMUX_PORT_AUDIO_INDEX) {
            ((OMX_PARAM_U32TYPE*)pComponentParameterStructure)->nU32 =
                                    *pAudioActiveStreamIndex;
        } else if (tmp1 == DEMUX_PORT_VIDEO_INDEX) {
            ((OMX_PARAM_U32TYPE*)pComponentParameterStructure)->nU32 =
                                    *pVideoActiveStreamIndex;;
        } else {
            eError = OMX_ErrorBadParameter;
        }

        break;
    case OMX_IndexParamAudioPortFormat://OMX_AUDIO_PARAM_PORTFORMATTYPE
        tmp1 = ((OMX_AUDIO_PARAM_PORTFORMATTYPE*)pComponentParameterStructure)->nPortIndex;
        tmp2 = ((OMX_AUDIO_PARAM_PORTFORMATTYPE*)pComponentParameterStructure)->nIndex;
        if (tmp1 != DEMUX_PORT_AUDIO_INDEX || tmp2 > pAudioStreamNum->nU32 -1) {
            eError = OMX_ErrorBadParameter;
            break;
        }
        ((OMX_AUDIO_PARAM_PORTFORMATTYPE*)pComponentParameterStructure)->eEncoding =
                    pDemuxerDataType->sAudioStream[tmp2].eEncoding;
        break;
    case OMX_IndexParamVideoPortFormat://OMX_VIDEO_PARAM_PORTFORMATTYPE
        tmp1 = ((OMX_VIDEO_PARAM_PORTFORMATTYPE*)pComponentParameterStructure)->nPortIndex;
        tmp2 = ((OMX_VIDEO_PARAM_PORTFORMATTYPE*)pComponentParameterStructure)->nIndex;
        logd("OMX_IndexParamVideoPortFormat,port:%"PRId32",index:%"PRId32",video stream_num:%"PRIu32"\n",tmp1,tmp1,pVideoStreamNum->nU32);
        if (tmp1 != DEMUX_PORT_VIDEO_INDEX || tmp2 > (pVideoStreamNum->nU32 -1)) {
            eError = OMX_ErrorBadParameter;
            break;
        }
        ((OMX_VIDEO_PARAM_PORTFORMATTYPE*)pComponentParameterStructure)->eCompressionFormat =
                    pDemuxerDataType->sVideoStream[tmp2].eCompressionFormat;
        ((OMX_VIDEO_PARAM_PORTFORMATTYPE*)pComponentParameterStructure)->eColorFormat =
                    pDemuxerDataType->sVideoStream[tmp2].eColorFormat;
        break;
    case OMX_IndexParamCompBufferSupplier: {
            OMX_PARAM_BUFFERSUPPLIERTYPE *sBufferSupplier = (OMX_PARAM_BUFFERSUPPLIERTYPE*)pComponentParameterStructure;
            if (sBufferSupplier->nPortIndex == DEMUX_PORT_AUDIO_INDEX) {
                sBufferSupplier->eBufferSupplier = pAudioBufSupplier->eBufferSupplier;
            } else if (sBufferSupplier->nPortIndex == DEMUX_PORT_VIDEO_INDEX) {
                sBufferSupplier->eBufferSupplier = pVideoBufSupplier->eBufferSupplier;
            } else if (sBufferSupplier->nPortIndex == DEMUX_PORT_CLOCK_INDEX) {
                sBufferSupplier->eBufferSupplier = pDemuxerDataType->sInBufSupplier.eBufferSupplier;
            } else {
                loge("error nPortIndex\n");
                eError = OMX_ErrorBadPortIndex;
            }
            break;
        }
    default:
        break;
    }
    return eError;
}

static OMX_AUDIO_CODINGTYPE OMX_DemuxerAudioFormatTrans(enum aic_audio_codec_type nAudioType)
{
    OMX_AUDIO_CODINGTYPE ret = OMX_AUDIO_CodingUnused;
    if (nAudioType == MPP_CODEC_AUDIO_DECODER_MP3) {
        ret = OMX_AUDIO_CodingMP3;
    } else if (nAudioType == MPP_CODEC_AUDIO_DECODER_AAC) {
        ret = OMX_AUDIO_CodingAAC;
    } else {
        loge("unspport codec!!!\n");
        ret = OMX_AUDIO_CodingMax;
    }

    return ret;

}

static OMX_VIDEO_CODINGTYPE OMX_DemuxerVideoFormatTrans(enum mpp_codec_type nVideoType)
{
    OMX_VIDEO_CODINGTYPE ret = OMX_VIDEO_CodingUnused;
    if (nVideoType == MPP_CODEC_VIDEO_DECODER_H264) {
        ret = OMX_VIDEO_CodingAVC;
    } else if (nVideoType == MPP_CODEC_VIDEO_DECODER_MJPEG) {
        ret = OMX_VIDEO_CodingMJPEG;
    } else {
        loge("unspport codec!!!\n");
        ret = OMX_VIDEO_CodingMax;
    }
    return ret;
}


/*
static OMX_COLOR_FORMATTYPE OMX_DemuxerVideoPixelFormatTrans(OMX_S32 nVideoType)
{
    OMX_COLOR_FORMATTYPE ret = OMX_COLOR_FormatUnused;
    return ret;
}
*/

static void OMX_DemuxerEventNotify(
        DEMUXER_DATA_TYPE * pDemuxerDataType,
        OMX_EVENTTYPE event,
        OMX_U32 nData1,
        OMX_U32 nData2,
        OMX_PTR pEventData)
{
    if (pDemuxerDataType && pDemuxerDataType->pCallbacks && pDemuxerDataType->pCallbacks->EventHandler) {
        pDemuxerDataType->pCallbacks->EventHandler(
                    pDemuxerDataType->hSelf,
                    pDemuxerDataType->pAppData,event,
                    nData1, nData2, pEventData);
    }

}

static OMX_ERRORTYPE OMX_DemuxerIndexParamContentURI(DEMUXER_DATA_TYPE *pDemuxerDataType,OMX_PARAM_CONTENTURITYPE *pContentURI)
{
    int ret = 0;
    OMX_BOOL bAudioFind = OMX_FALSE;
    OMX_BOOL bVideoFind = OMX_FALSE;
    OMX_PARAM_PORTDEFINITIONTYPE *pAudioPort,*pVideoPort;
    OMX_PARAM_U32TYPE *pAudioStreamNum,*pVideoStreamNum;
    OMX_S32 *pAudioActiveStreamIndex,*pVideoActiveStreamIndex;
    pAudioPort = &pDemuxerDataType->sOutPortDef[DEMUX_PORT_AUDIO_INDEX];
    pVideoPort = &pDemuxerDataType->sOutPortDef[DEMUX_PORT_VIDEO_INDEX];
    pAudioStreamNum = &pDemuxerDataType->sStreamNum[DEMUX_PORT_AUDIO_INDEX];
    pVideoStreamNum = &pDemuxerDataType->sStreamNum[DEMUX_PORT_VIDEO_INDEX];
    pAudioActiveStreamIndex = &pDemuxerDataType->sActiveStreamIndex[DEMUX_PORT_AUDIO_INDEX];
    pVideoActiveStreamIndex = &pDemuxerDataType->sActiveStreamIndex[DEMUX_PORT_VIDEO_INDEX];

    if (pDemuxerDataType->pDemuxerChnAttr == NULL) {
        pDemuxerDataType->pDemuxerChnAttr =(OMX_PARAM_CONTENTURITYPE *)mpp_alloc(sizeof(OMX_PARAM_CONTENTURITYPE) + OMX_MAX_STRINGNAME_SIZE);
        if (pDemuxerDataType->pDemuxerChnAttr == NULL) {
            loge("OMX_ErrorInsufficientResources\n");
            return OMX_ErrorFormatNotDetected;
        }
    }
    memcpy(pDemuxerDataType->pDemuxerChnAttr,pContentURI,pContentURI->nSize);
    if(pDemuxerDataType->pParser) {
        aic_parser_destroy(pDemuxerDataType->pParser);
        pDemuxerDataType->pParser = NULL;
        pDemuxerDataType->nEos = 0;
        pDemuxerDataType->nNeedPeek = 1;
    }
    ret = aic_parser_create(pDemuxerDataType->pDemuxerChnAttr->contentURI,&pDemuxerDataType->pParser);
    printf("[%s:%d]pParser=%p,contentURI:%s\n",__FUNCTION__,__LINE__,pDemuxerDataType->pParser
                                            ,pDemuxerDataType->pDemuxerChnAttr->contentURI);
    if (NULL == pDemuxerDataType->pParser) {/*create parser fail*/
        OMX_DemuxerEventNotify(pDemuxerDataType,OMX_EventError,OMX_ErrorFormatNotDetected
                                ,pDemuxerDataType->state,NULL);
        loge("OMX_ErrorInsufficientResources\n");
        return OMX_ErrorFormatNotDetected;
    }

    /*******************************************************************************
        Here,it will takes a lot of time,the larger the file,the longer it takes.
        so,if you want to optimize it,please optimize parser.
    *******************************************************************************/
    time_start(aic_parser_init);
    ret = aic_parser_init(pDemuxerDataType->pParser);
    if (0 != ret) {
        OMX_DemuxerEventNotify(pDemuxerDataType,OMX_EventError,OMX_ErrorFormatNotDetected
                            ,pDemuxerDataType->state,NULL);
        aic_parser_destroy(pDemuxerDataType->pParser);
        pDemuxerDataType->pParser = NULL;
        return OMX_ErrorFormatNotDetected;
    }
    time_end(aic_parser_init);
    memset(&pDemuxerDataType->sMediaInfo,0x00,sizeof(struct aic_parser_av_media_info));
    ret = aic_parser_get_media_info(pDemuxerDataType->pParser,&pDemuxerDataType->sMediaInfo);
    if (0 != ret) {/*get_media_info fail*/
        OMX_DemuxerEventNotify(pDemuxerDataType,OMX_EventError,OMX_ErrorFormatNotDetected
                        ,pDemuxerDataType->state,NULL);
        loge("OMX_ErrorIncorrectStateTransition\n");
        aic_parser_destroy(pDemuxerDataType->pParser);
        pDemuxerDataType->pParser = NULL;
        return OMX_ErrorFormatNotDetected;
    }
    //set port info
    if (pDemuxerDataType->sMediaInfo.has_audio) {
        pDemuxerDataType->sAudioStream[0].eEncoding =
                OMX_DemuxerAudioFormatTrans(pDemuxerDataType->sMediaInfo.audio_stream.codec_type);
        pAudioStreamNum->nU32 = 1;
        pAudioPort->format.audio.eEncoding = pDemuxerDataType->sAudioStream[0].eEncoding;
        *pAudioActiveStreamIndex = 0;
        bAudioFind = OMX_TRUE;
    } else {
        pAudioStreamNum->nU32 = 0;
    }
    if (pDemuxerDataType->sMediaInfo.has_video) {
        pDemuxerDataType->sVideoStream[0].eCompressionFormat =
                                OMX_DemuxerVideoFormatTrans(pDemuxerDataType->sMediaInfo.video_stream.codec_type);
        pVideoStreamNum->nU32 = 1;
        pVideoPort->format.video.eCompressionFormat =
                                pDemuxerDataType->sVideoStream[0].eCompressionFormat;
        *pVideoActiveStreamIndex = 0;
        bVideoFind = OMX_TRUE;
    } else {
        pVideoStreamNum->nU32 = 0;
    }
    if (bAudioFind || bVideoFind) {
        OMX_DemuxerEventNotify(pDemuxerDataType,OMX_EventPortFormatDetected,0,0,&pDemuxerDataType->sMediaInfo);
        printf("[%s:%d]OMX_EventPortFormatDetected\n",__FUNCTION__,__LINE__);
        if (bAudioFind && pDemuxerDataType->sMediaInfo.audio_stream.extra_data_size >0 && pDemuxerDataType->sMediaInfo.audio_stream.extra_data!=NULL)
        {
            DEMUXER_OUT_PACKET    *pPktNode = NULL;
            struct aic_parser_packet       sPkt;
            memset(&sPkt,0x00,sizeof(struct aic_parser_packet));
            logi("audio_stream extra_data_size:%d,extra_data:%p\n"
            ,pDemuxerDataType->sMediaInfo.audio_stream.extra_data_size
            ,pDemuxerDataType->sMediaInfo.audio_stream.extra_data);
            sPkt.size = pDemuxerDataType->sMediaInfo.audio_stream.extra_data_size;
            sPkt.flag |= PACKET_FLAG_EXTRA_DATA;
            aic_pthread_mutex_lock(&pDemuxerDataType->sAudioPktLock);
            pPktNode = mpp_list_first_entry(&pDemuxerDataType->sOutAudioEmptyPkt,DEMUXER_OUT_PACKET,sList);
            if (pPktNode->sBuff.nAllocLen < sPkt.size) {
                if (pPktNode->sBuff.pBuffer) {
                    mpp_free(pPktNode->sBuff.pBuffer);
                    pPktNode->sBuff.pBuffer = NULL;
                }
                pPktNode->sBuff.pBuffer = (OMX_U8 *)mpp_alloc(sPkt.size);
                logi("mpp_alloc pPktNode->sBuff.pBuffer=%p \n",pPktNode->sBuff.pBuffer);
                pPktNode->sBuff.nAllocLen = sPkt.size;
            }
            sPkt.data = pPktNode->sBuff.pBuffer;
            memcpy(sPkt.data,pDemuxerDataType->sMediaInfo.audio_stream.extra_data,pDemuxerDataType->sMediaInfo.audio_stream.extra_data_size);
            sPkt.type = MPP_MEDIA_TYPE_AUDIO;
            pPktNode->sBuff.nFilledLen = sPkt.size;
            pPktNode->sBuff.nTimeStamp = sPkt.pts;
            pPktNode->sBuff.nFlags = sPkt.flag;
            pPktNode->sBuff.nOutputPortIndex = DEMUX_PORT_AUDIO_INDEX;
            mpp_list_del(&pPktNode->sList);
            mpp_list_add_tail(&pPktNode->sList,&pDemuxerDataType->sOutAudioReadyPkt);
            pDemuxerDataType->nAudioPacketNum++;
            logi("add a Audio pkt to sOutAudioReadyPkt,pkt.size:%d\n",sPkt.size);
            //logd("sVideoPktLock unlock\n");
            aic_pthread_mutex_unlock(&pDemuxerDataType->sAudioPktLock);
        }
        logi("video_stream extra_data_size:%d,extra_data:%p\n"
            ,pDemuxerDataType->sMediaInfo.video_stream.extra_data_size
            ,pDemuxerDataType->sMediaInfo.video_stream.extra_data);

        if (bVideoFind && pDemuxerDataType->sMediaInfo.video_stream.extra_data_size >0 && pDemuxerDataType->sMediaInfo.video_stream.extra_data!=NULL)
        {
            int i = 0;
            DEMUXER_OUT_PACKET    *pPktNode = NULL;
            struct aic_parser_packet       sPkt;
            memset(&sPkt,0x00,sizeof(struct aic_parser_packet));
            sPkt.size = pDemuxerDataType->sMediaInfo.video_stream.extra_data_size;
            sPkt.flag |= PACKET_FLAG_EXTRA_DATA;
            logi("sPkt.flag:0x%x\n",sPkt.flag);
            aic_pthread_mutex_lock(&pDemuxerDataType->sVideoPktLock);
            pPktNode = mpp_list_first_entry(&pDemuxerDataType->sOutVideoEmptyPkt,DEMUXER_OUT_PACKET,sList);
            if (pPktNode->sBuff.nAllocLen < sPkt.size) {
                if (pPktNode->sBuff.pBuffer) {
                    mpp_free(pPktNode->sBuff.pBuffer);
                    pPktNode->sBuff.pBuffer = NULL;
                }
                pPktNode->sBuff.pBuffer = (OMX_U8 *)mpp_alloc(sPkt.size);
                logi("mpp_alloc pPktNode->sBuff.pBuffer=%p \n",pPktNode->sBuff.pBuffer);
                pPktNode->sBuff.nAllocLen = sPkt.size;
            }
            sPkt.data = pPktNode->sBuff.pBuffer;
            memcpy(sPkt.data,pDemuxerDataType->sMediaInfo.video_stream.extra_data,pDemuxerDataType->sMediaInfo.video_stream.extra_data_size);
            sPkt.type = MPP_MEDIA_TYPE_VIDEO;
            pPktNode->sBuff.nFilledLen = sPkt.size;
            pPktNode->sBuff.nTimeStamp = sPkt.pts;
            pPktNode->sBuff.nFlags = sPkt.flag;
            pPktNode->sBuff.nOutputPortIndex = DEMUX_PORT_VIDEO_INDEX;
            mpp_list_del(&pPktNode->sList);
            mpp_list_add_tail(&pPktNode->sList,&pDemuxerDataType->sOutVideoReadyPkt);
            pDemuxerDataType->nVideoPacketNum++;
            aic_pthread_mutex_unlock(&pDemuxerDataType->sVideoPktLock);
            logi("add a  Video pkt to sOutVideoReadyPkt,pkt.size:%d\n",sPkt.size);
            logi("-----------------------------extra_data,size:%d------------------------------------\n",pDemuxerDataType->sMediaInfo.video_stream.extra_data_size);
            for(i=0; i<pDemuxerDataType->sMediaInfo.video_stream.extra_data_size;i++) {
                printf("%02x ",pDemuxerDataType->sMediaInfo.video_stream.extra_data[i]);
            }
            logi("----------------------------------------------------------------------------\n");
        }
    } else {
        OMX_DemuxerEventNotify(pDemuxerDataType,OMX_EventError,OMX_ErrorFormatNotDetected,0,NULL);
        loge("OMX_ErrorFormatNotDetected\n");
        aic_parser_destroy(pDemuxerDataType->pParser);
        pDemuxerDataType->pParser = NULL;
        return OMX_ErrorFormatNotDetected;
    }

    return OMX_EventPortFormatDetected;
}

static OMX_ERRORTYPE OMX_DemuxerSetParameter(
        OMX_IN    OMX_HANDLETYPE hComponent,
        OMX_IN    OMX_INDEXTYPE nParamIndex,
        OMX_IN    OMX_PTR pComponentParameterStructure)
{
    DEMUXER_DATA_TYPE *pDemuxerDataType;
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    OMX_S32 tmp1,tmp2;
    OMX_S32 index =(OMX_S32)nParamIndex;
    OMX_PARAM_PORTDEFINITIONTYPE *pAudioPort,*pVideoPort;
    OMX_S32 *pAudioActiveStreamIndex,*pVideoActiveStreamIndex;
    OMX_PARAM_U32TYPE *pAudioStreamNum,*pVideoStreamNum;
    pDemuxerDataType = (DEMUXER_DATA_TYPE *)(((OMX_COMPONENTTYPE*)hComponent)->pComponentPrivate);
    pAudioPort = &pDemuxerDataType->sOutPortDef[DEMUX_PORT_AUDIO_INDEX];
    pVideoPort = &pDemuxerDataType->sOutPortDef[DEMUX_PORT_VIDEO_INDEX];
    pAudioStreamNum = &pDemuxerDataType->sStreamNum[DEMUX_PORT_AUDIO_INDEX];
    pVideoStreamNum = &pDemuxerDataType->sStreamNum[DEMUX_PORT_VIDEO_INDEX];
    pAudioActiveStreamIndex = &pDemuxerDataType->sActiveStreamIndex[DEMUX_PORT_AUDIO_INDEX];
    pVideoActiveStreamIndex = &pDemuxerDataType->sActiveStreamIndex[DEMUX_PORT_VIDEO_INDEX];

    switch (index) {
    case OMX_IndexConfigTimePosition:
        break;
    case OMX_IndexConfigTimeSeekMode:
        break;
    case OMX_IndexParamContentURI:
        OMX_DemuxerIndexParamContentURI(pDemuxerDataType,(OMX_PARAM_CONTENTURITYPE*)pComponentParameterStructure);
        break;
    case OMX_IndexParamPortDefinition:
        //OMX_IndexParamPortDefinition  OMX_PARAM_PORTDEFINITIONTYPE
        break;
    case OMX_IndexParamNumAvailableStreams://OMX_PARAM_U32TYPE
        break;
    case OMX_IndexParamActiveStream://OMX_PARAM_U32TYPE
            tmp1 = ((OMX_PARAM_U32TYPE*)pComponentParameterStructure)->nPortIndex;
            tmp2 = ((OMX_PARAM_U32TYPE*)pComponentParameterStructure)->nU32; // start from 0
            if (tmp1 == DEMUX_PORT_AUDIO_INDEX) {
                if (tmp2 > pAudioStreamNum->nU32 -1)
                    tmp2 = 0;
                pAudioPort->format.audio.eEncoding =
                        pDemuxerDataType->sAudioStream[tmp2].eEncoding;
                *pAudioActiveStreamIndex = tmp2;
            } else if (tmp1 == DEMUX_PORT_VIDEO_INDEX) {
                if (tmp2 > pVideoStreamNum->nU32 -1)
                    tmp2 = 0;
                pVideoPort->format.video.eCompressionFormat =
                        pDemuxerDataType->sVideoStream[tmp2].eCompressionFormat;
                pVideoPort->format.video.eColorFormat =
                        pDemuxerDataType->sVideoStream[tmp2].eColorFormat;
                *pVideoActiveStreamIndex = tmp2;
            } else {
                eError = OMX_ErrorBadParameter;
            }
        break;
    case OMX_IndexParamAudioPortFormat://OMX_AUDIO_PARAM_PORTFORMATTYPE
        break;
    case OMX_IndexParamVideoPortFormat://OMX_VIDEO_PARAM_PORTFORMATTYPE
        break;
    case OMX_IndexVendorDemuxerSkipTrack:
        tmp1 = ((OMX_PARAM_SKIP_TRACK*)pComponentParameterStructure)->nPortIndex;
        if (tmp1 == DEMUX_PORT_AUDIO_INDEX) {
            pDemuxerDataType->nSkipTrack |= DEMUX_SKIP_AUDIO_TRACK;
        } else if (tmp1 == DEMUX_PORT_VIDEO_INDEX) {
            pDemuxerDataType->nSkipTrack |= DEMUX_SKIP_VIDEO_TRACK;
        }
        break;
    default:
     break;
    }
    return eError;
}



static OMX_ERRORTYPE OMX_DemuxerGetConfig(
        OMX_IN    OMX_HANDLETYPE hComponent,
        OMX_IN    OMX_INDEXTYPE nIndex,
        OMX_INOUT OMX_PTR pComponentConfigStructure)
{
    //DEMUXER_DATA_TYPE *pDemuxerDataType;
    OMX_ERRORTYPE eError = OMX_ErrorNone;

    switch (nIndex) {
    case OMX_IndexConfigTimePosition:
        break;
    case OMX_IndexConfigTimeSeekMode:
        break;
    default:
        break;
    }
    return eError;
}



static OMX_ERRORTYPE OMX_DemuxerSetConfig(
        OMX_IN    OMX_HANDLETYPE hComponent,
        OMX_IN    OMX_INDEXTYPE nIndex,
        OMX_IN    OMX_PTR pComponentConfigStructure)
{
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    DEMUXER_DATA_TYPE *pDemuxerDataType = (DEMUXER_DATA_TYPE *)(((OMX_COMPONENTTYPE*)hComponent)->pComponentPrivate);
    switch ((OMX_S32)nIndex) {
    case OMX_IndexConfigTimePosition://do seek
        {
            // 1 when seeking ,stop peek/read
            // 2 async send command ,sync here do seek.
            // 3 when demux in seeking .other adec and vdec how to do
            int ret = 0;
            OMX_TIME_CONFIG_TIMESTAMPTYPE *sTimeStamp = (OMX_TIME_CONFIG_TIMESTAMPTYPE *)pComponentConfigStructure;
            logd("sTimeStamp.nTimestamp:%"PRId64"\n",sTimeStamp->nTimestamp);
            //1  seek
            ret = aic_parser_seek(pDemuxerDataType->pParser,sTimeStamp->nTimestamp);
            if (ret == 0) {
                eError = OMX_ErrorNone;
                pDemuxerDataType->nNeedPeek = 1;
            } else {
                eError =  OMX_ErrorUndefined;
            }
            break;
        }
    case OMX_IndexConfigTimeSeekMode:
        break;
    case OMX_IndexVendorClearBuffer:
            //2 clear packet
            OMX_DemuxerClearPackets(pDemuxerDataType);
            //3 clear flag
            pDemuxerDataType->nEos = 0;
            pDemuxerDataType->nSendAudioPacketNum = 0;
            pDemuxerDataType->nSendVideoPacketNum = 0;
        break;
    default:
        break;
    }
    return eError;
}



static OMX_ERRORTYPE OMX_DemuxerGetState(
        OMX_IN    OMX_HANDLETYPE hComponent,
        OMX_OUT OMX_STATETYPE* pState)
{
    DEMUXER_DATA_TYPE *pDemuxerDataType;
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    pDemuxerDataType = (DEMUXER_DATA_TYPE *)(((OMX_COMPONENTTYPE*)hComponent)->pComponentPrivate);
    aic_pthread_mutex_lock(&pDemuxerDataType->stateLock);
    *pState = pDemuxerDataType->state;
    aic_pthread_mutex_unlock(&pDemuxerDataType->stateLock);
    return eError;


}

static OMX_ERRORTYPE OMX_DemuxerComponentTunnelRequest(
    OMX_IN    OMX_HANDLETYPE hComp,
    OMX_IN    OMX_U32 nPort,
    OMX_IN    OMX_HANDLETYPE hTunneledComp,
    OMX_IN    OMX_U32 nTunneledPort,
    OMX_INOUT  OMX_TUNNELSETUPTYPE* pTunnelSetup)
{
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    OMX_PARAM_PORTDEFINITIONTYPE *pPort;
    OMX_PORT_TUNNELEDINFO *pTunneledInfo;
    OMX_PARAM_BUFFERSUPPLIERTYPE *pBufSupplier;
    DEMUXER_DATA_TYPE* pDemuxerDataType;
    pDemuxerDataType = (DEMUXER_DATA_TYPE *)(((OMX_COMPONENTTYPE*)hComp)->pComponentPrivate);
    if (pDemuxerDataType->state != OMX_StateLoaded && pDemuxerDataType->state != OMX_StateIdle)
    {
        loge("Component is not in OMX_StateLoaded,it is in%d,it can not tunnel\n",pDemuxerDataType->state);
        return OMX_ErrorInvalidState;
    }

    if (nPort == DEMUX_PORT_AUDIO_INDEX) {
        pPort = &pDemuxerDataType->sOutPortDef[DEMUX_PORT_AUDIO_INDEX];
        pTunneledInfo = &pDemuxerDataType->sOutPortTunneledInfo[DEMUX_PORT_AUDIO_INDEX];
        pBufSupplier = &pDemuxerDataType->sOutBufSupplier[DEMUX_PORT_AUDIO_INDEX];
    } else if (nPort == DEMUX_PORT_VIDEO_INDEX) {
        pPort = &pDemuxerDataType->sOutPortDef[DEMUX_PORT_VIDEO_INDEX];
        pTunneledInfo = &pDemuxerDataType->sOutPortTunneledInfo[DEMUX_PORT_VIDEO_INDEX];
        pBufSupplier = &pDemuxerDataType->sOutBufSupplier[DEMUX_PORT_VIDEO_INDEX];
    } else if (nPort == DEMUX_PORT_CLOCK_INDEX) {
        pPort = &pDemuxerDataType->sInPortDef;
        pTunneledInfo = &pDemuxerDataType->sInPortTunneledInfo;
        pBufSupplier = &pDemuxerDataType->sInBufSupplier;
    } else {
        loge("component can not find port:%"PRId32"\n",nPort);
        return OMX_ErrorBadParameter;
    }

    // cancle setup tunnel
    if (NULL == hTunneledComp && 0 == nTunneledPort && NULL == pTunnelSetup) {
            pTunneledInfo->nTunneledFlag = OMX_FALSE;
            pTunneledInfo->nTunnelPortIndex = nTunneledPort;
            pTunneledInfo->pTunneledComp = hTunneledComp;
        return OMX_ErrorNone;
    }

    if (pPort->eDir == OMX_DirOutput) {
        pTunneledInfo->nTunnelPortIndex = nTunneledPort;
        pTunneledInfo->pTunneledComp = hTunneledComp;
        pTunneledInfo->nTunneledFlag = OMX_TRUE;
        pTunnelSetup->nTunnelFlags = 0;
        pTunnelSetup->eSupplier = pBufSupplier->eBufferSupplier;
    } else if (pPort->eDir == OMX_DirInput) {
        OMX_PARAM_PORTDEFINITIONTYPE sTunneledPort;
        OMX_PARAM_BUFFERSUPPLIERTYPE sBuffSupplier;
        sTunneledPort.nPortIndex = nTunneledPort;
        sBuffSupplier.nPortIndex = nTunneledPort;
        if (pTunnelSetup->eSupplier == OMX_BufferSupplyMax)
        {
            loge("both ports are input.\n");
            return OMX_ErrorPortsNotCompatible;
        }
        OMX_GetParameter(hTunneledComp, OMX_IndexParamPortDefinition,&sTunneledPort);
        if (pPort->eDomain != sTunneledPort.eDomain) {
            loge("ports domain are not compatible: %d %d.\n",
                   pPort->eDomain, sTunneledPort.eDomain);
            return OMX_ErrorPortsNotCompatible;
        }
        if (sTunneledPort.eDir != OMX_DirOutput) {
            loge("both ports are input.\n");
            return OMX_ErrorPortsNotCompatible;
        }

        //negotiate buffer supplier
        OMX_GetParameter(hTunneledComp, OMX_IndexParamCompBufferSupplier,&sBuffSupplier);
        if (sBuffSupplier.eBufferSupplier != pTunnelSetup->eSupplier) {
             loge("out_port and in_port supplier are different,please check code!!!!\n");
              return OMX_ErrorPortsNotCompatible;
        }
        pTunneledInfo->nTunnelPortIndex = nTunneledPort;
        pTunneledInfo->pTunneledComp = hTunneledComp;
        pTunneledInfo->nTunneledFlag = OMX_TRUE;
        pBufSupplier->eBufferSupplier = pTunnelSetup->eSupplier;
    } else {
        loge("port is neither output nor input.\n");
        return OMX_ErrorPortsNotCompatible;
    }
    return eError;

}



static OMX_ERRORTYPE OMX_DemuxerEmptyThisBuffer(
        OMX_IN    OMX_HANDLETYPE hComponent,
        OMX_IN    OMX_BUFFERHEADERTYPE* pBuffer)
{
        OMX_ERRORTYPE eError = OMX_ErrorNone;
        return eError;

}


static OMX_ERRORTYPE OMX_DemuxerFillThisBuffer(
        OMX_IN    OMX_HANDLETYPE hComponent,
        OMX_IN    OMX_BUFFERHEADERTYPE* pBuffer)
{
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    DEMUXER_DATA_TYPE *pDemuxerDataType;
    OMX_BOOL bFind = OMX_FALSE;
    //logd("OMX_DemuxerFillThisBuffer\n");
    pDemuxerDataType = (DEMUXER_DATA_TYPE *)(((OMX_COMPONENTTYPE*)hComponent)->pComponentPrivate);
    if (pBuffer->nOutputPortIndex == DEMUX_PORT_VIDEO_INDEX) {
        aic_pthread_mutex_lock(&pDemuxerDataType->sVideoPktLock);
        if (!mpp_list_empty(&pDemuxerDataType->sOutVideoProcessingPkt)) {
            DEMUXER_OUT_PACKET  *pPktNode = NULL,*pPktNode1 = NULL;
            mpp_list_for_each_entry_safe(pPktNode, pPktNode1, &pDemuxerDataType->sOutVideoProcessingPkt, sList) {
                if (pPktNode->sBuff.pBuffer == pBuffer->pBuffer) {
                    mpp_list_del(&pPktNode->sList);
                    mpp_list_add_tail(&pPktNode->sList, &pDemuxerDataType->sOutVideoEmptyPkt);
                    bFind = OMX_TRUE;
                    break;
                }
            }
        }
        aic_pthread_mutex_unlock(&pDemuxerDataType->sVideoPktLock);
        if (OMX_FALSE == bFind) {
            eError = OMX_ErrorBadParameter;
            logd("OMX_ErrorBadParameter\n");
        } else {
            pDemuxerDataType->nSendBackVideoPacketNum++;
            // loge("%d:%d:%d\n"
            //         ,pDemuxerDataType->nSendVideoPacketNum
            //         ,pDemuxerDataType->nSendBackVideoPacketNum
            //         ,(mpp_list_empty(&pDemuxerDataType->sOutVideoEmptyPkt))?(0):(1));
            //logd("nSendBackVideoPacketNum:%d\n",pDemuxerDataType->nSendBackVideoPacketNum);
        }
        //loge("nSendBackVideoPacketNum:%d\n",pDemuxerDataType->nSendBackVideoPacketNum);
    } else if (pBuffer->nOutputPortIndex == DEMUX_PORT_AUDIO_INDEX) {
            aic_pthread_mutex_lock(&pDemuxerDataType->sAudioPktLock);
            if (!mpp_list_empty(&pDemuxerDataType->sOutAudioProcessingPkt)) {
                DEMUXER_OUT_PACKET  *pPktNode = NULL,*pPktNode1 = NULL;
                mpp_list_for_each_entry_safe(pPktNode, pPktNode1, &pDemuxerDataType->sOutAudioProcessingPkt, sList) {
                    if (pPktNode->sBuff.pBuffer == pBuffer->pBuffer) {
                        mpp_list_del(&pPktNode->sList);
                        mpp_list_add_tail(&pPktNode->sList, &pDemuxerDataType->sOutAudioEmptyPkt);
                        bFind = OMX_TRUE;
                        break;
                    }
                }
            }
            aic_pthread_mutex_unlock(&pDemuxerDataType->sAudioPktLock);
            if (OMX_FALSE == bFind) {
                eError = OMX_ErrorBadParameter;
                logd("OMX_ErrorBadParameter\n");
            } else {
                pDemuxerDataType->nSendBackAudioPacketNum++;
                //logd("nSendBackAudioPacketNum:%d\n",pDemuxerDataType->nSendBackAudioPacketNum);
            }
            //loge("nSendBackAudioPacketNum:%d\n",pDemuxerDataType->nSendBackAudioPacketNum);
    } else {
        loge(" error port index \n ");
    }
    if (eError == OMX_ErrorNone) {
        struct aic_message sMsg;
        sMsg.data_size = 0;
        sMsg.message_id = OMX_CommandNops;
        aic_msg_put(&pDemuxerDataType->sMsgQue, &sMsg);
    }
    return eError;

}

static OMX_ERRORTYPE OMX_DemuxerSetCallbacks(
        OMX_IN    OMX_HANDLETYPE hComponent,
        OMX_IN    OMX_CALLBACKTYPE* pCallbacks,
        OMX_IN    OMX_PTR pAppData)
{
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    DEMUXER_DATA_TYPE *pDemuxerDataType;
    pDemuxerDataType = (DEMUXER_DATA_TYPE *)(((OMX_COMPONENTTYPE*)hComponent)->pComponentPrivate);
    pDemuxerDataType->pCallbacks = pCallbacks;
    pDemuxerDataType->pAppData = pAppData;
    return eError;
}


OMX_ERRORTYPE OMX_DemuxerComponentDeInit(
        OMX_IN    OMX_HANDLETYPE hComponent) {
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    OMX_COMPONENTTYPE *pComp;
    DEMUXER_DATA_TYPE *pDemuxerDataType;
    pComp = (OMX_COMPONENTTYPE *)hComponent;
    struct aic_message sMsg;

    pDemuxerDataType = (DEMUXER_DATA_TYPE *)pComp->pComponentPrivate;
    aic_pthread_mutex_lock(&pDemuxerDataType->stateLock);
    if (pDemuxerDataType->state != OMX_StateLoaded) {
        logd("compoent is in %d,but not in OMX_StateLoaded(1),can ont FreeHandle.\n",pDemuxerDataType->state);
        aic_pthread_mutex_unlock(&pDemuxerDataType->stateLock);
        return OMX_ErrorInvalidState;
    }
    aic_pthread_mutex_unlock(&pDemuxerDataType->stateLock);

    sMsg.message_id = OMX_CommandStop;
    sMsg.data_size = 0;
    aic_msg_put(&pDemuxerDataType->sMsgQue, &sMsg);
    pthread_join(pDemuxerDataType->threadId, (void*)&eError);

    aic_pthread_mutex_lock(&pDemuxerDataType->sAudioPktLock);
    if (!mpp_list_empty(&pDemuxerDataType->sOutAudioEmptyPkt)) {
        DEMUXER_OUT_PACKET  *pPktNode = NULL,*pPktNode1 = NULL;
        mpp_list_for_each_entry_safe(pPktNode, pPktNode1, &pDemuxerDataType->sOutAudioEmptyPkt, sList) {
            if (pPktNode->sBuff.pBuffer) {
                mpp_free(pPktNode->sBuff.pBuffer);
                pPktNode->sBuff.pBuffer = NULL;
            }
        }
    }

    if (!mpp_list_empty(&pDemuxerDataType->sOutAudioReadyPkt)) {
        DEMUXER_OUT_PACKET  *pPktNode = NULL,*pPktNode1 = NULL;
        mpp_list_for_each_entry_safe(pPktNode, pPktNode1, &pDemuxerDataType->sOutAudioReadyPkt, sList) {
            if (pPktNode->sBuff.pBuffer) {
                mpp_free(pPktNode->sBuff.pBuffer);
                pPktNode->sBuff.pBuffer = NULL;
            }
            pDemuxerDataType->nLeftReadyAudioPktFrameWhenCompoentExitNum++;
        }
    }

   if (pDemuxerDataType->pAudioPktNodeHead) {
        mpp_free(pDemuxerDataType->pAudioPktNodeHead);
        pDemuxerDataType->pAudioPktNodeHead = NULL;
    }

    aic_pthread_mutex_unlock(&pDemuxerDataType->sAudioPktLock);

    aic_pthread_mutex_lock(&pDemuxerDataType->sVideoPktLock);
    if (!mpp_list_empty(&pDemuxerDataType->sOutVideoEmptyPkt)) {
        DEMUXER_OUT_PACKET  *pPktNode = NULL,*pPktNode1 = NULL;
        mpp_list_for_each_entry_safe(pPktNode, pPktNode1, &pDemuxerDataType->sOutVideoEmptyPkt, sList) {
            if (pPktNode->sBuff.pBuffer) {
                mpp_free(pPktNode->sBuff.pBuffer);
                pPktNode->sBuff.pBuffer = NULL;
            }
        }
    }
    if (!mpp_list_empty(&pDemuxerDataType->sOutVideoReadyPkt)) {
        DEMUXER_OUT_PACKET  *pPktNode = NULL,*pPktNode1 = NULL;
        mpp_list_for_each_entry_safe(pPktNode, pPktNode1, &pDemuxerDataType->sOutVideoReadyPkt, sList) {
            if (pPktNode->sBuff.pBuffer) {
                mpp_free(pPktNode->sBuff.pBuffer);
                pPktNode->sBuff.pBuffer = NULL;
            }
            pDemuxerDataType->nLeftReadyVideoPktFrameWhenCompoentExitNum++;
        }
    }

    if (pDemuxerDataType->pVideoPktNodeHead) {
        mpp_free(pDemuxerDataType->pVideoPktNodeHead);
        pDemuxerDataType->pVideoPktNodeHead = NULL;
    }

    aic_pthread_mutex_unlock(&pDemuxerDataType->sVideoPktLock);

    logi("nLeftReadyVideoPktFrameWhenCompoentExitNum:%"PRIu32""\
        "nLeftReadyAudioPktFrameWhenCompoentExitNum:%"PRIu32"\n"
        ,pDemuxerDataType->nLeftReadyVideoPktFrameWhenCompoentExitNum
        ,pDemuxerDataType->nLeftReadyAudioPktFrameWhenCompoentExitNum);

    pthread_mutex_destroy(&pDemuxerDataType->sAudioPktLock);
    pthread_mutex_destroy(&pDemuxerDataType->sVideoPktLock);
    pthread_mutex_destroy(&pDemuxerDataType->stateLock);

    aic_msg_destroy(&pDemuxerDataType->sMsgQue);

    if (pDemuxerDataType->pParser) {
        aic_parser_destroy(pDemuxerDataType->pParser);
        pDemuxerDataType->pParser = NULL;
    }

    if (pDemuxerDataType->pDemuxerChnAttr) {
        mpp_free(pDemuxerDataType->pDemuxerChnAttr);
        pDemuxerDataType->pDemuxerChnAttr = NULL;
    }

    mpp_free(pDemuxerDataType);
    pDemuxerDataType = NULL;

    logd("OMX_DemuxerComponentDeInit\n");

    return eError;
}


static int demuxer_thread_attr_init(pthread_attr_t *attr)
{
    // default stack size is 2K, it is not enough for decode thread
    if (attr == NULL) {
        return EINVAL;
    }
    pthread_attr_init(attr);
    attr->stacksize = 16*1024;
    attr->schedparam.sched_priority = 23;
    return 0;
}


OMX_ERRORTYPE OMX_DemuxerComponentInit(
        OMX_IN    OMX_HANDLETYPE hComponent)
{
    OMX_COMPONENTTYPE *pComp;
    DEMUXER_DATA_TYPE *pDemuxerDataType;
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    OMX_U32 err;
    OMX_U32 i;
    DEMUXER_OUT_PACKET  *pPktNode,*pPktNode1;

    OMX_PARAM_PORTDEFINITIONTYPE *pAudioPort,*pVideoPort;
    OMX_PARAM_U32TYPE *pAudioStreamNum,*pVideoStreamNum;
    OMX_PARAM_BUFFERSUPPLIERTYPE *pAudioBufSupplier,*pVideoBufSupplier;

    OMX_S8 nMsgCreateOk = 0;
    OMX_S8 nAudioPktLockInitOk = 0;
    OMX_S8 nVideoPktLockInitOk = 0;
    OMX_S8 nSateLockInitOk = 0;

    DEMUXER_OUT_PACKET *pVideoPktNodeHead;
    DEMUXER_OUT_PACKET *pAudioPktNodeHead;

    DEMUXER_OUT_PACKET *pVideoPktNode;
    DEMUXER_OUT_PACKET *pAudioPktNode;

    pthread_attr_t attr;

    demuxer_thread_attr_init(&attr);

    logd("OMX_DemuxerComponentInit....");

    pComp = (OMX_COMPONENTTYPE *)hComponent;

    pDemuxerDataType = (DEMUXER_DATA_TYPE *)mpp_alloc(sizeof(DEMUXER_DATA_TYPE));

    if (NULL == pDemuxerDataType) {
        loge("mpp_alloc(sizeof(DemuxerDATATYPE) fail!\n");
        return OMX_ErrorInsufficientResources;
    }

    memset(pDemuxerDataType, 0x0, sizeof(DEMUXER_DATA_TYPE));
    pComp->pComponentPrivate    = (void*) pDemuxerDataType;
    pDemuxerDataType->state            = OMX_StateLoaded;
    pDemuxerDataType->hSelf            = pComp;

    pComp->SetCallbacks         = OMX_DemuxerSetCallbacks;
    pComp->SendCommand            = OMX_DemuxerSendCommand;
    pComp->GetState             = OMX_DemuxerGetState;
    pComp->GetParameter          = OMX_DemuxerGetParameter;
    pComp->SetParameter         = OMX_DemuxerSetParameter;
    pComp->GetConfig            = OMX_DemuxerGetConfig;
    pComp->SetConfig            = OMX_DemuxerSetConfig;
    pComp->ComponentTunnelRequest = OMX_DemuxerComponentTunnelRequest;
    pComp->ComponentDeInit        = OMX_DemuxerComponentDeInit;
    pComp->FillThisBuffer        = OMX_DemuxerFillThisBuffer;
    pComp->EmptyThisBuffer        = OMX_DemuxerEmptyThisBuffer;

    pDemuxerDataType->sPortParam.nPorts = 3;
    pDemuxerDataType->sPortParam.nStartPortNumber = 0x0;
    pAudioPort = &pDemuxerDataType->sOutPortDef[DEMUX_PORT_AUDIO_INDEX];
    pVideoPort = &pDemuxerDataType->sOutPortDef[DEMUX_PORT_VIDEO_INDEX];
    pAudioBufSupplier = &pDemuxerDataType->sOutBufSupplier[DEMUX_PORT_AUDIO_INDEX];
    pVideoBufSupplier = &pDemuxerDataType->sOutBufSupplier[DEMUX_PORT_VIDEO_INDEX];
    pAudioStreamNum = &pDemuxerDataType->sStreamNum[DEMUX_PORT_AUDIO_INDEX];
    pVideoStreamNum = &pDemuxerDataType->sStreamNum[DEMUX_PORT_VIDEO_INDEX];

    pAudioPort->nPortIndex = DEMUX_PORT_AUDIO_INDEX;
    pAudioPort->bPopulated = OMX_TRUE;
    pAudioPort->bEnabled = OMX_TRUE;
    pAudioPort->eDomain = OMX_PortDomainAudio;
    pAudioPort->eDir = OMX_DirOutput;
    pVideoPort->nPortIndex = DEMUX_PORT_VIDEO_INDEX;
    pVideoPort->bPopulated = OMX_TRUE;
    pVideoPort->bEnabled = OMX_TRUE;
    pVideoPort->eDomain = OMX_PortDomainVideo;
    pVideoPort->eDir = OMX_DirOutput;
    pDemuxerDataType->sInPortDef.nPortIndex = DEMUX_PORT_CLOCK_INDEX;
    pDemuxerDataType->sInPortDef.bPopulated = OMX_TRUE;
    pDemuxerDataType->sInPortDef.bEnabled   = OMX_TRUE;
    pDemuxerDataType->sInPortDef.eDomain = OMX_PortDomainOther;
    pDemuxerDataType->sInPortDef.eDir = OMX_DirInput;

    //now  only support demux supoort buffer self.
    pAudioBufSupplier->nPortIndex = DEMUX_PORT_AUDIO_INDEX;
    pAudioBufSupplier->eBufferSupplier = OMX_BufferSupplyOutput;
    pVideoBufSupplier->nPortIndex = DEMUX_PORT_VIDEO_INDEX;
    pVideoBufSupplier->eBufferSupplier = OMX_BufferSupplyOutput;
    pDemuxerDataType->sInBufSupplier.nPortIndex = DEMUX_PORT_CLOCK_INDEX;
    pDemuxerDataType->sInBufSupplier.eBufferSupplier = OMX_BufferSupplyOutput;
    pAudioStreamNum->nPortIndex = DEMUX_PORT_AUDIO_INDEX;
    pVideoStreamNum->nPortIndex = DEMUX_PORT_VIDEO_INDEX;


    mpp_list_init(&pDemuxerDataType->sOutVideoEmptyPkt);
    mpp_list_init(&pDemuxerDataType->sOutVideoReadyPkt);
    mpp_list_init(&pDemuxerDataType->sOutVideoProcessingPkt);

    if (pthread_mutex_init(&pDemuxerDataType->sVideoPktLock, NULL)) {
        loge("pthread_mutex_init fail!\n");
        eError = OMX_ErrorInsufficientResources;
        goto _EXIT;
    }
    nVideoPktLockInitOk = 1;

    pVideoPktNodeHead = (DEMUXER_OUT_PACKET*)mpp_alloc(sizeof(DEMUXER_OUT_PACKET) * DEMUX_VIDEO_PACKET_NUM_MAX);

    if (pVideoPktNodeHead == NULL) {
        loge("mpp_alloc fail!\n");
        eError = OMX_ErrorInsufficientResources;
        goto _EXIT;
    }

    pDemuxerDataType->nVideoPktNodeNum = DEMUX_VIDEO_PACKET_NUM_MAX;
    pVideoPktNode = pVideoPktNodeHead;
    for(i = 0; i < DEMUX_VIDEO_PACKET_NUM_MAX; i++) {
        memset(pVideoPktNode,0x00,sizeof(DEMUXER_OUT_PACKET));
        mpp_list_init(&pVideoPktNode->sList);
        pVideoPktNode->sBuff.pBuffer = mpp_alloc(DEMUX_VIDEO_PACKET_BUFFER_SIZE);
        pVideoPktNode->sBuff.nAllocLen = DEMUX_VIDEO_PACKET_BUFFER_SIZE;
        if (NULL == pVideoPktNode->sBuff.pBuffer) {
            loge("mpp_alloc fail!\n");
            eError = OMX_ErrorInsufficientResources;
            goto _EXIT;
        }
        mpp_list_add_tail(&pVideoPktNode->sList, &pDemuxerDataType->sOutVideoEmptyPkt);
        pVideoPktNode++;
    }
    pDemuxerDataType->pVideoPktNodeHead = pVideoPktNodeHead;

    mpp_list_init(&pDemuxerDataType->sOutAudioEmptyPkt);
    mpp_list_init(&pDemuxerDataType->sOutAudioReadyPkt);
    mpp_list_init(&pDemuxerDataType->sOutAudioProcessingPkt);

    if (pthread_mutex_init(&pDemuxerDataType->sAudioPktLock, NULL)) {
        loge("pthread_mutex_init fail!\n");
        eError = OMX_ErrorInsufficientResources;
        goto _EXIT;
    }
    nAudioPktLockInitOk = 1;


    pAudioPktNodeHead = (DEMUXER_OUT_PACKET*)mpp_alloc(sizeof(DEMUXER_OUT_PACKET) * DEMUX_AUDIO_PACKET_NUM_MAX);

    if (pAudioPktNodeHead == NULL) {
        loge("mpp_alloc fail!\n");
        eError = OMX_ErrorInsufficientResources;
        goto _EXIT;
    }

    pDemuxerDataType->nAudioPktNodeNum = DEMUX_AUDIO_PACKET_NUM_MAX;
    pAudioPktNode = pAudioPktNodeHead;
    for(i = 0; i < DEMUX_AUDIO_PACKET_NUM_MAX; i++) {
        memset(pAudioPktNode,0x00,sizeof(DEMUXER_OUT_PACKET));
        mpp_list_init(&pAudioPktNode->sList);
        pAudioPktNode->sBuff.pBuffer = mpp_alloc(DEMUX_AUDIO_PACKET_BUFFER_SIZE);
        pAudioPktNode->sBuff.nAllocLen = DEMUX_AUDIO_PACKET_BUFFER_SIZE;
        if (NULL == pAudioPktNode->sBuff.pBuffer) {
            loge("mpp_alloc fail!\n");
            eError = OMX_ErrorInsufficientResources;
            goto _EXIT;
        }
        mpp_list_add_tail(&pAudioPktNode->sList, &pDemuxerDataType->sOutAudioEmptyPkt);
        pAudioPktNode++;
    }
    pDemuxerDataType->pAudioPktNodeHead = pAudioPktNodeHead;

    pDemuxerDataType->nEos = 0;

    if (aic_msg_create(&pDemuxerDataType->sMsgQue)<0) {
        loge("aic_msg_create fail!\n");
        eError = OMX_ErrorInsufficientResources;
        goto _EXIT;
    }
    nMsgCreateOk = 1;

    if (pthread_mutex_init(&pDemuxerDataType->stateLock, NULL)) {
        loge("pthread_mutex_init fail!\n");
        eError = OMX_ErrorInsufficientResources;
        goto _EXIT;
    }
    nSateLockInitOk = 1;

    // Create the component thread
    err = pthread_create(&pDemuxerDataType->threadId, &attr, OMX_DemuxerComponentThread, pDemuxerDataType);
    //if (err || !pDemuxerDataType->threadId)
    if (err) {
        loge("pthread_create fail!\n");
        eError = OMX_ErrorInsufficientResources;
        goto _EXIT;
    }

    return eError;

_EXIT:

    if (nVideoPktLockInitOk) {
        pthread_mutex_destroy(&pDemuxerDataType->sVideoPktLock);
    }
    if (nAudioPktLockInitOk) {
        pthread_mutex_destroy(&pDemuxerDataType->sAudioPktLock);
    }
    if (nSateLockInitOk) {
        pthread_mutex_destroy(&pDemuxerDataType->stateLock);
    }
    if (nMsgCreateOk) {
       aic_msg_destroy(&pDemuxerDataType->sMsgQue);
    }

    if (!mpp_list_empty(&pDemuxerDataType->sOutVideoEmptyPkt)) {
        mpp_list_for_each_entry_safe(pPktNode, pPktNode1, &pDemuxerDataType->sOutVideoEmptyPkt, sList) {
            if(pPktNode->sBuff.pBuffer) {
                mpp_free(pPktNode->sBuff.pBuffer);
                pPktNode->sBuff.pBuffer = NULL;
            }
        }
    }

    if (!mpp_list_empty(&pDemuxerDataType->sOutAudioEmptyPkt)) {
        mpp_list_for_each_entry_safe(pPktNode, pPktNode1, &pDemuxerDataType->sOutAudioEmptyPkt, sList) {
            if(pPktNode->sBuff.pBuffer) {
                mpp_free(pPktNode->sBuff.pBuffer);
                 pPktNode->sBuff.pBuffer = NULL;
            }
        }
    }

    if (pDemuxerDataType->pAudioPktNodeHead) {
        mpp_free(pDemuxerDataType->pAudioPktNodeHead);
        pDemuxerDataType->pAudioPktNodeHead = NULL;
    }

    if (pDemuxerDataType->pVideoPktNodeHead) {
        mpp_free(pDemuxerDataType->pVideoPktNodeHead);
        pDemuxerDataType->pVideoPktNodeHead = NULL;
    }

    if (pDemuxerDataType) {
        mpp_free(pDemuxerDataType);
        pDemuxerDataType = NULL;
    }

    return eError;
}

static void OMX_DemuxerStateChangeToInvalid(DEMUXER_DATA_TYPE * pDemuxerDataType)
{
    pDemuxerDataType->state = OMX_StateInvalid;
    OMX_DemuxerEventNotify(pDemuxerDataType
                        ,OMX_EventError
                        ,OMX_ErrorInvalidState,0,NULL);
    OMX_DemuxerEventNotify(pDemuxerDataType
                        ,OMX_EventCmdComplete
                        ,OMX_CommandStateSet
                        ,pDemuxerDataType->state,NULL);
}

static void OMX_DemuxerStateChangeToIdle(DEMUXER_DATA_TYPE * pDemuxerDataType)
{
    //int ret = 0;
    //int i = 0;
    //OMX_PARAM_PORTDEFINITIONTYPE *pAudioPort,*pVideoPort;
    //OMX_PARAM_U32TYPE *pAudioStreamNum,*pVideoStreamNum;
    //OMX_S32 *pAudioActiveStreamIndex,*pVideoActiveStreamIndex;
    //pAudioPort = &pDemuxerDataType->sOutPortDef[DEMUX_PORT_AUDIO_INDEX];
    //pVideoPort = &pDemuxerDataType->sOutPortDef[DEMUX_PORT_VIDEO_INDEX];
    //pAudioStreamNum = &pDemuxerDataType->sStreamNum[DEMUX_PORT_AUDIO_INDEX];
    //pVideoStreamNum = &pDemuxerDataType->sStreamNum[DEMUX_PORT_VIDEO_INDEX];
    //pAudioActiveStreamIndex = &pDemuxerDataType->sActiveStreamIndex[DEMUX_PORT_AUDIO_INDEX];
    //pVideoActiveStreamIndex = &pDemuxerDataType->sActiveStreamIndex[DEMUX_PORT_VIDEO_INDEX];

    if (OMX_StateLoaded == pDemuxerDataType->state) {

    } else if (OMX_StateExecuting == pDemuxerDataType->state) {

    } else if (OMX_StatePause == pDemuxerDataType->state) {

    } else {
        OMX_DemuxerEventNotify(pDemuxerDataType
                        ,OMX_EventError
                        ,OMX_ErrorIncorrectStateTransition
                        ,pDemuxerDataType->state,NULL);
        loge("OMX_ErrorIncorrectStateTransition\n");
        return;
    }
    pDemuxerDataType->state = OMX_StateIdle;
    OMX_DemuxerEventNotify(pDemuxerDataType
                        ,OMX_EventCmdComplete
                        ,OMX_CommandStateSet
                        ,pDemuxerDataType->state,NULL);
}

static void OMX_DemuxerStateChangeToLoaded(DEMUXER_DATA_TYPE * pDemuxerDataType)
{
    if (OMX_StateIdle == pDemuxerDataType->state) {
        OMX_DemuxerClearPackets(pDemuxerDataType);
        pDemuxerDataType->nEos = 0;
        pDemuxerDataType->nSendAudioPacketNum = 0;
        pDemuxerDataType->nSendVideoPacketNum = 0;
        pDemuxerDataType->nSkipTrack = 0;

        logi("OMX_DemuxerStateChangeToLoaded\n");

        pDemuxerDataType->state = OMX_StateLoaded;
        OMX_DemuxerEventNotify(pDemuxerDataType
                        ,OMX_EventCmdComplete
                        ,OMX_CommandStateSet
                        ,pDemuxerDataType->state,NULL);
        if (pDemuxerDataType->pParser) {
            aic_parser_destroy(pDemuxerDataType->pParser);
            pDemuxerDataType->pParser = NULL;
            pDemuxerDataType->nEos = 0;
            pDemuxerDataType->nNeedPeek = 1;
        }
    } else {
        OMX_DemuxerEventNotify(pDemuxerDataType
                        ,OMX_EventError
                        ,OMX_ErrorIncorrectStateTransition
                        , pDemuxerDataType->state,NULL);
        loge("OMX_ErrorIncorrectStateTransition\n");
    }

}


static void OMX_DemuxerStateChangeToExecuting(DEMUXER_DATA_TYPE * pDemuxerDataType)
{
    if (OMX_StateIdle == pDemuxerDataType->state) {
        if (NULL == pDemuxerDataType->pParser) {
            OMX_DemuxerEventNotify(pDemuxerDataType
                            ,OMX_EventError
                            ,OMX_ErrorIncorrectStateTransition
                            ,pDemuxerDataType->state,NULL);
            loge("pDemuxerDataType->pParser is not created,please set param OMX_IndexParamContentURI!!!!!\n");
            return;
        }
    } else if (OMX_StatePause == pDemuxerDataType->state) {
    //
    } else {
        OMX_DemuxerEventNotify(pDemuxerDataType
                        ,OMX_EventError
                        ,OMX_ErrorIncorrectStateTransition
                        ,pDemuxerDataType->state,NULL);
        loge("OMX_ErrorIncorrectStateTransition\n");
        return;
    }
    pDemuxerDataType->state = OMX_StateExecuting;

}

static void OMX_DemuxerStateChangeToPause(DEMUXER_DATA_TYPE * pDemuxerDataType)
{
    if (OMX_StateExecuting == pDemuxerDataType->state) {
    //
    } else {
        OMX_DemuxerEventNotify(pDemuxerDataType
                        ,OMX_EventError
                        ,OMX_ErrorIncorrectStateTransition
                        ,pDemuxerDataType->state,NULL);
        logd("OMX_ErrorIncorrectStateTransition\n");
        return;
    }
    pDemuxerDataType->state = OMX_StatePause;

}

static void* OMX_DemuxerComponentThread(void* pThreadData)
{
    struct aic_message message;
    OMX_S32 nCmd;        //OMX_COMMANDTYPE
    OMX_S32 nCmdData;    //OMX_STATETYPE
    DEMUXER_DATA_TYPE* pDemuxerDataType = (DEMUXER_DATA_TYPE*)pThreadData;
    struct aic_parser_packet       sPkt;
    //OMX_S32 i;
    OMX_S32 ret;
    OMX_S32 bNotifyFrameEnd = 0;
    DEMUXER_OUT_PACKET  *pPktNode,*pPktNode1;
    pDemuxerDataType->nNeedPeek = 1;
    //struct mpp_list sEmptyPktList;
    //struct mpp_list sReadyPktList;

    //prctl(PR_SET_NAME,(u32)"Demuxer");

    OMX_S8 bVideoHasVbvBuffer = 1;
    OMX_S8 bAudioHasVbvBuffer = 1;

    while(1) {
_AIC_MSG_GET_:
        if (aic_msg_get(&pDemuxerDataType->sMsgQue, &message) == 0) {
            nCmd = message.message_id;
            nCmdData = message.param;
            logi("nCmd:%"PRId32", nCmdData:%"PRId32"\n",nCmd,nCmdData);
             if (OMX_CommandStateSet == nCmd) {
                aic_pthread_mutex_lock(&pDemuxerDataType->stateLock);
                if (pDemuxerDataType->state == (OMX_STATETYPE)(nCmdData)) {
                    OMX_DemuxerEventNotify(pDemuxerDataType,OMX_EventError,OMX_ErrorSameState,0,NULL);
                    aic_pthread_mutex_unlock(&pDemuxerDataType->stateLock);
                    continue;
                }
                switch((OMX_STATETYPE)(nCmdData)) {
                case OMX_StateInvalid:
                    OMX_DemuxerStateChangeToInvalid(pDemuxerDataType);
                    break;
                case OMX_StateLoaded://idel->loaded means stop
                    OMX_DemuxerStateChangeToLoaded(pDemuxerDataType);
                    break;
                case OMX_StateIdle:
                    OMX_DemuxerStateChangeToIdle(pDemuxerDataType);
                    break;
                case OMX_StateExecuting:
                    OMX_DemuxerStateChangeToExecuting(pDemuxerDataType);
                    break;
                case OMX_StatePause:
                    OMX_DemuxerStateChangeToPause(pDemuxerDataType);
                    break;
                case OMX_StateWaitForResources:
                    break;
                default:
                    break;
                }
                aic_pthread_mutex_unlock(&pDemuxerDataType->stateLock);
             } else if (OMX_CommandFlush == nCmd) {

            } else if (OMX_CommandPortDisable == nCmd) {

            } else if (OMX_CommandPortEnable == nCmd) {

            } else if (OMX_CommandMarkBuffer == nCmd) {

            } else if (OMX_CommandStop == nCmd) {
                logi("OMX_DemuxerComponentThread ready to exit!!!\n");
                goto _EXIT;
            } else if (OMX_CommandNops == nCmd) {

            } else {

            }
        }

        if (pDemuxerDataType->state != OMX_StateExecuting)
        {
            //usleep(1000);
            aic_msg_wait_new_msg(&pDemuxerDataType->sMsgQue, 0);
            continue;
        }

        if (pDemuxerDataType->nEos) {
            if (OMX_DemuxerListEmpty(&pDemuxerDataType->sOutVideoReadyPkt,pDemuxerDataType->sVideoPktLock)
                && OMX_DemuxerListEmpty(&pDemuxerDataType->sOutAudioReadyPkt,pDemuxerDataType->sAudioPktLock)) {
                if (!bNotifyFrameEnd) {
                    OMX_DemuxerEventNotify(pDemuxerDataType,OMX_EventBufferFlag,0,0,NULL);
                    bNotifyFrameEnd = 1;
                }
                aic_msg_wait_new_msg(&pDemuxerDataType->sMsgQue, 0);
                continue;
            }
        }
        bNotifyFrameEnd = 0;

        bVideoHasVbvBuffer = 1;
        bAudioHasVbvBuffer = 1;
        if (!(pDemuxerDataType->nSkipTrack & DEMUX_SKIP_VIDEO_TRACK)
            && (!OMX_DemuxerListEmpty(&pDemuxerDataType->sOutVideoReadyPkt,pDemuxerDataType->sVideoPktLock))) {
            while(!OMX_DemuxerListEmpty(&pDemuxerDataType->sOutVideoReadyPkt,pDemuxerDataType->sVideoPktLock)) {
                aic_pthread_mutex_lock(&pDemuxerDataType->sVideoPktLock);
                pPktNode = mpp_list_first_entry(&pDemuxerDataType->sOutVideoReadyPkt,DEMUXER_OUT_PACKET,sList);
                aic_pthread_mutex_unlock(&pDemuxerDataType->sVideoPktLock);
                ret = OMX_ErrorNone;
                if (pDemuxerDataType->sOutPortTunneledInfo[DEMUX_PORT_VIDEO_INDEX].nTunneledFlag) {
                    ret = OMX_EmptyThisBuffer(pDemuxerDataType->sOutPortTunneledInfo[DEMUX_PORT_VIDEO_INDEX].pTunneledComp,
                                            &pPktNode->sBuff);
                } else {
                    if (pDemuxerDataType->pCallbacks->FillBufferDone) {
                        ret = pDemuxerDataType->pCallbacks->FillBufferDone(pDemuxerDataType->hSelf,pDemuxerDataType->pAppData,&pPktNode->sBuff);
                    }
                }
                if (ret != OMX_ErrorNone) {
                    //logw("send video pkt to other component or app fail\n");
                    //how to  do ,if send pkt to other component or app fail,
                    // just only noitfy error,do not move empty list,cause:adec/vdec decode slowly ,we can sleep some time
                    OMX_DemuxerEventNotify(pDemuxerDataType,OMX_EventError,OMX_ErrorUndefined,0,NULL);

                    bVideoHasVbvBuffer = 0;
                    //do not move empty list,cause:adec/vdec decode slowly or not in executing
                    //mpp_list_del(&pPktNode->sList);
                    //mpp_list_add_tail(&pPktNode->sList,&pDemuxerDataType->sOutVideoEmptyPkt);
                    break;
                } else {
                    aic_pthread_mutex_lock(&pDemuxerDataType->sVideoPktLock);
                    mpp_list_del(&pPktNode->sList);
                    mpp_list_add_tail(&pPktNode->sList,&pDemuxerDataType->sOutVideoProcessingPkt);
                    pDemuxerDataType->nSendVideoPacketNum++;
                    //loge("pDemuxerDataType->nSendVideoPacketNum:%d\n",pDemuxerDataType->nSendVideoPacketNum);
                    aic_pthread_mutex_unlock(&pDemuxerDataType->sVideoPktLock);
                }
            }
        }

        if (!(pDemuxerDataType->nSkipTrack & DEMUX_SKIP_AUDIO_TRACK)
            &&(!OMX_DemuxerListEmpty(&pDemuxerDataType->sOutAudioReadyPkt,pDemuxerDataType->sAudioPktLock))) {
            while(!OMX_DemuxerListEmpty(&pDemuxerDataType->sOutAudioReadyPkt,pDemuxerDataType->sAudioPktLock)) {
                aic_pthread_mutex_lock(&pDemuxerDataType->sAudioPktLock);
                pPktNode = mpp_list_first_entry(&pDemuxerDataType->sOutAudioReadyPkt,DEMUXER_OUT_PACKET,sList);
                aic_pthread_mutex_unlock(&pDemuxerDataType->sAudioPktLock);
                ret = OMX_ErrorNone;
                if (pDemuxerDataType->sOutPortTunneledInfo[DEMUX_PORT_AUDIO_INDEX].nTunneledFlag) {
                    ret = OMX_EmptyThisBuffer(pDemuxerDataType->sOutPortTunneledInfo[DEMUX_PORT_AUDIO_INDEX].pTunneledComp,
                                            &pPktNode->sBuff);
                } else {
                    if (pDemuxerDataType->pCallbacks->FillBufferDone) {
                        ret = pDemuxerDataType->pCallbacks->FillBufferDone(pDemuxerDataType->hSelf,pDemuxerDataType->pAppData,&pPktNode->sBuff);
                    }
                }
                if (ret != OMX_ErrorNone) {
                    //loge("send audio pkt to other component or app fail\n");
                    //how to  do ,if send pkt to other component or app fail,
                    // just only noitfy error,do not move empty list,cause:adec/vdec decode slowly ,we can sleep some time
                    OMX_DemuxerEventNotify(pDemuxerDataType,OMX_EventError,OMX_ErrorUndefined,0,NULL);
                    bAudioHasVbvBuffer = 0;
                    break;
                } else {
                    aic_pthread_mutex_lock(&pDemuxerDataType->sAudioPktLock);
                    mpp_list_del(&pPktNode->sList);
                    mpp_list_add_tail(&pPktNode->sList,&pDemuxerDataType->sOutAudioProcessingPkt);
                    pDemuxerDataType->nSendAudioPacketNum++;
                    //loge("pDemuxerDataType->nSendAudioPacketNum:%d\n",pDemuxerDataType->nSendAudioPacketNum);
                    aic_pthread_mutex_unlock(&pDemuxerDataType->sAudioPktLock);
                }
            }
        }

        if (!bAudioHasVbvBuffer || !bVideoHasVbvBuffer) {
            aic_msg_wait_new_msg(&pDemuxerDataType->sMsgQue, 10*1000);
            goto _AIC_MSG_GET_;
        }

        //process buffer
        if (pDemuxerDataType->nEos) {//end of stream
            continue;
        }

        if (pDemuxerDataType->nNeedPeek) {
            //memset(&sPkt,0x00,sizeof(struct aic_parser_packet));
            sPkt.flag = 0;
            ret = aic_parser_peek(pDemuxerDataType->pParser,&sPkt);
            if (!ret) {//peek ok
                logd("peek ok\n");
                pDemuxerDataType->nNeedPeek = 0;
            } else if (ret == PARSER_EOS) {//peek end
                printf("[%s:%d]*************PARSER_EOS*******************\n",__FUNCTION__,__LINE__);
                pDemuxerDataType->nEos = 1;
                goto _AIC_MSG_GET_;
            } else {// now  nothing to do ,becase no other return val
                loge("peek fail\n");
                goto _AIC_MSG_GET_;
            }
        }
        // skip other pkt type
        if (sPkt.type != MPP_MEDIA_TYPE_VIDEO && sPkt.type != MPP_MEDIA_TYPE_AUDIO) {
            pDemuxerDataType->nNeedPeek = 1;
            goto _AIC_MSG_GET_;
        }

        if (sPkt.type == MPP_MEDIA_TYPE_VIDEO) {
             OMX_BOOL bFind = OMX_FALSE;
            if (pDemuxerDataType->nSkipTrack & DEMUX_SKIP_VIDEO_TRACK) {
                pDemuxerDataType->nNeedPeek = 1;
                goto _AIC_MSG_GET_;
            }

            if (OMX_DemuxerListEmpty(&pDemuxerDataType->sOutVideoEmptyPkt,pDemuxerDataType->sVideoPktLock)) {
                if (OMX_DemuxerListEmpty(&pDemuxerDataType->sOutVideoReadyPkt,pDemuxerDataType->sVideoPktLock)
                    && OMX_DemuxerListEmpty(&pDemuxerDataType->sOutAudioReadyPkt,pDemuxerDataType->sAudioPktLock)) {
                    //loge("sOutVideoReadyPkt\n");
                    aic_msg_wait_new_msg(&pDemuxerDataType->sMsgQue, 0);
                }
                pDemuxerDataType->nNeedPeek = 0;
                goto _AIC_MSG_GET_;
            }

            aic_pthread_mutex_lock(&pDemuxerDataType->sVideoPktLock);
            mpp_list_for_each_entry_safe(pPktNode, pPktNode1, &pDemuxerDataType->sOutVideoEmptyPkt, sList) {
                if (pPktNode->sBuff.nAllocLen >= sPkt.size) {
                    bFind = OMX_TRUE;
                    break;
                }
            }
            if (!bFind) {
                pPktNode = mpp_list_first_entry(&pDemuxerDataType->sOutVideoEmptyPkt,DEMUXER_OUT_PACKET,sList);
            }
            aic_pthread_mutex_unlock(&pDemuxerDataType->sVideoPktLock);

            if (!bFind) {
                static  int malloc_total_num = 0;
                static  int print_num = 0;
                print_num++;
                malloc_total_num++;
                if (print_num > 100) {
                    loge("malloc_total_num:%d\n",malloc_total_num);
                    print_num = 0;
                }

                if (pPktNode->sBuff.pBuffer) {
                    mpp_free(pPktNode->sBuff.pBuffer);
                    pPktNode->sBuff.pBuffer = NULL;
                }
                pPktNode->sBuff.pBuffer = (OMX_U8 *)mpp_alloc(sPkt.size);
                //logi("mpp_alloc pPktNode->sBuff.pBuffer=%p,sPkt.size:%d\n",pPktNode->sBuff.pBuffer,sPkt.size);
                if (NULL == pPktNode->sBuff.pBuffer) {
                    loge("mpp_alloc fail :%d\n",sPkt.size);
                    pPktNode->sBuff.pBuffer = NULL;
                    pPktNode->sBuff.nAllocLen = 0;
                    if (sPkt.size > 16*1024*1024) {
                        OMX_DemuxerEventNotify(pDemuxerDataType,OMX_EventError,OMX_ErrorInsufficientResources,0,NULL);
                        aic_msg_wait_new_msg(&pDemuxerDataType->sMsgQue, 0);
                    }
                    goto _AIC_MSG_GET_;
                } else {
                    pPktNode->sBuff.nAllocLen = sPkt.size;
                }
            }
            sPkt.data = pPktNode->sBuff.pBuffer;

            {
                struct timespec before = {0},after = {0};
                long diff = 0;
                clock_gettime(CLOCK_REALTIME,&before);
                ret = aic_parser_read(pDemuxerDataType->pParser,&sPkt);
                clock_gettime(CLOCK_REALTIME,&after);
                diff = (after.tv_sec - before.tv_sec)*1000*1000 + (after.tv_nsec - before.tv_nsec)/1000;
                if (diff > 42*1000) {
                        printf("[%s:%d]:%ld\n",__FUNCTION__,__LINE__,diff);
                    }
            }

            logd("aic_parser_read,pts:%lld,type = %d,size:%d,flag:0x%x\n",sPkt.pts,sPkt.type,sPkt.size,sPkt.flag);
            if (!ret) {// read ok
                pDemuxerDataType->nNeedPeek = 1;
            } else {// now  nothing to do ,becase no other return val
                loge("read fail\n");
                pDemuxerDataType->nNeedPeek = 0;
                goto _AIC_MSG_GET_;
            }
            pPktNode->sBuff.nFilledLen = sPkt.size;
            pPktNode->sBuff.nTimeStamp = sPkt.pts;
            pPktNode->sBuff.nFlags = sPkt.flag;
            pPktNode->sBuff.nOutputPortIndex = DEMUX_PORT_VIDEO_INDEX;
            aic_pthread_mutex_lock(&pDemuxerDataType->sVideoPktLock);
            mpp_list_del(&pPktNode->sList);
            mpp_list_add_tail(&pPktNode->sList,&pDemuxerDataType->sOutVideoReadyPkt);
            aic_pthread_mutex_unlock(&pDemuxerDataType->sVideoPktLock);
            pDemuxerDataType->nVideoPacketNum++;
            //logi("pDemuxerDataType->nVideoPacketNum:%d\n",pDemuxerDataType->nVideoPacketNum);
        } else if (sPkt.type == MPP_MEDIA_TYPE_AUDIO) { // same with Video , later optmization
            OMX_BOOL bFind = OMX_FALSE;
            if (pDemuxerDataType->nSkipTrack & DEMUX_SKIP_AUDIO_TRACK) {
                pDemuxerDataType->nNeedPeek = 1;
                goto _AIC_MSG_GET_;
            }

            if (OMX_DemuxerListEmpty(&pDemuxerDataType->sOutAudioEmptyPkt,pDemuxerDataType->sAudioPktLock)) {
                if (OMX_DemuxerListEmpty(&pDemuxerDataType->sOutVideoReadyPkt,pDemuxerDataType->sVideoPktLock)
                    && OMX_DemuxerListEmpty(&pDemuxerDataType->sOutAudioReadyPkt,pDemuxerDataType->sAudioPktLock)) {
                    //loge("sOutAudioReadyPkt\n");
                    aic_msg_wait_new_msg(&pDemuxerDataType->sMsgQue, 0);
                }
                pDemuxerDataType->nNeedPeek = 0;
                goto _AIC_MSG_GET_;
            }
            aic_pthread_mutex_lock(&pDemuxerDataType->sAudioPktLock);
            mpp_list_for_each_entry_safe(pPktNode, pPktNode1, &pDemuxerDataType->sOutAudioEmptyPkt, sList) {
                if (pPktNode->sBuff.nAllocLen >= sPkt.size) {
                    bFind = OMX_TRUE;
                    break;
                }
            }
            if (!bFind) {
                pPktNode = mpp_list_first_entry(&pDemuxerDataType->sOutAudioEmptyPkt,DEMUXER_OUT_PACKET,sList);
            }
            aic_pthread_mutex_unlock(&pDemuxerDataType->sAudioPktLock);
            if (!bFind) {
                static  int malloc_total_num = 0;
                static  int print_num = 0;
                print_num++;
                malloc_total_num++;
                if (print_num > 100) {
                    loge("malloc_total_num:%d\n",malloc_total_num);
                    print_num = 0;
                }

                if (pPktNode->sBuff.pBuffer) {
                    mpp_free(pPktNode->sBuff.pBuffer);
                    pPktNode->sBuff.pBuffer = NULL;
                }
                pPktNode->sBuff.pBuffer = (OMX_U8 *)mpp_alloc(sPkt.size);
                //logi("mpp_alloc pPktNode->sBuff.pBuffer=%p,sPkt.size:%d\n",pPktNode->sBuff.pBuffer,sPkt.size);
                if (NULL == pPktNode->sBuff.pBuffer) {
                    loge("mpp_alloc fail :%d\n",sPkt.size);
                    pPktNode->sBuff.pBuffer = NULL;
                    pPktNode->sBuff.nAllocLen = 0;
                    if (sPkt.size > 16*1024*1024) {
                        OMX_DemuxerEventNotify(pDemuxerDataType,OMX_EventError,OMX_ErrorInsufficientResources,0,NULL);
                        aic_msg_wait_new_msg(&pDemuxerDataType->sMsgQue, 0);
                    }
                    goto _AIC_MSG_GET_;
                } else {
                    pPktNode->sBuff.nAllocLen = sPkt.size;
                }
            }

            sPkt.data = pPktNode->sBuff.pBuffer;
            {
                struct timespec before = {0},after = {0};
                long diff = 0;
                clock_gettime(CLOCK_REALTIME,&before);
                ret = aic_parser_read(pDemuxerDataType->pParser,&sPkt);
                clock_gettime(CLOCK_REALTIME,&after);
                diff = (after.tv_sec - before.tv_sec)*1000*1000 + (after.tv_nsec - before.tv_nsec)/1000;
                if (diff > 42*1000) {
                        printf("[%s:%d]:%ld\n",__FUNCTION__,__LINE__,diff);
                    }
            }

            logd("aic_parser_read,pts:%lld,type = %d,size:%d,flag:0x%x\n",sPkt.pts,sPkt.type,sPkt.size,sPkt.flag);
            if (!ret) {//read ok
                pDemuxerDataType->nNeedPeek = 1;
            } else {// now  nothing to do ,becase no other return val
                loge("peek fail\n");
                pDemuxerDataType->nNeedPeek = 0;
                goto _AIC_MSG_GET_;
            }
            pPktNode->sBuff.nFilledLen = sPkt.size;
            pPktNode->sBuff.nTimeStamp = sPkt.pts;
            pPktNode->sBuff.nFlags = sPkt.flag;
            pPktNode->sBuff.nOutputPortIndex = DEMUX_PORT_AUDIO_INDEX;
            aic_pthread_mutex_lock(&pDemuxerDataType->sAudioPktLock);
            mpp_list_del(&pPktNode->sList);
            mpp_list_add_tail(&pPktNode->sList,&pDemuxerDataType->sOutAudioReadyPkt);
            aic_pthread_mutex_unlock(&pDemuxerDataType->sAudioPktLock);

            {
                static u64 pre_pts = 0;
                u64 diff_pts = sPkt.pts - pre_pts;
                if (diff_pts > 24*1000) {
                    logd("pre:%lld,cur:%lld,diff:%lld",pre_pts,sPkt.pts,diff_pts);
                }
                pre_pts = sPkt.pts;
            }
            pDemuxerDataType->nAudioPacketNum++;
            //logi("pDemuxerDataType->nAudioPacketNum:%d\n",pDemuxerDataType->nAudioPacketNum);
        }
    }

_EXIT:
        printf("[%s:%d]nVideoPacketNum:%"PRId32","\
            "nAudioPacketNum:%"PRId32","\
            "nSendVideoPacketNum:%"PRId32","\
            "nSendAudioPacketNum:%"PRId32"\n"
            ,__FUNCTION__,__LINE__
            ,pDemuxerDataType->nVideoPacketNum
            ,pDemuxerDataType->nAudioPacketNum
            ,pDemuxerDataType->nSendVideoPacketNum
            ,pDemuxerDataType->nSendAudioPacketNum);
    printf("[%s:%d]DemuxerCompoent Exit\n",__FUNCTION__,__LINE__);
    return (void*)OMX_ErrorNone;
}

