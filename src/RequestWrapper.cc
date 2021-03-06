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
#include "RequestWrapper.h"
#include "Common/Stable.h"
#include "Database/Globals.h"
#include "Root/Root.h"
#include "Request/RequestInterface.h"

#include "Request/SearchAnime.h"
#include "Request/MalAjax.h"
#include "Request/MalAnimePageScrape.h"

#include "Converters.h"

Nan::Persistent<v8::Function> RequestWrapper::constructor;
ChiikaApi::Root* RequestWrapper::root_;

using namespace v8;
using namespace ChiikaApi;

struct CallbackIteratorParams
{
	Nan::Persistent<v8::Object,v8::CopyablePersistentTraits<v8::Object>> Caller;
	std::string Name;
	Nan::Persistent<v8::Function,v8::CopyablePersistentTraits<v8::Function>> Callback;
	RequestInterface* Request;
};

RequestWrapper::RequestWrapper()
{
	loop = uv_default_loop();
	uv_async_init(loop,&async,&RequestWrapper::TaskOnMainThread);
}

RequestWrapper::~RequestWrapper()
{

}

void RequestWrapper::Init(Nan::ADDON_REGISTER_FUNCTION_ARGS_TYPE target,ChiikaApi::Root* r)
{
	root_ = r;

	Local<v8::FunctionTemplate> tpl = Nan::New<FunctionTemplate>(New);
	tpl->SetClassName(Nan::New("Request").ToLocalChecked());
	tpl->InstanceTemplate()->SetInternalFieldCount(1);
	Local<ObjectTemplate> inst = tpl->InstanceTemplate();


	//API Function definitions
	//This will be
	// var request = new Chiika.Request();
	// request.verifyUser(success,error)
	// ~
	Nan::SetPrototypeMethod(tpl,"VerifyUser",RequestWrapper::VerifyUser);
	Nan::SetPrototypeMethod(tpl,"GetMyAnimelist",RequestWrapper::GetMyAnimelist);
	Nan::SetPrototypeMethod(tpl,"GetMyMangalist",RequestWrapper::GetMyMangalist);
	Nan::SetPrototypeMethod(tpl,"AnimeScrape",RequestWrapper::AnimeScrape);
	Nan::SetPrototypeMethod(tpl,"RefreshAnimeDetails",RequestWrapper::RefreshAnimeDetails);
	Nan::SetPrototypeMethod(tpl,"GetAnimeDetails",RequestWrapper::GetAnimeDetails);
	Nan::SetPrototypeMethod(tpl,"UpdateAnime",RequestWrapper::UpdateAnime);
	Nan::SetPrototypeMethod(tpl,"testo",RequestWrapper::TestoDicto);

	constructor.Reset(Nan::GetFunction(tpl).ToLocalChecked());
	Nan::Set(target,Nan::New("Request").ToLocalChecked(),Nan::GetFunction(tpl).ToLocalChecked());


}

void RequestWrapper::OnSuccess(ChiikaApi::RequestInterface* r)
{

	CallbackMap::iterator It = m_CallbackMap.find(r->GetName() + "Success");

	if(It != m_CallbackMap.end())
	{
		CallbackIteratorParams* callback = new CallbackIteratorParams;
		callback->Callback = It->second.second;
		callback->Caller = It->second.first;
		callback->Name = It->first;
		callback->Request = r;
		async.data = reinterpret_cast<void*>(callback);

		uv_async_send(&async);
	}

}

void RequestWrapper::OnError(ChiikaApi::RequestInterface* r)
{

	CallbackMap::iterator It = m_CallbackMap.find(r->GetName() + "Error");

	if(It != m_CallbackMap.end())
	{

		CallbackIteratorParams* callback = new CallbackIteratorParams;
		callback->Callback = It->second.second;
		callback->Caller = It->second.first;
		callback->Request = r;
		callback->Name = It->first;
		async.data = reinterpret_cast<void*>(callback);

		uv_async_send(&async);
	}
}

void RequestWrapper::TaskOnMainThread(uv_async_t* req)
{
	CallbackIteratorParams* params = reinterpret_cast<CallbackIteratorParams*>(req->data);
	RequestWrapper *obj = new RequestWrapper;


	if(params)
	{
		obj->Wrap(Nan::New(params->Caller));
		Local<Function> local = Nan::New(params->Callback);

		PersistentValue returnval = obj->ParseRequest(params->Name,params->Request);
		Local<Value> ret[1] ={Nan::New(returnval)};
		local->Call(Null(Isolate::GetCurrent()),1,ret);
	}


}

