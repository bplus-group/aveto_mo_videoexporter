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
* \brief Implementation of Exporter Measurement Object
* \author Thomas Hochstrasser, Erik Verhoeven
* \copyright (C)2015-2021 b-plus technologies GmbH
* \date 09.11.2021
* \version 2.4.240
*
******************************************************************************/

#include "../include/VideoExporter.h"

AVETO::Core::TStatus CVideoExporter::Initialize()
{
	std::lock_guard<std::mutex> lock(mtx_Settings);
	m_bAllowEncoding = true;
	m_uiCounter = 0;

	return AVETO_S_OK;
}


AVETO::Core::TStatus CVideoExporter::Terminate()
{
	EndEncoding();
	return AVETO_S_OK;
}


void CVideoExporter::SetImage(const AVETO::Core::SDataPacket* rgsDataPackets, uint32_t uiPackets)
{
	if (!uiPackets || !rgsDataPackets) return;

	for (uint32_t i = 0; i < uiPackets; ++i)
	{
		if (rgsDataPackets[i].uiDataLen < m_uiWidth * m_uiHeight * 4) continue;

		if (m_bAllowEncoding)
		{
			{
				std::lock_guard<std::mutex> lock_Enc(mtx_Encoding);
				if (!m_bEncodingInProgress)
				{
					StartEncoding();
				}
			}
			{
				std::lock_guard<std::mutex> lock2(mtx_Queue);
				m_queue.push(AvCore::SDataPacketPtr(rgsDataPackets[i]));
			}
		}
	}
}


void CVideoExporter::OnConnectedConnectorChanged(const char* szOwnConnectorName,
	uint32_t uiOwnConnectorFlags,
	AVETO::Core::TObjID tConnectedConnectorID)
{
	// Check whether this is the input connector.
	if (!AvCore::CompareRestrictedBitmask(uiOwnConnectorFlags, AVETO::Core::EConnectorFlags::direction_flag,
		AVETO::Core::EConnectorFlags::direction_in)) return;

	// There is only one input connector... Therefore name-check is not necessary.
	{
		std::lock_guard<std::mutex> lock(mtx_Settings);
		m_uiWidth = AvCore::GetProp<uint32_t>(tConnectedConnectorID, "Width");
		m_uiHeight = AvCore::GetProp<uint32_t>(tConnectedConnectorID, "Height");
	}
	ChangeEncoding();
}

void CVideoExporter::OnConnect(const AVETO::Core::SConnectionEvent& rsConnectInfo)
{
	// This connection is between a output connector of another object and the input connector of this object.
	{
		std::lock_guard<std::mutex> lock(mtx_Settings);
		m_uiWidth = AvCore::GetProp<uint32_t>(rsConnectInfo.tOutConnectorID, "Width");
		m_uiHeight = AvCore::GetProp<uint32_t>(rsConnectInfo.tOutConnectorID, "Height");
	}
	ChangeEncoding();
}

void CVideoExporter::OnDisconnect(const AVETO::Core::SConnectionEvent& rsConnectInfo)
{
	EndEncoding();
}

void CVideoExporter::ResetCounter()
{
	m_uiCounter = 0;
}

void CVideoExporter::StartEncoding()
{
	std::lock_guard<std::mutex> lock(mtx_Settings);
	m_bCancelThread = true;
	if (m_thWorkerThread.joinable())
	{
		m_thWorkerThread.join();
	}
	m_bCancelThread = false;
	m_thWorkerThread = std::thread(&CVideoExporter::ThreadFunction, this);
}


void CVideoExporter::EndEncoding()
{
	m_bAllowEncoding = false;
	m_bCancelThread = true;
	if (m_thWorkerThread.joinable())
	{
		m_thWorkerThread.join();
	}
	m_bCancelThread = false;
}

void CVideoExporter::ChangeEncoding()
{
	m_bAllowEncoding = false;
	m_bCancelThread = true;
	if (m_thWorkerThread.joinable())
	{
		m_thWorkerThread.join();
	}
	m_bAllowEncoding = true;
	m_bCancelThread = false;
}

