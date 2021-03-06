/***************************************************************************
 *   Copyright (C) 2007-2014 by Glen Masgai                                *
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

#ifndef CSLGAMEPROCESS_H
#define CSLGAMEPROCESS_H

#include <wx/process.h>

BEGIN_DECLARE_EVENT_TYPES()
DECLARE_LOCAL_EVENT_TYPE(wxCSL_EVT_PROCESS, wxID_ANY)
END_DECLARE_EVENT_TYPES()


#define CSL_EVT_PROCESS(id,fn) \
    DECLARE_EVENT_TABLE_ENTRY( \
                               wxCSL_EVT_PROCESS,id,wxID_ANY, \
                               (wxObjectEventFunction)(wxEventFunction) \
                               wxStaticCastEvent(wxCommandEventFunction,&fn), \
                               (wxObject*)NULL \
                             ),


#define CSL_PROCESS_BUFFER_SIZE  2048


class CslGameProcess : public wxProcess
{
    public:
        enum { INPUT_STREAM, ERROR_STREAM };

        CslGameProcess(CslServerInfo *info,const wxString& cmd);

        static void ProcessOutput(wxInt32 type);
        static void ProcessErrorStream();

    protected:
        static CslGameProcess *m_self;
        char m_buffer[CSL_PROCESS_BUFFER_SIZE];
        CslServerInfo *m_info;
        wxString m_cmd;
        wxStopWatch m_watch;

        virtual void OnTerminate(int pid,int code);
};

#endif //CSLGAMEPROCESS_H