PersistentValue RequestWrapper::ParseRequest(const std::string& r,ChiikaApi::RequestInterface* request)
{
	//Individual handling of success events
	//UserVerify
	if(r == "UserVerifySuccess")
	{
		Local<Object> val = Nan::New<v8::Object>();

		std::vector<const char*> returnKeylist ={kUserId,kUserName}; //c++11 rules
		UserInfo userInfo = root_->GetUser();

		FOR_(returnKeylist,i)
		{
			Nan::Set(val,Nan::New(returnKeylist[i]).ToLocalChecked(),
				Nan::New(userInfo.GetKeyValue(returnKeylist[i])).ToLocalChecked());
		}
		Nan::Set(val,Nan::New("request_name").ToLocalChecked(),Nan::New(r).ToLocalChecked());

		PersistentValue persistent;
		persistent.Reset(val);

		return persistent;
	}

	//

	//GetMyAnimelist
	if(r == "GetMyAnimelistSuccess")
	{
		Local<Object> val = Nan::New<v8::Object>();
		val = Converters::AnimeListToV8(root_);
		Nan::Set(val,Nan::New("request_name").ToLocalChecked(),Nan::New(r).ToLocalChecked());
		PersistentValue persistent;
		persistent.Reset(val);

		return persistent;
	}

	//GetMyMangalist
	if(r == "GetMyMangalistSuccess")
	{
		Local<Object> val = Nan::New<v8::Object>();
		val = Converters::MangaListToV8(root_);
		Nan::Set(val,Nan::New("request_name").ToLocalChecked(),Nan::New(r).ToLocalChecked());
		PersistentValue persistent;
		persistent.Reset(val);

		return persistent;
	}
	if(r == "GetImageSuccess")
	{
		Local<Object> val = Nan::New<v8::Object>();
		PersistentValue persistent;
		Nan::Set(val,Nan::New("request_name").ToLocalChecked(),Nan::New(r).ToLocalChecked());
		persistent.Reset(val);
		return persistent;

	}

	if(r == "GetAnimePageScrapeSuccess")
	{
		Local<Object> val = Nan::New<v8::Object>();
		val = Converters::AnimeListToV8(root_);

		ChiikaApi::AnimePageScrapeRequest* scrapeRequest = static_cast<ChiikaApi::AnimePageScrapeRequest*>(request);
		if(scrapeRequest)
		{
			val = Converters::AnimeToV8(root_,std::to_string(scrapeRequest->GetAnimeId()));
		}

		Nan::Set(val,Nan::New("request_name").ToLocalChecked(),Nan::New(r).ToLocalChecked());
		PersistentValue persistent;
		persistent.Reset(val);

		return persistent;
	}

	if(r == "GetMalAjaxSuccess")
	{
		Local<Object> val = Nan::New<v8::Object>();
		PersistentValue persistent;
		ChiikaApi::MalAjaxRequest* ajaxRequest = static_cast<ChiikaApi::MalAjaxRequest*>(request);

		if(ajaxRequest)
		{
			val = Converters::AnimeToV8(root_,std::to_string(ajaxRequest->GetAnimeId()));
		}

		Nan::Set(val,Nan::New("request_name").ToLocalChecked(),Nan::New(r).ToLocalChecked());
		persistent.Reset(val);

		return persistent;
	}

	if(r == "GetSearchAnimeSuccess")
	{
		Local<Object> val = Nan::New<v8::Object>();
		PersistentValue persistent;

		ChiikaApi::SearchAnimeRequest* searchRequest = static_cast<ChiikaApi::SearchAnimeRequest*>(request);

		if(searchRequest)
		{
			val = Converters::AnimeToV8(root_,std::to_string(searchRequest->GetAnimeId()));
		}
		Nan::Set(val,Nan::New("request_name").ToLocalChecked(),Nan::New(r).ToLocalChecked());
		persistent.Reset(val);

		return persistent;
	}
	if(r == "GetUpdateAnimeSuccess")
	{
		Local<Object> val = Nan::New<v8::Object>();
		PersistentValue persistent;

		Nan::Set(val,Nan::New("request_name").ToLocalChecked(),Nan::New(r).ToLocalChecked());
		persistent.Reset(val);

		return persistent;
	}

	if(r == "FakeRequestSuccess")
	{
		Local<Object> val = Nan::New<v8::Object>();
		PersistentValue persistent;

		Nan::Set(val,Nan::New("request_name").ToLocalChecked(),Nan::New(r).ToLocalChecked());
		persistent.Reset(val);

		return persistent;
	}

	//Handling of all failed requests
	size_t f = r.find("Error");
	if(f != std::string::npos)
	{
		Local<Object> val = Nan::New<v8::Object>();

		std::vector<const char*> returnKeylist ={"errorCode","errorDesc","request_name"}; //c++11 rules
		UserInfo userInfo = root_->GetUser();


		Nan::Set(val,Nan::New(returnKeylist[0]).ToLocalChecked(),
			Nan::New(request->Get()->GetRequestResult()));

		Nan::Set(val,Nan::New(returnKeylist[1]).ToLocalChecked(),
			Nan::New(request->Get()->GetResponse()).ToLocalChecked());

		Nan::Set(val,Nan::New(returnKeylist[2]).ToLocalChecked(),
			Nan::New(request->GetName()).ToLocalChecked());

		PersistentValue persistent;
		persistent.Reset(val);

		return persistent;
	}
}
NAN_METHOD(RequestWrapper::TestoDicto)
{
	RequestWrapper *obj = new RequestWrapper;
	obj->Wrap(info.This());

	v8::Isolate* isolate = info.GetIsolate();
}

