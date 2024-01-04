//
// Generated by dtk
// Translation Unit: d_resorce.cpp
//

#include "d/d_resorce.h"
#include "d/d_bg_s.h"
#include "d/d_com_inf_game.h"
#include "m_Do/m_Do_printf.h"
#include "m_Do/m_Do_ext.h"
#include "m_Do/m_Do_dvd_thread.h"
#include "JSystem/J3DGraphLoader/J3DAnmLoader.h"
#include "JSystem/J3DGraphLoader/J3DClusterLoader.h"
#include "JSystem/J3DGraphLoader/J3DModelLoader.h"
#include "JSystem/J3DGraphAnimator/J3DAnimation.h"
#include "JSystem/J3DGraphAnimator/J3DMaterialAnm.h"
#include "JSystem/JKernel/JKRArchive.h"
#include "JSystem/JKernel/JKRFileFinder.h"
#include "JSystem/JKernel/JKRSolidHeap.h"
#include "JSystem/JUtility/JUTConsole.h"
#include "JSystem/JUtility/JUTTexture.h"
#include "stdio.h"
#include "string.h"
#include "dolphin/os/OSCache.h"

/* 8006D804-8006D824       .text __ct__11dRes_info_cFv */
dRes_info_c::dRes_info_c()
    : mCount(0)
    , mpDMCommand(NULL)
    , mpArchive(NULL)
    , mpParentHeap(NULL)
    , mDataHeap(NULL)
    , mRes(NULL)
{
}

/* 8006D824-8006D8F4       .text __dt__11dRes_info_cFv */
dRes_info_c::~dRes_info_c() {
    if (mpDMCommand != NULL) {
        delete mpDMCommand;
        mpDMCommand = NULL;
    } else if (mpArchive != NULL) {
        if (mDataHeap != NULL) {
            mDoExt_destroySolidHeap(mDataHeap);
            mDataHeap = NULL;
        }
        if (mRes != NULL)
            mRes = NULL;

        mpArchive->unmount();
        mpArchive = NULL;
    }
}

/* 8006D8F4-8006D990       .text set__11dRes_info_cFPCcPCcUcP7JKRHeap */
int dRes_info_c::set(char const* pArcName, char const* pArcPath, u8 direction, JKRHeap* pHeap) {
    char path[40];

    snprintf(path, 40, "%s%s.arc", pArcPath, pArcName);
    mpDMCommand = mDoDvdThd_mountArchive_c::create(path, direction, pHeap);

    if (mpDMCommand == NULL)
        return false;

    strncpy(mArchiveName, pArcName, 14);
    return true;
}

/* 8006D990-8006DCEC       .text setToonTex__FP12J3DModelData */
static void setToonTex(J3DModelData* pModel) {
    J3DTexture * pTexture = pModel->getTexture();
    if (pTexture != NULL) {
        JUTNameTab * pTextureName = pModel->getTextureName();
        if (pTextureName != NULL) {
            for (u16 i = 0; i < pTexture->getNum(); i++) {
                const char * pName = pTextureName->getName(i);
                if (pName[0] == 'Z') {
                    if (pName[1] == 'A')
                        pTexture->setResTIMG(i, *dDlst_list_c::getToonImage());
                    else if (pName[1] == 'B')
                        pTexture->setResTIMG(i, *dDlst_list_c::getToonExImage());
                }
            }

            j3dSys.setTexture(pTexture);

            s32 isBDL = (pModel->getJointTree().getModelDataType() == 1);

            for (u16 i = 0; i < pModel->getMaterialNum() ; i++) {
                J3DMaterial * pMaterial = pModel->getMaterialNodePointer(i);
                J3DTevBlock * pTevBlock = pMaterial->getTevBlock();

                if (pTevBlock != NULL) {
                    GXColorS10 * pTev3 = &pTevBlock->getTevColor(3)->mColor;
                    if (pTev3 != NULL)
                        pTev3->a = pTevBlock->getTevStageNum();

                    if (isBDL) {
                        J3DDisplayListObj* pDL = pMaterial->getSharedDisplayListObj();
                        BOOL ret = OSDisableInterrupts();
                        GDInitGDLObj(&J3DDisplayListObj::sGDLObj, pDL->getDisplayList(0), pDL->getDisplayListSize());
                        GDSetCurrent(&J3DDisplayListObj::sGDLObj);
                        pTevBlock->patchTexNoAndTexCoordScale();
                        OSRestoreInterrupts(ret);
                        GDSetCurrent(NULL);
                    }
                }
            }
        }
    }
}

