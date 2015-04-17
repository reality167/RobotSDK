#include"sync.h"

using namespace RobotSDK;

Sync::Sync(uint portNum, uint basePortID)
{
    paramsbuffer.resize(portNum);
    databuffer.resize(portNum);
    syncparams.resize(portNum);
    syncdata.resize(portNum);
    baseportid=basePortID;
    if(baseportid>=portNum)
    {
        baseportid=0;
    }
    syncrecordid=0;
}

bool Sync::addParamsData(PORT_PARAMS_CAPSULE & inputParams, PORT_DATA_CAPSULE & inputData)
{
    uint i,n=inputParams.size();
    for(i=0;i<n;i++)
    {
        paramsbuffer[i]=inputParams[i]+paramsbuffer[i];
        databuffer[i]=inputData[i]+databuffer[i];
    }
    return generateSyncData();
}

bool Sync::generateSyncData()
{
    uint i,n=databuffer.size();
    QTime basetimestamp=databuffer[baseportid].back()->timestamp;
    if(basetimestamp.isNull())
    {
        return 0;
    }
    for(i=syncrecordid;i<n;i++)
    {
        if(i==baseportid)
        {
            syncrecordid=i+1;
            continue;
        }
        uint j,m=databuffer[i].size();
        for(j=m-1;j>0;j--)
        {
            QTime targettimestamp=databuffer[i].at(j)->timestamp;
            int delta=basetimestamp.msecsTo(targettimestamp);
            if(delta>=0)
            {
                break;
            }
            else
            {
                QTime nexttargettimestamp=databuffer[i].at(j-1)->timestamp;
                int nextdelta=basetimestamp.msecsTo(nexttargettimestamp);
                if(nextdelta<0||(nextdelta<-delta))
                {
                    paramsbuffer[i].pop_back();
                    databuffer[i].pop_back();
                }
                else
                {
                    break;
                }
            }
        }
        if(j==0)
        {
            return 0;
        }
        syncparams[i]=paramsbuffer[i].back();
        syncdata[i]=databuffer[i].back();
        syncrecordid=i+1;
    }
    for(i=0;i<n;i++)
    {
        paramsbuffer[i].pop_back();
        databuffer[i].pop_back();
    }
    syncrecordid=0;
    return 1;
}

TRANSFER_PORT_PARAMS_TYPE Sync::getParams(uint portID)
{
    return syncparams[portID];
}

TRANSFER_PORT_DATA_TYPE Sync::getData(uint portID)
{
    return syncdata[portID];
}

void Sync::clear()
{
    uint i,n=databuffer.size();
    for(i=0;i<n;i++)
    {
        paramsbuffer[i].clear();
        databuffer[i].clear();
    }
    syncrecordid=0;
}