void CVideoExporter::ThreadFunction()
{
	{
		std::lock_guard<std::mutex> lock_Enc(mtx_Encoding);
		m_bEncodingInProgress = true;
	}
	SetThreadDescription(
		GetCurrentThread(),
		L"VideoExporter Worker Thread"
	);
	cv::VideoWriter videoWriter;
	try
	{
		Thread_Init(videoWriter);
		Thread_EncodingLoop(videoWriter);
	}
	catch (std::runtime_error & e)
	{
		std::cerr << "Error Encoding Video Stream: " << e.what() << "\n";
	}

	Thread_Shutdown();
	if (videoWriter.isOpened())
		videoWriter.release();
}

void CVideoExporter::Thread_Init(cv::VideoWriter & videoWriter)
{
	std::lock_guard<std::mutex> lock(mtx_Settings);
	m_ssCurrentFilenameUsed = m_ssOutfileName + (m_bUseCounterSuffix ? (m_uiCounter < 10 ? "00" : (m_uiCounter < 100 ? "0" : "")) + std::to_string(m_uiCounter) : "");
	++m_uiCounter;

	if (!std::filesystem::exists(m_ssOutputDirectory))
	{
		std::filesystem::create_directory(m_ssOutputDirectory);
	}

	std::string path = m_ssOutputDirectory;;
	if (path.back() != '\\')
	{
		path += '\\';
	}
	path += m_ssCurrentFilenameUsed + ".avi";

	const int fourcc = cv::VideoWriter::fourcc('M', 'J', 'P', 'G');

	const double fps = m_uiFPS;
	const cv::Size size = cv::Size(m_uiWidth, m_uiHeight);
	const bool open = videoWriter.open(path, fourcc, fps, size);

	if (!open)
	{
		std::lock_guard<std::mutex> lock_Enc(mtx_Encoding);
		m_bEncodingInProgress = false;
		throw std::runtime_error("Could not create/open outputfile '" + path + "' for writing! Possible reasons are the output folder does not exist, missing permissions, or used codec is not installed or available.");
	}
}

void CVideoExporter::Thread_EncodingLoop(cv::VideoWriter & videoWriter)
{
	uint32_t delayCounter = 0;
	uint32_t frameCounter = 0;
	std::chrono::time_point<std::chrono::high_resolution_clock> lastCycleTime = std::chrono::high_resolution_clock::now();
	std::chrono::time_point<std::chrono::high_resolution_clock> currentCycleTime;

	while (m_bAllowEncoding && !m_bCancelThread)
	{
		AvCore::SDataPacketPtr packet;
		{
			currentCycleTime = std::chrono::high_resolution_clock::now();
			std::unique_lock<std::mutex> lock(mtx_Queue);
			if (m_queue.empty())
			{
				lock.unlock();
				delayCounter = static_cast<uint32_t>(std::chrono::duration_cast<std::chrono::milliseconds>(currentCycleTime - lastCycleTime).count());
				if (delayCounter > m_uiTimeout_ms)
				{
					m_bAllowEncoding = false;
					break;
				}
				std::this_thread::sleep_for(std::chrono::milliseconds(10));
				continue;
			}
			packet = m_queue.front();
			m_queue.pop();
		}
		cv::Mat RGBAFrame(m_uiHeight, m_uiWidth, CV_8UC4, const_cast<void *>(packet.GetData())); // !!! Careful to not write to this data !!! this would affect all other MOs that get the same Package
		try
		{
			videoWriter.write(RGBAFrame);
		}
		catch (cv::Exception & e)
		{
			std::cerr << "Error Encoding Video Stream: " << e.what() << "\n";
		}
		lastCycleTime = currentCycleTime;
	}
}

void CVideoExporter::Thread_Shutdown()
{
	{
		std::lock_guard<std::mutex> lock(mtx_Encoding);
		m_bEncodingInProgress = false;
	}
}