/* 8006DCEC-8006DFD4       .text setToonTex__FP16J3DMaterialTable */
static void setToonTex(J3DMaterialTable* pMaterialTable) {
    J3DTexture * pTexture = pMaterialTable->getTexture();
    if (pTexture != NULL) {
        JUTNameTab * pTextureName = pMaterialTable->getTextureName();
        if (pTextureName != NULL) {
            for (u16 i = 0; i < pTexture->getNum(); i++) {
                const char * pName = pTextureName->getName(i);
                if (pName[0] == 'Z') {
                    if (pName[1] == 'A')
                        pTexture->setResTIMG(i, *dDlst_list_c::getToonImage());
                    else if (pName[1] == 'B')
                        pTexture->setResTIMG(i, *dDlst_list_c::getToonExImage());
                }
            }

            for (u16 i = 0; i < pMaterialTable->getMaterialNum() ; i++) {
                J3DMaterial * pMaterial = pMaterialTable->getMaterialNodePointer(i);
                J3DTevBlock * pTevBlock = pMaterial->getTevBlock();

                if (pTevBlock != NULL) {
                    GXColorS10 * pTev3 = &pTevBlock->getTevColor(3)->mColor;
                    if (pTev3 != NULL)
                        pTev3->a = pTevBlock->getTevStageNum();
                }
            }
        }
    }
}

