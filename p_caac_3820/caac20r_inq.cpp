/*************************************************
Copyright:	Baosight Software LTD.co Copyright (c) 2012
Author:		
Version:    3.0
Date:		2012-2-9
Description：按时间查询机组
**************************************************/
//框架用头文件
#include "stdafx.h"

// service入口
BM2F_ENTERACE(caac20r_inq)
//-EP_SYSTEM_HEAD_END

int f_caac20r_inq(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	int doFlag = 0;
	int fetchRowCount = 0;

	CString		begin_date = " ";
	CString		end_date = " ";
	CString		dept_code = " ";
	CString		price_terms = " ";
	CString		sub_backlog_code = " ";



	CString  sqlstr("");

	CDbCommand cmd_inq(conn);

	try
	{
		begin_date = bcls_rec->Tables[0].Rows[0]["BEGIN_DATE"].ToString().Trim();
		end_date = bcls_rec->Tables[0].Rows[0]["END_DATE"].ToString().Trim();
		dept_code = bcls_rec->Tables[0].Rows[0]["DEPT_CODE"].ToString().Trim();
		price_terms = bcls_rec->Tables[0].Rows[0]["PRICE_TERMS"].ToString().Trim();
		sub_backlog_code = bcls_rec->Tables[0].Rows[0]["SUB_BACKLOG_CODE"].ToString().Trim();
		Log::Trace("", __FUNCTION__, "begin_date=[{0}]  ", begin_date);
		Log::Trace("", __FUNCTION__, "end_date=[{0}]  ", end_date);

		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:	        // MS SQL Server数据库
		case DB_KIND_ORACLE:	        // Oracle 数据库
		default: // 所有数据库适用，通用SQL语句		

			sqlstr = "	SELECT distinct	@sub_backlog_code as sub_backlog_code,@begin_date as begin_date,@end_date as end_date,@price_terms as price_terms,equ_no"
				" from tcaai03"
				" where 1=1"
				" AND stats_period >=@begin_date"
				" and stats_period<=@end_date"
				" and dept_code='R'"
				/*" group by  sub_backlog_code "*/
				" order by sub_backlog_code,equ_no"
				;
			break;
		}

		cmd_inq.SetCommandText(sqlstr);
		//cmd_inq.Parameters.Set("dept_code", dept_code);
		cmd_inq.Parameters.Set("begin_date", begin_date);
		cmd_inq.Parameters.Set("end_date", end_date);
		cmd_inq.Parameters.Set("price_terms", price_terms);
		cmd_inq.Parameters.Set("sub_backlog_code", sub_backlog_code);
		cmd_inq.ExecuteQuery(bcls_ret->Tables[0]);
		cmd_inq.Close();
	}
	catch (CDbException& ex)  //捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg, _RES("GCRSS0000006")/*数据库处理出错，sqlcode=[{0}]。请联系系统维护人员。*/, arguments, 1);
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char*)str, sizeof(s.sysmsg) - 1);  //返回前台，与EI.EIInfo对象的sys_info.sysmsg参数对应

		doFlag = -1;                 //数据库异常时返回-1，事务将被回滚
	}
	catch (CApplicationException& ex)  //捕获应用错误
	{
		s.flag = ex.GetCode();
		doFlag = -1;
	}
	catch (CException& ex)
	{
		strncpy(s.msg, (const char*)ex.GetMsg(), sizeof(s.msg) - 1);
		s.flag = ex.GetCode();
		doFlag = -1;
	}

	s.flag = doFlag;
	bcls_ret->SetSYS(s);
	return doFlag;
}