NAN_METHOD(RequestWrapper::AnimeScrape)
{
	RequestWrapper *obj = new RequestWrapper;
	obj->Wrap(info.This());

	v8::Isolate* isolate = info.GetIsolate();

	v8::Local<v8::Array> args = info[2].As<v8::Array>();

	int Id = -1;
	//Set up callbacks
	PersistentFunction callbackSuccess;
	callbackSuccess.Reset((info[0].As<Function>()));

	PersistentFunction callbackError;
	callbackError.Reset((info[1].As<Function>()));

	Local<Object> animeId;
	V8Value id = args->Get(v8::String::NewFromUtf8(isolate,"animeId"));

	Id = id->Int32Value();

	PersistentObject caller;
	caller.Reset(info.This());

	obj->m_CallbackMap.insert(std::make_pair("GetMalAjaxSuccess",
		std::make_pair(caller,callbackSuccess)));
	obj->m_CallbackMap.insert(std::make_pair("GetMalAjaxError",
		std::make_pair(caller,callbackError)));

	obj->m_CallbackMap.insert(std::make_pair("GetImageSuccess",
		std::make_pair(caller,callbackSuccess)));
	obj->m_CallbackMap.insert(std::make_pair("GetImageError",
		std::make_pair(caller,callbackError)));

	obj->m_CallbackMap.insert(std::make_pair("GetSearchAnimeSuccess",
		std::make_pair(caller,callbackSuccess)));
	obj->m_CallbackMap.insert(std::make_pair("GetSearchAnimeError",
		std::make_pair(caller,callbackError)));



	root_->MalAjax(obj,Id);
}

NAN_METHOD(RequestWrapper::VerifyUser)
{
	RequestWrapper *obj = new RequestWrapper;
	obj->Wrap(info.This());

	v8::Isolate* isolate = info.GetIsolate();



	//Set up callbacks
	PersistentFunction callbackSuccess;
	callbackSuccess.Reset((info[0].As<Function>()));

	PersistentFunction callbackError;
	callbackError.Reset((info[1].As<Function>()));

	PersistentObject caller;
	caller.Reset(info.This());

	//Verify
	obj->m_CallbackMap.insert(std::make_pair("UserVerifySuccess",
		std::make_pair(caller,callbackSuccess)));
	obj->m_CallbackMap.insert(std::make_pair("UserVerifyError",
		std::make_pair(caller,callbackError)));

	//Animelist
	obj->m_CallbackMap.insert(std::make_pair("GetMyAnimelistSuccess",
		std::make_pair(caller,callbackSuccess)));
	obj->m_CallbackMap.insert(std::make_pair("GetMyAnimelistError",
		std::make_pair(caller,callbackError)));

	//Mangalist
	obj->m_CallbackMap.insert(std::make_pair("GetMyMangalistSuccess",
		std::make_pair(caller,callbackSuccess)));
	obj->m_CallbackMap.insert(std::make_pair("GetMyMangalistError",
		std::make_pair(caller,callbackError)));

	//Download Image
	obj->m_CallbackMap.insert(std::make_pair("GetImageSuccess",
		std::make_pair(caller,callbackSuccess)));
	obj->m_CallbackMap.insert(std::make_pair("GetImageError",
		std::make_pair(caller,callbackError)));

	//Post the request
	ChiikaApi::Root::Get()->VerifyUser(obj);
}

