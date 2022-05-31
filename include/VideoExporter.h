/******************************************************************************/
/*! \file
*
* \verbatim
******************************************************************************
*                                                                            *
*    Copyright (c) 2015-2021, b-plus technologies GmbH.                      *
*                                                                            *
*    All rights are reserved by b-plus technologies GmbH.                    *
*    The Customer is entitled to modify this software under his              *
*    own license terms.                                                      *
*                                                                            *
*    You may use this code according to the license terms of b-plus.         *
*    Please contact b-plus at services@b-plus.com to get the actual          *
*    terms and conditions.                                                   *
*                                                                            *
******************************************************************************
\endverbatim
*
* \brief Definition of Exporter Measurement Object
* \author Thomas Hochstrasser, Erik Verhoeven
* \copyright (C)2015-2021 b-plus technologies GmbH
* \date 09.11.2021
* \version 2.4.240
*
******************************************************************************/

// Suppress the deprecated use of the codecvt library for C++17 and Visual Studio.
#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING

// Suppress the deprecated use of the filesystem library for c++14 and Visual Studio 2019.
#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING

// Product information
#pragma once
#define VERSIONINFO_PRODUCT             "Exporter Measurement Object"
#define VERSIONINFO_AUTHOR              "Your Name"
#define VERSIONINFO_COMMENTS            "Your comment"

// Version information
#define VERSIONINFO_VERSION             2.4
#define VERSIONINFO_BUILD               240
#define VERSIONINFO_SUBBUILD            13227

#if defined(_MSC_VER)
#	define NOMINMAX
#   include <windows.h>
#endif

#include <Core/AvCore.h>
#include <Dev/Support/AvExporterObj.h> // The AVETO Exporter Object (includes all necessary files)

#include <queue>

#include <opencv2/videoio.hpp>
//#include "D:\\supportDebug\\Stopwatch.h"

class CVideoExporter : public AVETO::Dev::Support::CAvetoExporterObject
{
public:
	CVideoExporter() = default;

	~CVideoExporter() override = default;

	DECLARE_OBJECT_CLASS_NAME("Video Exporter")	// Give your class a unique display name
	DECLARE_OBJECT_GROUP_ASSOC(AVETO::Core::g_szGroupGeneric)

	// Connector map
	BEGIN_AVETO_CONNECTOR_MAP()
		AVETO_CONNECTOR_INPUT("IMAGE_RGBa8", "RGBA image connector", SetImage)
	END_AVETO_CONNECTOR_MAP()

	// Interface map
	BEGIN_AVETO_INTERFACE_MAP()
		// Chain from base classes to gain access to their interfaces.
		AVETO_INTERFACE_CHAIN_BASE(AVETO::Dev::Support::CAvetoExporterObject)
		// Add interfaces implemented by this class e.g. via the AVETO_INTERFACE_ENTRY macro.
	END_AVETO_INTERFACE_MAP()

	// Property map
	BEGIN_AVETO_PROPERTY_MAP()
		AVETO_PROPERTY_CHAIN_BASE(AvDev::CAvetoExporterObject)
		AVETO_PROPERTY_ENTRY(m_bAllowEncoding, "Allow Encoding", "Set to true to start encoding of incoming packages")
		//AVETO_PROPERTY_ENTRY_FUNC(GetAllowEncoding, SetAllowEncoding, "Allow Encoding", "Start or Stop encoding")
		AVETO_PROPERTY_ENTRY(m_uiFPS, "Frames per second", "Frames per second")
		//AVETO_PROPERTY_ENTRY(m_uiDatarate, "Datarate", "Video Datarate in kB/s")
		AVETO_PROPERTY_ENTRY(m_ssOutfileName, "Output Filename", "Name of the output file")
		AVETO_PROPERTY_ENTRY(m_ssOutputDirectory, "Output Directory", "Name of the output directory")
		AVETO_PROPERTY_ENTRY(m_bUseCounterSuffix, "Use Numeration as Suffix", "Add incrementing Counter to the End of the Filename")
		AVETO_PROPERTY_ENTRY(m_uiTimeout_ms, "Timeout[ms]", "Minimum time between two packages to stop the video conversion")
	END_AVETO_PROPERTY_MAP()

	/**
	 * \brief Override of IInitialize::Initialize.
	 *		Initialization function called when this object is created.
	 * \return Returns non-error status (e.g. AVETO_S_OK) if the initialization was successfull, 
	 *		or an error status (e.g. AVETO_E_GENERIC_ERROR) if it failed.
	 */
	AVETO::Core::TStatus Initialize() override;

	/**
	 * \brief Override of IInitialize::Terminate.
	 *		Termination function called when this object is to be destroyed.
	 * \return Returns non-error status (e.g. AVETO_S_OK) if the termination was successfull, 
	 *		or an error status (e.g. AVETO_E_GENERIC_ERROR) if it failed.
	 */
	AVETO::Core::TStatus Terminate() override;

	/**
	* \brief Set image frames.
	* \param[in] rgsDataPackets Array of packets to set.
	* \param[in] uiPackets The amount of packets in the array.
	*/
	void SetImage(const AVETO::Core::SDataPacket* rgsDataPackets, uint32_t uiPackets);


private:
	void StartEncoding();

	void EndEncoding();

	// e.g. if the source changes image resolution, finish last encoding, start new encoding
	void ChangeEncoding();

	void ThreadFunction();

	void Thread_Init(cv::VideoWriter &);
	void Thread_EncodingLoop(cv::VideoWriter &);
	void Thread_Shutdown();

	void OnConnect(const AVETO::Core::SConnectionEvent & rsConnectInfo) override;

	void OnConnectedConnectorChanged(const char* szOwnConnectorName, 
		uint32_t uiOwnConnectorFlags,
		AVETO::Core::TObjID tConnectedConnectorID) override;

	void OnDisconnect(const AVETO::Core::SConnectionEvent & rsConnectInfo) override;

	void ResetCounter();

	std::mutex mtx_Encoding;
	std::thread m_thWorkerThread;

	std::mutex mtx_Settings;
	uint32_t m_uiFPS = 25;
	uint32_t m_uiWidth;
	uint32_t m_uiHeight;
	uint32_t m_uiCounter;
	uint32_t m_uiTimeout_ms = 2000;
	bool m_bEncodingInProgress = false;
	bool m_bAllowEncoding = false;
	bool m_bUseCounterSuffix = true;
	std::atomic<bool> m_bCancelThread = false;
	std::string m_ssOutfileName = "Output";
	std::string m_ssOutputDirectory = "C:\\";

	std::mutex mtx_Queue;
	std::queue<AvCore::SDataPacketPtr> m_queue;
	std::string m_ssCurrentFilenameUsed;
};

// Defining this class to be an Aveto object to be found by the application.
DEFINE_AVETO_OBJECT(CVideoExporter)