/* 8006DFD4-8006E7A4       .text loadResource__11dRes_info_cFv */
int dRes_info_c::loadResource() {
    JUT_ASSERT(0x25f, mRes == 0);

    s32 fileNum = getResNum();
    mRes = new void*[fileNum];
    if (mRes == NULL) {
        OSReport_Error("<%s.arc> setRes: res pointer buffer nothing !!\n", this);
        return -1;
    }

    static u32 l_readResType[] = {
        'BMD ',
        'BMDM',
        'BMDC',
        'BMDS',
        'BSMD',
        'BMT ',
        'BLS ',
        'BCK ',
        'BPK ',
        'BRK ',
        'BLK ',
        'BTP ',
        'BTK ',
        'BAS ',
        'BDL ',
        'BDLM',
        'BDLC',
        'BDLI',
        'DZB ',
        'DZR ',
        'DZS ',
        'TIM ',
        'MSG ',
        'TEX ',
        'STB ',
        'BCKS',
        'DAT ',
        'BVA ',
        'BMTM',
    };

    u32 i = 0;
    while (fileNum-- > 0)
        mRes[i++] = NULL;

    u32 *pResType = &l_readResType[0];
    for (i = 0; i < ARRAY_SIZE(l_readResType); i++, pResType++) {
        JKRArcFinder * pArcFinder = mpArchive->getFirstResource(*pResType);

        for (; pArcFinder->isAvailable(); pArcFinder->findNextFile()) {
            u32 resType;
            void * pRes = JKRArchive::getGlbResource(*pResType, pArcFinder->mEntryName, mpArchive);
            if (pRes == NULL) {
                OSReport_Error("<%s> res == NULL !!\n", pArcFinder->mEntryName);
                goto next;
            }

            resType = *pResType;
            if (resType == 'BMD ') {
                pRes = J3DModelLoaderDataBase::load(pRes, 0x51240020);
                if (pRes == NULL)
                    return -1;

                setToonTex(((J3DModelData*)pRes));
            } else if (resType == 'BMDM') {
                pRes = J3DModelLoaderDataBase::load(pRes, 0x51240020);
                if (pRes == NULL)
                    return -1;

                for (u16 j = 0; j < ((J3DModelData*)pRes)->getMaterialNum(); j++) {
                    J3DMaterial* pMaterial = ((J3DModelData*)pRes)->getMaterialNodePointer(j);
                    pMaterial->change();

                    J3DMaterialAnm* pAnm = new J3DMaterialAnm();
                    if (pAnm == NULL)
                        return -1;
                    pMaterial->setMaterialAnm(pAnm);
                }

                setToonTex(((J3DModelData*)pRes));
            } else if (resType == 'BMDC') {
                pRes = J3DModelLoaderDataBase::load(pRes, 0x51240020);
                if (pRes == NULL)
                    return -1;

                for (u16 j = 0; j < ((J3DModelData*)pRes)->getMaterialNum(); j++) {
                    J3DMaterial* pMaterial = ((J3DModelData*)pRes)->getMaterialNodePointer(j);
                    pMaterial->change();
                }

                setToonTex(((J3DModelData*)pRes));
            } else if (resType == 'BMDS') {
                pRes = J3DModelLoaderDataBase::load(pRes, 0x00220020);
                if (pRes == NULL)
                    return -1;

                for (u16 j = 0; j < ((J3DModelData*)pRes)->getMaterialNum(); j++) {
                    J3DMaterial* pMaterial = ((J3DModelData*)pRes)->getMaterialNodePointer(j);
                    pMaterial->change();

                    J3DMaterialAnm* pAnm = new J3DMaterialAnm();
                    if (pAnm == NULL)
                        return -1;
                    pMaterial->setMaterialAnm(pAnm);
                }

                setToonTex(((J3DModelData*)pRes));
            } else if (resType == 'BSMD') {
                pRes = J3DModelLoaderDataBase::load(pRes, 0x01020020);
                if (pRes == NULL)
                    return -1;

                for (u16 j = 0; j < ((J3DModelData*)pRes)->getMaterialNum(); j++) {
                    J3DMaterial* pMaterial = ((J3DModelData*)pRes)->getMaterialNodePointer(j);

                    J3DMaterialAnm* pAnm = new J3DMaterialAnm();
                    if (pAnm == NULL)
                        return -1;

                    pMaterial->setMaterialAnm(pAnm);
                }

                setToonTex(((J3DModelData*)pRes));
            } else if (resType == 'BDL ') {
                pRes = J3DModelLoaderDataBase::loadBinaryDisplayList(pRes, 0x00002020);
                if (pRes == NULL)
                    return -1;

                setToonTex(((J3DModelData*)pRes));
            } else if (resType == 'BDLL') {
                pRes = J3DModelLoaderDataBase::loadBinaryDisplayList(pRes, 0x00001020);
                if (pRes == NULL)
                    return -1;
            } else if (resType == 'BDLM') {
                pRes = J3DModelLoaderDataBase::loadBinaryDisplayList(pRes, 0x00002020);
                if (pRes == NULL)
                    return -1;

                for (u16 j = 0; j < ((J3DModelData*)pRes)->getMaterialNum(); j++) {
                    J3DMaterial* pMaterial = ((J3DModelData*)pRes)->getMaterialNodePointer(j);

                    J3DMaterialAnm* pAnm = new J3DMaterialAnm();
                    if (pAnm == NULL)
                        return -1;

                    pMaterial->setMaterialAnm(pAnm);
                }

                setToonTex(((J3DModelData*)pRes));
            } else if (resType == 'BDLI') {
                pRes = J3DModelLoaderDataBase::loadBinaryDisplayList(pRes, 0x01002020);
                if (pRes == NULL)
                    return -1;

                for (u16 j = 0; j < ((J3DModelData*)pRes)->getMaterialNum(); j++) {
                    J3DMaterial* pMaterial = ((J3DModelData*)pRes)->getMaterialNodePointer(j);

                    J3DMaterialAnm* pAnm = new J3DMaterialAnm();
                    if (pAnm == NULL)
                        return -1;
                    pMaterial->setMaterialAnm(pAnm);
                }

                setToonTex(((J3DModelData*)pRes));
            } else if (resType == 'BDLC') {
                pRes = J3DModelLoaderDataBase::loadBinaryDisplayList(pRes, 0x00002020);
                if (pRes == NULL)
                    return -1;

                setToonTex(((J3DModelData*)pRes));
            } else if (resType == 'BLS ') {
                pRes = J3DClusterLoaderDataBase::load(pRes);
                if (pRes == NULL)
                    return -1;
            } else if (resType == 'BCKS' || resType == 'BCK ') {
                void *pBasData;
                if (*((u32*)((char*)pRes + 0x1C)) != 0xFFFFFFFF)
                    pBasData = (void*)(*((u32*)((char*)pRes + 0x1C)) + ((u32)pRes));
                else
                    pBasData = NULL;

                mDoExt_transAnmBas *pAnm  = new mDoExt_transAnmBas(pBasData);
                if (pAnm == NULL)
                    return -1;

                J3DAnmLoaderDataBase::setResource(pAnm, pRes);
                pRes = pAnm;
            } else if (resType == 'BTP ' || resType == 'BTK ' || resType == 'BPK ' || resType == 'BRK ' || resType == 'BLK ' || resType == 'BVA ') {
                pRes = J3DAnmLoaderDataBase::load(pRes);
                if (pRes == NULL)
                    return -1;
            } else if (resType == 'BMT ') {
                pRes = J3DModelLoaderDataBase::loadMaterialTable(pRes);
                if (pRes == NULL)
                    return -1;

                for (u16 j = 0; j < ((J3DMaterialTable*)pRes)->getMaterialNum(); j++) {
                    J3DMaterial* pMaterial = ((J3DMaterialTable*)pRes)->getMaterialNodePointer(j);
                    pMaterial->change();
                }

                setToonTex((J3DMaterialTable*)pRes);
            } else if (resType == 'BMTM') {
                pRes = J3DModelLoaderDataBase::loadMaterialTable(pRes);
                if (pRes == NULL)
                    return -1;

                for (u16 j = 0; j < ((J3DMaterialTable*)pRes)->getMaterialNum(); j++) {
                    J3DMaterial* pMaterial = ((J3DMaterialTable*)pRes)->getMaterialNodePointer(j);
                    pMaterial->change();

                    J3DMaterialAnm* pAnm = new J3DMaterialAnm();
                    if (pAnm == NULL)
                        return -1;
                    pMaterial->setMaterialAnm(pAnm);
                }

                setToonTex((J3DMaterialTable*)pRes);
            } else if (resType == 'DZB ') {
                pRes = cBgS::ConvDzb(pRes);
            }

next:
            mRes[pArcFinder->mEntryFileIndex] = pRes;
        }

        delete pArcFinder;
    }

    return 0;
}

