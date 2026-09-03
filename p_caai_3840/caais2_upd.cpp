/*************************************************
Copyright:	Baosight Software LTD.co Copyright (c) 2012
Author:		zhouli
Version:    3.0
Date:		2012-2-9
Description:分摊摊销信息修改
**************************************************/
//框架用头文件
#include "stdafx.h"

// service入口
BM2F_ENTERACE(caais2_upd)
//-EP_SYSTEM_HEAD_END

int f_caais2_upd(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);
	
	int doFlag			= 0;
	

	CString		datetime = " ";	
	CModel tcaais2("TCAAIS2");

	
	CString  sqlstr("");
	

	CDbCommand cmd_inq(conn);
	
	try
	{
		datetime = CDateTime::Now().ToString("yyyyMMddHHmmss");
		
		for (int i = 0; i < bcls_rec->Tables[0].Rows.get_Count(); i++)
		{
			tcaais2.Reset();

			tcaais2.MergeFrom(bcls_rec->Tables[0].Rows[i]);

			tcaais2["REC_REVISOR"] = s.userid;   //记录修改责任者
			tcaais2["REC_REVISE_TIME"] = datetime;   //记录修改时刻
			sqlstr = " select distinct code_line from tcaac14 where sub_backlog_code=@sub_backlog_code ";
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.Parameters.Set("sub_backlog_code", tcaais2["SUB_BACKLOG_CODE"]);
			cmd_inq.ExecuteReader();
			if (cmd_inq.Read())
			{
				tcaais2["DEPT_CODE"] = cmd_inq.GetString(1);
			}
			cmd_inq.Close();

			sqlstr = " select MEASURE_MODE "
				" FROM TCAAC07 "
				" WHERE SUB_BACKLOG_CODE =@sub_backlog_code "
				" AND MAT_CODE = @mat_code "
				;
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.Parameters.Set("sub_backlog_code", tcaais2["SUB_BACKLOG_CODE"]);
			cmd_inq.Parameters.Set("mat_code", tcaais2["MAT_CODE"]);
			cmd_inq.ExecuteReader();
			while (cmd_inq.Read())
			{
				tcaais2["DIVVY_TYPE"] = cmd_inq.GetString(1);
			}
			cmd_inq.Close();
			//修改一条记录
			tcaais2.Update("*", " STATS_PERIOD,SUB_BACKLOG_CODE,KEY_SEQ");

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