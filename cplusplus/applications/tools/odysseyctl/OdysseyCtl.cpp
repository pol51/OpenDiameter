// OdysseyCtl.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "OdysseyCtlClasses.h"

#define USAGE "OdysseyCtl version : 1.0.0\n\
\n\
Usage:\n\
\n\
OdysseyCtl command [..options]\n\
\n\
commands:\n\
    add      Add an network connection to the Odyssey Client Manager\n\
             GUI. This allows addition of a network connection either\n\
             in Open mode or WPA-PSK mode depending on the options\n\
             specified. In Open mode the following command line format\n\
             is required:\n\
\n\
                 OdysseyCtl add [AP SSID]\n\
\n\
             In WPA-PSK mode, the following command line format is\n\
             required:\n\
\n\
                 OdysseyCtl add [AP SSID] [Key in 64 char hex string]\n\
\n\
             Note that adding a network connection will not initiate\n\
             a connection to that network. An explicit call using \n\
             \"OdysseyCtl connect ...\" is required\n\
\n\
    connect  Attempt to connect an existing network connection in the\n\
             Odyssey Client Manager GUI. The connection must exists \n\
             and maybe added using \"OdysseyCtl add ...\" command. The\n\
             following command line format is required: \n\
\n\
                 OdysseyCtl connect [Device Name] [AP SSID]\n\
\n\
    disconn  Attempt to disconnect an existing network connection in\n\
             the Odyssey Client Manager GUI. The connection must exists\n\
             and maybe added using \"OdysseyCtl add ...\" command. If\n\
             the connection is currently associated then a disassociation\n\
             will occur. If not, then nothing happens. The following command\n\
             line format is required:\n\
\n\
                 OdysseyCtl disconn [AP SSID]\n\
\n\
    remove   Attempt to remove an existing network connection in the\n\
             Odyssey Client Manager GUI. If the connection is currently\n\
             associated then a disassociation will occur before the network\n\
             connection is removed. The following command line format\n\
             is required:\n\
\n\
                 OdysseyCtl remove [AP SSID]\n\n"

class CmdArgs
{
public:
    typedef enum {
        ADD_OPEN,
        ADD_KEY,
        CONNECT,
        DISCONNECT,
        REMOVE
    } MODE;

    CmdArgs(int argc, _TCHAR* argv[]);

    MODE Mode() {
        return m_Mode;
    }
    std::string &DeviceName() {
        return m_DeviceName;
    }
    std::string &AccessPoint() {
        return m_AccessPoint;
    }
    std::string &Key() {
        return m_WPAKey;
    }
private:
    MODE m_Mode;
    std::string m_DeviceName;
    std::string m_AccessPoint;
    std::string m_WPAKey;
};

CmdArgs::CmdArgs(int argc, _TCHAR* argv[])
{
    try {
        if (argc < 2) {
            throw(1);
        }
        if (!stricmp(argv[1], "add")) {
            if (argc == 3) {
                m_Mode = ADD_OPEN;
                m_AccessPoint = argv[2];
            }
            else if (argc == 4) {
                m_Mode = ADD_KEY;
                m_AccessPoint = argv[2];
                m_WPAKey = argv[3];
            }
            else {
                throw(1);
            }
        }
        else if (!stricmp(argv[1], "connect")) {
            if (argc == 4) {
                m_Mode = CONNECT;
                m_DeviceName = argv[2];
                m_AccessPoint = argv[3];
            }
            else {
                throw(1);
            }
        }
        else if (!stricmp(argv[1], "disconn")) {
            if (argc == 3) {
                m_Mode = DISCONNECT;
                m_AccessPoint = argv[2];
            }
            else {
                throw(1);
            }
        }
        else if (!stricmp(argv[1], "remove")) {
            if (argc == 3) {
                m_Mode = REMOVE;
                m_AccessPoint = argv[2];
            }
            else {
                throw(1);
            }
        }
        else {
            throw (1);
        }
    }
    catch (int rc) {
        STDOUT << USAGE;
        exit(rc);
    }
}

class CmdProc
{
public:
    virtual int Process(Odyssey_ClientMgr &mgr, 
                        CmdArgs &args) = 0;
};

class CmdMultiplex
{
public:
    CmdMultiplex(CmdArgs &args) :
        m_Args(args) {
        memset(m_Actions, 0, 
            sizeof(CmdProc*)*(CmdArgs::REMOVE+1));
    }
    void Register(CmdProc &proc, CmdArgs::MODE mode) {
        m_Actions[mode] = &proc;
    }
    void Remove(CmdArgs::MODE mode) {
        m_Actions[mode] = 0;
    }
    int Process() {
        if (m_Actions[m_Args.Mode()]) {
            return m_Actions[m_Args.Mode()]->Process(m_Mngr, m_Args);
        }
        return -1;
    }

private:
    CmdProc *m_Actions[CmdArgs::REMOVE+1];
    CmdArgs &m_Args;
    Odyssey_ClientMgr m_Mngr;
};

