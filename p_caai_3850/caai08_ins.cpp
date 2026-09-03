/*************************************************
Copyright:   Baosight Software LTD.co Copyright (c) 2010
Author:      zhouli
Version:     3.0
Date:        2011-39-14 09:39:03
Description: 库存新增
**************************************************/

//框架公用头文件，勿删
#include "stdafx.h"

//程序用头文件

//外部函数声明

// service入口
BM2F_ENTERACE(caai08_ins)
int f_caai08_ins(EIClass * bcls_rec, EIClass * bcls_ret,CDbConnection * conn) 
{
	CTracer log(__FUNCTION__);

    /* 程序内部变量 */
	int   doFlag = 0;
    int   fetchRowCount = 0;
	

    /* 业务变量 */
	CString  datetime("");

	/* 实体类定义 */
	CModel tcaaia8("TCAAIA8");

	// 数据库SQL操作字符串
	CString  sqlstr("");
	CDbCommand cmd_inq(conn);

    try
    { 
		datetime = CDateTime::Now().ToString("yyyyMMddHHmmss");       
		
		for (int i = 0; i < bcls_rec->Tables[0].Rows.get_Count(); i++)
		{
            //重置头文件,获得头文件中的默认值
			tcaaia8.Reset();		

			//取得一行数据， MergeFrom方法将获得bcls_rec指定表的指定行的数据，并赋值给CTCAAC11类型的对象
			tcaaia8.MergeFrom(bcls_rec->Tables[0].Rows[i]);	
			
			tcaaia8["REC_CREATOR"] = s.userid;   //记录创建责任者
			tcaaia8["REC_CREATE_TIME"] = datetime;   //记录创建时刻

			tcaaia8.TrimOrBlank();	
			tcaaia8.Insert();
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