NAN_METHOD(RequestWrapper::GetMyAnimelist)
{
	RequestWrapper *obj = new RequestWrapper;
	obj->Wrap(info.This());

	v8::Isolate* isolate = info.GetIsolate();



	//Set up callbacks
	PersistentFunction callbackSuccess;
	callbackSuccess.Reset((info[0].As<Function>()));

	PersistentFunction callbackError;
	callbackError.Reset((info[1].As<Function>()));

	PersistentObject caller;
	caller.Reset(info.This());

	obj->m_CallbackMap.insert(std::make_pair("GetMyAnimelistSuccess",
		std::make_pair(caller,callbackSuccess)));
	obj->m_CallbackMap.insert(std::make_pair("GetMyAnimelistError",
		std::make_pair(caller,callbackError)));


	//Post the request
	ChiikaApi::Root::Get()->GetMyAnimelist(obj);
}

NAN_METHOD(RequestWrapper::GetMyMangalist)
{
	RequestWrapper *obj = new RequestWrapper;
	obj->Wrap(info.This());

	v8::Isolate* isolate = info.GetIsolate();



	//Set up callbacks
	PersistentFunction callbackSuccess;
	callbackSuccess.Reset((info[0].As<Function>()));

	PersistentFunction callbackError;
	callbackError.Reset((info[1].As<Function>()));

	PersistentObject caller;
	caller.Reset(info.This());

	obj->m_CallbackMap.insert(std::make_pair("GetMyMangalistSuccess",
		std::make_pair(caller,callbackSuccess)));
	obj->m_CallbackMap.insert(std::make_pair("GetMyMangalistError",
		std::make_pair(caller,callbackError)));


	//Post the request
	ChiikaApi::Root::Get()->GetMyMangalist(obj);
}

NAN_METHOD(RequestWrapper::RefreshAnimeDetails)
{
	RequestWrapper *obj = new RequestWrapper;
	obj->Wrap(info.This());

	v8::Isolate* isolate = info.GetIsolate();

	v8::Local<v8::Array> args = info[2].As<v8::Array>();

	int Id = -1;
	//Set up callbacks
	PersistentFunction callbackSuccess;
	callbackSuccess.Reset((info[0].As<Function>()));

	PersistentFunction callbackError;
	callbackError.Reset((info[1].As<Function>()));

	Local<Object> animeId;
	V8Value id = args->Get(v8::String::NewFromUtf8(isolate,"animeId"));

	Id = id->Int32Value();

	PersistentObject caller;
	caller.Reset(info.This());

	obj->m_CallbackMap.insert(std::make_pair("GetMalAjaxSuccess",
		std::make_pair(caller,callbackSuccess)));
	obj->m_CallbackMap.insert(std::make_pair("GetMalAjaxError",
		std::make_pair(caller,callbackError)));

	obj->m_CallbackMap.insert(std::make_pair("GetImageSuccess",
		std::make_pair(caller,callbackSuccess)));
	obj->m_CallbackMap.insert(std::make_pair("GetImageError",
		std::make_pair(caller,callbackError)));

	obj->m_CallbackMap.insert(std::make_pair("GetSearchAnimeSuccess",
		std::make_pair(caller,callbackSuccess)));
	obj->m_CallbackMap.insert(std::make_pair("GetSearchAnimeError",
		std::make_pair(caller,callbackError)));

	obj->m_CallbackMap.insert(std::make_pair("GetAnimePageScrapeSuccess",
		std::make_pair(caller,callbackSuccess)));
	obj->m_CallbackMap.insert(std::make_pair("GetAnimePageScrapeError",
		std::make_pair(caller,callbackError)));



	root_->RefreshAnimeDetails(obj,Id);
}

