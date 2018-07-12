#pragma once 

#include "EVComm.h"

namespace evwork
{

	class CClientConn
		: public IConn
	{
	public:
		CClientConn(const std::string& strPeerIp, uint16_t uPeerPort16);
		CClientConn(int fd, const std::string& strPeerIp, uint16_t uPeerPort16);
		virtual ~CClientConn();
		
		virtual void getPeerInfo(std::string& strPeerIp, uint16_t& uPeerPort16);
		virtual bool sendBin(const char* pData, size_t uSize);

		void cbEvent(int revents);

	private:

		void __noblock();

		void __initTimerNoData();
		void __destroyTimerNoData();
		void __updateTimerNoData();
		static void __cbTimerNoData(struct ev_loop *loop, struct ev_timer *w, int revents);

		void __initTimerDestry();
		void __destroyTimerDestry();
		static void __cbTimerDestry(struct ev_loop *loop, struct ev_timer *w, int revents);

		void __onRead();

		void __onWrite();

		void __appendBuffer(const char* pData, size_t uSize);

		void __sendBuffer();

		size_t __sendData(const char* pData, size_t uSize);

		size_t __recvData(char* pData, size_t uSize);

		void __willFreeMyself(const std::string& strDesc);

	private:
		std::string m_strPeerIp;
		uint16_t m_uPeerPort16;

		bool m_bConnected;

		std::string m_strInput;
		std::string m_strOutput;

		ev_timer m_evTimerNoData;
		ev_timer m_evTimerDestroy;
		bool m_bTimerNoDataStart;
		bool m_bTimerDestroyStart;

		THandle<CClientConn, &CClientConn::cbEvent> m_hRead;
		THandle<CClientConn, &CClientConn::cbEvent> m_hWrite;
	};

}
