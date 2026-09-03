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
BM2F_ENTERACE(caais2_inq)
int f_caais2_inq(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
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

	CModel tcaais2("TCAAIS2");
	CDbCommand cmd_inq(conn);

	/* ***** 静态变量定义 ***** */
	int 	doFlag = 0;
	int 	fetchRowCount = 0;
	int     RowCount = 0;

	try
	{
		tcaais2.MergeFrom(bcls_rec->Tables[0].Rows[0]);
		
		switch(conn->DatabaseKind)
		{
		case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:	        // MS SQL Server数据库
		case DB_KIND_ORACLE:	        // Oracle 数据库
		default: // 通用
			sqlstr = "SELECT * FROM tcaais2"
				" WHERE 1=1"
					;
			if (tcaais2["DEPT_CODE"].ToString().Trim() != "")         sqlstr = sqlstr + " AND  DEPT_CODE = @dept_code";
			if (tcaais2["STATS_PERIOD"].ToString().Trim() != "")         sqlstr = sqlstr + " AND  STATS_PERIOD = @stats_period";
			if (tcaais2["COST_CENTER"].ToString().Trim() != "")         sqlstr = sqlstr + " AND  COST_CENTER = @cost_center";
			if (tcaais2["RULE_TYPE"].ToString().Trim() != "")         sqlstr = sqlstr + " AND  RULE_TYPE = @rule_type";
			if (tcaais2["DIVVY_TYPE"].ToString().Trim() != "")         sqlstr = sqlstr + " AND  DIVVY_TYPE = @divvy_type";
			if (tcaais2["MAT_CODE"].ToString().Trim() != "")         sqlstr = sqlstr + " AND  MAT_CODE = @mat_code";
			if (tcaais2["SUB_BACKLOG_CODE"].ToString().Trim() != "")         sqlstr = sqlstr + " AND  SUB_BACKLOG_CODE = @sub_backlog_code";
			if (tcaais2["EQU_NO"].ToString().Trim() != "")         sqlstr = sqlstr + " AND  EQU_NO = @equ_no";
			//if (tcaais2.PROD_DATE.Trim() != "")         sqlstr = sqlstr + " AND  PROD_DATE = @tcaais2.PROD_DATE";
			if (tcaais2["PROD_SHIFT_GROUP"].ToString().Trim() != "")         sqlstr = sqlstr + " AND  PROD_SHIFT_GROUP = @prod_shift_group";
			if (tcaais2["PROD_SHIFT_NO"].ToString().Trim() != "")         sqlstr = sqlstr + " AND  PROD_SHIFT_NO = @prod_shift_no";
			if (tcaais2["MAT_NAME"].ToString().Trim() != "")      sqlstr = sqlstr + " AND  MAT_NAME LIKE '%'||TRIM(@mat_name)||'%'";
			if (tcaais2["DATA_FROM"].ToString().Trim() != "")      sqlstr = sqlstr + " AND  DATA_FROM =@data_from";
			sqlstr =sqlstr + " order BY  MAT_CODE desc" ;
			break;
		}
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("mat_code", tcaais2["MAT_CODE"]);
		cmd_inq.Parameters.Set("sub_backlog_code", tcaais2["SUB_BACKLOG_CODE"]);
		cmd_inq.Parameters.Set("dept_code", tcaais2["DEPT_CODE"]);
		cmd_inq.Parameters.Set("stats_period", tcaais2["STATS_PERIOD"]);
		cmd_inq.Parameters.Set("cost_center", tcaais2["COST_CENTER"]);
		cmd_inq.Parameters.Set("rule_type", tcaais2["RULE_TYPE"]);
		cmd_inq.Parameters.Set("divvy_type", tcaais2["DIVVY_TYPE"]);
		cmd_inq.Parameters.Set("equ_no", tcaais2["EQU_NO"]);
		cmd_inq.Parameters.Set("prod_shift_group", tcaais2["PROD_SHIFT_GROUP"]);
		cmd_inq.Parameters.Set("prod_shift_no", tcaais2["PROD_SHIFT_NO"]);
		cmd_inq.Parameters.Set("mat_name", tcaais2["MAT_NAME"]);
		cmd_inq.Parameters.Set("data_from", tcaais2["DATA_FROM"].ToString());
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