/* 8006E8FC-8006EBD0       .text setRes__11dRes_info_cFv */
int dRes_info_c::setRes() {
    if (mpArchive == NULL) {
        if (mpDMCommand == NULL) {
            return -1;
        }
        if ((int)mpDMCommand->mIsDone == 0) {
            return 1;
        }

        mpArchive = mpDMCommand->getArchive();
        mpParentHeap = mpDMCommand->getHeap();

        delete mpDMCommand;
        mpDMCommand = NULL;

        if (mpArchive == NULL) {
            OSReport_Error("<%s.arc> setRes: archive mount error !!\n", mArchiveName);
            return -1;
        }
        if (mpParentHeap != NULL) {
            mDataHeap = mDoExt_createSolidHeapToCurrent(0, mpParentHeap, 0x20);
            JUT_ASSERT(0x3f5, mDataHeap != 0);

            loadResource();
            mDoExt_restoreCurrentHeap();
            mDoExt_adjustSolidHeap(mDataHeap);
        } else {
            mDataHeap = mDoExt_createSolidHeapFromGameToCurrent(0, 0);
            if (mDataHeap == NULL) {
                OSReport_Error("<%s.arc> mDMCommandsetRes: can\'t alloc memory\n", this);
                return 0xFFFFFFFF;
            }

            loadResource();
            mDoExt_restoreCurrentHeap();

            static volatile s32 mode = 1;
            static JKRExpHeap::EAllocMode allocMode = JKRExpHeap::ALLOC_MODE_1;
            mDoExt_getGameHeap()->setAllocationMode(allocMode);
            mode = 2;

            if (mode == 0) {
                u32 allocSize = (mDataHeap->getSize() - mpParentHeap->getFreeSize()) + 0x8c;
                JKRSolidHeap * pHeap = mDoExt_createSolidHeapFromGameToCurrent(allocSize, 0);
                if (pHeap != NULL) {
                    mDoExt_adjustSolidHeap(mDataHeap);
                    mRes = NULL;
                    mDoExt_destroySolidHeap(mDataHeap);
                    loadResource();
                    mDoExt_restoreCurrentHeap();
                    mDoExt_adjustSolidHeap(pHeap);
                    mDataHeap = pHeap;
                } else {
                    mDoExt_adjustSolidHeap(mDataHeap);
                }
            } else if (mode == 1) {
                JKRSolidHeap * pHeap = mDoExt_createSolidHeapFromGameToCurrent(mDoExt_adjustSolidHeap(mDataHeap) + 0x10, 0);
                if (pHeap != NULL) {
                    if (pHeap < mDataHeap) {
                        mRes = NULL;
                        mDoExt_destroySolidHeap(mDataHeap);
                        loadResource();
                        mDoExt_adjustSolidHeap(pHeap);
                        mDataHeap = pHeap;
                    } else {
                        mDoExt_destroySolidHeap(pHeap);
                    }

                    mDoExt_restoreCurrentHeap();
                }
            } else {
                mDoExt_adjustSolidHeap(mDataHeap);
            }

            JKRExpHeap * pGameHeap = mDoExt_getGameHeap();
            pGameHeap->mAllocMode = 0;
        }

        u32 heapSize = mDataHeap->getHeapSize();
        void* heapStartAddr = mDataHeap->getStartAddr();
        DCStoreRangeNoSync(heapStartAddr, heapSize);
    }

    return 0;
}

