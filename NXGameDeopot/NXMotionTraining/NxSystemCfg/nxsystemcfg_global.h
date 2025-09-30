/******************************************************************************
 *  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。                   
 *  作者 : 宋志杰 zhijie.song@neuroxess.com
 *  创建日期:   September 2023
 *  修改记录: 
 *  <修改者姓名><邮件地址>   <修改内容>
******************************************************************************/
#ifndef __NX_SYSTEM_CFG_GLOBAL_H
#define __NX_SYSTEM_CFG_GLOBAL_H

#include <QtCore/qglobal.h>
#include <CLogger.h>

#ifndef BUILD_STATIC
# if defined(NXSYSTEMCFG_LIB)
#  define NXSYSTEMCFG_EXPORT Q_DECL_EXPORT
# else
#  define NXSYSTEMCFG_EXPORT Q_DECL_IMPORT
# endif
#else
# define NXSYSTEMCFG_EXPORT
#endif

#endif