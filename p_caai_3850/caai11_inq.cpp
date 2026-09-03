/*************************************************
Copyright:	Baosight Software LTD.co Copyright (c) 2012
Author:		
Version:    3.0
Date:		2020-04-09
Description:查询月总耗用信息
**************************************************/
//框架用头文件
#include "stdafx.h"


// service入口
BM2F_ENTERACE(caai11_inq)
//-EP_SYSTEM_HEAD_END

int f_caai11_inq(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);
	
	int doFlag			= 0;
	int fetchRowCount	= 0;

	CString   stats_period = "";
	CString   account_period = "";
	CString   cost_center = "";
	CString   mat_code = "";
	CString   mat_name = "";

	CModel tcaaia11("TCAAIA11");

	CString  sqlstr("");

	CDbCommand cmd_inq(conn);
	
	try
	{
		//系统的分页类信息。
		CPageInfo pageInfo;

		tcaaia11.MergeFrom(bcls_rec->Tables[0].Rows[0]);
		account_period = bcls_rec->Tables[0].Rows[0]["ACCOUNT_PERIOD"].ToString().Substring(0, 6);
		
		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:	        // MS SQL Server数据库
		case DB_KIND_ORACLE:	        // Oracle 数据库
		default: // 通用
			sqlstr = " SELECT  * FROM TCAAIA11 "
				" WHERE 1=1 ";
			if (account_period.Trim() != "")
			{
				sqlstr = sqlstr + " AND ACCOUNT_PERIOD=@account_period ";
			}
			if (tcaaia11["COST_CENTER"].ToString().Trim() != "")
			{
				sqlstr = sqlstr + " AND COST_CENTER  like  '%'||@cost_center||'%' ";
			}
			if (tcaaia11["MAT_CODE"].ToString().Trim() != "")
			{
				sqlstr = sqlstr + " AND MAT_CODE=@mat_code ";
			}
			if (tcaaia11["MAT_NAME"].ToString().Trim() != "")
			{
				sqlstr = sqlstr + " AND MAT_NAME=@mat_name ";
			}
			break;
		}
		Log::Trace("", __FUNCTION__, "sql=[{0}]  ", sqlstr);
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("account_period", account_period);
		Log::Trace("", __FUNCTION__, "会计期=[{0}]  ", account_period);
		cmd_inq.Parameters.Set("cost_center", tcaaia11["COST_CENTER"].ToString());
		cmd_inq.Parameters.Set("mat_code", tcaaia11["MAT_CODE"].ToString());
		cmd_inq.Parameters.Set("mat_name", tcaaia11["MAT_NAME"].ToString());
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