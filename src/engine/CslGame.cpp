/***************************************************************************
 *   Copyright (C) 2007-2013 by Glen Masgai                                *
 *   mimosius@users.sourceforge.net                                        *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "Csl.h"
#include "CslGame.h"


/**
 *  class CslMaster
 */

void CslMaster::AddServer(CslServerInfo *info)
{
    loopv(m_servers)
    {
        if (*m_servers[i]==*info)
        {
            info->AddMaster(m_id);
            return;
        }
    }

    m_servers.push_back(info);
    info->AddMaster(m_id);
}

bool CslMaster::UnrefServer(CslServerInfo *info,wxInt32 pos)
{
    if (pos<0 && (pos=m_servers.Index(info))==wxNOT_FOUND)
        return false;

    m_servers.RemoveAt(pos);
    info->RemoveMaster(m_id);

    return true;
}

void CslMaster::UnrefServers()
{
    loopvrev(m_servers) UnrefServer(m_servers[i],i);
}

void CslMaster::Init(CslGame *game,wxUint32 id)
{
    m_game=game;
    m_id=id;
}

/**
 *  class CslGame
 */

CslGame::CslGame() :
        m_fourcc(0), m_capabilities(0)
{
}

CslGame::~CslGame()
{
    WX_CLEAR_ARRAY(m_icons);
    DeleteMasters();
    WX_CLEAR_ARRAY(m_servers);
}

wxInt32 CslGame::AddMaster(CslMaster *master)
{
    loopv(m_masters)
    {
        if (*m_masters[i]==*master)
            return -1;
    }

    wxInt32 id=m_masters.size();
    master->Init(this,id);
    m_masters.push_back(master);

    return id;
}

bool CslGame::DeleteMaster(wxInt32 masterID,wxInt32 pos)
{
    if (pos<0)
    {
        loopv(m_masters)
        {
            if (m_masters[i]->GetId()==masterID)
            {
                pos=i;
                break;
            }
        }
        if (pos<0)
            return false;
    }

    delete m_masters[pos];
    m_masters.erase(m_masters.begin()+pos);

    return true;
}

void CslGame::DeleteMasters()
{
    loopvrev(m_masters) DeleteMaster(m_masters[i]->GetId(),i);
    m_masters.Empty();
}

bool CslGame::AddServer(CslServerInfo *info,wxInt32 masterID)
{
    CslMaster *master=NULL;
    CslServerInfo *server=NULL;
    bool found=false;

    loopv(m_servers)
    {
        if (*m_servers[i]==*info)
        {
            server=m_servers[i];
            found=true;
            break;
        }
    }

    if (!found)
    {
        m_servers.push_back(info);
        server=info;
    }

    if (masterID>-1)
    {
        loopv(m_masters)
        {
            master=m_masters[i];
            if (masterID==master->GetId())
            {
                master->AddServer(server);
                break;
            }
            else
                master=NULL;
        }
        if (!master)
        {
            if (!found)
                m_servers.pop_back();
            return false;
        }
    }

    return !found;
}

bool CslGame::DeleteServer(CslServerInfo *info)
{
    loopv(info->m_masterIDs)
    {
        wxInt32 id=info->m_masterIDs[i];
        loopvj(m_masters)
        {
            CslMaster *master=m_masters[j];
            if (master->GetId()==id)
            {
                master->UnrefServer(info);
                break;
            }
        }
    }

    loopv(m_servers)
    {
        if (m_servers[i]==info)
        {
            m_servers.erase(m_servers.begin()+i);
            delete info;
            return true;
        }
    }

    return false;
}

CslServerInfo* CslGame::FindServerByHost(const wxString& host, wxUint16 port)
{
    loopv(m_servers)
    {
        CslServerInfo *info = m_servers[i];

        if (info->m_addr.GetPort()==port && info->Host==host)
            return info;
    }

    return NULL;
}

CslServerInfo* CslGame::FindServerByAddr(const CslIPV4Addr& addr)
{
    if (addr.IsOk()) loopv(m_servers)
    {
        CslServerInfo *info = m_servers[i];

        if (info->m_addr.IsOk() && info->m_addr==addr)
            return info;
    }

    return NULL;
}

wxString& CslGame::ProcessScript(const CslServerInfo& info,wxUint32 connectMode,wxString& script)
{
    if (!script.IsEmpty())
    {
        script.Replace(wxT("#csl_server_host#"),info.Host);
        script.Replace(wxT("#csl_server_port#"),wxString::Format(wxT("%d"),info.GamePort));
        script.Replace(wxT("#csl_server_pass#"),info.Password);
        script.Replace(wxT("#csl_server_adminpass#"),info.PasswordAdmin);
        script.Replace(wxT("#csl_server_used_pass#"),
                       connectMode==CslServerInfo::CSL_CONNECT_ADMIN_PASS ? wxT("admin") :
                       (connectMode==CslServerInfo::CSL_CONNECT_PASS ? wxT("server") : wxT("")));
    }

    return script;
}

