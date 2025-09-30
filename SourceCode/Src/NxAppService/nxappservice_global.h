/******************************************************************************
 *  版权所有（C）2022-2023，江西脑虎科技有限公司，保留所有权利。                   
 *  作者 : 宋志杰 zhijie.song@neuroxess.com
 *  创建日期:   September 2023
 *  修改记录: 
 *  <修改者姓名><邮件地址>   <修改内容>
******************************************************************************/
#pragma once
#include <CLogger.h>
#include <QtCore/qglobal.h>

using namespace NX;

#ifndef BUILD_STATIC
# if defined(NXAPPSERVICE_LIB)
#  define NX_APP_SERVICE_EXPORT Q_DECL_EXPORT
# else
#  define NX_APP_SERVICE_EXPORT Q_DECL_IMPORT
# endif
#else
# define NX_APP_SERVICE_EXPORT
#endif
