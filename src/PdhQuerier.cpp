#include "PdhQuerier.h"

PdhQuerier::PdhQuerier() : query(NULL), cpuCount(NULL) {}

PdhQuerier::~PdhQuerier() {
	if(query!=NULL){
		PdhCloseQuery(query);
	}
}

void PdhQuerier::SetupCounters(){
	PdhOpenQuery(NULL, 0, &query);
	
	PDH_COUNTER_PATH_ELEMENTS counterCpu = {0};
	
	counterCpu.szObjectName="Processor";
	counterCpu.szInstanceName="_Total";
	counterCpu.szCounterName="% Processor Time";
	
	char pathName[128] = {0};
	unsigned long bufferSize = sizeof(pathName);
	
	PdhMakeCounterPath(&counterCpu, pathName, &bufferSize, 0);
	PdhAddCounter(query, pathName, 0, &cpuCount);
	
	// Call for the first time to get the initial data
	PdhCollectQueryData(query);
}

int PdhQuerier::GetCounters(PerfCounter* outCounters){
	PdhCollectQueryData(query);
    PDH_FMT_COUNTERVALUE counterResult = {0};
    PdhGetFormattedCounterValue(cpuCount, PDH_FMT_DOUBLE, 0, &counterResult);
    if(counterResult.CStatus==0){
    	outCounters->cpuUsage = counterResult.doubleValue;
	}else{
		outCounters->status=counterResult.CStatus;
	}
	
    MEMORYSTATUSEX memInfo = {0};
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    if (GlobalMemoryStatusEx(&memInfo)) {
       outCounters->memUsage = memInfo.dwMemoryLoad;
    }
    
    if(isUpdateBatteryLv){
    	isUpdateBatteryLv=false;
		SYSTEM_POWER_STATUS status = {0};
		GetSystemPowerStatus(&status);
		if(status.BatteryFlag==128){
			outCounters->powerAcStatus=true;
			outCounters->powerBatteryLv=-1;
		}else{
			outCounters->powerAcStatus=status.ACLineStatus==1;
			outCounters->powerBatteryLv=status.BatteryLifePercent;
		}
		cachedPowerAcStatus = outCounters->powerAcStatus;
		cachedPowerBatteryLv = outCounters->powerBatteryLv;
	} else {
		outCounters->powerAcStatus=cachedPowerAcStatus;
		outCounters->powerBatteryLv=cachedPowerBatteryLv;
	}
	return counterResult.CStatus;
}

void PdhQuerier::UpdateBatteryNextTick(){
	isUpdateBatteryLv = true;
}
