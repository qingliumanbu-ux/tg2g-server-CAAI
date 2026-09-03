/*************************************************
Copyright:	Baosight Software LTD.co Copyright (c) 2012
Author:		zhouli
Version:    3.0
Date:		2012-2-9
Description:根据未设置价格的物料代码
**************************************************/
//框架用头文件
#include "stdafx.h"

// service入口
BM2F_ENTERACE(caac12_inq_a)
//-EP_SYSTEM_HEAD_END

int f_caac12_inq_a(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	
	CTracer log(__FUNCTION__);	
	
	int doFlag			= 0;
	int fetchRowCount	= 0;
	CString  mat_type("");
	
	CString  sqlstr("");

	CDbCommand cmd_inq(conn);
	
	try
	{
		
		
		//tcaac12.MergeFrom(bcls_rec->Tables[0].Rows[0]);
		mat_type = bcls_rec->Tables[0].Rows[0]["mat_type"].ToString();

		switch(conn->DatabaseKind)
		{
			case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:	        // MS SQL Server数据库
			case DB_KIND_ORACLE:	        // Oracle 数据库
			default: // 所有数据库适用，通用SQL语句
				  				
			sqlstr = "	SELECT	MAT_CODE,MAT_NAME,'JH' AS PRICE_TERMS,MAT_UNIT as UNIT "
					" FROM	TCAAC11 "
					" WHERE	  MAT_CODE NOT IN (SELECT MAT_CODE FROM TCAAC12 )"
					" AND VALID_FLAG = '1'"
					; 
			if (mat_type.Trim() != "")
			{
				sqlstr = sqlstr + " and mat_type = @mat_type";
			}
			
    		break;
		}		          	
		cmd_inq.SetCommandText(sqlstr);	
		cmd_inq.Parameters.Set("mat_type", mat_type);
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