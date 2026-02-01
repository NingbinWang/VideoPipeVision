#include "PagesNotify.h"
#include "PagesNotify_Def.h"


using namespace PagesNotifyDef;

static void onLoad(Notification* account)
{
    
}

static void onNotify(Notification* account, STORAGE_INFO_T* info)
{
    switch (info->eCmd)
    {
        case STORAGE_CMD_LOAD:
            onLoad(account);
            break;
        case STORAGE_CMD_SAVE:
            //storageService.SaveFile();
            break;
        case STORAGE_CMD_ADD:
            
            break;
        case STORAGE_CMD_REMOVE:
            //storageService.Remove(info->key);
            break;
        default:
            break;
    }
}

static int onEvent(Notification* account, Notification::EventParam_t* param)
{
    if (param->event == Notification::EVENT_SUB_PULL)
    {
        if (param->size != sizeof(STORAGE_BASIC_INFO_T))
        {
            return Notification::ERROR_SIZE_MISMATCH;
        }

        STORAGE_BASIC_INFO_T* info = (STORAGE_BASIC_INFO_T*) param->data_p;
        //info->isDetect = HAL::SD_GetReady();
        //info->totalSizeMB = HAL::SD_GetCardSizeMB();
        //info->freeSizeMB = 0.0f;
        return 0;
    }

    if (param->event != Notification::EVENT_NOTIFY)
    {
        return Notification::ERROR_UNSUPPORTED_REQUEST;
    }

    if (param->size != sizeof(STORAGE_INFO_T))
    {
        return Notification::ERROR_SIZE_MISMATCH;
    }

    STORAGE_INFO_T* info = (STORAGE_INFO_T*) param->data_p;
    onNotify(account, info);

    return 0;
}

/*
static void onSDEvent(bool insert)
{
    if (insert)
    {
        SystemInfoDef::Storage_Info_t info;
        info.cmd = SystemInfoDef::STORAGE_CMD_LOAD;
        NotifyCenter::Broker()->AccountMaster.Notify("Storage", &info, sizeof(info));
    }
}
*/

void SYS_Storage_Init(Notification* account){
    account->SetEventCallback(onEvent);
    //account->Subscribe("SysStorage");
    //HAL::SD_SetEventCallback(onSDEvent);
}