/* 8006EBD0-8006EBF8       .text getArcHeader__FP10JKRArchive */
static SArcHeader* getArcHeader(JKRArchive* pArchive) {
    if (pArchive != NULL) {
        switch (pArchive->getMountMode()) {
        case JKRArchive::MOUNT_MEM:
            return ((JKRMemArchive*)pArchive)->getArcHeader();
        }
    }

    return NULL;
}

static void dummy() {
    OSReport("%5.1f %5x %5.1f %5x %3d %s\n");
}

/* 8006EBF8-8006ECF4       .text dump_long__11dRes_info_cFP11dRes_info_ci */
void dRes_info_c::dump_long(dRes_info_c* pRes, int num) {
    // regalloc
    s32 size;
    JKRArchive* archive;
    mDoDvdThd_command_c* command;
    SArcHeader* header;
    s32 refCount;
    s32 heapSize;
    s32 i;

    JUTReportConsole_f("dRes_info_c::dump %08x %d\n", pRes, num);
    JUTReportConsole_f("No Command  Archive  ArcHeader(size) SolidHeap(size) Resource Count ArchiveName\n");

    for (i = 0; i < num; i++) {
        refCount = pRes->getCount();
        if (refCount != 0) {
            heapSize = JKRHeap::getSize(pRes->mDataHeap, NULL);
            size = JKRHeap::getSize(getArcHeader(pRes->getArchive()), NULL);
            archive = pRes->getArchive();
            command = pRes->getDMCommand();
            header = getArcHeader(archive);

            JUTReportConsole_f("%2d %08x %08x %08x(%5x) %08x(%5x) %08x %3d   %s\n",
                i, command, archive, header, size, pRes->mDataHeap,
                heapSize, pRes->mRes, refCount, pRes->getArchiveName());
        }
        pRes++;
    }
}

// This hack fixes the .sdata2 section, since dRes_info_c::dump puts the floats in the wrong order
static f32 dummy(int i) {
    return i;
}

/* 8006ECF4-8006EE6C       .text dump__11dRes_info_cFP11dRes_info_ci */
void dRes_info_c::dump(dRes_info_c* pRes, int num) {
    int totalArcHeaderSize;
    int totalHeapSize;
    int arcHeaderSize;
    int heapSize;
    char* archiveName;
    JUTReportConsole_f("dRes_info_c::dump %08x %d\n", pRes, num);
    JUTReportConsole_f("No ArchiveSize(KB) SolidHeapSize(KB) Cnt ArchiveName\n");
    totalArcHeaderSize = 0;
    totalHeapSize = 0;
    for (int i = 0; i < num; i++) {
        if (pRes->getCount()) {
            arcHeaderSize = JKRGetMemBlockSize(NULL, getArcHeader(pRes->getArchive()));
            heapSize = JKRGetMemBlockSize(NULL, pRes->mDataHeap);
            archiveName = pRes->getArchiveName();
            JUTReportConsole_f("%2d %6.1f %6x %6.1f %6x %3d %s\n", i, arcHeaderSize / 1024.0f,
                               arcHeaderSize, heapSize / 1024.0f, heapSize, pRes->getCount(),
                               archiveName);
            totalArcHeaderSize += arcHeaderSize;
            totalHeapSize += heapSize;
        }
        pRes++;
    }
    JUTReportConsole_f(
        "----------------------------------------------\n   %6.1f %6x %6.1f %6x   Total\n\n",
        totalArcHeaderSize / 1024.0f, totalArcHeaderSize, totalHeapSize / 1024.0f, totalHeapSize);
}

