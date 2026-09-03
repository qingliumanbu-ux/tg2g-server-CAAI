/*************************************************
Copyright:	Baosight Software LTD.co Copyright (c) 2012
Author:		zhouli
Version:    1.0
Date:		2012-2-9
Description:分摊量信息查询:根据输入的查询条件，查询显示分摊量信息
**************************************************/
//框架用头文件
#include "stdafx.h"
// service入口
BM2F_ENTERACE(caac10_inq)
//-EP_SYSTEM_HEAD_END

int f_caac10_inq(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);
	int doFlag			= 0;
	int fetchRowCount	= 0;
	CString v_dept_code,mat_code,mat_name,cost_center;
	CString  sqlstr("");
	
	CDbCommand cmd_inq(conn);
	try
	{
		mat_code = bcls_rec->Tables[0].Rows[0]["mat_code"].ToString();
		mat_name = bcls_rec->Tables[0].Rows[0]["mat_name"].ToString();
		cost_center = bcls_rec->Tables[0].Rows[0]["sub_backlog_code"].ToString();
		v_dept_code = bcls_rec->Tables[0].Rows[0]["dept_code"].ToString();
		
		switch(conn->DatabaseKind)
		{
			case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:	        // MS SQL Server数据库
			case DB_KIND_ORACLE:	        // Oracle 数据库
			default: // 所有数据库适用，通用SQL语句
				  				 				
			sqlstr = "	SELECT	*"
					" FROM	TCAAC10"
					" WHERE	1=1 "
					" and MAT_CODE LIKE  '%'||'" + mat_code + "'||'%'"
					" AND MAT_NAME LIKE  '%'||'" + mat_name + "'||'%'"
					;
				if (cost_center.Trim() != "")
				{
					sqlstr += " AND COST_CENTER	= '" + cost_center + "' ";
				}
				
				if (v_dept_code.Trim() != "")
				{
					sqlstr += " AND DEPT_CODE	= '" + v_dept_code + "' ";
				}
				sqlstr	+=	" ORDER	BY MAT_CODE	";
       		
     		break;
		}    		
		
		cmd_inq.SetCommandText(sqlstr);	

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