void CslGame::GetFavourites(CslArrayCslServerInfo& servers)
{
    loopv(m_servers)
    {
        if (m_servers[i]->IsFavourite())
            servers.push_back(m_servers[i]);
    }
}

void CslGame::GetExtServers(CslArrayCslServerInfo& servers, bool all)
{
    loopv(m_servers)
    {
        CslServerInfo *server=m_servers[i];

        if ((all || server->PingExt()) && server->ExtInfoStatus==CSL_EXT_STATUS_OK)
            servers.Add(server);
    }
}

wxInt32 CslGame::GetPlayerstatsDescriptions(const wxChar ***desc) const
{
    static const wxChar *descriptions[]=
    {
        _("Player"), _("Team"), _("Frags"), _("Deaths"), _("Teamkills"),
        _("Ping"), _("KpD"), _("Accuracy"), _("Health"), _("Armour"), _("Weapon")
    };

    *desc=descriptions;

    return sizeof(descriptions)/sizeof(descriptions[0]);
}

void CslGame::AddIcon(wxInt32 type, wxInt32 size, const void *data, wxUint32 datasize)
{
    m_icons.push_back(new CslGameIcon(type, size, data, datasize));
}

void CslGame::FreeIcon(wxUint32 pos)
{
    if (pos<m_icons.GetCount())
    {
        delete m_icons[pos];
        m_icons.RemoveAt(pos);
    }
}

const CslGameIcon* CslGame::GetIcon(wxInt32 size) const
{
    loopv(m_icons)
    {
        if (m_icons[i]->Size==size)
            return m_icons[i];
    }

    return NULL;
}

/**
 *  class CslServerInfo
 */

#define CSL_SERVERINFO_CTOR_ARGS_CPP \
    CslGame *game, \
    const wxString& host, wxUint16 gamePort, wxUint16 infoPort, \
    wxUint32 view, \
    wxUint32 lastSeen, wxUint32 playedLast, wxUint32 playTimeLast, \
    wxUint32 playTimeTotal, wxUint32 connectedTimes, \
    const wxString& oldDescription, \
    const wxString& pass, const wxString& passAdm \

CslServerInfo::CslServerInfo(CSL_SERVERINFO_CTOR_ARGS_CPP)
{
    Init(game,
         host, gamePort, infoPort,
         view,
         lastSeen, playedLast, playTimeLast, playTimeTotal, connectedTimes,
         oldDescription,
         pass, passAdm);
}

void CslServerInfo::Init(CSL_SERVERINFO_CTOR_ARGS_CPP)
{
    m_game=game;
    Host=host;
    GamePort=gamePort ? gamePort:game ? game->GetDefaultGamePort():0;
    Protocol=-1;
    Ping=-1;
    TimeRemain=-2;
    Players=-1;
    PlayersMax=-1;
    View=view;
    PingSend=0;
    PingResp=0;
    LastSeen=lastSeen;
    PlayedLast=playedLast;
    PlayTimeLast=playTimeLast;
    PlayTimeTotal=playTimeTotal;
    ConnectedTimes=connectedTimes;
    ConnectWait=0;
    DescriptionOld=oldDescription;
    MMDescription=wxEmptyString;
    MM=-2;
    Password=pass;
    PasswordAdmin=passAdm;
    Search=false;
    m_lock=0;
       m_addr.Create(host, infoPort ? infoPort : (GamePort ? GamePort+1 : 0));
}

CslServerInfo* CslServerInfo::Create(CSL_SERVERINFO_CTOR_ARGS_CPP)
{
    return new CslServerInfo(game,
                             host, gamePort, infoPort,
                             view,
                             lastSeen, playedLast, playTimeLast, playTimeTotal, connectedTimes,
                             oldDescription,
                             pass, passAdm);
}

#undef CSL_SERVERINFO_CTOR_ARGS_CPP

void CslServerInfo::SetLastPlayTime(wxUint32 time)
{
    PlayTimeLast=time;
    PlayTimeTotal+=time;
}

void CslServerInfo::SetDescription(const wxString& description)
{
    Description=description;
    DescriptionOld=description;
}

wxString CslServerInfo::GetBestDescription() const
{
    if (Description.IsEmpty())
    {
        if (DescriptionOld.IsEmpty()) return Host;
        else return DescriptionOld;
    }
    return Description;
}

void CslServerInfo::Lock(bool lock)
{
    lock ? m_lock++:m_lock-=m_lock>0;
}

void CslServerInfo::AddMaster(wxInt32 id)
{
    m_masterIDs.Add(id);
    SetDefault();
}
void CslServerInfo::RemoveMaster(wxInt32 id)
{
    m_masterIDs.Remove(id);

    if (!m_masterIDs.GetCount())
        RemoveDefault();
}

bool CslServerInfo::HasStats() const
{
    return PlayedLast>0 || PlayTimeLast>0 ||
           PlayTimeTotal>0 || ConnectedTimes>0;
}