/* 8006EE6C-8006EF34       .text __dt__14dRes_control_cFv */
dRes_control_c::~dRes_control_c() {
    for (s32 i = 0; i < (s32)ARRAY_SIZE(mObjectInfo); i++)
        mObjectInfo[i].~dRes_info_c();
    for (s32 i = 0; i < (s32)ARRAY_SIZE(mStageInfo); i++)
        mStageInfo[i].~dRes_info_c();
}

/* 8006EF34-8006F01C       .text setRes__14dRes_control_cFPCcP11dRes_info_ciPCcUcP7JKRHeap */
int dRes_control_c::setRes(const char* pArcName, dRes_info_c* pInfoArr, int infoNum, const char* pArcPath, u8 direction, JKRHeap* pHeap) {
    dRes_info_c * pInfo = getResInfo(pArcName, pInfoArr, infoNum);

    if (pInfo == NULL) {
        pInfo = newResInfo(pInfoArr, infoNum);
        if (pInfo == NULL) {
            OSReport_Error("<%s.arc> dRes_control_c::setRes: 空きリソース情報ポインタがありません\n", pArcName);
            pInfo->~dRes_info_c();
            return FALSE;
        }

        if (!pInfo->set(pArcName, pArcPath, direction, pHeap)) {
            OSReport_Error("<%s.arc> dRes_control_c::setRes: res info set error !!\n", pArcName);
            pInfo->~dRes_info_c();
            return FALSE;
        }
    }

    pInfo->incCount();
    return TRUE;
}

/* 8006F01C-8006F074       .text syncRes__14dRes_control_cFPCcP11dRes_info_ci */
int dRes_control_c::syncRes(char const* pArcName, dRes_info_c* pInfo, int infoNum) {
    dRes_info_c* resInfo = getResInfo(pArcName, pInfo, infoNum);

    if (resInfo == NULL) {
        OSReport_Error("<%s.arc> syncRes: リソース未登録!!\n", pArcName);
        return 1;
    } else {
        return resInfo->setRes();
    }
}

/* 8006F074-8006F0E8       .text deleteRes__14dRes_control_cFPCcP11dRes_info_ci */
int dRes_control_c::deleteRes(char const* pArcName, dRes_info_c* pInfo, int infoNum) {
    dRes_info_c* resInfo = getResInfo(pArcName, pInfo, infoNum);

    if (resInfo == NULL) {
        OSReport_Error("<%s.arc> deleteRes: res nothing !!\n(未登録のリソースを削除してるのを発見しました！修正してください。)\n", pArcName);
        return 0;
    } else {
        if (resInfo->decCount() == 0) {
            resInfo->~dRes_info_c();
        }
        return 1;
    }
}

/* 8006F0E8-8006F164       .text getResInfo__14dRes_control_cFPCcP11dRes_info_ci */
dRes_info_c* dRes_control_c::getResInfo(char const* pArcName, dRes_info_c* pResInfo, int infoNum) {
    for (s32 i = 0; i < infoNum; i++) {
        if (pResInfo->getCount() != 0)
            if (!strcmp(pArcName, pResInfo->getArchiveName()))
                return pResInfo;
        pResInfo++;
    }

    return NULL;
}

/* 8006F164-8006F18C       .text newResInfo__14dRes_control_cFP11dRes_info_ci */
dRes_info_c* dRes_control_c::newResInfo(dRes_info_c* pResInfo, int infoNum) {
    for (int i = 0; i < infoNum; i++) {
        if (pResInfo->getCount() == 0) {
            return pResInfo;
        }
        pResInfo++;
    }
    return NULL;
}

