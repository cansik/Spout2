﻿//
//		SpoutSender
//
// ====================================================================================
//		Revisions :
//
//		23.09.14	- return DirectX 11 capability in SetDX9
//		28.09.14	- Added GL format for SendImage
//					- Added bAlignment (4 byte alignment) flag for SendImage
//					- Added Host FBO for SendTexture, DrawToSharedTexture
//		08.02.15	- Changed default texture format for SendImage in header to GL_RGBA
//		29.05.15	- Included SetAdapter for multiple adapters - Franz Hildgen.
//		02.06.15	- Added GetAdapter, GetNumAdapters, GetAdapterName
//		08.06.15	- Added SelectSenderPanel for user adapter output selection
//		24.08.15	- Added GetHostPath to retrieve the path of the host that produced the sender
//		25.09.15	- Changed SetMemoryShareMode for 2.005 - now will only set true for 2.005 and above
//		09.10.15	- DrawToSharedTexture - invert default false instead of true
//		10.10.15	- Added transition flag to set invert true for 2.004 rather than default false for 2.005
//					- currently not used - see SpoutSDK.cpp CreateSender
//		14.11.15	- changed functions to "const char *" where required
//		17.03.16	- changed to const unsigned char for Sendimage buffer
//		17.09.16	- removed CheckSpout2004() from constructor
//		13.01.17	- Add SetCPUmode, GetCPUmode, SetBufferMode, GetBufferMode
//		15.01.17	- Add GetShareMode, SetShareMode
//		06.06.17	- Add OpenSpout
//		10.06.17	- Add SetFrameReady
//					- Changed CreateSender from (const char* sendername)
//					  to (char* sendername) to return the sender name if changed
//		18.08.18	- Changed CreateSender, InitSender back to const char
//		23.08.18	- Added SendFboTexture
//		05.11.18	- Add IsInitialized
//		11.11.18	- Add high level application functions
//		13.11.18	- Remove CPU mode functions
//		27.11.18	- Add RemovePadding
//		01.12.18	- Add GetFps and GetFrame
//		11.12.18	- Add utility functions
//		14.12.18	- Clean up
//		16.01.19	- Initialize class variables
//		21.01.19	- Add Bind and UnBindSharedTexture
//		26.02.19	- Add IsFrameCountEnabled
//		07.05.19	- Add HoldFps
//		18.06.19	- Change sender Update to include sender name
//		26.06.19	- Changes to Update and spout.UpdateSender
//		13.09.19	- UpdateSender - update class variables for 2.007 methods
//		18.09.19	- Remove UseDX9 from GetDX9 to avoid registry change
//		18.09.19	- Remove redundant 2.007 functions SetupSender and Update
//					- Add invert argument to CreateSender
//		15.10.19	- Check zero width and height for SendData functions 
//		13.01.20	- Remove send data functions and replace with overloads of 2.006 functions
//		19.01.20	- Remove send data functions entirely to simplify
//					- Change SendFboTexture to SendFbo
//		21.01.20	- Remove auto sender update in send functions
//		24.01.20	- Add GetSharedTextureID and CopyTexture for sender as well as receiver
//					- Removed SelectSenderPanel from sender
//
// ====================================================================================
/*

	Copyright (c) 2014-2020, Lynn Jarvis. All rights reserved.

	Redistribution and use in source and binary forms, with or without modification, 
	are permitted provided that the following conditions are met:

		1. Redistributions of source code must retain the above copyright notice, 
		   this list of conditions and the following disclaimer.

		2. Redistributions in binary form must reproduce the above copyright notice, 
		   this list of conditions and the following disclaimer in the documentation 
		   and/or other materials provided with the distribution.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"	AND ANY 
	EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES 
	OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE	ARE DISCLAIMED. 
	IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, 
	INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
	PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
	INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
	LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
	OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/
#include "SpoutSender.h"

SpoutSender::SpoutSender()
{
	m_SenderName[0] = 0;
	m_TextureID = 0;
	m_TextureTarget = 0;
	m_Width = 0;
	m_Height = 0;
	m_dwFormat = 0;

}

//---------------------------------------------------------
SpoutSender::~SpoutSender()
{
	ReleaseSender();
}


// ================= 2.007 functions ======================

//---------------------------------------------------------
unsigned int SpoutSender::GetWidth()
{
	return m_Width;
}

//---------------------------------------------------------
unsigned int SpoutSender::GetHeight()
{
	return m_Height;
}

//---------------------------------------------------------
long SpoutSender::GetFrame()
{
	return (spout.interop.frame.GetSenderFrame());
}
//---------------------------------------------------------
double SpoutSender::GetFps()
{
	return (spout.interop.frame.GetSenderFps());
}

//---------------------------------------------------------
void SpoutSender::HoldFps(int fps)
{
	spout.interop.frame.HoldFps(fps);
}

//---------------------------------------------------------
bool SpoutSender::IsInitialized()
{
	return spout.IsSpoutInitialized();
}

//---------------------------------------------------------
void SpoutSender::DisableFrameCount()
{
	spout.interop.frame.DisableFrameCount();
}

//---------------------------------------------------------
bool SpoutSender::IsFrameCountEnabled()
{
	return spout.interop.frame.IsFrameCountEnabled();
}

// ================= end 2.007 functions ===================


//---------------------------------------------------------
bool SpoutSender::OpenSpout()
{
	return spout.OpenSpout();
}

//---------------------------------------------------------
bool SpoutSender::CreateSender(const char* name, unsigned int width, unsigned int height, DWORD dwFormat)
{
	if (spout.CreateSender(name, width, height, dwFormat)) {
		strcpy_s(m_SenderName, 256, name);
		m_Width = width;
		m_Height = height;
		m_dwFormat = dwFormat;
		return true;
	}
	else {
		m_SenderName[0] = 0;
		m_TextureID = 0;
		m_TextureTarget = 0;
		m_Width = 0;
		m_Height = 0;
		m_dwFormat = 0;
	}

	return false;

}

//---------------------------------------------------------
bool SpoutSender::UpdateSender(const char* name, unsigned int width, unsigned int height)
{
	bool bRet = false;
	// For a name change, close the sender and set up again
	if (strcmp(name, m_SenderName) != 0) {
		ReleaseSender();
		bRet = CreateSender(name, width, height, m_dwFormat);
	}
	else if (width != m_Width || height != m_Height) {
		// For sender update, only width and height are necessary
		m_Width = width;
		m_Height = height;
		bRet = spout.UpdateSender(m_SenderName, m_Width, m_Height);
	}
	return bRet;
}

//---------------------------------------------------------
void SpoutSender::ReleaseSender(DWORD dwMsec)
{
	// Reset class variables
	m_SenderName[0] = 0;
	m_Width = 0;
	m_Height = 0;
	m_dwFormat = 0;
	// Release resources
	spout.ReleaseSender(dwMsec);
}

//---------------------------------------------------------
bool SpoutSender::SendTexture(GLuint TextureID, GLuint TextureTarget, unsigned int width, unsigned int height, bool bInvert, GLuint HostFBO)
{
	return spout.SendTexture(TextureID, TextureTarget, width, height, bInvert, HostFBO);
}

//---------------------------------------------------------
// Send texture attached to the currently bound fbo
bool SpoutSender::SendFbo(GLuint FboID, unsigned int width, unsigned int height, bool bInvert)
{
	return spout.SendFbo(FboID, width, height, bInvert);
}

#ifdef legacyOpenGL
//---------------------------------------------------------
bool SpoutSender::DrawToSharedTexture(GLuint TextureID, GLuint TextureTarget, unsigned int width, unsigned int height, float max_x, float max_y, float aspect, bool bInvert, GLuint HostFBO)
{
	return spout.DrawToSharedTexture(TextureID, TextureTarget, width, height, max_x, max_y, aspect, bInvert, HostFBO);
}
#endif

//---------------------------------------------------------
bool SpoutSender::SendImage(const unsigned char* pixels, unsigned int width, unsigned int height, GLenum glFormat, bool bInvert, GLuint HostFBO)
{
	return spout.SendImage(pixels, width, height, glFormat, bInvert, HostFBO);
}

//---------------------------------------------------------
void SpoutSender::RemovePadding(const unsigned char *source, unsigned char *dest,
	unsigned int width, unsigned int height, unsigned int stride, GLenum glFormat)
{
	return spout.RemovePadding(source, dest, width, height, stride, glFormat);
}

//---------------------------------------------------------
bool SpoutSender::BindSharedTexture()
{
	return spout.BindSharedTexture();
}

//---------------------------------------------------------
bool SpoutSender::UnBindSharedTexture()
{
	return spout.UnBindSharedTexture();
}

//---------------------------------------------------------
GLuint SpoutSender::GetSharedTextureID()
{
	return spout.interop.GetSharedTextureID();
}

//---------------------------------------------------------
bool SpoutSender::SetMemoryShareMode(bool bMem)
{
	return spout.SetMemoryShareMode(bMem);
}

//---------------------------------------------------------
bool SpoutSender::GetMemoryShareMode()
{
	return spout.GetMemoryShareMode();
}

//---------------------------------------------------------
int SpoutSender::GetShareMode()
{
	return (spout.GetShareMode());
}

//---------------------------------------------------------
bool SpoutSender::SetShareMode(int mode)
{
	return (spout.SetShareMode(mode));
}

//---------------------------------------------------------
void SpoutSender::SetBufferMode(bool bActive)
{
	spout.SetBufferMode(bActive);
}

//---------------------------------------------------------
bool SpoutSender::GetBufferMode()
{
	return spout.GetBufferMode();
}

//---------------------------------------------------------
bool SpoutSender::SetDX9(bool bDX9)
{
	return spout.SetDX9(bDX9);
}

//---------------------------------------------------------
bool SpoutSender::GetDX9()
{
	return spout.interop.GetDX9();
}

//---------------------------------------------------------
void SpoutSender::SetDX9compatible(bool bCompatible)
{
	if(bCompatible) {
		// DX11 -> DX9 only works if the DX11 format is set to DXGI_FORMAT_B8G8R8A8_UNORM
		spout.interop.SetDX11format(DXGI_FORMAT_B8G8R8A8_UNORM);
	}
	else {
		// DX11 -> DX11 only
		spout.interop.SetDX11format(DXGI_FORMAT_R8G8B8A8_UNORM);
	}
}

//---------------------------------------------------------
bool SpoutSender::GetDX9compatible()
{
	if(spout.interop.DX11format == DXGI_FORMAT_B8G8R8A8_UNORM)
		return true;
	else
		return false;
	
}

//---------------------------------------------------------
bool SpoutSender::SetAdapter(int index)
{
	return spout.SetAdapter(index);
}

//---------------------------------------------------------
int SpoutSender::GetAdapter()
{
	return spout.GetAdapter();
}

//---------------------------------------------------------
int SpoutSender::GetNumAdapters()
{
	return spout.GetNumAdapters();
}

//---------------------------------------------------------
bool SpoutSender::GetAdapterName(int index, char* adaptername, int maxchars)
{
	return spout.GetAdapterName(index, adaptername, maxchars);
}

//---------------------------------------------------------
int SpoutSender::GetMaxSenders()
{
	// Get the maximum senders allowed from the sendernames class
	return(spout.interop.senders.GetMaxSenders());
}

//---------------------------------------------------------
void SpoutSender::SetMaxSenders(int maxSenders)
{
	// Sets the maximum senders allowed
	spout.interop.senders.SetMaxSenders(maxSenders);
}

//---------------------------------------------------------
bool SpoutSender::GetHostPath(const char* sendername, char* hostpath, int maxchars)
{
	return spout.GetHostPath(sendername, hostpath, maxchars);
}

//---------------------------------------------------------
int SpoutSender::GetVerticalSync()
{
	return spout.interop.GetVerticalSync();
}

//---------------------------------------------------------
bool SpoutSender::SetVerticalSync(bool bSync)
{
	return spout.interop.SetVerticalSync(bSync);
}

//---------------------------------------------------------
bool SpoutSender::CopyTexture(GLuint SourceID, GLuint SourceTarget,
	GLuint DestID, GLuint DestTarget,
	unsigned int width, unsigned int height,
	bool bInvert, GLuint HostFBO)
{
	return spout.interop.CopyTexture(SourceID, SourceTarget, DestID, DestTarget,
		width, height, bInvert, HostFBO);
}

//------------------ debugging aid only --------------------
bool SpoutSender::SenderDebug(char* Sendername, int size)
{
	return spout.interop.senders.SenderDebug(Sendername, size);

}
