/*
 * ECmd.h
 *
 *  Created on: 24 Mar 2016
 *      Author: Xiao WANG
 */

#ifndef SHSCPP_CMD_ECMD_H_
#define SHSCPP_CMD_ECMD_H_
#include "cmd.h"
namespace SHS {

SHS_CMD_CLASS_CREATE(ZB_onoff,{
public:
		virtual void onRabbitMQReceive();
		virtual void onTimeOut();
		virtual void onATReceive();
});

} /* namespace SHS */

#endif /* SHSCPP_CMD_ECMD_H_ */