/* 8006F18C-8006F208       .text getResInfoLoaded__14dRes_control_cFPCcP11dRes_info_ci */
dRes_info_c* dRes_control_c::getResInfoLoaded(char const* pArcName, dRes_info_c* pResInfo,
                                              int infoNum) {
    dRes_info_c* resInfo = getResInfo(pArcName, pResInfo, infoNum);

    if (resInfo == NULL) {
        OSReport_Error("<%s.arc> getRes: res nothing !!\n", pArcName);
        resInfo = NULL;
    } else if (resInfo->getArchive() == NULL) {
        OSReport_Error("<%s.arc> getRes: res during reading !!\n", pArcName);
        resInfo = NULL;
    }
    return resInfo;
}

/* 8006F208-8006F298       .text getRes__14dRes_control_cFPCclP11dRes_info_ci */
void* dRes_control_c::getRes(char const* pArcName, s32 resIdx, dRes_info_c* pInfo, int infoNum) {
    dRes_info_c* resInfo = getResInfoLoaded(pArcName, pInfo, infoNum);

    if (resInfo == NULL)
        return resInfo;

    u32 fileCount = resInfo->getResNum();

    if (resIdx >= (int)fileCount) {
        OSReport_Error("<%s.arc> getRes: res index over !! index=%d count=%d\n", pArcName, resIdx,
                       fileCount);
        return NULL;
    }
    return resInfo->getRes(resIdx);
}

/* 8006F298-8006F34C       .text getRes__14dRes_control_cFPCcPCcP11dRes_info_ci */
void* dRes_control_c::getRes(char const* pArcName, char const* resName, dRes_info_c* pInfo, int infoNum) {
    dRes_info_c* resInfo = getResInfoLoaded(pArcName, pInfo, infoNum);

    if (resInfo == NULL)
        return resInfo;

    s32 fileNum = resInfo->getResNum();
    for (s32 i = 0; i < fileNum; i++) {
        if (resInfo->getRes(i) != NULL) {
            JKRArchive::SDirEntry dirEntry;
            resInfo->getArchive()->getDirEntry(&dirEntry, i);
            if (strcmp(resName, dirEntry.name) == 0)
                return resInfo->getRes(i);
        }
    }

    return NULL;
}

/* 8006F34C-8006F3BC       .text getIDRes__14dRes_control_cFPCcUsP11dRes_info_ci */
void* dRes_control_c::getIDRes(char const* pArcName, u16 param_1, dRes_info_c* pInfo, int infoNum) {
    dRes_info_c* resInfo = getResInfoLoaded(pArcName, pInfo, infoNum);

    if (resInfo == NULL) {
        return resInfo;
    }

    JKRArchive* archive = resInfo->getArchive();
    s32 index = mDoExt_resIDToIndex(archive, param_1);

    if (index < 0) {
        return 0;
    }

    return resInfo->getRes(index);
}

/* 8006F3BC-8006F430       .text syncAllRes__14dRes_control_cFP11dRes_info_ci */
int dRes_control_c::syncAllRes(dRes_info_c* pInfo, int infoNum) {
    for (int i = 0; i < infoNum; i++) {
        if (pInfo->getDMCommand() != NULL && pInfo->setRes() > 0) {
            return 1;
        }
        pInfo++;
    }
    return 0;
}

/* 8006F430-8006F500       .text setStageRes__14dRes_control_cFPCcP7JKRHeap */
int dRes_control_c::setStageRes(char const* pArcName, JKRHeap* pHeap) {
    char path[20];
    snprintf(path, sizeof(path), "/res/Stage/%s/", strcmp(dComIfGp_getStartStageName(), "ma2room") == 0 && dComIfGs_isEventBit(0x1820) ? "ma3room" : dComIfGp_getStartStageName());
    return setRes(pArcName, &mStageInfo[0], ARRAY_SIZE(mStageInfo), path, 1, pHeap);
}

/* 8006F500-8006F580       .text dump__14dRes_control_cFv */
void dRes_control_c::dump() {
    JUTReportConsole_f("\ndRes_control_c::dump mObjectInfo\n");
    dRes_info_c::dump(&mObjectInfo[0], ARRAY_SIZE(mObjectInfo));
    dRes_info_c::dump_long(&mObjectInfo[0], ARRAY_SIZE(mObjectInfo));

    JUTReportConsole_f("\ndRes_control_c::dump mStageInfo\n");
    dRes_info_c::dump(&mStageInfo[0], ARRAY_SIZE(mStageInfo));
    dRes_info_c::dump_long(&mStageInfo[0], ARRAY_SIZE(mStageInfo));
}
