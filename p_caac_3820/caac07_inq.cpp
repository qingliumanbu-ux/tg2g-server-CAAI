/*************************************************
Copyright:	Baosight Software LTD.co Copyright (c) 2012
Author:		zhouli
Version:    3.0
Date:		2012-2-9
Description:费用分摊规则查询:根据输入的查询条件，查询显示费用分摊规则信息
**************************************************/
//框架用头文件
#include "stdafx.h"

// service入口
BM2F_ENTERACE(caac07_inq)
//-EP_SYSTEM_HEAD_END

int f_caac07_inq(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{

	CTracer log(__FUNCTION__);

	int doFlag = 0;
	int fetchRowCount = 0;
	CString v_dept_code("");

	CModel tcaac07("TCAAC07");


	CString  sqlstr("");

	CDbCommand cmd_inq(conn);

	try
	{
		//系统的分页类信息。
		CPageInfo pageInfo;

		try{//获取前台DEV控件传入的分页信息
			pageInfo.MergeFrom(bcls_rec->Tables[1].Rows[0]);
		}
		catch (CException& ex)
		{//2个变量信息是由前台的分页控件信息传入的，获取失败时,人工赋值一下。 
			pageInfo.RecordFrom = 0;
			pageInfo.PageSize = 10000;
		}

		tcaac07.MergeFrom(bcls_rec->Tables[0].Rows[0]);
		v_dept_code = bcls_rec->Tables[0].Rows[0]["dept_code"].ToString();

		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:	        // MS SQL Server数据库
		case DB_KIND_ORACLE:	        // Oracle 数据库
		default: // 所有数据库适用，通用SQL语句

			sqlstr = "	SELECT	* "
				" FROM	TCAAC07 "
				" WHERE	MAT_CODE LIKE  '%'||@tcaac07.MAT_CODE "
				" AND MAT_NAME LIKE  '%'||@tcaac07.MAT_NAME||'%' "
				;

			if (tcaac07["SUB_BACKLOG_CODE"].ToString().Trim() != "")
			{
				sqlstr += " AND SUB_BACKLOG_CODE	= @tcaac07.SUB_BACKLOG_CODE ";
			}
			if (v_dept_code.Trim() != "")sqlstr = sqlstr + " AND dept_code =@v_dept_code ";
			sqlstr += " ORDER	BY MAT_CODE	";

			break;
		}

		cmd_inq.SetCommandText(sqlstr);
		if (tcaac07["SUB_BACKLOG_CODE"].ToString().Trim() != "")
		{
			cmd_inq.Parameters.Set("tcaac07.SUB_BACKLOG_CODE", tcaac07["SUB_BACKLOG_CODE"].ToString().Trim());
		}
		cmd_inq.Parameters.Set("tcaac07.MAT_CODE", tcaac07["MAT_CODE"].ToString().Trim());
		cmd_inq.Parameters.Set("tcaac07.MAT_NAME", tcaac07["MAT_NAME"].ToString().Trim());
		cmd_inq.Parameters.Set("v_dept_code", v_dept_code);

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