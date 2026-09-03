/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:   zhouli
Version:    1.0
Date:     2013-03-19
Description: 备用表使用删除
**************************************************/

//框架公用头文件，勿删
#include "stdafx.h"

//外部函数声明

// service入口
BM2F_ENTERACE(caaib1_del)

int f_caaib1_del(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

    /* 程序内部变量 */
	int   doFlag = 0;
    int   v_count=0;

    /* 业务变量 */
	CString  datetime("");

	/* 实体类定义 */
	CModel tcaaib1("TCAAIB1");

	// 数据库SQL操作字符串
	CString  sqlstr("");


    try
    { 
		datetime = CDateTime::Now().ToString("yyyyMMddHHmmss");
		
		for (int i = 0; i < bcls_rec->Tables[0].Rows.get_Count(); i++)
		{
			tcaaib1.Reset();
			tcaaib1.MergeFrom(bcls_rec->Tables[0].Rows[i]);

            //修改一条记录
			tcaaib1.Delete("KEY_SEQ,PROJECT_ID");
		}  
	}
	catch(CDbException& ex)  //捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg,  _RES("GCRSS0000006")/*数据库处理出错，sqlcode=[{0}]。请联系系统维护人员。*/, arguments, 1);
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char*)str, sizeof(s.sysmsg)-1);  //返回前台，与EI.EIInfo对象的sys_info.sysmsg参数对应
		s.flag = -1;
		doFlag = -1;                 //数据库异常时返回-1，事务将被回滚
	}
	catch(CApplicationException& ex)  //捕获应用错误
	{
		s.flag = ex.GetCode();
		doFlag = -1;
	}
	catch(CException& ex)
	{
		strncpy(s.msg, (const char*)ex.GetMsg(), sizeof(s.msg)-1);
		s.flag = ex.GetCode();
		doFlag = -1;
	}
	return doFlag;
}
