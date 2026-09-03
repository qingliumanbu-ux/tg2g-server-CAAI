/*************************************************
Copyright:	Baosight Software LTD.co Copyright (c) 2012
Author:		
Version:    3.0
Date:		2020-04-09
Description:查询月平衡抛帐sap信息查询
**************************************************/
//框架用头文件
#include "stdafx.h"


// service入口
BM2F_ENTERACE(caai20_inq)
//-EP_SYSTEM_HEAD_END

int f_caai20_inq(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);
	
	int doFlag			= 0;
	int fetchRowCount	= 0;

	CString   stats_period = "";
	CString   account_period = "";
	CString   cost_center = "";
	CString   mat_code = "";
	CString   mat_name = "";

	CModel tcaac20("TCAAC20");

	CString  sqlstr("");

	CDbCommand cmd_inq(conn);
	
	try
	{
		//系统的分页类信息。
		CPageInfo pageInfo;

		tcaac20.MergeFrom(bcls_rec->Tables[0].Rows[0]);

		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:	        // MS SQL Server数据库
		case DB_KIND_ORACLE:	        // Oracle 数据库
		default: // 通用
			sqlstr = " SELECT  * FROM TCAAC20 "
				" WHERE 1=1 "
				;			
			
			if (tcaac20["MAT_CODE"].ToString().Trim() != "")
			{
				sqlstr = sqlstr + " AND MAT_CODE like @mat_code||'%' ";
			}
			if (tcaac20["MAT_NAME"].ToString().Trim() != "")
			{
				sqlstr = sqlstr + " AND MAT_CODE IN ( SELECT MAT_CODE FROM TCAAC11 WHERE MAT_NAME LIKE '%'||@mat_name||'%') ";
			}
			if (tcaac20["SUB_BACKLOG_CODE"].ToString().Trim() != "")
			{
				sqlstr = sqlstr + " AND SUB_BACKLOG_CODE=@sub_backlog_code ";
			}
			if (tcaac20["DEPT_CODE"].ToString().Trim() != "")
			{
				sqlstr = sqlstr + " AND DEPT_CODE = @dept_code ";
			}
			if (tcaac20["ACCOUNT_PERIOD"].ToString() != "")
			{
				sqlstr = sqlstr + " AND ACCOUNT_PERIOD=@account_period ";
			}
			break;
		}
		
		Log::Trace("", "", "sqlstr = [{0}],ACCOUNT_PERIOD = [{1}]", sqlstr, tcaac20["ACCOUNT_PERIOD"].ToString().SubstringNE(0, 6));
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("account_period", tcaac20["ACCOUNT_PERIOD"].ToString().SubstringNE(0,6));
		cmd_inq.Parameters.Set("dept_code", tcaac20["DEPT_CODE"].ToString());
		cmd_inq.Parameters.Set("mat_code", tcaac20["MAT_CODE"].ToString());
		cmd_inq.Parameters.Set("mat_name", tcaac20["MAT_NAME"].ToString());
		cmd_inq.Parameters.Set("sub_backlog_code", tcaac20["SUB_BACKLOG_CODE"].ToString());
		cmd_inq.ExecuteQuery(bcls_ret->Tables[0]);
		cmd_inq.Close();


	}
	catch(CDbException& ex)  //捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg,  _RES("GCRSS0000006")/*数据库处理出错，sqlcode=[{0}]。请联系系统维护人员。*/, arguments, 1);
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char*)str, sizeof(s.sysmsg)-1);  //返回前台，与EI.EIInfo对象的sys_info.sysmsg参数对应
		//Log::Trace("", "", "sqlstr={0}", sqlstr);
		//Log::Trace("", "", "str={0}", str);
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