class CmdAdd : public CmdProc
{
public:
    virtual int Process(Odyssey_ClientMgr &mgr,
                        CmdArgs &args) {
        std::cout << "Adding network ";
        std::cout << args.AccessPoint().data() << std::endl;

        Odyssey_SelectAction action(mgr);
        Odyssey_ConnectionWindow conWin(mgr);
        Odyssey_NetworksWindow netWin(mgr);

        BOOL disconnect = FALSE;
        TCHAR szBuf[1024];
        TCHAR szItem[1024];

        action.Select("Connection");
        if (conWin.GetConnection(szBuf, sizeof(szBuf))) {
            sprintf(szItem, "%s <%s>", 
                    args.AccessPoint().data(),
                    args.AccessPoint().data());
            if (! strcmp(szBuf, szItem)) {
                disconnect = TRUE;
            }
        }

        if (disconnect) {
            conWin.SetConnection(FALSE);
            conWin.SelectNetwork("", "[any]");
        }

        action.Select("Networks");
        netWin.Remove(args.AccessPoint().data(), 
                      args.AccessPoint().data());
        if (args.Mode() == CmdArgs::ADD_KEY) {
            netWin.Add(args.AccessPoint().data(),
                    args.AccessPoint().data(),
                    args.Key().data());
        }
        else {
            netWin.Add(args.AccessPoint().data(),
                    args.AccessPoint().data());
        }
        return (0);
    }
};

class CmdConnect : public CmdProc
{
public:
    virtual int Process(Odyssey_ClientMgr &mgr,
                        CmdArgs &args) {
        std::cout << "Connecting to network ";
        std::cout << args.AccessPoint().data();
        std::cout << ":";
        std::cout << args.DeviceName().data() << std::endl;

        TCHAR szBuf[1024];
        TCHAR szItem[1024];

        Odyssey_SelectAction action(mgr);
        Odyssey_ConnectionWindow conWin(mgr);

        action.Select("Connection");
        if (conWin.GetConnection(szBuf, sizeof(szBuf))) {
            sprintf(szItem, "%s <%s>", 
                    args.AccessPoint().data(),
                    args.AccessPoint().data());
            if (! strcmp(szBuf, szItem)) {
                return CheckedConnection(conWin);                
            }
        }

        conWin.SetConnection(FALSE);
        if (conWin.SelectNetwork(args.AccessPoint().data(), 
                                 args.AccessPoint().data())) {
            if (conWin.SelectDevice(args.DeviceName().data())) {
                return CheckedConnection(conWin);
            }
        }
        return (0);
    }
private:
    int CheckedConnection(Odyssey_ConnectionWindow &conWin) {
        TCHAR szBuf[1024];
        conWin.SetConnection(TRUE);
        for (int i = 0; i < 10; i ++) {
            conWin.Status(szBuf, sizeof(szBuf));
            if (!strcmp(szBuf, "open")) {
                std::cout << "Connected !!!" << std::endl;
                return (0);
            }
            SleepEx(1000, FALSE);
            conWin.Status(szBuf, sizeof(szBuf));
            if (strcmp(szBuf, "open")) {
                conWin.ReConnect();
            }
        }
        std::cout << "Failed to connect" << std::endl;
        return (0);
    }
};

class CmdDisconnect : public CmdProc
{
public:
    virtual int Process(Odyssey_ClientMgr &mgr,
                        CmdArgs &args) {
        std::cout << "Disconnecting from network ";
        std::cout << args.AccessPoint().data() << std::endl;

        TCHAR szBuf[1024];
        TCHAR szItem[1024];

        Odyssey_SelectAction action(mgr);
        Odyssey_ConnectionWindow conWin(mgr);

        action.Select("Connection");
        if (conWin.GetConnection(szBuf, sizeof(szBuf))) {
            sprintf(szItem, "%s <%s>", 
                    args.AccessPoint().data(),
                    args.AccessPoint().data());
            if (! strcmp(szBuf, szItem)) {
                conWin.SetConnection(FALSE);
                std::cout << "Disconnected" << std::endl;
                return (0);
            }
        }
        return (0);
    }
};

class CmdRemove : public CmdProc
{
public:
    virtual int Process(Odyssey_ClientMgr &mgr,
                        CmdArgs &args) {
        std::cout << "Removing network ";
        std::cout << args.AccessPoint().data() << std::endl;

        Odyssey_SelectAction action(mgr);
        Odyssey_ConnectionWindow conWin(mgr);
        Odyssey_NetworksWindow netWin(mgr);

        BOOL disconnect = FALSE;
        TCHAR szBuf[1024];
        TCHAR szItem[1024];

        action.Select("Connection");
        if (conWin.GetConnection(szBuf, sizeof(szBuf))) {
            sprintf(szItem, "%s <%s>", 
                    args.AccessPoint().data(),
                    args.AccessPoint().data());
            if (! strcmp(szBuf, szItem)) {
                disconnect = TRUE;
            }
        }

        if (disconnect) {
            conWin.SetConnection(FALSE);
            conWin.SelectNetwork("", "[any]");
        }

        action.Select("Networks");
        if (netWin.Remove(args.AccessPoint().data(), 
                          args.AccessPoint().data())) {
            std::cout << "Item " << args.AccessPoint().data();
            std::cout << "removed " << std::endl;
        }
        return (0);
    }
};

int _tmain(int argc, _TCHAR* argv[])
{
    CmdArgs params(argc, argv);
    CmdMultiplex mux(params);

    CmdAdd add;
    CmdConnect connect;
    CmdDisconnect disconnect;
    CmdRemove remove;

    mux.Register(add, CmdArgs::ADD_OPEN);
    mux.Register(add, CmdArgs::ADD_KEY);
    mux.Register(connect, CmdArgs::CONNECT);
    mux.Register(disconnect, CmdArgs::DISCONNECT);
    mux.Register(remove, CmdArgs::REMOVE);

    return mux.Process();
}