NAN_METHOD(RequestWrapper::GetAnimeDetails)
{
	RequestWrapper *obj = new RequestWrapper;
	obj->Wrap(info.This());

	v8::Isolate* isolate = info.GetIsolate();

	v8::Local<v8::Array> args = info[2].As<v8::Array>();

	int Id = -1;
	//Set up callbacks
	PersistentFunction callbackSuccess;
	callbackSuccess.Reset((info[0].As<Function>()));

	PersistentFunction callbackError;
	callbackError.Reset((info[1].As<Function>()));

	Local<Object> animeId;
	V8Value id = args->Get(v8::String::NewFromUtf8(isolate,"animeId"));

	Id = id->Int32Value();

	PersistentObject caller;
	caller.Reset(info.This());

	obj->m_CallbackMap.insert(std::make_pair("GetMalAjaxSuccess",
		std::make_pair(caller,callbackSuccess)));
	obj->m_CallbackMap.insert(std::make_pair("GetMalAjaxError",
		std::make_pair(caller,callbackError)));

	obj->m_CallbackMap.insert(std::make_pair("GetImageSuccess",
		std::make_pair(caller,callbackSuccess)));
	obj->m_CallbackMap.insert(std::make_pair("GetImageError",
		std::make_pair(caller,callbackError)));

	obj->m_CallbackMap.insert(std::make_pair("GetSearchAnimeSuccess",
		std::make_pair(caller,callbackSuccess)));
	obj->m_CallbackMap.insert(std::make_pair("GetSearchAnimeError",
		std::make_pair(caller,callbackError)));

	obj->m_CallbackMap.insert(std::make_pair("GetAnimePageScrapeSuccess",
		std::make_pair(caller,callbackSuccess)));
	obj->m_CallbackMap.insert(std::make_pair("GetAnimePageScrapeError",
		std::make_pair(caller,callbackError)));

	obj->m_CallbackMap.insert(std::make_pair("FakeRequestSuccess",
		std::make_pair(caller,callbackSuccess)));




	root_->GetAnimeDetails(obj,Id);
}

NAN_METHOD(RequestWrapper::UpdateAnime)
{
	RequestWrapper *obj = new RequestWrapper;
	obj->Wrap(info.This());

	v8::Isolate* isolate = info.GetIsolate();

	v8::Local<v8::Array> args = info[2].As<v8::Array>();

	int Id = -1;
	int Progress = -1;
	int Score = -1;
	int Status = -1;
	//Set up callbacks
	PersistentFunction callbackSuccess;
	callbackSuccess.Reset((info[0].As<Function>()));

	PersistentFunction callbackError;
	callbackError.Reset((info[1].As<Function>()));


	V8Value id = args->Get(v8::String::NewFromUtf8(isolate,"animeId"));
	V8Value progress = args->Get(v8::String::NewFromUtf8(isolate,"progress"));
	V8Value score = args->Get(v8::String::NewFromUtf8(isolate,"score"));
	V8Value status = args->Get(v8::String::NewFromUtf8(isolate,"status"));

	Id = id->Int32Value();
	Progress = progress->Int32Value();
	Score = score->Int32Value();
	Status = status->Int32Value();

	PersistentObject caller;
	caller.Reset(info.This());

	obj->m_CallbackMap.insert(std::make_pair("GetUpdateAnimeSuccess",
		std::make_pair(caller,callbackSuccess)));
	obj->m_CallbackMap.insert(std::make_pair("GetUpdateAnimeError",
		std::make_pair(caller,callbackError)));




	root_->UpdateAnime(obj,Id,Score,Progress,Status);
}

NAN_METHOD(RequestWrapper::New)
{
	if(info.IsConstructCall())
	{
		v8::Isolate* isolate = info.GetIsolate();
		RequestWrapper *obj = new RequestWrapper;
		obj->Wrap(info.This());

	}
	else
	{
		const int argc = 1;
		Local<Value> argv[argc] ={info[0]};
		Local<Function> cons = Nan::New(constructor);
		info.GetReturnValue().Set(cons->NewInstance(argc,argv));
	}
}