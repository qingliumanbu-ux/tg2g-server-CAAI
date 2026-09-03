/*************************************************
Copyright:	Baosight Software LTD.co Copyright (c) 2012
Author:		zhouli
Version:    3.0
Date:		2012-2-9
Description:分摊摊销信息修改
**************************************************/
//框架用头文件
#include "stdafx.h"

//业务用头文件




// service入口
BM2F_ENTERACE(caaicx11_upd)
//-EP_SYSTEM_HEAD_END

int f_caaicx11_upd(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);
	
	int doFlag			= 0;
	

	CString		datetime = " ";	
	CModel tcaai07("TCAAI07");

	
	CString  sqlstr("");
	

	CDbCommand cmd_inq(conn);
	
	try
	{
		datetime = CDateTime::Now().ToString("yyyyMMddHHmmss");
		
		for (int i = 0; i < bcls_rec->Tables[0].Rows.get_Count(); i++)
		{
			tcaai07.Reset();

			tcaai07.MergeFrom(bcls_rec->Tables[0].Rows[i]);

			tcaai07["REC_REVISOR"] = s.userid;   //记录修改责任者
			tcaai07["REC_REVISE_TIME"] = datetime;   //记录修改时刻

			//修改一条记录
			tcaai07.Update("*", "DEPT_CODE, TYPE_CODE,COST_CENTER,SUB_BACKLOG_CODE, MAT_CODE");

		}

	}
	catch(CDbException& ex)  //捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg,  _RES("GCRSS0000006")/*数据库处理出错，sqlcode=[{0}]。请联系系统维护人员。*/, arguments, 1);
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char*)str, sizeof(s.sysmsg)-1);  //返回前台，与EI.EIInfo对象的sys_info.sysmsg参数对应
		 
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
	
	s.flag = doFlag;
	bcls_ret->SetSYS(s);
	return doFlag;
}
