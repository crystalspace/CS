#ifndef __INSTP_H__
#define __INSTP_H__

#include "csutil/scf.h"
#include "isystem.h"
#include "iplugin.h"
#include "icmdmgr.h"

#define PROTOCOL_SETUP_OK     (1)
#define PROTOCOL_CONNECT_FAIL (2)
#define PROTOCOL_CONNECT_OK   (3)
#define PROTOCOL_HANDLED_OK   (4)

SCF_VERSION (iProto, 0, 0, 1);

struct iProto: public iBase
{
  public:
    virtual int GetStatus () =0;
    virtual void SetStatus(int status) =0;
};


SCF_VERSION (iPROTO, 0, 0, 2);

struct iPROTO: public iPlugIn
{
  virtual bool Initialize (iSystem *iSys) = 0;
  virtual bool Open()=0;
  virtual bool Close()=0;

  ///
  /// Various Network Events coming from network that need to be handled
  ///
  virtual int OnServerStart(int NetPort) = 0;
  virtual int OnServerStop(int NetPort) = 0;
  virtual int OnConnect(int NetPort) = 0;
  virtual int OnAccept(int NetPort) = 0;
  virtual int OnReceive(int NetPort, size_t len, char *msg) = 0;
  virtual int OnDisconnect(int port) = 0;

  ///
  /// Command processor - for things coming from above
  ///
  virtual int OnCmd(int NetPort, csNode *cmd) = 0;

  ///
  /// Create an outbound protocol "pipe" connection
  ///
  virtual int InitiateClient(char *hostname, int port, int clienttype) = 0;

  ///
  /// Create a server that listens
  ///
  virtual int InitiateServer(char *hostname, 
			     int ipPortNumber,
			     int ServerType,
			     int MaxConnections) = 0;

  /// Setup Network Command Manager. - this wires the protocol upwards
  /// For any call backs required.
  virtual bool AddCmdManager(iCMDMGR *NewCmdManager) = 0;

  /// Utility stuff
  virtual int GetLastError() = 0;

};


#endif 



