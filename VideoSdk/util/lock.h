/***************************************************************************************
 *
 *  IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
 *
 *  By downloading, copying, installing or using the software you agree to this license.
 *  If you do not agree to this license, do not download, install, 
 *  copy or use the software.
 *
 *  Copyright (C) 2014-2019, Happytimesoft Corporation, all rights reserved.
 *
 *  Redistribution and use in binary forms, with or without modification, are permitted.
 *
 *  Unless required by applicable law or agreed to in writing, software distributed 
 *  under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 *  CONDITIONS OF ANY KIND, either express or implied. See the License for the specific
 *  language governing permissions and limitations under the License.
 *
****************************************************************************************/

#ifndef _LOCK_H
#define _LOCK_H

#include "sys_inc.h"

class CLock 
{
public:
    CLock(void * pMutex) : m_pMutex(pMutex) {sys_os_mutex_enter(m_pMutex);}
    ~CLock() {sys_os_mutex_leave(m_pMutex);}

private:
    void * m_pMutex;
};

#endif // _LOCK_H


