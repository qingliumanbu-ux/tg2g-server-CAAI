/*************************************************
Copyright:	Baosight Software LTD.co Copyright (c) 2012
Author:		zhouli
Version:    3.0
Date:		2012-2-9
Description:会计期间查询
**************************************************/
//框架用头文件
#include "stdafx.h"

// service入口
BM2F_ENTERACE(caac06_inq)
//-EP_SYSTEM_HEAD_END

int f_caac06_inq(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	
	CTracer log(__FUNCTION__);
	
	int doFlag			= 0;
	int fetchRowCount	= 0;
	CString v_dept_code("");
	CString  sqlstr("");
	CDbCommand cmd_inq(conn);

	
	try
	{	
		//系统的分页类信息。
		CPageInfo pageInfo;

	  
		switch(conn->DatabaseKind)
		{
			case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:	        // MS SQL Server数据库
			case DB_KIND_ORACLE:	        // Oracle 数据库
			default: // 所有数据库适用，通用SQL语句
			//pageInfo.MergeFrom(bcls_rec->Tables[0].Rows[0]);
			v_dept_code = bcls_rec->Tables[0].Rows[0]["DEPT_CODE"].ToString().Trim();
			Log::Trace("", __FUNCTION__, "长别=[{0}]  ", v_dept_code);
			sqlstr = "	SELECT	*"
					" FROM	TCAAC06";
			//if (v_dept_code.Trim() != "")
			sqlstr += " WHERE DEPT_CODE = '"+v_dept_code+"'  ORDER	BY  ACCOUNT_PERIOD,STATS_PERIOD DESC ";
				;
					
    			break;
		} 		
		Log::Trace("", __FUNCTION__, "sqlstr=[{0}]  ", sqlstr);
		cmd_inq.SetCommandText(sqlstr);	 
		//cmd_inq.Parameters.Set("v_dept_code", v_dept_code);
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

	return doFlag;
}