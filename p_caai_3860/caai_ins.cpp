/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:   zhouli
Version:    1.0
Date:     2012-10-15
Description: 实绩重收
**************************************************/

//框架公用头文件，勿删
#include "stdafx.h"

int f_caai_xnmes(CString stats_period, CString dept_code, CDbConnection * conn);

// service入口
BM2F_ENTERACE(caai_ins)

int f_caai_ins(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection *conn)
{
	CTracer log(__FUNCTION__);  // 系统日志，必须在代码段开始处定义

	//程序用变量
	CString sqlstr("");
	CString begin_time("");
	CString end_time("");
	CString e_datetime("");
	CString s_datetime("");

	CString a_begin_time("");
	CString a_end_time("");
	CString stats_period("");
	CString   dept_code = "";
	CString   v_heat_no = "";


	CString e_current_status="0";


	int doFlag = 0;
	int i;
	int fetchRowCount = 0;
	int flag = 0;
	
	/* 数据库操作类定义：统一放在Service或函数前段 */
	CDbCommand cmd_inq(conn);

	try
	{
		/*获得传入参数*/
		stats_period = bcls_rec->Tables[0].Rows[0]["account_period"].ToString(); //开始时间
		dept_code	= bcls_rec->Tables[0].Rows[0]["DEPT_CODE"].ToString(); 
		Log::Trace("", "", "begin_time={0},end_time={1}", begin_time,end_time);
		

		doFlag = f_caai_xnmes(stats_period, dept_code, conn);
		if (doFlag != 0)
		{
			strcpy(s.msg, "实绩收集失败!");
			throw CApplicationException(-1, s.msg, log.Location);
		}
		
				
	}
	catch(CDbException& ex)  //捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg,  _RES("GCRSS0000006")/*数据库处理出错，sqlcode=[{0}]。请联系系统维护人员。*/, arguments, 1);
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char*)str, sizeof(s.sysmsg)-1);
		s.flag = -1;
		doFlag = -1;      //数据库异常时返回-1，事务将被回滚
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
