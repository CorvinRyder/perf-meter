#pragma once
#include <windows.h>
#include <pdh.h>

struct PerfCounter {
	int count; //Reserved for future refaction
	int status;
	double cpuUsage;
	int memTotal;
	int memUsed;
	int memUsage;
	int powerBatteryLv;
	int powerAcStatus;
};

class PdhQuerier {
	private:
		PDH_HCOUNTER cpuCount;
		PDH_HQUERY query;
		PerfCounter cachedBatteryStatus;
		bool isUpdateBatteryLv;
		bool cachedPowerAcStatus;
		int cachedPowerBatteryLv;
				
	public:
		PdhQuerier();
		~PdhQuerier();
		void SetupCounters();
		int GetCounters(PerfCounter* outCounters);
		void UpdateBatteryNextTick();
};
