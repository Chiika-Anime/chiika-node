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
#include "DatabaseWrapper.h"
#include "Root\Root.h"
#include "Logging\LogManager.h"
#include "Common.h"
#include "Converters.h"
//----------------------------------------------------------------------------

Nan::Persistent<v8::Function> DatabaseWrapper::constructor;
ChiikaApi::Root* DatabaseWrapper::root_;

using namespace v8;
using namespace ChiikaApi;

const char* kAnimeListProperty = "AnimeList";

DatabaseWrapper::DatabaseWrapper()
{


}

DatabaseWrapper::~DatabaseWrapper()
{

}

void DatabaseWrapper::Init(Nan::ADDON_REGISTER_FUNCTION_ARGS_TYPE target, ChiikaApi::Root* r)
{
	root_ = r;
	v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
	tpl->SetClassName(Nan::New("Database").ToLocalChecked());
	tpl->InstanceTemplate()->SetInternalFieldCount(1);

	v8::Local<v8::ObjectTemplate> inst = tpl->InstanceTemplate();

	tpl->Set(DEFINE_PROPERTY(kAnimeListProperty, Nan::New<Object>()));

	Nan::SetNamedPropertyHandler(inst,
		DatabaseWrapper::DatabaseGetter);


	constructor.Reset(Nan::GetFunction(tpl).ToLocalChecked());
	Nan::Set(target, Nan::New("Database").ToLocalChecked(), Nan::GetFunction(tpl).ToLocalChecked());


}

NAN_METHOD(DatabaseWrapper::New)
{
	if (info.IsConstructCall())
	{
		v8::Isolate* isolate = info.GetIsolate();
		DatabaseWrapper *obj = new DatabaseWrapper;
		obj->Wrap(info.This());
	}
	else
	{
		const int argc = 1;
		v8::Local<v8::Value> argv[argc] = { info[0] };
		v8::Local<v8::Function> cons = Nan::New(constructor);
		info.GetReturnValue().Set(cons->NewInstance(argc, argv));
	}


}

NAN_PROPERTY_GETTER(DatabaseWrapper::DatabaseGetter)
{
	DatabaseWrapper* obj = Nan::ObjectWrap::Unwrap<DatabaseWrapper>(info.This());

	const char* prop = *v8::String::Utf8Value(property);

	if (strcmp(prop, kAnimeListProperty) == 0)
	{
		v8::Local<v8::Value> val = Converters::AnimeListToV8Value(obj->root_);
		info.GetReturnValue().Set(val);
	}

}