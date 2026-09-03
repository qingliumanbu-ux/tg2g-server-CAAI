/*************************************************
Copyright:	Baosight Software LTD.co Copyright (c) 2012
Author:		178258
Version:    1.0
Date:		2012-2-9
Description:成本科目查询:根据输入的查询条件，查询显示成本科目信息
**************************************************/
//框架用头文件
#include "stdafx.h"

// service入口
BM2F_ENTERACE(caac13_inq)
//-EP_SYSTEM_HEAD_END

int f_caac13_inq(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	
	CTracer log(__FUNCTION__);
	
	int doFlag			= 0;
	CString   dept_code = "",stats_period;

	CString  sqlstr("");

	CDbCommand cmd_inq(conn);
	
	try
	{
		dept_code = bcls_rec->Tables[0].Rows[0]["DEPT_CODE"].ToString();
		stats_period = bcls_rec->Tables[0].Rows[0]["ACCOUNT_PERIOD"].ToString();

		if (stats_period != "")
		{
			sqlstr = "	SELECT	count(1) "
				" FROM	TCAAC13 "
				" where stats_period = @stats_period "
				" and dept_code=@dept_code "
				;
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.Parameters.Set("dept_code", dept_code);
			cmd_inq.Parameters.Set("stats_period", stats_period);
			if (cmd_inq.ExecuteScalar() < 1)
			{
				sqlstr = " insert into tcaac13 (dept_code, stats_period, job_code, job_code_cname, call_func, call_func_desc, status_code, call_form, succs_flag)"
					" select distinct @dept_code, @stats_period, job_code, job_code_cname, call_func, call_func_desc, status_code, call_form, ' '"
					" from tcaac13"
					" where stats_period = (select max(stats_period) from tcaac13)"
					;
				cmd_inq.SetCommandText(sqlstr);
				cmd_inq.Parameters.Set("dept_code", dept_code);
				cmd_inq.Parameters.Set("stats_period", stats_period);
				cmd_inq.ExecuteNonQuery();
				cmd_inq.Close();
			}
		}
		
		
		switch(conn->DatabaseKind)
		{
			case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:	        // MS SQL Server数据库
			case DB_KIND_ORACLE:	        // Oracle 数据库
			default: // 所有数据库适用，通用SQL语句		
					
				sqlstr = "	SELECT	* "
					" FROM	TCAAC13 "
					" where 1=1 "
					;
				if (dept_code.Trim() != "")
					sqlstr = sqlstr + " and  dept_code =@dept_code";
				sqlstr = sqlstr + " and  stats_period =@stats_period";
				sqlstr = sqlstr + " ORDER	BY JOB_CODE	";
   			break;
		}  
		
		cmd_inq.SetCommandText(sqlstr);	
		cmd_inq.Parameters.Set("dept_code", dept_code);
		cmd_inq.Parameters.Set("stats_period", stats_period);
		cmd_inq.ExecuteQuery(bcls_ret->Tables[0]);
				
        cmd_inq.Close();  	 
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