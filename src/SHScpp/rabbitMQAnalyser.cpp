/*
 * rabbitMQAnalyser.cpp
 *
 *  Created on: 21 Mar 2016
 *      Author: Xiao WANG
 */
#include "vector"
#include "rabbitMQAnalyser.h"
#include "sstream"
using std::string;
namespace SHS {

RabbitMQAnalyser::RabbitMQAnalyser(Container * pContainer):pContainer(pContainer) {
}

RabbitMQAnalyser::~RabbitMQAnalyser() {

}

void RabbitMQAnalyser::startAnalyse(MyMQ<Json::Value> * pMQ){
	if(!pMQ) Log::log.error("RabbitMQAnalyser: invalid MyMQ obj with addr:%d\n",pMQ);
	if(!this->pContainer) Log::log.error("RabbitMQAnalyser: invalid container obj with addr:%d\n",this->pContainer);
	pthread_mutex_t mutex;
	pthread_mutex_init(&mutex,NULL);
	Log::log.debug("RabbitMQAnalyser: start addr:%d\n",this);

	while(1){
		Json::Value root=pMQ->getOneMSG();
		Json::Value type=root["type"];
		if(type.isNull()){
			Log::log.warning("RabbitMQAnalyser: get message error: the message do not has \"type\" attribute\n");
		}else{
			if(type.isString()){
				string type_str = type.asString();
				std::istringstream iss(type_str);
				std::string token;
				while (std::getline(iss, token, '.')) {
				    if (!token.empty()){
				    	root["token"].append(token);
				    }
				}
				//assembling command name
				std::string cmdName(root["token"][0].asString());
				for(unsigned int i =1;i<root["token"].size()-1;i++)
			    	cmdName = cmdName+"_" + root["token"][i].asCString();
				if(this->pContainer){
					Cmd* cmdObj=this->pContainer->getCmdObj(cmdName);
					if(cmdObj){
						Log::log.debug("RabbitMQAnalyser: calling object:%s\n",cmdName.c_str());
						this->pContainer->lockContainer();
						//register the command object to the active command list, which meaning the object will keep alive to receive the serial port message
						this->pContainer->regActCmd(cmdObj);
						//call the onRabbitMQReceive function, which is a callback function to deal with the message further
						cmdObj->_onRabbitMQReceive(root);
						this->pContainer->unlockContainer();
						//wait this command finished and then start analyse next command
						if(root["__attribute_uart_exclusive"].isNull() || root["__attribute_uart_exclusive"].asBool()){
							cmdObj->waitCmdFinish(&mutex);
						}
					}else{
						Log::log.warning("RabbitMQAnalyser: Unsupported message type:%s\n",type_str.c_str());
					}
				}else{
					Log::log.warning("RabbitMQAnalyser: message \"type\" attribute format error\n");
				}
			}else{
				Log::log.warning("RabbitMQAnalyser:message format error: the message \"type\" attribute is not a string\n");
			}
		}
	}

	Log::log.debug("RabbitMQAnalyser:receive next\n");
}

struct rabbitmq_analyser_temp_t {
	RabbitMQAnalyser* pAnalyser;
	MyMQ<Json::Value>* pMQ;
};

static void* rabbitMQAnalyserThread(void *ptr){
	rabbitmq_analyser_temp_t & para = *((rabbitmq_analyser_temp_t*)ptr);
	para.pAnalyser->startAnalyse(para.pMQ);
	return NULL;
}

void RabbitMQAnalyser::startAnalyseAsThread(MyMQ<Json::Value> * pMQ){
	pthread_t thread;
	static struct rabbitmq_analyser_temp_t para;//avoid the variable lost when function finished
	para. pAnalyser=this;
	para.pMQ=pMQ;
	pthread_create (&thread, NULL,rabbitMQAnalyserThread, (void*) &para);
	pthread_detach(thread);

}

} /* namespace SHS */
