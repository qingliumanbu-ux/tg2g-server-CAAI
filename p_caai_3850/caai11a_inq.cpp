/*************************************************
Copyright:	Baosight Software LTD.co Copyright (c) 2012
Author:
Version:    3.0
Date:		2020-09-22
Description:查询月总耗用信息
**************************************************/
//框架用头文件
#include "stdafx.h"


// service入口
BM2F_ENTERACE(caai11a_inq)
//-EP_SYSTEM_HEAD_END

int f_caai11a_inq(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	int doFlag = 0;
	int fetchRowCount = 0;

	CString   account_period = "";
	CString   mat_code = "";
	CString   mat_name = "";
	CString   flag_backlog = "";
	CString   flag_sign = "";
	CString   flag_vfree1 = "";
	CString   mat_type = "";


	CModel tcaai06("TCAAI06");

	CString  sqlstr("");
	CString  sqlstr_where("");

	CDbCommand cmd_inq(conn);

	try
	{
		//系统的分页类信息。
		CPageInfo pageInfo;

		tcaai06.MergeFrom(bcls_rec->Tables[0].Rows[0]);
		account_period = bcls_rec->Tables[0].Rows[0]["ACCOUNT_PERIOD"].ToString().Substring(0, 6);
		mat_type = bcls_rec->Tables[0].Rows[0]["MAT_TYPE"].ToString();
		flag_backlog = bcls_rec->Tables[0].Rows[0]["FLAG_BACKLOG"].ToString().Trim();
		flag_sign = bcls_rec->Tables[0].Rows[0]["FLAG_SIGN"].ToString().Trim();
		flag_vfree1 = bcls_rec->Tables[0].Rows[0]["FLAG_VFREE1"].ToString().Trim();

		if (flag_backlog == "1" || tcaai06["SUB_BACKLOG_CODE"].ToString().Trim() != "")
		{
			sqlstr_where += ",SUB_BACKLOG_CODE";
		}
		if (flag_sign == "1")
		{
			sqlstr_where += ",SG_SIGN";
		}
		if (flag_vfree1 == "1")
		{
			sqlstr_where += ",VFREE1 ";
		}

		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:	        // MS SQL Server数据库
		case DB_KIND_ORACLE:	        // Oracle 数据库
		default: // 通用
			sqlstr = " SELECT  ACCOUNT_PERIOD,DEPT_CODE,MAT_CODE,MAT_NAME,SUM(WT) as WT,SUM(PASS_WT) as PASS_WT "
				+ sqlstr_where+
				" FROM TCAAI06 "
				" WHERE 1=1 ";
			if (account_period.Trim() != "")
			{
				sqlstr = sqlstr + " AND ACCOUNT_PERIOD=@account_period ";
			}
			if (tcaai06["DEPT_CODE"].ToString().Trim() != "")
			{
				sqlstr = sqlstr + " AND DEPT_CODE = @dept_code ";
			}
			if (tcaai06["SUB_BACKLOG_CODE"].ToString().Trim() != "")
			{
				sqlstr = sqlstr + " AND SUB_BACKLOG_CODE = @sub_backlog_code ";
			}
			if (mat_type.Trim() != "")
			{
				sqlstr += " AND	MAT_CODE in (select mat_code from tcaac11 where MAT_TYPE =@mat_type) ";
			}
			if (tcaai06["MAT_CODE"].ToString().Trim() != "")
			{
				sqlstr = sqlstr + " AND MAT_CODE=@mat_code ";
			}
			sqlstr += " GROUP BY  ACCOUNT_PERIOD,DEPT_CODE,MAT_CODE,MAT_NAME" + sqlstr_where;
			break;
		}
		Log::Trace("", __FUNCTION__, "sql=[{0}]  ", sqlstr);
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("account_period", account_period);
		
		cmd_inq.Parameters.Set("dept_code", tcaai06["DEPT_CODE"].ToString());
		cmd_inq.Parameters.Set("sub_backlog_code", tcaai06["SUB_BACKLOG_CODE"].ToString());
		cmd_inq.Parameters.Set("mat_type", mat_type);
		cmd_inq.Parameters.Set("mat_code", tcaai06["MAT_CODE"].ToString());
		cmd_inq.ExecuteQuery(bcls_ret->Tables[0]);
		cmd_inq.Close();


	}
	catch (CDbException& ex)  //捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg, _RES("GCRSS0000006")/*数据库处理出错，sqlcode=[{0}]。请联系系统维护人员。*/, arguments, 1);
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char*)str, sizeof(s.sysmsg) - 1);  //返回前台，与EI.EIInfo对象的sys_info.sysmsg参数对应
		//Log::Trace("", "", "sqlstr={0}", sqlstr);
		//Log::Trace("", "", "str={0}", str);
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
