/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2010
Author:   zhouli
Version:    1.0
Date:     2013-03-19
Description: 分摊摊销信息查询
**************************************************/
//框架公用头文件，勿删
#include "stdafx.h"


// service入口
BM2F_ENTERACE(caaicx11_inq)
int f_caaicx11_inq(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);  // 系统日志，必须在代码段开始处定义

	CDbCommand cmd(conn);
	CString  sqlstr("");
	CString begin_time("");
	CString end_time("");
	CString mat_code("");
	CString sub_backlog_code("");
	CString shift_group("");
	CString sg_sign("");
	

	int i =0;

	CModel tcaai07("TCAAI07");
	CDbCommand cmd_inq(conn);

	/* ***** 静态变量定义 ***** */
	int 	doFlag = 0;
	int 	fetchRowCount = 0;
	int     RowCount = 0;

	try
	{
		tcaai07.MergeFrom(bcls_rec->Tables[0].Rows[0]);
		
		switch(conn->DatabaseKind)
		{
		case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:	        // MS SQL Server数据库
		case DB_KIND_ORACLE:	        // Oracle 数据库
		default: // 通用
			sqlstr = "SELECT * FROM tcaai07"
				" WHERE 1=1"
					;
			if (tcaai07["MAT_CODE"].ToString().Trim() != "")         sqlstr = sqlstr + " AND  MAT_CODE = @tcaai07.MAT_CODE";
			if (tcaai07["SUB_BACKLOG_CODE"].ToString().Trim() != "") sqlstr = sqlstr + " AND  SUB_BACKLOG_CODE = @tcaai07.SUB_BACKLOG_CODE";
			if (tcaai07["MAT_NAME"].ToString().Trim() != "")      sqlstr = sqlstr + " AND  MAT_NAME LIKE '%'||TRIM(@tcaai07.MAT_NAME)||'%'";
			sqlstr =sqlstr + " order BY  MAT_CODE desc" ;
			break;
		}
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("tcaai07.MAT_CODE", tcaai07["MAT_CODE"].ToString());
		cmd_inq.Parameters.Set("tcaai07.SUB_BACKLOG_CODE", tcaai07["SUB_BACKLOG_CODE"].ToString());
		cmd_inq.Parameters.Set("tcaai07.MAT_NAME", tcaai07["MAT_NAME"].ToString());		
		cmd_inq.ExecuteQuery(bcls_ret->Tables[0]);
		cmd_inq.Close(); 

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

