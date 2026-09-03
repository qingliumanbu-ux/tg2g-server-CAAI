/*************************************************
Copyright:	Baosight Software LTD.co Copyright (c) 2012
Author:		
Version:    3.0
Date:		2020-04-09
Description:副产品信息查询
**************************************************/
//框架用头文件
#include "stdafx.h"


// service入口
BM2F_ENTERACE(caai08_inq)
//-EP_SYSTEM_HEAD_END

int f_caai08_inq(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);
	
	int doFlag			= 0;
	int fetchRowCount	= 0;

	CString		begin_time = " ";
	CString		end_time = " ";
	CString   mat_code = "";
	CString   mat_name = "";

	CModel tcaaia8("TCAAIA8");

	CString  sqlstr("");

	CDbCommand cmd_inq(conn);
	
	try
	{
		//系统的分页类信息。
		CPageInfo pageInfo;

		begin_time = bcls_rec->Tables[0].Rows[0]["BEGIN_TIME"].ToString();
		end_time = bcls_rec->Tables[0].Rows[0]["END_TIME"].ToString();

		tcaaia8.MergeFrom(bcls_rec->Tables[0].Rows[0]);
		
		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:	        // MS SQL Server数据库
		case DB_KIND_ORACLE:	        // Oracle 数据库
		default: // 通用
			sqlstr = " SELECT  * FROM TCAAIA8 "
				" WHERE 1=1 ";			
			if (tcaaia8["PLAN_NO"].ToString().Trim() != "")
			{
				sqlstr = sqlstr + " AND PLAN_NO = @plan_no ";
			}
			if (tcaaia8["MAT_CODE"].ToString().Trim() != "")
			{
				sqlstr = sqlstr + " AND MAT_CODE=@mat_code ";
			}
			if (tcaaia8["MAT_NAME"].ToString().Trim() != "")
			{
				sqlstr = sqlstr + " AND MAT_NAME=@mat_name ";
			}
			if (end_time.Trim() != "")
			{
				sqlstr = sqlstr + " AND PROD_DATE<=@end_time ";
			}
			if (begin_time.Trim() != "")
			{
				sqlstr = sqlstr + " AND PROD_DATE>=@begin_time ";
			}
			break;
		}
		Log::Trace("", __FUNCTION__, "sql=[{0}]  ", sqlstr);
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("begin_time", begin_time);
		cmd_inq.Parameters.Set("end_time", end_time);
		cmd_inq.Parameters.Set("plan_no", tcaaia8["PLAN_NO"].ToString());
		cmd_inq.Parameters.Set("mat_code", tcaaia8["MAT_CODE"].ToString());
		cmd_inq.Parameters.Set("mat_name", tcaaia8["MAT_NAME"].ToString());
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