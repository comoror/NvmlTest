// NvmlTest.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <Windows.h>
#include "nvml.h"

//#pragma comment(lib, "nvml.lib")

//define the nvml function pointer
typedef nvmlReturn_t(*nvmlInit_t)();
typedef nvmlReturn_t(*nvmlShutdown_t)();
typedef nvmlReturn_t(*nvmlDeviceGetCount_t)(unsigned int*);
typedef nvmlReturn_t(*nvmlDeviceGetHandleByIndex_t)(unsigned int, nvmlDevice_t*);
typedef nvmlReturn_t(*nvmlDeviceGetGraphicsRunningProcesses_t)(nvmlDevice_t, unsigned int*, nvmlProcessInfo_t*);
typedef nvmlReturn_t(*nvmlDeviceGetComputeRunningProcesses_t)(nvmlDevice_t, unsigned int*, nvmlProcessInfo_t*);
typedef nvmlReturn_t(*nvmlDeviceGetMPSComputeRunningProcesses_t)(nvmlDevice_t, unsigned int*, nvmlProcessInfo_t*);
typedef nvmlReturn_t(*nvmlSystemGetProcessName_t)(unsigned int, char*, unsigned int);

//ref: https://docs.nvidia.com/deploy/nvml-api/nvml-api-reference.html
//ref: https://docs.nvidia.com/deploy/nvml-api/group__nvmlDeviceQueries.html
int main()
{
    //load nvml.dll from the system dir
    HMODULE hNvml = LoadLibraryW(L"nvml.dll");
    if (hNvml == NULL)
    {
		std::cout << "LoadLibrary nvml.dll failed: driver not installed or no nvidia gpu device\n";
		return -1;
	}

	//get the function pointer
	nvmlInit_t fn_nvmlInit = (nvmlInit_t)GetProcAddress(hNvml, "nvmlInit_v2");
	nvmlShutdown_t fn_nvmlShutdown = (nvmlShutdown_t)GetProcAddress(hNvml, "nvmlShutdown");
	nvmlDeviceGetCount_t fn_nvmlDeviceGetCount = (nvmlDeviceGetCount_t)GetProcAddress(hNvml, "nvmlDeviceGetCount_v2");
	nvmlDeviceGetHandleByIndex_t fn_nvmlDeviceGetHandleByIndex = (nvmlDeviceGetHandleByIndex_t)GetProcAddress(hNvml, "nvmlDeviceGetHandleByIndex_v2");
	nvmlDeviceGetGraphicsRunningProcesses_t fn_nvmlDeviceGetGraphicsRunningProcesses = (nvmlDeviceGetGraphicsRunningProcesses_t)GetProcAddress(hNvml, "nvmlDeviceGetGraphicsRunningProcesses_v3");
	nvmlDeviceGetComputeRunningProcesses_t fn_nvmlDeviceGetComputeRunningProcesses = (nvmlDeviceGetComputeRunningProcesses_t)GetProcAddress(hNvml, "nvmlDeviceGetComputeRunningProcesses_v3");
	nvmlDeviceGetMPSComputeRunningProcesses_t fn_nvmlDeviceGetMPSComputeRunningProcesses = (nvmlDeviceGetMPSComputeRunningProcesses_t)GetProcAddress(hNvml, "nvmlDeviceGetMPSComputeRunningProcesses_v3");
	nvmlSystemGetProcessName_t fn_nvmlSystemGetProcessName = (nvmlSystemGetProcessName_t)GetProcAddress(hNvml, "nvmlSystemGetProcessName");
	if (fn_nvmlInit == NULL || fn_nvmlShutdown == NULL 
		|| fn_nvmlDeviceGetCount == NULL || fn_nvmlDeviceGetHandleByIndex == NULL 
		|| fn_nvmlDeviceGetGraphicsRunningProcesses == NULL || fn_nvmlDeviceGetComputeRunningProcesses == NULL
		|| fn_nvmlDeviceGetMPSComputeRunningProcesses == NULL || fn_nvmlSystemGetProcessName == NULL)
	{
		std::cout << "GetProcAddress failed\n";
		FreeLibrary(hNvml);
		return -1;
	}

	do
	{
		//call the nvmlInit function
		nvmlReturn_t result = fn_nvmlInit();
		if (result != NVML_SUCCESS)
		{
			std::cout << "nvmlInit failed: " << result << "\n";
			break;
		}

		//get the device count
		unsigned int deviceCount = 0;
		result = fn_nvmlDeviceGetCount(&deviceCount);
		if (result != NVML_SUCCESS)
		{
			std::cout << "nvmlDeviceGetCount failed: " << result << "\n";
		}
		else
		{
			std::cout << "found nvidia device count: " << deviceCount << std::endl;

			//get the running processes
			for (unsigned int i = 0; i < deviceCount; i++)
			{
				nvmlDevice_t device;
				result = fn_nvmlDeviceGetHandleByIndex(i, &device);
				if (result != NVML_SUCCESS)
				{
					std::cout << "nvmlDeviceGetHandleByIndex failed: " << result << "\n";
					break;
				}

				unsigned int runningProcessCount = 256;
				nvmlProcessInfo_t infos[256] = { 0 };

				//1. query Graphics running processes
				result = fn_nvmlDeviceGetGraphicsRunningProcesses(device, &runningProcessCount, infos);
				if (result != NVML_SUCCESS)
				{
					std::cout << "nvmlDeviceGetGraphicsRunningProcesses failed: " << result << "\n";
				}
				else
				{
					std::cout << "device " << i << " running " << runningProcessCount << " Graphics process" << std::endl;
					for (unsigned int j = 0; j < runningProcessCount; j++)
					{
						//get the process name from the pid
						char pname[256] = { 0 };
						result = fn_nvmlSystemGetProcessName(infos[j].pid, pname, 256);
						if (result != NVML_SUCCESS)
						{
							std::cout << "nvmlSystemGetProcessName failed(4: require admin privilege): " << result << "\n";
						}
						std::cout << "\tpid: " << infos[j].pid << " name: " << pname << std::endl;
					}
				}

				//2. query Compute running processes
				runningProcessCount = 256;
				ZeroMemory(infos, sizeof(infos));
				result = fn_nvmlDeviceGetComputeRunningProcesses(device, &runningProcessCount, infos);
				if (result != NVML_SUCCESS)
				{
					std::cout << "nvmlDeviceGetComputeRunningProcesses failed: " << result << "\n";
				}
				else
				{
					std::cout << "device " << i << " running " << runningProcessCount << " Compute process" << std::endl;
					for (unsigned int j = 0; j < runningProcessCount; j++)
					{
						//get the process name from the pid
						char pname[256] = { 0 };
						result = fn_nvmlSystemGetProcessName(infos[j].pid, pname, 256);
						if (result != NVML_SUCCESS)
						{
							std::cout << "nvmlSystemGetProcessName failed(4: require admin privilege): " << result << "\n";
						}
						std::cout << "\tpid: " << infos[j].pid << " name: " << pname << std::endl;
					}
				}

				//3. query MPSCompute running processes
				runningProcessCount = 256;
				ZeroMemory(infos, sizeof(infos));
				result = fn_nvmlDeviceGetMPSComputeRunningProcesses(device, &runningProcessCount, infos);
				if (result != NVML_SUCCESS)
				{
					std::cout << "nvmlDeviceGetMPSComputeRunningProcesses failed: " << result << "\n";
				}
				else
				{
					std::cout << "device " << i << " running " << runningProcessCount << " MPSCompute process" << std::endl;
					for (unsigned int j = 0; j < runningProcessCount; j++)
					{
						//get the process name from the pid
						char pname[256] = { 0 };
						result = fn_nvmlSystemGetProcessName(infos[j].pid, pname, 256);
						if (result != NVML_SUCCESS)
						{
							std::cout << "nvmlSystemGetProcessName failed(4: require admin privilege): " << result << "\n";
						}
						std::cout << "\tpid: " << infos[j].pid << " name: " << pname << std::endl;
					}
				}
			}
		}

		//shutdown nvml
		fn_nvmlShutdown();
	} while (0);

	//free the nvml.dll
	FreeLibrary(hNvml);
}

// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门使用技巧: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
