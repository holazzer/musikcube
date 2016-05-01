//////////////////////////////////////////////////////////////////////////////
//
// License Agreement:
//
// The following are Copyright � 2008, Daniel �nnerby
//
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//    * Redistributions of source code must retain the above copyright notice,
//      this list of conditions and the following disclaimer.
//
//    * Redistributions in binary form must reproduce the above copyright
//      notice, this list of conditions and the following disclaimer in the
//      documentation and/or other materials provided with the distribution.
//
//    * Neither the name of the author nor the names of other contributors may
//      be used to endorse or promote products derived from this software
//      without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
//////////////////////////////////////////////////////////////////////////////

#include "pch.hpp"

#include <core/filestreams/Factory.h>
#include <core/config_filesystem.h>
#include <core/PluginFactory.h>
#include <core/filestreams/LocalFileStream.h>

using namespace musik::core::io;

//////////////////////////////////////////////////////////////////////////////

Factory Factory::sInstance;

Factory::Factory(){

    // Get all IDataStreamFactory plugins
    typedef IDataStreamFactory PluginType;
    typedef musik::core::PluginFactory::DestroyDeleter<PluginType> Deleter;

    this->dataStreamFactories = musik::core::PluginFactory::Instance().QueryInterface<PluginType, Deleter>("GetFileStreamFactory");
}


Factory::DataStreamPtr Factory::OpenUri(const utfchar *uri){
#ifdef _DEBUG
    std::cerr << "Factory::OpenFile(" << uri << ")" << std::endl;
#endif
    typedef musik::core::PluginFactory::DestroyDeleter<IDataStream> StreamDeleter;

    if(uri){
        for(DataStreamFactoryVector::iterator factory=sInstance.dataStreamFactories.begin();factory!=sInstance.dataStreamFactories.end();++factory){
            if( (*factory)->CanReadFile(uri) ){
                IDataStream* fileStream( (*factory)->OpenFile(uri) );
                if(fileStream){
                    return DataStreamPtr(fileStream,StreamDeleter());
                }else{
                    return DataStreamPtr();
                }
            }
        }

        // If non of the plugins match, lets create a regular file stream
        DataStreamPtr regularFile( new LocalFileStream(),StreamDeleter() );
        if(regularFile->Open(uri)){
            return regularFile;
        }
    }
    return DataStreamPtr();
}

bool Factory::IsLocalFileStream(const utfchar *uri){
    typedef musik::core::PluginFactory::DestroyDeleter<IDataStream> StreamDeleter;

    if(uri){
        for(DataStreamFactoryVector::iterator factory=sInstance.dataStreamFactories.begin();factory!=sInstance.dataStreamFactories.end();++factory){
            if( (*factory)->CanReadFile(uri) ){
                return false;
            }
        }

        //Check for local file
        boost::filesystem::utfpath filename(uri);
        try{
            if(boost::filesystem::exists(filename)){
                return true;
            }
        }
        catch(...){
        }
    }

    return false;
}
