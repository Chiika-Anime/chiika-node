//----------------------------------------------------------------------------
//Chiika-Node
//Copyright (C) 2016 arkenthera
//This program is free software; you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation; either version 2 of the License, or
//(at your option) any later version.
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//You should have received a copy of the GNU General Public License along
//with this program; if not, write to the Free Software Foundation, Inc.,
//51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.*/
//
//	Date: 20.1.2016
//	authors: arkenthera
//	Description:
//----------------------------------------------------------------------------
#ifndef __DatabaseWrapper_h__
#define __DatabaseWrapper_h__
//----------------------------------------------------------------------------
#include <nan.h>
#include "Common/Required.h"
#include "Root/Root.h"
#include "Common.h"
//----------------------------------------------------------------------------
class DatabaseWrapper : public Nan::ObjectWrap,public ChiikaApi::SystemEventListener
{
public:
	static void Init(Nan::ADDON_REGISTER_FUNCTION_ARGS_TYPE target,ChiikaApi::Root* r);

public:
	explicit DatabaseWrapper();
	~DatabaseWrapper();

	static NAN_METHOD(New);

	static NAN_METHOD(SetMalUser);

	static NAN_PROPERTY_GETTER(DatabaseGetter);

	static Nan::Persistent<v8::Function> constructor;

	static ChiikaApi::Root* root_;

	void OnEvent(ChiikaApi::SystemEvents);

	PersistentFunction m_SystemEventCallback;
	PersistentObject m_Caller;

	static void TaskOnMainThread(uv_async_t* req);

	uv_async_t async;
	uv_loop_t *loop;

};


#endif
//----------------------------------------------------------------------------